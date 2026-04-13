// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/AnansiCheatManager.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "AI/EnemyGuard.h"
#include "AI/EnemyBase.h"
#include "Traversal/WallRunVolume.h"
#include "Traversal/WebSwingAnchor.h"
#include "World/TestArenaBuilder.h"
#include "AI/CaptainOfTheMask.h"
#include "UI/AnansiDevHUD.h"
#include "Core/CombatStats.h"
#include "World/StoryFragmentPickup.h"
#include "Narrative/DialogueManager.h"
#include "World/HazardZone.h"
#include "World/Checkpoint.h"
#include "AI/EnemyRanged.h"
#include "World/TimeOfDayManager.h"
#include "Traversal/GrapplePoint.h"
#include "Traversal/Zipline.h"
#include "World/MovingPlatform.h"
#include "World/BreakableObject.h"
#include "World/BouncePad.h"
#include "World/GravityZone.h"
#include "AI/EnemyTurret.h"
#include "World/SpawnerPortal.h"
#include "World/TrapPlate.h"
#include "World/PowerupPickup.h"
#include "World/WaveSurvival.h"
#include "Core/AnansiSaveManager.h"
#include "Core/QuestSystem.h"
#include "World/DestructibleWall.h"
#include "Core/AchievementSystem.h"
#include "World/LootChest.h"
#include "World/KeyPickup.h"
#include "Combat/StatusEffectComponent.h"
#include "Core/PlayerProgression.h"
#include "AI/AllyCompanion.h"
#include "Combat/MeleeDamageDealer.h"
#include "Core/DifficultySettings.h"
#include "EngineUtils.h"
#include "Combat/CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/PlayerController.h"

static AAnansiCharacter* GetAnansi(const UCheatManager* CM)
{
	if (!CM || !CM->GetOuterAPlayerController())
	{
		return nullptr;
	}
	return Cast<AAnansiCharacter>(CM->GetOuterAPlayerController()->GetPawn());
}

static FVector GetSpawnLocationInFront(const AAnansiCharacter* Anansi, float Distance = 500.0f)
{
	return Anansi->GetActorLocation() + Anansi->GetActorForwardVector() * Distance;
}

// ---------------------------------------------------------------------------

void UAnansiCheatManager::Anansi_SpawnGuard()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi);
	const FRotator Rot = (Anansi->GetActorLocation() - Loc).Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyGuard* Guard = GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(), Loc, Rot, Params);
	if (Guard)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned Melee Guard at %s"), *Loc.ToString());
	}
}

void UAnansiCheatManager::Anansi_SpawnShieldGuard()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi);
	const FRotator Rot = (Anansi->GetActorLocation() - Loc).Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyGuard* Guard = GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(), Loc, Rot, Params);
	if (Guard)
	{
		// Shield variant needs to be configured — in a real setup this would be
		// a separate Blueprint. For dev testing we log the spawn.
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned Shield Guard at %s (configure variant in BP)"), *Loc.ToString());
	}
}

void UAnansiCheatManager::Anansi_SpawnGuardWave(int32 Count)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	Count = FMath::Clamp(Count, 1, 20);

	const FVector Center = Anansi->GetActorLocation();
	const float Radius = 600.0f;

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = (static_cast<float>(i) / Count) * 360.0f;
		const float Rad = FMath::DegreesToRadians(Angle);
		const FVector Offset(FMath::Cos(Rad) * Radius, FMath::Sin(Rad) * Radius, 0.0f);
		const FVector Loc = Center + Offset;
		const FRotator Rot = (Center - Loc).Rotation();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(), Loc, Rot, Params);
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned wave of %d guards"), Count);
}

void UAnansiCheatManager::Anansi_RestoreAll()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	Anansi->ApplyHealing(Anansi->GetMaxHealth());
	Anansi->RestoreWebEnergy(Anansi->GetMaxWebEnergy());
	// Stamina auto-regens so just log
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Restored all resources"));
}

