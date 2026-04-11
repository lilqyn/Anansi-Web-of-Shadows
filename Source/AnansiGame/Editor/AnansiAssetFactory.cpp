// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Editor/AnansiAssetFactory.h"

#if WITH_EDITOR

#include "AnansiGame.h"
#include "Combat/ComboData.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputTriggers.h"

#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"

void UAnansiAssetFactory::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Asset generation is manual only — use console: AnansiAssetFactory.GenerateAllAssets
	UE_LOG(LogAnansi, Log, TEXT("AnansiAssetFactory: Ready. Use Output Log console to run GenerateAllAssets."));
}

void UAnansiAssetFactory::GenerateAllAssets()
{
	GenerateInputAssets();
	GenerateComboDataAssets();
	UE_LOG(LogAnansi, Log, TEXT("AnansiAssetFactory: All content assets generated."));
}

// ---------------------------------------------------------------------------
// Input Actions
// ---------------------------------------------------------------------------

void UAnansiAssetFactory::GenerateInputAssets()
{
	const FString InputPath = TEXT("/Game/Input");

	CreateInputAction(InputPath, TEXT("IA_Move"), true);
	CreateInputAction(InputPath, TEXT("IA_Look"), true);
	CreateInputAction(InputPath, TEXT("IA_Jump"));
	CreateInputAction(InputPath, TEXT("IA_LightAttack"));
	CreateInputAction(InputPath, TEXT("IA_HeavyAttack"));
	CreateInputAction(InputPath, TEXT("IA_Dodge"));
	CreateInputAction(InputPath, TEXT("IA_Parry"));
	CreateInputAction(InputPath, TEXT("IA_Crouch"));
	CreateInputAction(InputPath, TEXT("IA_LockOn"));
	CreateInputAction(InputPath, TEXT("IA_WebSwing"));
	CreateInputAction(InputPath, TEXT("IA_Pause"));
	CreateInputAction(InputPath, TEXT("IA_Interact"));
	CreateInputAction(InputPath, TEXT("IA_SpiderSense"));
	CreateInputAction(InputPath, TEXT("IA_SilkenPath"));
	CreateInputAction(InputPath, TEXT("IA_Sprint"));
	CreateInputAction(InputPath, TEXT("IA_SwitchTargetLeft"));
	CreateInputAction(InputPath, TEXT("IA_SwitchTargetRight"));

	CreateDefaultMappingContext(InputPath, TEXT("IMC_Default"));

	UE_LOG(LogAnansi, Log, TEXT("AnansiAssetFactory: Input assets generated."));
}

UInputAction* UAnansiAssetFactory::CreateInputAction(const FString& AssetPath,
	const FString& AssetName, bool bIsAxis2D)
{
	const FString FullPath = AssetPath / AssetName;
	if (AssetExists(FullPath))
	{
		return LoadObject<UInputAction>(nullptr, *FullPath);
	}

	UPackage* Package = CreatePackage(*FullPath);
	Package->FullyLoad();

	UInputAction* Action = NewObject<UInputAction>(Package, *AssetName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet);
	Action->ValueType = bIsAxis2D ? EInputActionValueType::Axis2D : EInputActionValueType::Boolean;
	Action->bConsumeInput = true;

	Action->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Action);

	const FString FilePath = FPackageName::LongPackageNameToFilename(FullPath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, Action, *FilePath, SaveArgs);

	UE_LOG(LogAnansi, Log, TEXT("  Created: %s"), *FullPath);
	return Action;
}

