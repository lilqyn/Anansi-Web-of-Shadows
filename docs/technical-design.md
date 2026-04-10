# Anansi: Web of Shadows -- Technical Design Document

**Version:** 1.0  
**Date:** 2026-04-10  
**Engine:** Unreal Engine 5.4+  
**Languages:** C++ (core systems), Blueprints (gameplay scripting, prototyping)  
**Target Platforms:** PC (Windows), PlayStation 5, Xbox Series X|S  
**Minimum Spec (PC):** RTX 2060 / RX 5700, 16 GB RAM, Ryzen 5 3600 / i5-10400  

---

## Table of Contents

1. [UE5 Architecture Overview](#1-ue5-architecture-overview)
2. [Class Hierarchy](#2-class-hierarchy)
3. [Player Character Architecture](#3-player-character-architecture)
4. [Combat System Architecture](#4-combat-system-architecture)
5. [Gameplay Ability System Integration](#5-gameplay-ability-system-integration)
6. [AI Architecture](#6-ai-architecture)
7. [Stealth System Architecture](#7-stealth-system-architecture)
8. [Traversal System Architecture](#8-traversal-system-architecture)
9. [Narrative System Architecture](#9-narrative-system-architecture)
10. [World Streaming and Level Design](#10-world-streaming-and-level-design)
11. [Animation System Design](#11-animation-system-design)
12. [Audio Integration](#12-audio-integration)
13. [Performance Budgets](#13-performance-budgets)
14. [Save System Design](#14-save-system-design)
15. [Network Considerations](#15-network-considerations)
16. [Build Pipeline and CI](#16-build-pipeline-and-ci)

---

## 1. UE5 Architecture Overview

### Module Structure

The project is divided into the following C++ modules to enforce separation of concerns, reduce compile times, and support parallel development.

```
AnansiGame/
├── Source/
│   ├── AnansiCore/              -- Core types, interfaces, data assets, utilities
│   ├── AnansiCharacter/         -- Player character, NPC base, animation interfaces
│   ├── AnansiCombat/            -- Combat system, damage pipeline, combo graph
│   ├── AnansiAbilities/         -- GAS integration, ability definitions, effects
│   ├── AnansiAI/                -- AI controllers, behavior trees, perception, EQS
│   ├── AnansiStealth/           -- Stealth detection, awareness, alert system
│   ├── AnansiTraversal/         -- Parkour, climbing, swinging, traversal volumes
│   ├── AnansiNarrative/         -- Dialogue, quests, choice system, trust meters
│   ├── AnansiWorld/             -- World streaming, region management, interactables
│   ├── AnansiUI/                -- HUD, menus, accessibility overlays
│   ├── AnansiAudio/             -- Wwise integration layer, audio events
│   └── AnansiSave/              -- Save/load, serialization, profile management
├── Plugins/
│   ├── AnansiComboGraph/        -- Custom editor for combo graph authoring
│   ├── AnansiDialogueEditor/    -- Custom editor for dialogue tree authoring
│   └── AnansiDebugTools/        -- Development-only debug overlays, cheats
└── Config/
    └── ...
```

### Module Dependencies

```
AnansiCore          <-- depended on by all modules (no outward dependencies)
AnansiCharacter     <-- AnansiCore
AnansiCombat        <-- AnansiCore, AnansiCharacter
AnansiAbilities     <-- AnansiCore, AnansiCharacter, AnansiCombat (GAS bridge)
AnansiAI            <-- AnansiCore, AnansiCharacter, AnansiCombat, AnansiStealth
AnansiStealth       <-- AnansiCore, AnansiCharacter
AnansiTraversal     <-- AnansiCore, AnansiCharacter
AnansiNarrative     <-- AnansiCore, AnansiCharacter
AnansiWorld         <-- AnansiCore, AnansiNarrative
AnansiUI            <-- AnansiCore (reads from subsystems, no gameplay deps)
AnansiAudio         <-- AnansiCore
AnansiSave          <-- AnansiCore, AnansiNarrative, AnansiWorld
```

### Key Subsystems (UGameInstanceSubsystem / UWorldSubsystem)

| Subsystem | Lifetime | Responsibility |
|-----------|----------|----------------|
| `UAnansiCombatSubsystem` | World | Damage pipeline, combo registration, hit-stop management |
| `UAnansiStealthSubsystem` | World | Global alert level, shared AI knowledge base, detection queries |
| `UAnansiNarrativeSubsystem` | GameInstance | Quest state, trust meters, dialogue state (persists across levels) |
| `UAnansiSaveSubsystem` | GameInstance | Save/load orchestration, autosave triggers |
| `UAnansiAudioSubsystem` | GameInstance | Wwise state management, music layering |
| `UAnansiTraversalSubsystem` | World | Swing point registration, traversal surface queries |

### Plugin Usage

| Plugin | Purpose |
|--------|---------|
| **Gameplay Ability System (GAS)** | Ability activation, cooldowns, effects, attribute sets |
| **Enhanced Input** | Input mapping, context switching (combat/traversal/UI) |
| **Motion Warping** | Traversal alignment, attack target snapping |
| **CommonUI** | Cross-platform UI framework |
| **Niagara** | VFX (spirit threads, ability effects, environment particles) |
| **Wwise UE Integration** | Audio middleware |
| **Metasound** | Procedural audio for spirit-world ambience |
| **World Partition** | Open-world streaming |
| **Mass Entity / State Trees** | Crowd NPCs in city zones |

---

## 2. Class Hierarchy

```
AActor
├── ACharacter
│   ├── AAnansiCharacterBase                    [AnansiCharacter]
│   │   ├── AAnansiPlayerCharacter              [AnansiCharacter]
│   │   ├── AAnansiNPCCharacter                 [AnansiCharacter]
│   │   │   ├── AAnansiCombatNPC                [AnansiAI]
│   │   │   │   ├── AAnansiEnemyBase            [AnansiAI]
│   │   │   │   │   ├── AAnansiEnemy_Guard      [AnansiAI]
│   │   │   │   │   ├── AAnansiEnemy_Zealot     [AnansiAI]
│   │   │   │   │   ├── AAnansiEnemy_Mercenary  [AnansiAI]
│   │   │   │   │   ├── AAnansiEnemy_Spirit     [AnansiAI]
│   │   │   │   │   └── AAnansiEnemy_Boss       [AnansiAI]
│   │   │   │   └── AAnansiCompanion            [AnansiAI]
│   │   │   │       └── AAnansiAmaSerwaa        [AnansiAI]
│   │   │   └── AAnansiCivilianNPC              [AnansiAI]
│   │   └── AAnansiCrowdNPC                     [AnansiAI] (lightweight)
│   └── ...
├── AAnansiInteractable                          [AnansiWorld]
│   ├── AAnansiStoryFragment                     [AnansiWorld]
│   ├── AAnansiTraversalPoint                    [AnansiTraversal]
│   │   ├── AAnansiSwingPoint                    [AnansiTraversal]
│   │   ├── AAnansiClimbSurface                  [AnansiTraversal]
│   │   └── AAnansiWallRunSurface                [AnansiTraversal]
│   ├── AAnansiPuzzleElement                     [AnansiWorld]
│   └── AAnansiDialogueTrigger                   [AnansiNarrative]
└── ...

UActorComponent
├── UAnansiCombatComponent                       [AnansiCombat]
├── UAnansiStealthTargetComponent                [AnansiStealth]
├── UAnansiTraversalComponent                    [AnansiTraversal]
├── UAnansiAbilitySystemComponent (wraps GAS)    [AnansiAbilities]
├── UAnansiHealthComponent                       [AnansiCore]
├── UAnansiStaggerComponent                      [AnansiCombat]
├── UAnansiInteractionComponent                  [AnansiWorld]
├── UAnansiDialogueComponent                     [AnansiNarrative]
└── UAnansiDetectionComponent                    [AnansiStealth]

AController
├── APlayerController
│   └── AAnansiPlayerController                  [AnansiCharacter]
└── AAIController
    └── AAnansiAIController                      [AnansiAI]

UAnimInstance
└── UAnansiAnimInstance                           [AnansiCharacter]
    ├── UAnansiPlayerAnimInstance                 [AnansiCharacter]
    └── UAnansiEnemyAnimInstance                  [AnansiAI]
```

---

## 3. Player Character Architecture

### Component Breakdown

`AAnansiPlayerCharacter` is a composition of specialized components to avoid monolithic character classes.

```
AAnansiPlayerCharacter
├── USkeletalMeshComponent          (inherited, character mesh)
├── UCameraComponent + SpringArm    (third-person camera)
├── UAnansiCombatComponent          (attack state, combo tracking, parry state)
├── UAnansiAbilitySystemComponent   (GAS wrapper -- attributes, abilities, effects)
├── UAnansiTraversalComponent       (traversal state, surface detection, grip meter)
├── UAnansiStealthTargetComponent   (visibility, noise generation, disguise state)
├── UAnansiHealthComponent          (HP, damage reception, death handling)
├── UAnansiStaggerComponent         (stagger bar for player -- enemy attacks fill it)
├── UAnansiInteractionComponent     (overlap detection for interactables)
├── UMotionWarpingComponent         (UE5 motion warping for attacks/traversal)
└── UNiagaraComponent               (persistent VFX: spider-mark glow, thread trail)
```

### Player State Machine

The player character operates on a hierarchical state machine implemented in C++ with Blueprint-exposed transitions.

```
[Root State Machine]
│
├── Exploration
│   ├── Idle
│   ├── Walk / Run / Sprint
│   ├── Interact
│   └── Social Blend (stealth in crowds)
│
├── Combat
│   ├── Idle (combat stance)
│   ├── Light Attack (chain states: L1, L2, L3, L4, L5)
│   ├── Heavy Attack (charge states: H_Start, H_Charge, H_Release)
│   ├── Combo Finisher
│   ├── Dodge (Tap, Directional, Perfect)
│   ├── Parry (Attempt, Success, Perfect, Fail)
│   ├── Counter Attack
│   ├── Hit Reaction (Light, Heavy, Knockdown)
│   ├── Execution
│   ├── Exhaustion (stamina depleted)
│   └── Ability Use (delegates to GAS)
│
├── Traversal
│   ├── Wall Run (Start, Running, End/Jump-off)
│   ├── Wall Climb (Start, Climbing, Leap)
│   ├── Ledge (Hang, Shimmy, Pull-up, Drop)
│   ├── Web Swing (Attach, Swing, Release)
│   ├── Web Zip (to point)
│   ├── Beam Balance
│   ├── Vault
│   ├── Slide
│   └── Ceiling Cling
│
├── Stealth
│   ├── Crouch Idle / Move
│   ├── Cover Lean
│   ├── Silent Takedown (Rear, Above, Below, Corner, Web)
│   ├── Body Carry
│   └── Disguise (Trick Mirror mask)
│
└── Cinematic
    ├── Dialogue
    ├── Cutscene
    └── Scripted Sequence
```

State transitions are governed by priority (Combat > Traversal > Stealth > Exploration) with explicit interrupt rules. For example, taking damage during traversal forces a transition to Combat.Hit Reaction, but only after the current traversal action reaches a safe exit point (no mid-wall-run stagger).

### Animation Blueprint Structure

The Animation Blueprint for the player character uses a layered approach:

```
[Locomotion Layer]          -- Blend space: idle/walk/run/sprint, directional
    ↓ (merged via Layered Blend Per Bone)
[Upper Body Override Layer] -- Combat attacks, ability use (upper body only during movement)
    ↓
[Full Body Override Layer]  -- Traversal moves, takedowns, hit reactions (full body, highest priority)
    ↓
[Additive Layer]            -- Breathing, look-at IK, injury limp, sash physics
    ↓
[IK Layer]                  -- Foot IK (terrain adaptation), hand IK (wall contact, ledge grip)
    ↓
[Post-Process]              -- Ragdoll blend (for death/knockdown), cloth sim influence
```

---

## 4. Combat System Architecture

### Combo Graph

The combo system is driven by a data-driven **Combo Graph** asset, editable in a custom editor plugin (`AnansiComboGraph`).

**Combo Graph Structure:**

```
ComboGraphAsset
├── Nodes[]
│   ├── ComboNode
│   │   ├── NodeID: FName
│   │   ├── InputType: EComboInput (Light, Heavy, Special)
│   │   ├── AnimMontage: UAnimMontage*
│   │   ├── DamageProfile: FDamageProfile
│   │   │   ├── BaseDamage: float
│   │   │   ├── DamageType: EDamageType
│   │   │   ├── StaggerDamage: float
│   │   │   ├── KnockbackForce: FVector
│   │   │   └── HitStopDuration: float
│   │   ├── StaminaCost: float
│   │   ├── InputWindow: FFloatRange (when during the anim the next input is accepted)
│   │   ├── CancelWindow: FFloatRange (when dodge/parry can cancel)
│   │   ├── Children: TArray<FComboTransition>
│   │   └── bIsFinisher: bool
│   └── ...
└── EntryNodes: TArray<FName> (valid starting nodes from idle)
```

**Combo Flow:**

```
Player Input (Light/Heavy)
    │
    ├── Is in Combat Idle? ──> Check EntryNodes for matching input
    │
    └── Is mid-combo? ──> Check current node's Children for matching input
                              │
                              ├── Match found, within InputWindow? ──> Buffer input, transition on window open
                              │
                              └── No match or outside window? ──> Drop to idle (combo ended)
```

Input buffering: the system stores the last input for up to 0.3s, allowing slightly early inputs to register.

### Hit Detection

**Approach:** Animation-notify-driven collision traces, not persistent hitboxes.

1. `AnimNotify_MeleeTraceBegin` fires during the attack animation's damage window start
2. Each tick during the damage window, the `UAnansiCombatComponent` performs a sphere sweep along the weapon's trajectory (using socket positions on the staff mesh)
3. Hit results are filtered: no duplicate hits on the same target per swing, no self-hits, friendly fire checks
4. `AnimNotify_MeleeTraceEnd` stops the sweep
5. On hit:
   - Hit-stop: brief freeze-frame (0.03--0.08s based on damage profile)
   - Camera shake (scaled to damage)
   - Damage event dispatched through the damage pipeline

### Damage Pipeline

```
[Hit Detected]
    │
    ▼
[FDamageEvent Created]
    ├── Source: instigator character
    ├── Target: hit character
    ├── BaseDamage
    ├── DamageType
    ├── HitLocation
    ├── HitDirection
    └── ComboMultiplier
    │
    ▼
[UAnansiCombatSubsystem::ProcessDamage()]
    │
    ├── 1. Check target invulnerability (i-frames, cinematic immunity)
    ├── 2. Apply source modifiers (combo tier multiplier, ability buffs, gear bonuses)
    ├── 3. Apply target modifiers (armor, resistances, active parry)
    ├── 4. Calculate final damage
    ├── 5. Apply to UAnansiHealthComponent
    ├── 6. Apply stagger damage to UAnansiStaggerComponent
    ├── 7. Determine hit reaction tier (light/heavy/knockdown/stagger-break)
    ├── 8. Dispatch FOnDamageTaken delegate (UI listens, VFX listens, audio listens)
    └── 9. Check for death / stagger-break state
```

All modifiers are implemented as Gameplay Effects via GAS, ensuring a single unified pipeline for all damage sources (melee, abilities, environmental, spirit).

---

## 5. Gameplay Ability System Integration

### Attribute Set

`UAnansiAttributeSet` defines the core attributes managed by GAS:

```cpp
UPROPERTY() FGameplayAttributeData Health;
UPROPERTY() FGameplayAttributeData MaxHealth;
UPROPERTY() FGameplayAttributeData Stamina;
UPROPERTY() FGameplayAttributeData MaxStamina;
UPROPERTY() FGameplayAttributeData ThreadEnergy;       // Ability resource (0-5 segments)
UPROPERTY() FGameplayAttributeData MaxThreadEnergy;
UPROPERTY() FGameplayAttributeData PhysicalDamage;
UPROPERTY() FGameplayAttributeData SpiritDamage;
UPROPERTY() FGameplayAttributeData PhysicalResistance;
UPROPERTY() FGameplayAttributeData SpiritResistance;
UPROPERTY() FGameplayAttributeData StaggerResistance;
UPROPERTY() FGameplayAttributeData ComboMultiplier;
UPROPERTY() FGameplayAttributeData GripMeter;          // Traversal climbing resource
UPROPERTY() FGameplayAttributeData MaxGripMeter;
```

### Ability Class Hierarchy

```
UGameplayAbility
└── UAnansiGameplayAbility                        (base: cost checking, cooldown, anim montage play)
    ├── UAnansiAbility_MeleeAttack                (combo node execution, hit trace, damage application)
    ├── UAnansiAbility_Dodge                       (i-frame granting, movement execution)
    ├── UAnansiAbility_Parry                       (parry window, counter state)
    ├── UAnansiAbility_SilkenPath                  (base for all web abilities)
    │   ├── UAnansiAbility_WebSwing
    │   ├── UAnansiAbility_WebPull
    │   ├── UAnansiAbility_WebTrap
    │   └── UAnansiAbility_WebTether
    ├── UAnansiAbility_BorrowedVoice
    │   ├── UAnansiAbility_VoiceLure
    │   └── UAnansiAbility_CommandMimic
    ├── UAnansiAbility_SpiderSense
    │   ├── UAnansiAbility_SpiderSensePulse
    │   └── UAnansiAbility_SpiderSenseSustained
    ├── UAnansiAbility_TrickMirror
    │   ├── UAnansiAbility_Decoy
    │   ├── UAnansiAbility_Swap
    │   ├── UAnansiAbility_PhantomWalk
    │   └── UAnansiAbility_MirrorMask
    ├── UAnansiAbility_StoryBind
    │   ├── UAnansiAbility_ThreadFreeze
    │   ├── UAnansiAbility_RewindThread            (passive, auto-triggered)
    │   ├── UAnansiAbility_StorySnare
    │   └── UAnansiAbility_NarrativeCollapse
    ├── UAnansiAbility_Execution                   (cinematic finisher)
    └── UAnansiAbility_Interact                    (world interaction)
```

### Gameplay Effects Strategy

| Effect Type | Usage | Examples |
|-------------|-------|---------|
| **Instant** | One-shot damage, healing, resource grants | Melee hit damage, story fragment pickup |
| **Duration** | Buffs/debuffs with timer | Burn DoT (3s), web immobilize (3s), combo multiplier buff |
| **Infinite** | Persistent state modifiers | Talisman passives, gear bonuses, difficulty modifiers |
| **Periodic** | Repeating effects | Stamina regen (tick every 0.1s), Thread Energy regen (tick every 1s) |

### Gameplay Tags

Tags are used extensively to govern state, ability activation rules, and system communication.

```
State.Combat
State.Combat.Attacking
State.Combat.Dodging
State.Combat.Parrying
State.Combat.HitReaction
State.Combat.Exhausted
State.Traversal
State.Traversal.WallRun
State.Traversal.Climbing
State.Traversal.Swinging
State.Stealth
State.Stealth.Crouching
State.Stealth.Disguised
State.Stealth.Carrying

Ability.SilkenPath
Ability.SilkenPath.WebSwing
Ability.SilkenPath.WebPull
Ability.BorrowedVoice
Ability.SpiderSense
Ability.TrickMirror
Ability.StoryBind

Damage.Physical
Damage.Spirit
Damage.Fire
Damage.Binding
Damage.Ancestral

Status.Invulnerable
Status.Staggered
Status.Immobilized
Status.Burning
Status.Bound
Status.Slowed

Input.Attack.Light
Input.Attack.Heavy
Input.Dodge
Input.Parry
Input.Ability
```

### Ability Upgrade Implementation

Upgrades are implemented as `UDataAsset` entries (`UAnansiAbilityUpgradeData`) containing:
- Prerequisite tags (required prior upgrades)
- Cost in Story Thread Points
- Gameplay Effects to apply on unlock (modify ability parameters)
- Optional replacement Gameplay Ability class (for fundamental mechanic changes)

The `UAnansiAbilitySystemComponent` maintains a list of unlocked upgrades. On ability activation, the component checks active upgrades and applies modifier effects before execution.

---

## 6. AI Architecture

### AI Controller

`AAnansiAIController` extends `AAIController` and manages:
- Behavior Tree execution (via `UBehaviorTreeComponent`)
- Perception updates (via `UAIPerceptionComponent`)
- Blackboard state
- Group membership (registering with `UAnansiAIGroupManager`)

### Behavior Tree Structure

Each enemy type has a behavior tree composed from reusable subtrees:

```
[Root]
├── [Selector: Top-Level State]
│   ├── [Sequence: Death]
│   │   └── BTTask_Die
│   │
│   ├── [Sequence: Staggered]
│   │   └── BTTask_PlayStaggerReaction
│   │
│   ├── [Sequence: Combat] (activated when BB.CombatTarget is set)
│   │   ├── [Selector: Combat Behavior]
│   │   │   ├── [Sequence: Attack]
│   │   │   │   ├── BTDecorator_IsInAttackRange
│   │   │   │   ├── BTTask_SelectAttack (queries combo graph for available attacks)
│   │   │   │   └── BTTask_ExecuteAttack
│   │   │   │
│   │   │   ├── [Sequence: Reposition]
│   │   │   │   ├── BTDecorator_ShouldReposition (cooldown, group spacing)
│   │   │   │   └── BTTask_MoveToFlank (EQS query for flank position)
│   │   │   │
│   │   │   ├── [Sequence: Block/Dodge]
│   │   │   │   ├── BTDecorator_PlayerIsAttacking
│   │   │   │   └── BTTask_DefensiveAction
│   │   │   │
│   │   │   └── [Sequence: Pursue]
│   │   │       └── BTTask_MoveToTarget
│   │   │
│   │   └── [SubTree: GroupTactics] (enemy-type-specific)
│   │
│   ├── [Sequence: Searching] (activated when BB.AlertState == Searching)
│   │   ├── BTTask_MoveToLastKnownPosition
│   │   ├── BTTask_SearchArea (EQS)
│   │   └── BTTask_ReturnToPatrol (after timeout)
│   │
│   ├── [Sequence: Suspicious] (activated when BB.AlertState == Suspicious)
│   │   ├── BTTask_InvestigateStimulus
│   │   ├── BTTask_LookAround
│   │   └── BTTask_ReturnToPatrol
│   │
│   └── [Sequence: Patrol] (default)
│       ├── BTTask_FollowPatrolPath
│       └── BTTask_IdleBehavior (context-dependent: guard post, conversation, etc.)
```

### Perception System Configuration

| Sense | Purpose | Config |
|-------|---------|--------|
| **Sight** | Primary detection | FOV: 90 degrees, Range: 25m (guard) / 35m (zealot), peripheral range: 40 degrees at 50% detection rate |
| **Hearing** | Noise detection | Range: varies by noise source (see Stealth System). Processes `UAnansiNoiseEvent` |
| **Damage** | Alert on being hit | Immediate full alert, shares info with group |
| **Custom: Spirit Sense** | Zealot-only, detect ability use | Range: 10m, triggers when player uses abilities nearby |

### EQS (Environment Query System) Queries

| Query | Purpose | Used By |
|-------|---------|---------|
| `EQS_FlankPosition` | Find position on the opposite side of the target from allies | Melee enemies during combat |
| `EQS_CoverPosition` | Find position with line-of-sight break to target | Archers repositioning, enemies under fire |
| `EQS_SearchPoint` | Generate search positions around last known player location | All enemies during Searching state |
| `EQS_PatrolPoint` | Next patrol waypoint with variety scoring | Patrol behavior |
| `EQS_GroupSpacing` | Ensure enemies don't cluster (min 2m spacing) | Group combat coordinator |
| `EQS_AmbushPosition` | Find elevated or hidden positions for ambush setup | Spirit enemies, advanced mercenaries |

### Group Tactics

`UAnansiAIGroupManager` is a world subsystem that coordinates enemy behavior within encounter zones:

- **Attack Tokens:** Only N enemies may attack simultaneously (N = difficulty-scaled, default 2 for normal). Others circle, feint, or reposition. This prevents "swarm" feeling while maintaining threat.
- **Callout System:** When one enemy detects the player, it broadcasts to its group after a 0.5s delay (shout animation). Stealth players can interrupt this.
- **Role Assignment:** In groups of 3+, the manager assigns roles:
  - Aggressor (has attack token, engages directly)
  - Flanker (moves to sides/rear)
  - Support (ranged attacks, buffs allies, calls reinforcements)
- **Escalation:** If the player kills 50%+ of a group within 10s, survivors may flee, become enraged (aggressive buffs), or call for reinforcements depending on enemy type.

---

## 7. Stealth System Architecture

### Detection Component (`UAnansiDetectionComponent`)

Attached to every AI enemy. Manages per-enemy awareness of the player.

```cpp
USTRUCT()
struct FAnansiDetectionState
{
    float DetectionMeter;           // 0.0 (unaware) to 1.0 (fully detected)
    EAnansiAlertState AlertState;   // Unaware, Suspicious, Searching, Alert, Alarmed
    FVector LastKnownPlayerPos;
    float TimeSinceLastStimulus;
    TArray<FAnansiStimulusRecord> RecentStimuli;
};
```

**Detection Meter Fill Rate Calculation:**

```
FillRate = BaseRate
         * DistanceMultiplier(distance)       // 1.0 at 5m, 0.3 at 15m, 0.0 at max range
         * LightingMultiplier(lightLevel)      // 0.5 in shadow, 1.0 in full light
         * MovementMultiplier(playerSpeed)      // 0.4 (still), 0.7 (crouch walk), 1.0 (walk), 1.5 (run)
         * CoverMultiplier(inFoliage, behind)  // 0.6 in foliage, 0.0 behind solid cover
         * DisguiseMultiplier(disguised)        // 0.2 if Trick Mirror mask active
         * AlertModifier(zoneAlertLevel)        // 1.0 (calm), 1.3 (cautious), 1.6 (high alert), 2.0 (lockdown)
```

**Detection Thresholds:**

| Meter Value | Transition |
|-------------|------------|
| 0.0 -- 0.3 | Unaware |
| 0.3 | Suspicious (investigate) |
| 0.7 | Searching (active search, call allies) |
| 1.0 | Alert (combat engaged) |

Meter drains at 0.15/s when no stimulus is present. Faster drain in darkness (0.25/s).

### Noise System

`UAnansiNoiseEvent` is broadcast via the Perception System's hearing sense.

```cpp
USTRUCT()
struct FAnansiNoiseEvent
{
    FVector Location;
    float Loudness;          // Radius in meters
    EAnansiNoiseType Type;   // Footstep, Impact, Voice, Distraction, Combat, Alarm
    AActor* Source;
    bool bIsDistraction;     // If true, enemies investigate but don't enter Alert
};
```

Noise events are generated by:
- Player movement (automatic, scaled by surface type and movement speed)
- Player actions (throwing objects, combat, ability use)
- Borrowed Voice ability (intentional, marked as distraction)
- Environmental events (explosions, collapses, animal cries)

### AI Knowledge Sharing (`UAnansiStealthSubsystem`)

The stealth subsystem maintains a shared knowledge base per faction:

```cpp
struct FAnansiSharedKnowledge
{
    EAnansiZoneAlertLevel ZoneAlertLevel;
    FVector LastConfirmedPlayerPosition;
    float TimeSinceLastConfirmed;
    TArray<FVector> BodyLocations;           // Discovered bodies
    int32 DownedAllyCount;
    bool bAlarmRaised;
};
```

When an enemy enters Alert state, it writes to shared knowledge after its callout delay. Other enemies read from shared knowledge during their behavior tree ticks to decide whether to investigate, tighten patrols, or rush to the confirmed position.

### Stealth Volumes

Level designers place `AAnansiStealthVolume` actors to define:
- **Hiding spots:** Bushes, dark corners, carts (body hide locations)
- **Noise zones:** Areas where ambient noise masks player noise (waterfalls, crowds)
- **Light probes:** Sample points for the detection system's lighting multiplier (baked from light data)
- **Social blend zones:** Areas where social stealth is available (crowds present)

---

## 8. Traversal System Architecture

### Traversal Component (`UAnansiTraversalComponent`)

Manages traversal state, surface detection, and grip meter.

**Surface Detection:**
- Forward ray casts (5 rays in a fan) detect traversable surfaces
- Surfaces are tagged via actor tags or physical materials: `Climbable`, `WallRunnable`, `Swingable`
- Detection runs at 30 Hz during movement, 10 Hz when idle (performance optimization)

**Traversal Action Execution:**

```
[Input Detected (Jump near wall, approach ledge, etc.)]
    │
    ▼
[UAnansiTraversalComponent::EvaluateTraversalAction()]
    │
    ├── Query nearby traversal surfaces/points
    ├── Score candidates by: distance, angle, momentum alignment, player intent direction
    ├── Select best candidate
    │
    ▼
[Begin Traversal Action]
    ├── Set traversal state on state machine
    ├── Activate Motion Warping target (aligns character to surface/point)
    ├── Play traversal AnimMontage
    ├── Apply movement physics (custom: wall run gravity curve, swing pendulum, climb step)
    └── On completion/cancel: return to appropriate state
```

### Motion Warping Integration

Motion Warping is critical for making traversal feel connected to the environment:

| Action | Warp Target | Purpose |
|--------|-------------|---------|
| Vault | Obstacle top edge | Align hands to obstacle surface |
| Ledge grab | Ledge edge | Snap hands to ledge regardless of approach angle |
| Wall run start | Wall surface | Align feet to wall at correct height |
| Swing attach | Swing point | Extend web to exact anchor position |
| Takedown | Enemy position | Close gap smoothly during stealth takedown |
| Attack lunge | Target position | Slight forward magnetism on melee attacks |

### IK Systems

| IK System | Purpose | Implementation |
|-----------|---------|----------------|
| **Foot IK** | Terrain adaptation during locomotion | Two-bone IK + trace from hips to ground, adjust pelvis height |
| **Hand IK** | Wall contact, ledge grip, climbing hand placement | Two-bone IK targeting surface contact points from ray traces |
| **Look-at IK** | Head/spine tracking of targets, points of interest | FABRIK chain on spine + head, blended with locomotion |
| **Staff IK** | Staff plants on ground during idle, contacts surfaces during traversal | Single-bone IK on weapon socket |

### Swing Point System

`AAnansiSwingPoint` actors are placed in the world by level designers (or procedurally in open areas based on geometry analysis).

```cpp
UCLASS()
class AAnansiSwingPoint : public AActor
{
    UPROPERTY() float MaxWebLength;         // Max distance player can be from this point
    UPROPERTY() float MinApproachSpeed;     // Player must be moving at least this fast
    UPROPERTY() FVector LaunchDirection;     // Suggested launch direction on release
    UPROPERTY() bool bOneTimeUse;            // Spirit threads that break after use
    UPROPERTY() bool bRequiresSpiderSense;   // Only visible in Spider Sense mode
};
```

The `UAnansiTraversalSubsystem` maintains a spatial hash of all registered swing points for O(1) lookups by the player's traversal component.

### Root Motion vs. In-Place Animation

| Action | Approach | Reason |
|--------|----------|--------|
| Combat attacks | Root motion | Precise positioning, hit detection alignment |
| Dodge | Root motion | Consistent distance, i-frame sync |
| Wall run | Procedural + root motion blend | Root motion for foot placement, procedural for wall-following |
| Climbing | Procedural | Dynamic surface, variable spacing |
| Swing | Fully procedural | Physics-based pendulum |
| Vault/Slide | Root motion | Consistent timing, art-directed feel |
| Locomotion | In-place + procedural | Blend space driven, character controller physics |

---

## 9. Narrative System Architecture

### Dialogue Tree System

Dialogue is authored in the custom `AnansiDialogueEditor` plugin and stored as `UAnansiDialogueAsset` data assets.

**Dialogue Asset Structure:**

```
UAnansiDialogueAsset
├── DialogueID: FName
├── Participants: TArray<FAnansiDialogueParticipant>
│   ├── CharacterID
│   ├── DisplayName
│   └── Portrait
├── Nodes: TMap<FName, FAnansiDialogueNode>
│   ├── FAnansiDialogueNode
│   │   ├── NodeID: FName
│   │   ├── Speaker: FName
│   │   ├── Text: FText (localized)
│   │   ├── VoiceAsset: USoundBase*
│   │   ├── AnimationTag: FGameplayTag (drives speaker animation)
│   │   ├── CameraTag: FGameplayTag (drives camera behavior)
│   │   ├── Conditions: TArray<FAnansiCondition> (trust checks, item checks, flag checks)
│   │   ├── Effects: TArray<FAnansiEffect> (modify trust, set flags, give items)
│   │   ├── Responses: TArray<FAnansiDialogueResponse>
│   │   │   ├── Text: FText
│   │   │   ├── ToneTag: EAnansiDialogueTone (Clever, Direct, Kind, Silent)
│   │   │   ├── Conditions: TArray<FAnansiCondition>
│   │   │   ├── Effects: TArray<FAnansiEffect>
│   │   │   └── NextNodeID: FName
│   │   └── NextNodeID: FName (for non-branching nodes)
│   └── ...
├── EntryNodeID: FName
└── ExitEffects: TArray<FAnansiEffect>
```

### Quest State Machine

Quests are managed by `UAnansiNarrativeSubsystem` using a state machine per quest.

```
[Quest States]
    Unknown ──> Available ──> Active ──> Completed / Failed / Abandoned
                                 │
                                 ├── Objective 1: Incomplete / Complete
                                 ├── Objective 2: Incomplete / Complete
                                 └── Objective N: Incomplete / Complete
```

**Quest Data:**

```cpp
USTRUCT()
struct FAnansiQuestData
{
    FName QuestID;
    FText DisplayName;
    FText Description;
    EAnansiQuestState State;
    TArray<FAnansiObjective> Objectives;
    TArray<FAnansiCondition> ActivationConditions;  // When does this quest become available?
    TArray<FAnansiEffect> CompletionRewards;
    FName RegionID;
    EAnansiQuestType Type;  // Main, Side, Regional
};
```

Quests are defined as data assets and registered with the narrative subsystem at world load. The subsystem evaluates activation conditions each time game state changes (flag set, trust changed, region entered).

### Trust Meter Implementation

```cpp
USTRUCT()
struct FAnansiTrustMeter
{
    FName CharacterID;
    float CurrentValue;     // Clamped to MinValue..MaxValue
    float MinValue;         // e.g., -100
    float MaxValue;         // e.g., +100
    TArray<FAnansiTrustThreshold> Thresholds;  // Named thresholds that trigger events
};
```

Trust modifications are always applied through `FAnansiEffect` objects, ensuring full traceability (which dialogue/quest/action caused the change). The UI displays trust change notifications.

### Save/Load Integration

The narrative subsystem serializes:
- All quest states and objective progress
- All trust meter values
- All set narrative flags (a `TSet<FName>`)
- Dialogue history (which nodes have been visited, for "already said" tracking)
- Story fragment collection state

This data is encapsulated in `FAnansiNarrativeSaveData` and passed to the save subsystem.

---

## 10. World Streaming and Level Design

### World Partition

The game uses UE5 World Partition for its open regions. Each of the 6 regions is a World Partition level.

**Streaming Strategy:**

```
[World Partition Grid]
    ├── Cell Size: 128m x 128m (exteriors)
    ├── Loading Range: 256m (gameplay), 512m (LOD/imposters)
    ├── Priority: Distance-based with manual priority overrides for key landmarks
    └── Data Layers:
        ├── Landscape (always loaded when in region)
        ├── Buildings_Exterior (standard streaming)
        ├── Buildings_Interior (proximity + trigger-based)
        ├── Gameplay_Actors (enemies, interactables -- standard streaming)
        ├── Narrative_Actors (quest NPCs -- loaded based on quest state)
        └── Spirit_World (overlay layer, toggled by gameplay state)
```

### Level Streaming for Interiors

Major interiors (dungeons, boss arenas, puzzle rooms) use traditional level streaming via trigger volumes. This allows:
- Full art control over interior lighting (not affected by exterior time-of-day)
- Isolated AI navigation meshes
- Performance isolation (interior budget separate from exterior)

### Region Transitions

Regions are connected via narrative transition points (story moments that justify a loading screen). During transition:
1. Fade to black with Anansi narration (voice-over during load)
2. Unload previous region's World Partition level
3. Load new region's World Partition level
4. Stream in spawn area cells
5. Fade in from black

Target transition time: under 5 seconds on SSD (PS5/XSX/NVMe PC).

### Spirit World Overlay

The spirit world is implemented as an overlay data layer within World Partition. When active:
- Post-process volume applies spirit-world visual treatment (desaturation, golden highlights, particle overlay)
- Spirit-world data layer streams in (spirit-only geometry, platforms, enemies)
- Mortal-world interactive actors are disabled (but geometry remains for grounding)
- This allows seamless toggling without level loads

---

## 11. Animation System Design

### State Machine Overview

```
[Main State Machine]
├── Locomotion (Blend Space 2D: Speed x Direction)
│   ├── Idle
│   ├── Walk (Blend Space 1D: Direction)
│   ├── Run (Blend Space 1D: Direction)
│   ├── Sprint
│   └── Transitions: Start/Stop/Turn animations
│
├── Combat (sub-state machine)
│   ├── Combat Idle
│   ├── Attack States (driven by combo graph -- montage slots)
│   ├── Dodge (directional montage)
│   ├── Parry (success/fail montage)
│   ├── Hit Reaction (directional, scaled by damage)
│   └── Execution (cinematic montage)
│
├── Traversal (sub-state machine)
│   ├── Wall Run (blend space: speed x wall angle)
│   ├── Climb (blend space: direction)
│   ├── Ledge (hang, shimmy, pull-up)
│   ├── Swing (procedural + additive)
│   ├── Vault/Slide (montages)
│   └── Air (fall, land, double jump)
│
├── Stealth (sub-state machine)
│   ├── Crouch Locomotion (blend space)
│   ├── Takedown (montage per type)
│   ├── Body Carry (locomotion override)
│   └── Social Blend (context montages: browse, chat, carry)
│
└── Cinematic
    └── Driven entirely by montages / sequencer
```

### Blend Spaces

| Blend Space | Axes | Purpose |
|-------------|------|---------|
| `BS_Locomotion` | Speed (0--600) x Direction (-180--180) | Standard movement |
| `BS_CrouchLocomotion` | Speed (0--200) x Direction (-180--180) | Stealth movement |
| `BS_WallRun` | Speed (200--500) x WallAngle (-30--30) | Wall run lean |
| `BS_Climb` | Horizontal (-1--1) x Vertical (-1--1) | Climbing direction |
| `BS_CombatIdle` | AlertLevel (0--1) | Relaxed to tense combat stance |

### Anim Notifies

| Notify | Purpose | System |
|--------|---------|--------|
| `AN_MeleeTraceBegin/End` | Activate/deactivate melee hit detection | Combat |
| `AN_FootstepSound` | Trigger footstep audio event (passes surface type) | Audio |
| `AN_FootstepNoise` | Generate noise event for stealth system | Stealth |
| `AN_ComboWindowOpen/Close` | Define when next combo input is accepted | Combat |
| `AN_CancelWindowOpen/Close` | Define when dodge/parry can interrupt | Combat |
| `AN_MotionWarpTarget` | Activate motion warp to target | Traversal/Combat |
| `AN_VFXSpawn` | Spawn Niagara effect at socket | VFX |
| `AN_CameraShake` | Trigger camera shake | Camera |
| `AN_InvulnerableStart/End` | Grant/remove i-frames | Combat |
| `AN_TraversalExit` | Mark safe exit point for traversal interruption | Traversal |

### Animation Budget

- Player character: up to 120 unique animations (locomotion: 30, combat: 40, traversal: 30, stealth: 10, cinematic: 10)
- Per enemy type: 30--50 animations
- Companion (Ama): 60 animations (locomotion + combat + contextual)
- Crowd NPCs: 10--15 shared animations (State Tree driven)

---

## 12. Audio Integration

### Wwise Architecture

```
[Wwise Project Structure]
├── Master Bus
│   ├── Music Bus
│   │   ├── Exploration Music
│   │   ├── Combat Music
│   │   ├── Stealth Music
│   │   └── Cinematic Music
│   ├── SFX Bus
│   │   ├── Player SFX
│   │   ├── Enemy SFX
│   │   ├── Environment SFX
│   │   ├── Ability SFX
│   │   └── UI SFX
│   ├── Voice Bus
│   │   ├── Dialogue
│   │   ├── Anansi Narration
│   │   └── Ambient Voice (crowd, NPCs)
│   └── Ambience Bus
│       ├── Region Ambience
│       └── Spirit World Ambience
```

### Music System

Music uses Wwise Interactive Music with the following state-driven layers:

**Music States:**
- `Region` (Nkran, Bosumtwi, Volta, Elmina, Kwahu, Kumasi)
- `Intensity` (Exploration, Tension, Combat_Low, Combat_High, Boss)
- `SpiritWorld` (Mortal, Spirit, Transitioning)

**Layer Strategy:**
- Base layer: Region-specific theme (always playing, volume modulated)
- Percussion layer: Added during Tension and Combat states (talking drums)
- Melody layer: Highlife guitar during Exploration, aggressive motifs during Combat
- Spirit layer: Drone and reverb overlay during Spirit World sections

Transitions use musical cues (beat-synced transitions) rather than crossfades for seamless feel.

### Audio Events Integration

Audio events are triggered via:
1. **Anim Notifies** (`AN_FootstepSound`, `AN_WeaponSwish`, etc.)
2. **Gameplay Events** (damage taken, ability activated, detection state change)
3. **Blueprint Events** (door open, puzzle solved, environmental trigger)
4. **Subsystem Callbacks** (combat start/end, stealth state change, music state change)

### Spatial Audio

- Wwise Spatial Audio for 3D positioning
- Rooms and Portals for interior acoustic modeling
- Reverb zones for caves, temples, enclosed spaces
- Obstruction/occlusion calculated per-frame for key audio sources (enemies, ambient hazards)

### Metasound Usage

Metasound handles procedural audio elements:
- Spirit-world ambient drones (generative, non-looping)
- Spider-sense proximity indicators (pitch/volume mapped to threat distance)
- Heartbeat at low health (BPM increases as health decreases)
- Thread energy resonance (subtle hum when abilities are charged)

---

## 13. Performance Budgets

### Frame Budget (Target: 60 FPS on PS5/XSX, 30 FPS minimum)

| System | Budget (ms) | Notes |
|--------|-------------|-------|
| **Game Thread** | 8.0 | Character updates, AI, gameplay logic |
| **Render Thread** | 8.0 | Draw call submission, culling |
| **GPU** | 14.0 | Rendering (with 2ms headroom) |
| **Total Frame** | 16.67 | 60 FPS target |

### Draw Call Budget

| Category | Budget | Notes |
|----------|--------|-------|
| **Environment** | 1500 | Nanite handles most geometry; budget is for non-Nanite objects |
| **Characters** | 300 | Player + up to 15 visible NPCs/enemies |
| **VFX** | 200 | Niagara particle systems |
| **UI** | 50 | HUD elements |
| **Total** | 2050 | Target maximum |

### Polygon Budgets

| Asset | Triangle Budget | Notes |
|-------|----------------|-------|
| Player character | 80,000 | Highest detail; always on screen |
| Main NPCs (Ama, Nana Bekoe) | 60,000 | Frequent close-up during dialogue |
| Standard enemy | 30,000 | Multiple on screen |
| Crowd NPC | 8,000 | Mass Entity, distance-LOD aggressively |
| Environment props | Nanite-managed | Virtual geometry; no hard budget per-prop |

### AI Tick Budgets

| AI Type | Tick Rate | Budget Per Tick | Max Concurrent |
|---------|-----------|-----------------|----------------|
| Combat AI (active) | 10 Hz | 0.3 ms | 8 |
| Combat AI (passive, out of combat) | 2 Hz | 0.1 ms | 15 |
| Perception updates | 5 Hz | 0.2 ms per agent | 15 |
| EQS queries | On-demand | 1.0 ms max | 2 simultaneous |
| Crowd (State Tree) | 1 Hz | 0.02 ms per entity | 100 |
| Group Manager | 5 Hz | 0.5 ms total | 1 |

**Total AI budget: 4.0 ms** (within game thread allocation).

### Memory Budgets

| Category | Budget | Platform |
|----------|--------|----------|
| **Textures** | 4 GB | Streaming pool, virtual textures |
| **Meshes** | 2 GB | Nanite virtual geometry |
| **Animation** | 512 MB | Compressed, streamed |
| **Audio** | 256 MB | Wwise streaming bank |
| **AI/Gameplay** | 256 MB | Behavior trees, perception, navigation |
| **UI** | 128 MB | Textures, fonts, widget state |
| **Navigation Mesh** | 128 MB | Per-region, streamed |
| **Total Target** | 8 GB | Fits within 16 GB console unified memory with OS overhead |

---

## 14. Save System Design

### Save Architecture

```
UAnansiSaveSubsystem (GameInstance)
    │
    ├── ManualSave()          -- Player-initiated
    ├── AutoSave()            -- Triggered at checkpoints
    ├── QuickSave()           -- PC only
    ├── LoadSave(SlotIndex)
    └── DeleteSave(SlotIndex)
```

### Save Data Structure

```cpp
USTRUCT()
struct FAnansiSaveData
{
    // Header
    int32 SaveVersion;
    FDateTime SaveTimestamp;
    FString RegionName;
    float PlaytimeSeconds;
    UTexture2D* Thumbnail;       // Screenshot at save time

    // Player State
    FTransform PlayerTransform;
    FAnansiAttributeSnapshot Attributes;    // Health, stamina, energy, grip
    TArray<FName> UnlockedAbilities;
    TArray<FName> UnlockedUpgrades;
    TArray<FName> EquippedTalismans;
    int32 StoryThreadPoints;

    // Narrative State
    FAnansiNarrativeSaveData NarrativeData;  // Quests, trust, flags, dialogue history
    TSet<FName> CollectedStoryFragments;

    // World State
    TMap<FName, FAnansiRegionState> RegionStates;  // Per-region: enemies killed, doors opened, etc.
    TArray<FAnansiPersistentActorState> PersistentActors;  // State of specific world actors

    // Settings (saved separately but referenced)
    FName DifficultyPreset;
    FAnansiAccessibilitySettings AccessibilitySettings;
};
```

### Serialization Strategy

- Binary serialization via `FArchive` for performance and size
- Save file validation via CRC32 checksum appended to file
- Save format is versioned; old saves are migrated forward via `MigrateSaveData(OldVersion, NewVersion)`
- Maximum 10 manual save slots + 3 rotating autosave slots + 1 quicksave slot
- Save file size target: under 2 MB per slot

### Autosave Triggers

Autosaves fire at:
- Region entry (after transition load completes)
- Before boss encounters
- After major narrative choices
- After quest completion
- Every 15 minutes of playtime (configurable)

Autosave is non-blocking: data is gathered on the game thread (under 1ms), serialization and disk write happen on a background thread.

### Platform Integration

| Platform | Storage | Notes |
|----------|---------|-------|
| PC | `%LOCALAPPDATA%/AnansiWebOfShadows/SaveGames/` | Standard UE5 save path |
| PS5 | PS5 Save Data system | Requires Activity integration for save context |
| XSX | Connected Storage | Cloud-synced via Xbox services |

---

## 15. Network Considerations

### Single-Player Architecture with Future-Proofing

The game is single-player only at launch. However, the architecture should not preclude future multiplayer features (co-op, challenge modes). Design principles:

1. **Authority model:** All gameplay logic runs through subsystems that could be made server-authoritative. No raw state mutation -- always use request/response patterns through subsystem APIs.

2. **Replication-ready components:** Core components (`UAnansiHealthComponent`, `UAnansiCombatComponent`) use `UPROPERTY(Replicated)` metadata even though replication is disabled. This ensures member variables are tagged for future net serialization without refactoring.

3. **Event-driven communication:** Systems communicate via delegates and events (not direct function calls across modules). This pattern maps cleanly to RPC boundaries.

4. **Deterministic simulation:** Combat calculations are deterministic given the same inputs. Random elements use seeded RNG (`FRandomStream`) that can be synchronized.

5. **Latency tolerance:** Input buffering (already present for combo system) provides a natural framework for latency compensation if multiplayer is added.

### Analytics Integration

The game ships with optional telemetry (player opt-in) for:
- Difficulty spikes (death locations, retry counts)
- Ability usage frequency
- Choice distribution at branch points
- Session length and quit points
- Performance metrics (frame time percentiles by region)

Telemetry is sent via HTTPS to a backend analytics service. No PII is collected. Implementation: lightweight event queue flushed every 60 seconds.

---

## 16. Build Pipeline and CI

### Source Control

- **Repository:** Perforce (P4) for assets and code, mirrored to Git for code-only CI
- **Branch strategy:**
  - `//depot/main/` -- integration branch, always buildable
  - `//depot/dev/<feature>/` -- feature branches, merged to main via reviewed changelists
  - `//depot/release/<version>/` -- release candidates, branched from main
- **Asset locking:** Enabled for binary assets (maps, meshes, textures) to prevent merge conflicts

### Build System

| Build Type | Trigger | Duration Target | Output |
|------------|---------|-----------------|--------|
| **Editor Build (Win64)** | Every P4 submit to main | 10 min | Editor binaries, used by team |
| **Cook + Package (Win64 Development)** | Nightly at 02:00 | 45 min | Playable development build |
| **Cook + Package (PS5 Development)** | Nightly at 03:00 | 60 min | PS5 dev kit build |
| **Cook + Package (XSX Development)** | Nightly at 03:00 | 60 min | Xbox dev kit build |
| **Shipping Build (All Platforms)** | Manual trigger (release candidate) | 120 min | Submission-ready packages |

### CI Pipeline (Jenkins / TeamCity)

```
[P4 Submit to //depot/main/]
    │
    ▼
[Stage 1: Compile]
    ├── Compile C++ (Win64 Editor, Development)
    ├── Compile C++ (Win64 Shipping)       -- parallel
    └── Static analysis (PVS-Studio)       -- parallel
    │
    ▼
[Stage 2: Automated Tests]
    ├── Unit tests (Automation Framework)
    │   ├── Combat system tests (damage calc, combo graph traversal)
    │   ├── Narrative system tests (quest state transitions, trust math)
    │   ├── Save system tests (serialize/deserialize round-trip)
    │   └── Ability system tests (cost, cooldown, effect application)
    ├── Functional tests (Gauntlet framework)
    │   ├── Player movement smoke test (load map, move to waypoints)
    │   ├── Combat encounter smoke test (engage enemies, verify no crashes)
    │   └── Save/load smoke test (save at checkpoint, load, verify state)
    └── Asset validation
        ├── Missing references check
        ├── Texture format validation
        └── Blueprint compile check (all BPs)
    │
    ▼
[Stage 3: Nightly Cook (triggered by schedule)]
    ├── Cook content (all platforms in parallel)
    ├── Package builds
    ├── Deploy to QA shared drive
    └── Notify QA channel (Slack/Teams)
```

### Automated Test Coverage Targets

| Module | Unit Test Coverage | Functional Tests |
|--------|-------------------|-----------------|
| AnansiCore | 80% | -- |
| AnansiCombat | 70% | 5 scenarios |
| AnansiAbilities | 70% | 5 scenarios |
| AnansiNarrative | 80% | 3 scenarios |
| AnansiSave | 90% | 2 scenarios |
| AnansiStealth | 60% | 3 scenarios |
| AnansiTraversal | 50% | 3 scenarios |
| AnansiAI | 50% | 5 scenarios |

### Code Standards

- C++ style: Unreal Engine coding standard (Epic's guidelines)
- Blueprints: Naming convention enforced (`BP_`, `WBP_`, `ABP_`, `BT_`, `EQS_`, `DA_`)
- Code review required for all submits to `//depot/main/`
- No `UObject::FindObject` or `LoadObject` in gameplay code (use asset references and soft references)
- No raw `new`/`delete` (use UE memory management)
- All user-facing strings must use `FText` for localization support

---

*End of Technical Design Document*