void UAnansiCheatManager::Anansi_GodMode()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	bGodMode = !bGodMode;

	if (Anansi->CombatComponent)
	{
		// Toggling invulnerability via the combat component's i-frame system
		// is a hack but works for dev testing.
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: God Mode %s"), bGodMode ? TEXT("ON") : TEXT("OFF"));
}

void UAnansiCheatManager::Anansi_KillAllEnemies()
{
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	int32 Killed = 0;
	for (AActor* Enemy : Enemies)
	{
		if (AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy))
		{
			if (!EnemyBase->IsDead())
			{
				// Apply lethal damage
				FDamageEvent DamageEvent;
				EnemyBase->TakeDamage(99999.0f, DamageEvent, nullptr, nullptr);
				Killed++;
			}
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Killed %d enemies"), Killed);
}

void UAnansiCheatManager::Anansi_SpawnWall()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 400.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWallRunVolume* Wall = GetWorld()->SpawnActor<AWallRunVolume>(AWallRunVolume::StaticClass(), Loc,
		Anansi->GetActorRotation(), Params);

	if (Wall)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned wall-run surface at %s"), *Loc.ToString());
	}
}

void UAnansiCheatManager::Anansi_SpawnWebAnchor()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = Anansi->GetActorLocation() + FVector(0.0f, 0.0f, 1500.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWebSwingAnchor* Anchor = GetWorld()->SpawnActor<AWebSwingAnchor>(AWebSwingAnchor::StaticClass(), Loc,
		FRotator::ZeroRotator, Params);

	if (Anchor)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned web-swing anchor above player at %s"), *Loc.ToString());
	}
}

void UAnansiCheatManager::Anansi_SetCombo(int32 Count)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi || !Anansi->CombatComponent) return;

	// Reset and increment to desired count (triggers style rank update)
	Anansi->CombatComponent->ResetCombo();
	for (int32 i = 0; i < Count; ++i)
	{
		Anansi->CombatComponent->OnWeaponHitDetected(FVector::ZeroVector, FVector::UpVector, Anansi);
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Set combo to %d"), Count);
}

void UAnansiCheatManager::Anansi_BuildArena()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATestArenaBuilder* Builder = GetWorld()->SpawnActor<ATestArenaBuilder>(
		ATestArenaBuilder::StaticClass(),
		Anansi->GetActorLocation(),
		FRotator::ZeroRotator,
		Params);

	if (Builder)
	{
		Builder->BuildArena();
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Test arena built at player location"));
	}
}

void UAnansiCheatManager::Anansi_SpawnBoss()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 800.0f);
	const FRotator Rot = (Anansi->GetActorLocation() - Loc).Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACaptainOfTheMask* Boss = GetWorld()->SpawnActor<ACaptainOfTheMask>(
		ACaptainOfTheMask::StaticClass(), Loc, Rot, Params);

	if (Boss)
	{
		// Wire up boss health bar on the HUD
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
			{
				HUD->SetActiveBoss(Boss);
			}
		}
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned Captain of the Mask!"));
	}
}

void UAnansiCheatManager::Anansi_Elevate(float Height)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector NewLoc = Anansi->GetActorLocation() + FVector(0.0f, 0.0f, Height);
	Anansi->SetActorLocation(NewLoc);

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Elevated player to Z=%.0f"), NewLoc.Z);
}

void UAnansiCheatManager::Anansi_Respawn()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	Anansi->Respawn();
}

void UAnansiCheatManager::Anansi_ShowStats()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;

	UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>();
	if (!Stats) return;

	UE_LOG(LogAnansi, Log, TEXT("=== COMBAT STATS ==="));
	UE_LOG(LogAnansi, Log, TEXT("%s"), *Stats->GetStatsSummary());
	UE_LOG(LogAnansi, Log, TEXT("===================="));
}

