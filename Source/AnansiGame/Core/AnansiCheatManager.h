// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "AnansiCheatManager.generated.h"

/**
 * UAnansiCheatManager
 *
 * Dev-only cheat commands for testing. Access via console (` key) in-game.
 * Commands prefixed with "Anansi." — e.g. "Anansi.SpawnGuard"
 */
UCLASS()
class ANANSIGAME_API UAnansiCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** Spawn a melee Palace Guard 500 units in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnGuard();

	/** Spawn a shield Palace Guard 500 units in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnShieldGuard();

	/** Spawn N melee guards in a semicircle around the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnGuardWave(int32 Count = 5);

	/** Fully restore player health, web energy, and stamina. */
	UFUNCTION(Exec)
	void Anansi_RestoreAll();

	/** Toggle god mode (invulnerability). */
	UFUNCTION(Exec)
	void Anansi_GodMode();

	/** Kill all enemies in the level. */
	UFUNCTION(Exec)
	void Anansi_KillAllEnemies();

	/** Spawn a wall-run wall at the player's look target. */
	UFUNCTION(Exec)
	void Anansi_SpawnWall();

	/** Spawn a web-swing anchor above the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnWebAnchor();

	/** Set combo count to a specific value (for testing style ranks). */
	UFUNCTION(Exec)
	void Anansi_SetCombo(int32 Count);

	/** Teleport the player to a specific height (for fall/swing testing). */
	UFUNCTION(Exec)
	void Anansi_Elevate(float Height = 2000.0f);

	/** Build a complete test arena around the player. */
	UFUNCTION(Exec)
	void Anansi_BuildArena();

	/** Spawn the Captain of the Mask boss in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnBoss();

	/** Respawn the player (reset health, return to spawn). */
	UFUNCTION(Exec)
	void Anansi_Respawn();

	/** Show combat stats summary. */
	UFUNCTION(Exec)
	void Anansi_ShowStats();

	/** Deal damage to the player (for testing death/game over). */
	UFUNCTION(Exec)
	void Anansi_Hurt(float Damage = 30.0f);

	/** Spawn a story fragment pickup in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnFragment();

	/** Start a test dialogue. */
	UFUNCTION(Exec)
	void Anansi_TestDialogue();

	/** Spawn a fire hazard zone in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnHazard();

	/** Spawn a checkpoint at the player's location. */
	UFUNCTION(Exec)
	void Anansi_SpawnCheckpoint();

	/** Spawn a ranged attacker enemy. */
	UFUNCTION(Exec)
	void Anansi_SpawnRanged();

	/** Spawn a mixed combat encounter (melee + ranged + shield). */
	UFUNCTION(Exec)
	void Anansi_SpawnMixedWave();

	/** Spawn an ambush (enemies behind and to sides of player). */
	UFUNCTION(Exec)
	void Anansi_SpawnAmbush();

	/** Set time of day (0-24). */
	UFUNCTION(Exec)
	void Anansi_SetTime(float Hour = 12.0f);

	/** Spawn a grapple point above and in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnGrapplePoint();

	/** Set difficulty (0=Easy, 1=Normal, 2=Hard). */
	UFUNCTION(Exec)
	void Anansi_SetDifficulty(int32 Level = 1);

	/** Spawn a moving platform in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnPlatform();

	/** Spawn a zipline in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnZipline();

	/** Spawn breakable crates around the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnCrates(int32 Count = 5);

	/** Spawn a bounce pad in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnBouncePad();

	/** Spawn a turret enemy in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnTurret();

	/** Spawn a gravity zone in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnGravityZone(float Scale = 0.3f);

	/** Spawn an enemy spawner portal. */
	UFUNCTION(Exec)
	void Anansi_SpawnPortal();

	/** Spawn a trap plate in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnTrap();

	/** Spawn a speed boost powerup. */
	UFUNCTION(Exec)
	void Anansi_SpawnPowerup();

	/** Print instructions for importing character mesh from Mixamo. */
	UFUNCTION(Exec)
	void Anansi_MeshSetup();

	/** Spawn a complete gameplay arena with all actor types. */
	UFUNCTION(Exec)
	void Anansi_FullArena();

	/** Save current progress to a slot. */
	UFUNCTION(Exec)
	void Anansi_Save(int32 Slot = 0);

	/** Load progress from a slot. */
	UFUNCTION(Exec)
	void Anansi_Load(int32 Slot = 0);

	/** Start wave survival mode. */
	UFUNCTION(Exec)
	void Anansi_Survival();

	/** Show the credits screen. */
	UFUNCTION(Exec)
	void Anansi_Credits();

	/** Start a kill X enemies quest. */
	UFUNCTION(Exec)
	void Anansi_QuestKill(int32 Count = 5);

	/** Start a collect X fragments quest. */
	UFUNCTION(Exec)
	void Anansi_QuestCollect(int32 Count = 3);

	/** Spawn a destructible wall in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnWeakWall();

	/** Show a tutorial hint popup. */
	UFUNCTION(Exec)
	void Anansi_Tutorial();

	/** Unlock an achievement by name. */
	UFUNCTION(Exec)
	void Anansi_UnlockAchievement(FString ID = TEXT("FirstBlood"));

	/** Toggle photo mode (freezes game). */
	UFUNCTION(Exec)
	void Anansi_PhotoMode();

	/** Spawn a loot chest in front of the player. */
	UFUNCTION(Exec)
	void Anansi_SpawnChest(bool bLocked = false);

	/** Spawn a key pickup. */
	UFUNCTION(Exec)
	void Anansi_SpawnKey();

	/** Apply burn status effect to all nearby enemies. */
	UFUNCTION(Exec)
	void Anansi_BurnEnemies();

	/** Award XP to the player. */
	UFUNCTION(Exec)
	void Anansi_AwardXP(int32 Amount = 100);

	/** Spawn an ally companion. */
	UFUNCTION(Exec)
	void Anansi_SpawnAlly();

	/** Toggle rage mode (2x damage for 10 seconds). */
	UFUNCTION(Exec)
	void Anansi_RageMode();

private:
	bool bGodMode = false;
	bool bPhotoMode = false;
};