UInputMappingContext* UAnansiAssetFactory::CreateDefaultMappingContext(
	const FString& AssetPath, const FString& AssetName)
{
	const FString FullPath = AssetPath / AssetName;
	if (AssetExists(FullPath))
	{
		return LoadObject<UInputMappingContext>(nullptr, *FullPath);
	}

	UPackage* Package = CreatePackage(*FullPath);
	Package->FullyLoad();

	UInputMappingContext* MC = NewObject<UInputMappingContext>(Package, *AssetName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet);

	auto AddMapping = [&](const FString& ActionName, FKey Key, bool bNegate = false, bool bSwizzle = false,
		EInputAxisSwizzle SwizzleOrder = EInputAxisSwizzle::YXZ)
	{
		UInputAction* Action = LoadObject<UInputAction>(nullptr, *(AssetPath / ActionName));
		if (!Action) return;

		FEnhancedActionKeyMapping& Mapping = MC->MapKey(Action, Key);
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(MC));
		}
		if (bSwizzle)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(MC);
			Swizzle->Order = SwizzleOrder;
			Mapping.Modifiers.Add(Swizzle);
		}
	};

	// Keyboard + Mouse
	AddMapping(TEXT("IA_Move"), EKeys::W);
	AddMapping(TEXT("IA_Move"), EKeys::S, true);
	AddMapping(TEXT("IA_Move"), EKeys::D, false, true);
	AddMapping(TEXT("IA_Move"), EKeys::A, true, true);
	AddMapping(TEXT("IA_Look"), EKeys::Mouse2D);
	AddMapping(TEXT("IA_Jump"),        EKeys::SpaceBar);
	AddMapping(TEXT("IA_LightAttack"), EKeys::LeftMouseButton);
	AddMapping(TEXT("IA_HeavyAttack"), EKeys::RightMouseButton);
	AddMapping(TEXT("IA_Dodge"),       EKeys::LeftAlt);
	AddMapping(TEXT("IA_Parry"),       EKeys::Q);
	AddMapping(TEXT("IA_Crouch"),      EKeys::LeftControl);
	AddMapping(TEXT("IA_LockOn"),      EKeys::MiddleMouseButton);
	AddMapping(TEXT("IA_WebSwing"),    EKeys::E);
	AddMapping(TEXT("IA_Pause"),       EKeys::Escape);
	AddMapping(TEXT("IA_Interact"),    EKeys::F);
	AddMapping(TEXT("IA_SpiderSense"), EKeys::R);
	AddMapping(TEXT("IA_SilkenPath"),  EKeys::T);
	AddMapping(TEXT("IA_Sprint"),      EKeys::LeftShift);

	// Gamepad
	AddMapping(TEXT("IA_Move"), EKeys::Gamepad_Left2D);
	AddMapping(TEXT("IA_Look"), EKeys::Gamepad_Right2D);
	AddMapping(TEXT("IA_Jump"),          EKeys::Gamepad_FaceButton_Bottom);
	AddMapping(TEXT("IA_LightAttack"),   EKeys::Gamepad_FaceButton_Right);
	AddMapping(TEXT("IA_HeavyAttack"),   EKeys::Gamepad_FaceButton_Top);
	AddMapping(TEXT("IA_Dodge"),         EKeys::Gamepad_FaceButton_Left);
	AddMapping(TEXT("IA_Parry"),         EKeys::Gamepad_LeftTrigger);
	AddMapping(TEXT("IA_LockOn"),        EKeys::Gamepad_RightThumbstick);
	AddMapping(TEXT("IA_WebSwing"),      EKeys::Gamepad_RightTrigger);
	AddMapping(TEXT("IA_Crouch"),        EKeys::Gamepad_LeftThumbstick);
	AddMapping(TEXT("IA_Pause"),         EKeys::Gamepad_Special_Right);
	AddMapping(TEXT("IA_Interact"),      EKeys::Gamepad_LeftShoulder);
	AddMapping(TEXT("IA_SpiderSense"),   EKeys::Gamepad_RightShoulder);
	AddMapping(TEXT("IA_SilkenPath"),    EKeys::Gamepad_DPad_Up);
	AddMapping(TEXT("IA_Sprint"),        EKeys::Gamepad_LeftThumbstick);
	AddMapping(TEXT("IA_SwitchTargetLeft"),  EKeys::Gamepad_DPad_Left);
	AddMapping(TEXT("IA_SwitchTargetRight"), EKeys::Gamepad_DPad_Right);

	MC->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(MC);

	const FString FilePath = FPackageName::LongPackageNameToFilename(FullPath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, MC, *FilePath, SaveArgs);

	UE_LOG(LogAnansi, Log, TEXT("  Created: %s"), *FullPath);
	return MC;
}

// ---------------------------------------------------------------------------
// Combo Data
// ---------------------------------------------------------------------------

void UAnansiAssetFactory::GenerateComboDataAssets()
{
	const FString ComboPath = TEXT("/Game/Combat/ComboData");

	CreateComboChainData(ComboPath, TEXT("CDC_LightCombo"),
		NSLOCTEXT("Anansi", "LightCombo", "Light Attack Chain (L-L-L)"),
		3, {10.0f, 12.0f, 18.0f});

	CreateComboChainData(ComboPath, TEXT("CDC_LightLightHeavy"),
		NSLOCTEXT("Anansi", "LLHCombo", "Light-Light-Heavy Chain (L-L-H)"),
		3, {10.0f, 12.0f, 30.0f});

	CreateComboChainData(ComboPath, TEXT("CDC_HeavyCombo"),
		NSLOCTEXT("Anansi", "HeavyCombo", "Heavy Attack Chain (H-H)"),
		2, {25.0f, 35.0f});

	UE_LOG(LogAnansi, Log, TEXT("AnansiAssetFactory: Combo data generated."));
}

UComboChainData* UAnansiAssetFactory::CreateComboChainData(const FString& AssetPath,
	const FString& AssetName, const FText& DisplayName, int32 HitCount,
	const TArray<float>& DamageValues)
{
	const FString FullPath = AssetPath / AssetName;
	if (AssetExists(FullPath))
	{
		return LoadObject<UComboChainData>(nullptr, *FullPath);
	}

	UPackage* Package = CreatePackage(*FullPath);
	Package->FullyLoad();

	UComboChainData* ComboData = NewObject<UComboChainData>(Package, *AssetName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet);
	ComboData->ChainDisplayName = DisplayName;

	for (int32 i = 0; i < HitCount; ++i)
	{
		FComboHitDefinition HitDef;
		HitDef.BaseDamage = DamageValues.IsValidIndex(i) ? DamageValues[i] : 10.0f;
		HitDef.DamageType = EAnansiDamageType::Physical;
		HitDef.HitArcAngle = 90.0f;
		HitDef.NextHitInputWindow = 0.4f;
		HitDef.CancelWindow = FVector2D(0.5f, 1.0f);

		if (i == HitCount - 1)
		{
			HitDef.bTriggerHitStop = true;
			HitDef.HitStopDuration = 0.08f;
			HitDef.LaunchPower = 400.0f;
		}
		else
		{
			HitDef.LaunchPower = 150.0f;
		}

		ComboData->Hits.Add(HitDef);
	}

	ComboData->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(ComboData);

	const FString FilePath = FPackageName::LongPackageNameToFilename(FullPath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, ComboData, *FilePath, SaveArgs);

	UE_LOG(LogAnansi, Log, TEXT("  Created: %s (%d hits)"), *FullPath, HitCount);
	return ComboData;
}

bool UAnansiAssetFactory::AssetExists(const FString& ObjectPath) const
{
	const FAssetRegistryModule& Reg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData Data = Reg.Get().GetAssetByObjectPath(FSoftObjectPath(ObjectPath));
	return Data.IsValid();
}

#endif // WITH_EDITOR