void UAnansiCheatManager::Anansi_Hurt(float Damage)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	FDamageEvent DamageEvent;
	Anansi->TakeDamage(Damage, DamageEvent, nullptr, nullptr);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Dealt %.0f damage to player"), Damage);
}

void UAnansiCheatManager::Anansi_SpawnFragment()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 300.0f) + FVector(0, 0, 50);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStoryFragmentPickup* Fragment = GetWorld()->SpawnActor<AStoryFragmentPickup>(
		AStoryFragmentPickup::StaticClass(), Loc, FRotator::ZeroRotator, Params);

	if (Fragment)
	{
		static int32 FragIndex = 0;
		Fragment->FragmentID = FName(*FString::Printf(TEXT("Fragment_%d"), FragIndex++));
		Fragment->FragmentTitle = FText::FromString(TEXT("Tale of the Spider King"));
		Fragment->LoreText = FText::FromString(TEXT("Long ago, when the world was young, Anansi wove the first story..."));

		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned story fragment"));
	}
}

void UAnansiCheatManager::Anansi_TestDialogue()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;

	UDialogueManager* DM = GI->GetSubsystem<UDialogueManager>();
	if (!DM) return;

	// Create a simple test dialogue in-memory
	// Since we don't have a DataTable asset, we'll start an empty dialogue
	// and manually set the current node for display testing
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Dialogue system ready — needs DataTable asset for full test"));
	UE_LOG(LogAnansi, Log, TEXT("       Use Blueprint to create a DataTable with FDialogueNode rows"));
}

void UAnansiCheatManager::Anansi_SpawnHazard()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 400.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHazardZone* Hazard = GetWorld()->SpawnActor<AHazardZone>(
		AHazardZone::StaticClass(), Loc, FRotator::ZeroRotator, Params);

	if (Hazard)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned fire hazard zone"));
	}
}

void UAnansiCheatManager::Anansi_SpawnCheckpoint()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACheckpoint* CP = GetWorld()->SpawnActor<ACheckpoint>(
		ACheckpoint::StaticClass(), Anansi->GetActorLocation(), FRotator::ZeroRotator, Params);

	if (CP)
	{
		CP->Activate();
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Checkpoint placed and activated"));
	}
}

void UAnansiCheatManager::Anansi_SpawnRanged()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 800.0f);
	const FRotator Rot = (Anansi->GetActorLocation() - Loc).Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyRanged* Ranged = GetWorld()->SpawnActor<AEnemyRanged>(
		AEnemyRanged::StaticClass(), Loc, Rot, Params);

	if (Ranged)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned ranged enemy"));
	}
}

void UAnansiCheatManager::Anansi_SpawnMixedWave()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Center = Anansi->GetActorLocation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 3 melee guards in front
	for (int32 i = 0; i < 3; ++i)
	{
		const float Angle = -30.0f + i * 30.0f;
		const FVector Dir = Anansi->GetActorForwardVector().RotateAngleAxis(Angle, FVector::UpVector);
		const FVector Loc = Center + Dir * 500.0f;
		GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(), Loc, (Center - Loc).Rotation(), Params);
	}

	// 2 ranged behind melee
	for (int32 i = 0; i < 2; ++i)
	{
		const float Angle = -20.0f + i * 40.0f;
		const FVector Dir = Anansi->GetActorForwardVector().RotateAngleAxis(Angle, FVector::UpVector);
		const FVector Loc = Center + Dir * 1000.0f;
		GetWorld()->SpawnActor<AEnemyRanged>(AEnemyRanged::StaticClass(), Loc, (Center - Loc).Rotation(), Params);
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned mixed wave (3 melee + 2 ranged)"));
}

