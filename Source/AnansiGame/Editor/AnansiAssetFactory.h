// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR

#include "EditorSubsystem.h"
#include "AnansiAssetFactory.generated.h"

class UInputAction;
class UInputMappingContext;
class UComboChainData;

/**
 * UAnansiAssetFactory
 *
 * Editor subsystem that generates all required content assets (Input Actions,
 * Mapping Contexts, Combo Data) on first project open. Safe to call repeatedly.
 */
UCLASS()
class ANANSIGAME_API UAnansiAssetFactory : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Anansi|Editor")
	void GenerateAllAssets();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Anansi|Editor")
	void GenerateInputAssets();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Anansi|Editor")
	void GenerateComboDataAssets();

private:
	UInputAction* CreateInputAction(const FString& AssetPath, const FString& AssetName,
		bool bIsAxis2D = false);

	UInputMappingContext* CreateDefaultMappingContext(const FString& AssetPath, const FString& AssetName);

	UComboChainData* CreateComboChainData(const FString& AssetPath, const FString& AssetName,
		const FText& DisplayName, int32 HitCount, const TArray<float>& DamageValues);

	bool AssetExists(const FString& ObjectPath) const;
};

#endif // WITH_EDITOR
