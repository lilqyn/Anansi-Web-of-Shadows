// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/AnansiAIController.h"
#include "AnansiGame.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AAnansiAIController::AAnansiAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create perception component
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	ConfigurePerception();
}

void AAnansiAIController::ConfigurePerception()
{
	// Sight
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = SightFieldOfView;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// Hearing
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = HearingRange;
	HearingConfig->SetMaxAge(8.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	// Damage
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageConfig->SetMaxAge(10.0f);
	AIPerceptionComp->ConfigureSense(*DamageConfig);

	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

// ---------------------------------------------------------------------------
// Possession
// ---------------------------------------------------------------------------

void AAnansiAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Bind perception delegate
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAnansiAIController::OnPerceptionUpdated);

	// Start behavior tree
	if (DefaultBehaviorTree)
	{
		RunBehaviorTree(DefaultBehaviorTree);
		UpdateBlackboardState();
	}

	UE_LOG(LogAnansi, Log, TEXT("AnansiAI possessed %s"), *InPawn->GetName());
}

void AAnansiAIController::OnUnPossess()
{
	AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAnansiAIController::OnPerceptionUpdated);

	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(GetBrainComponent()))
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}

	Super::OnUnPossess();
}

void AAnansiAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBlackboardState();
}

// ---------------------------------------------------------------------------
// State management
// ---------------------------------------------------------------------------

void AAnansiAIController::SetAIState(EAnansiAIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	UE_LOG(LogAnansi, Log, TEXT("AnansiAI [%s] state: %d -> %d"),
		*GetName(), static_cast<int32>(CurrentState), static_cast<int32>(NewState));

	CurrentState = NewState;
	UpdateBlackboardState();
}

void AAnansiAIController::UpdateBlackboardState()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	BB->SetValueAsEnum(BB_AIState, static_cast<uint8>(CurrentState));
}

// ---------------------------------------------------------------------------
// Target tracking
// ---------------------------------------------------------------------------

AActor* AAnansiAIController::GetCurrentTarget() const
{
	const UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return nullptr;
	}
	return Cast<AActor>(BB->GetValueAsObject(BB_TargetActor));
}

void AAnansiAIController::SetCurrentTarget(AActor* NewTarget)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	BB->SetValueAsObject(BB_TargetActor, NewTarget);

	if (NewTarget)
	{
		BB->SetValueAsVector(BB_LastKnownLocation, NewTarget->GetActorLocation());
	}
}

void AAnansiAIController::ClearTarget()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->ClearValue(BB_TargetActor);
	}
}

// ---------------------------------------------------------------------------
// Threat assessment
// ---------------------------------------------------------------------------

float AAnansiAIController::EvaluateThreatLevel(AActor* Actor) const
{
	if (!Actor || !GetPawn())
	{
		return 0.0f;
	}

	float Threat = 0.0f;

	// Distance factor — closer = more threatening
	const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
	const float DistanceFactor = FMath::Clamp(1.0f - (Distance / SightRadius), 0.0f, 1.0f);
	Threat += DistanceFactor * 0.5f;

	// Player is always high threat
	if (Actor == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		Threat += 0.5f;
	}

	return FMath::Clamp(Threat, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// Alert propagation
// ---------------------------------------------------------------------------

void AAnansiAIController::AlertNearbyAI(const FVector& ThreatLocation, float AlertRadius)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AAnansiAIController> It(World); It; ++It)
	{
		AAnansiAIController* OtherAI = *It;
		if (OtherAI == this || !OtherAI->GetPawn())
		{
			continue;
		}

		const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherAI->GetPawn()->GetActorLocation());
		if (Distance <= AlertRadius)
		{
			OtherAI->ReceiveAlert(ThreatLocation, GetCurrentTarget());
		}
	}
}

void AAnansiAIController::ReceiveAlert(const FVector& ThreatLocation, AActor* ThreatActor)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	BB->SetValueAsVector(BB_InvestigateLocation, ThreatLocation);

	if (CurrentState == EAnansiAIState::Patrol)
	{
		SetAIState(EAnansiAIState::Investigate);
	}

	if (ThreatActor && CurrentState != EAnansiAIState::Combat)
	{
		SetCurrentTarget(ThreatActor);
		SetAIState(EAnansiAIState::Combat);
	}

	UE_LOG(LogAnansi, Log, TEXT("AnansiAI [%s] received alert at %s"), *GetName(), *ThreatLocation.ToString());
}

// ---------------------------------------------------------------------------
// Perception
// ---------------------------------------------------------------------------

void AAnansiAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		const float Threat = EvaluateThreatLevel(Actor);

		if (Threat > 0.3f)
		{
			SetCurrentTarget(Actor);
			SetAIState(EAnansiAIState::Combat);
			AlertNearbyAI(Actor->GetActorLocation());
		}
		else if (CurrentState == EAnansiAIState::Patrol)
		{
			BB->SetValueAsVector(BB_InvestigateLocation, Stimulus.StimulusLocation);
			SetAIState(EAnansiAIState::Investigate);
		}
	}
	else
	{
		// Lost stimulus — switch to search
		if (CurrentState == EAnansiAIState::Combat && GetCurrentTarget() == Actor)
		{
			BB->SetValueAsVector(BB_LastKnownLocation, Stimulus.StimulusLocation);
			SetAIState(EAnansiAIState::Search);
		}
	}
}