void UAnansiCheatManager::Anansi_SpawnAmbush()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Center = Anansi->GetActorLocation();
	const FVector Forward = Anansi->GetActorForwardVector();
	const FVector Right = Anansi->GetActorRightVector();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Behind
	GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(),
		Center - Forward * 400.0f, Forward.Rotation(), Params);
	GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(),
		Center - Forward * 400.0f + Right * 200.0f, Forward.Rotation(), Params);

	// Left
	GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(),
		Center - Right * 500.0f, Right.Rotation(), Params);

	// Right
	GetWorld()->SpawnActor<AEnemyRanged>(AEnemyRanged::StaticClass(),
		Center + Right * 600.0f, (-Right).Rotation(), Params);

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned ambush (4 enemies surrounding player)"));
}

void UAnansiCheatManager::Anansi_SetTime(float Hour)
{
	for (TActorIterator<ATimeOfDayManager> It(GetWorld()); It; ++It)
	{
		It->SetTime(Hour);
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Set time to %.1f"), Hour);
		return;
	}

	// No TimeOfDayManager exists — spawn one
	FActorSpawnParameters Params;
	ATimeOfDayManager* TOD = GetWorld()->SpawnActor<ATimeOfDayManager>(
		ATimeOfDayManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (TOD)
	{
		TOD->SetTime(Hour);
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned TimeOfDayManager and set time to %.1f"), Hour);
	}
}

void UAnansiCheatManager::Anansi_SpawnGrapplePoint()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = Anansi->GetActorLocation()
		+ Anansi->GetActorForwardVector() * 800.0f
		+ FVector(0, 0, 400.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AGrapplePoint>(AGrapplePoint::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned grapple point"));
}

void UAnansiCheatManager::Anansi_SetDifficulty(int32 Level)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UDifficultySubsystem* Diff = GI->GetSubsystem<UDifficultySubsystem>())
		{
			EDifficulty NewDiff = static_cast<EDifficulty>(FMath::Clamp(Level, 0, 2));
			Diff->SetDifficulty(NewDiff);
		}
	}
}

void UAnansiCheatManager::Anansi_SpawnPlatform()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 500.0f) + FVector(0, 0, 100);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMovingPlatform* Plat = GetWorld()->SpawnActor<AMovingPlatform>(
		AMovingPlatform::StaticClass(), Loc, FRotator::ZeroRotator, Params);

	if (Plat)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned moving platform (vertical, 500 units)"));
	}
}

void UAnansiCheatManager::Anansi_SpawnZipline()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = Anansi->GetActorLocation() + FVector(0, 0, 50);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AZipline* Zip = GetWorld()->SpawnActor<AZipline>(
		AZipline::StaticClass(), Loc, Anansi->GetActorRotation(), Params);

	if (Zip)
	{
		Zip->EndOffset = Anansi->GetActorForwardVector() * 1500.0f + FVector(0, 0, -300);
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned zipline forward"));
	}
}

void UAnansiCheatManager::Anansi_SpawnCrates(int32 Count)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	Count = FMath::Clamp(Count, 1, 20);

	for (int32 i = 0; i < Count; ++i)
	{
		const FVector Offset(
			FMath::FRandRange(-400.0f, 400.0f),
			FMath::FRandRange(-400.0f, 400.0f),
			25.0f);

		const FVector Loc = Anansi->GetActorLocation() + Anansi->GetActorForwardVector() * 300.0f + Offset;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<ABreakableObject>(ABreakableObject::StaticClass(), Loc,
			FRotator(0, FMath::FRandRange(0.0f, 360.0f), 0), Params);
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned %d breakable crates"), Count);
}

void UAnansiCheatManager::Anansi_SpawnBouncePad()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 300.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<ABouncePad>(ABouncePad::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned bounce pad"));
}

void UAnansiCheatManager::Anansi_SpawnTurret()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 800.0f);
	const FRotator Rot = (Anansi->GetActorLocation() - Loc).Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AEnemyTurret>(AEnemyTurret::StaticClass(), Loc, Rot, Params);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned turret enemy"));
}

void UAnansiCheatManager::Anansi_SpawnGravityZone(float Scale)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 400.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGravityZone* Zone = GetWorld()->SpawnActor<AGravityZone>(
		AGravityZone::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	if (Zone)
	{
		Zone->GravityScale = Scale;
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned gravity zone (scale: %.1f)"), Scale);
	}
}

void UAnansiCheatManager::Anansi_SpawnPortal()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 600.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASpawnerPortal* Portal = GetWorld()->SpawnActor<ASpawnerPortal>(
		ASpawnerPortal::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	if (Portal)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned enemy portal (destroy it to stop spawns!)"));
	}
}

void UAnansiCheatManager::Anansi_SpawnTrap()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 300.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<ATrapPlate>(ATrapPlate::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned trap plate"));
}

void UAnansiCheatManager::Anansi_SpawnPowerup()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 300.0f) + FVector(0, 0, 50);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<APowerupPickup>(APowerupPickup::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned speed boost powerup"));
}

void UAnansiCheatManager::Anansi_MeshSetup()
{
	UE_LOG(LogAnansi, Log, TEXT(""));
	UE_LOG(LogAnansi, Log, TEXT("=== CHARACTER MESH SETUP ==="));
	UE_LOG(LogAnansi, Log, TEXT("1. Go to https://www.mixamo.com (free Adobe account)"));
	UE_LOG(LogAnansi, Log, TEXT("2. Pick a character > Download as FBX"));
	UE_LOG(LogAnansi, Log, TEXT("3. In UE Editor: Content Browser > /Game/Characters/Kweku/Meshes/"));
	UE_LOG(LogAnansi, Log, TEXT("4. Right-click > Import > select the FBX file"));
	UE_LOG(LogAnansi, Log, TEXT("5. Name it 'SK_Kweku'"));
	UE_LOG(LogAnansi, Log, TEXT("6. Restart Play — mesh auto-loads!"));
	UE_LOG(LogAnansi, Log, TEXT(""));
	UE_LOG(LogAnansi, Log, TEXT("=== ANIMATIONS ==="));
	UE_LOG(LogAnansi, Log, TEXT("Download from Mixamo (same character, FBX, With Skin):"));
	UE_LOG(LogAnansi, Log, TEXT("  - Idle, Walking, Running, Jump, Fall, Land"));
	UE_LOG(LogAnansi, Log, TEXT("  - Punch x3 (light combo), Heavy Slash, Roll"));
	UE_LOG(LogAnansi, Log, TEXT("Import into /Game/Characters/Kweku/Animations/"));
	UE_LOG(LogAnansi, Log, TEXT("Then create Animation Blueprint (right-click > Animation > Animation Blueprint)"));
	UE_LOG(LogAnansi, Log, TEXT("Name it 'ABP_Kweku' — it auto-loads!"));
	UE_LOG(LogAnansi, Log, TEXT(""));
	UE_LOG(LogAnansi, Log, TEXT("=== SOUNDS ==="));
	UE_LOG(LogAnansi, Log, TEXT("Import .wav files to these exact paths:"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Combat/Hit"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Combat/Whoosh"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Combat/Parry"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Combat/Death"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Traversal/Jump"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Traversal/Land"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/Traversal/WebShoot"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/UI/Pickup"));
	UE_LOG(LogAnansi, Log, TEXT("  /Game/Audio/SFX/UI/Checkpoint"));
	UE_LOG(LogAnansi, Log, TEXT("They auto-play when imported!"));
	UE_LOG(LogAnansi, Log, TEXT("============================"));
}

void UAnansiCheatManager::Anansi_FullArena()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	// Build the arena
	Anansi_BuildArena();

	// Add gameplay elements
	Anansi_SpawnCheckpoint();
	Anansi_SpawnMixedWave();
	Anansi_SpawnCrates(8);
	Anansi_SpawnFragment();
	Anansi_SpawnBouncePad();
	Anansi_SpawnPowerup();

	// Spawn a portal at distance
	const FVector PortalLoc = Anansi->GetActorLocation() + FVector(2000, 0, 0);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ASpawnerPortal>(ASpawnerPortal::StaticClass(), PortalLoc, FRotator::ZeroRotator, Params);

	// Turret on a platform
	const FVector TurretLoc = Anansi->GetActorLocation() + FVector(-1500, 500, 300);
	GetWorld()->SpawnActor<AEnemyTurret>(AEnemyTurret::StaticClass(), TurretLoc,
		(Anansi->GetActorLocation() - TurretLoc).Rotation(), Params);

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Full arena spawned — arena + enemies + crates + fragment + portal + turret + powerup"));
}

void UAnansiCheatManager::Anansi_Save(int32 Slot)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UAnansiSaveManager* SM = GI->GetSubsystem<UAnansiSaveManager>())
		{
			if (SM->SaveGame(Slot))
			{
				if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
				{
					if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
					{
						HUD->ShowToast(FString::Printf(TEXT("Saved to slot %d"), Slot), FColor(100, 255, 100));
					}
				}
			}
		}
	}
}

void UAnansiCheatManager::Anansi_Load(int32 Slot)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UAnansiSaveManager* SM = GI->GetSubsystem<UAnansiSaveManager>())
		{
			if (SM->LoadGame(Slot))
			{
				if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
				{
					if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
					{
						HUD->ShowToast(FString::Printf(TEXT("Loaded slot %d"), Slot), FColor(100, 200, 255));
					}
				}
			}
		}
	}
}

void UAnansiCheatManager::Anansi_Survival()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWaveSurvival* Survival = GetWorld()->SpawnActor<AWaveSurvival>(
		AWaveSurvival::StaticClass(), Anansi->GetActorLocation(), FRotator::ZeroRotator, Params);

	if (Survival)
	{
		Survival->StartSurvival();
	}
}

void UAnansiCheatManager::Anansi_Credits()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowCredits();
		}
	}
}

void UAnansiCheatManager::Anansi_QuestKill(int32 Count)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UQuestSystem* Quest = GI->GetSubsystem<UQuestSystem>())
		{
			FQuestObjective Obj;
			Obj.DisplayText = FText::FromString(FString::Printf(TEXT("Defeat %d enemies"), Count));
			Obj.Type = EObjectiveType::KillEnemies;
			Obj.TargetCount = Count;
			Quest->SetObjective(Obj);
		}
	}
}

void UAnansiCheatManager::Anansi_QuestCollect(int32 Count)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UQuestSystem* Quest = GI->GetSubsystem<UQuestSystem>())
		{
			FQuestObjective Obj;
			Obj.DisplayText = FText::FromString(FString::Printf(TEXT("Collect %d story fragments"), Count));
			Obj.Type = EObjectiveType::CollectFragments;
			Obj.TargetCount = Count;
			Quest->SetObjective(Obj);
		}
	}
}

void UAnansiCheatManager::Anansi_SpawnWeakWall()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 500.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADestructibleWall* Wall = GetWorld()->SpawnActor<ADestructibleWall>(
		ADestructibleWall::StaticClass(), Loc, Anansi->GetActorRotation(), Params);

	if (Wall)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Destructible wall spawned (break with heavy attacks)"));
	}
}

void UAnansiCheatManager::Anansi_Tutorial()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowTutorial(
				TEXT("COMBAT BASICS"),
				TEXT("[LMB] Light Attack   [RMB] Heavy Attack   [Q] Parry   [Alt] Dodge"),
				6.0f);
		}
	}
}

void UAnansiCheatManager::Anansi_UnlockAchievement(FString ID)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UAchievementSystem* Ach = GI->GetSubsystem<UAchievementSystem>())
		{
			Ach->UnlockAchievement(FName(*ID));
		}
	}
}

void UAnansiCheatManager::Anansi_PhotoMode()
{
	bPhotoMode = !bPhotoMode;

	UWorld* World = GetWorld();
	if (!World) return;

	// Freeze/unfreeze time
	UGameplayStatics::SetGlobalTimeDilation(World, bPhotoMode ? 0.0f : 1.0f);

	// Show toast
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(bPhotoMode ? TEXT("PHOTO MODE ON") : TEXT("PHOTO MODE OFF"),
				FColor(200, 200, 255));
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Photo mode: %s"), bPhotoMode ? TEXT("ON") : TEXT("OFF"));
}

void UAnansiCheatManager::Anansi_SpawnChest(bool bLocked)
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 250.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALootChest* Chest = GetWorld()->SpawnActor<ALootChest>(
		ALootChest::StaticClass(), Loc, FRotator::ZeroRotator, Params);

	if (Chest)
	{
		Chest->bLocked = bLocked;
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned %s chest"), bLocked ? TEXT("locked") : TEXT("unlocked"));
	}
}

void UAnansiCheatManager::Anansi_SpawnKey()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 200.0f) + FVector(0, 0, 50);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AKeyPickup>(AKeyPickup::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned brass key"));
}

void UAnansiCheatManager::Anansi_BurnEnemies()
{
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	int32 Count = 0;
	for (AActor* Enemy : Enemies)
	{
		if (!Enemy) continue;

		// Add status effect component if it doesn't exist
		UStatusEffectComponent* SEC = Enemy->FindComponentByClass<UStatusEffectComponent>();
		if (!SEC)
		{
			SEC = NewObject<UStatusEffectComponent>(Enemy, TEXT("StatusEffects"));
			SEC->RegisterComponent();
		}

		SEC->ApplyEffect(EStatusEffectType::Burn, 5.0f, 4.0f);
		Count++;
	}

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Burning %d enemies"), Count);
}

void UAnansiCheatManager::Anansi_AwardXP(int32 Amount)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UPlayerProgression* Prog = GI->GetSubsystem<UPlayerProgression>())
		{
			Prog->AwardXP(Amount);
			UE_LOG(LogAnansi, Log, TEXT("Cheat: Awarded %d XP"), Amount);
		}
	}
}

void UAnansiCheatManager::Anansi_SpawnAlly()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi) return;

	const FVector Loc = GetSpawnLocationInFront(Anansi, 300.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAllyCompanion* Ally = GetWorld()->SpawnActor<AAllyCompanion>(
		AAllyCompanion::StaticClass(), Loc, Anansi->GetActorRotation(), Params);

	if (Ally)
	{
		UE_LOG(LogAnansi, Log, TEXT("Cheat: Spawned ally companion"));
	}
}

void UAnansiCheatManager::Anansi_RageMode()
{
	AAnansiCharacter* Anansi = GetAnansi(this);
	if (!Anansi || !Anansi->MeleeDamage) return;

	const float OriginalDamage = Anansi->MeleeDamage->BaseDamage;
	Anansi->MeleeDamage->BaseDamage *= 2.0f;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(TEXT("RAGE MODE! 2x damage for 10s"), FColor(255, 80, 80));
			HUD->FlashScreen(FLinearColor(1.0f, 0.2f, 0.1f), 0.3f);
		}
	}

	// Revert after 10 seconds
	FTimerHandle RageTimer;
	GetWorld()->GetTimerManager().SetTimer(RageTimer,
		FTimerDelegate::CreateLambda([Anansi, OriginalDamage]()
		{
			if (Anansi && Anansi->MeleeDamage)
			{
				Anansi->MeleeDamage->BaseDamage = OriginalDamage;
				UE_LOG(LogAnansi, Log, TEXT("Rage mode ended"));
			}
		}), 10.0f, false);

	UE_LOG(LogAnansi, Log, TEXT("Cheat: Rage mode activated (2x damage)"));
}
