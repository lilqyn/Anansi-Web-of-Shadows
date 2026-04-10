# Anansi: Web of Shadows -- Vertical Slice Plan

**Version:** 1.0  
**Date:** 2026-04-10  
**Target Completion:** Month 6 of production (24 weeks from start)  
**Slice Region:** The Festival Capital (Nkran) -- Region 1

---

## Table of Contents

1. [Slice Scope Definition](#1-slice-scope-definition)
2. [Phase 1: Foundation (Weeks 1--4)](#2-phase-1-foundation-weeks-14)
3. [Phase 2: Combat (Weeks 5--7)](#3-phase-2-combat-weeks-57)
4. [Phase 3: Traversal (Weeks 8--10)](#4-phase-3-traversal-weeks-810)
5. [Phase 4: Stealth (Weeks 11--12)](#5-phase-4-stealth-weeks-1112)
6. [Phase 5: Abilities (Weeks 13--15)](#6-phase-5-abilities-weeks-1315)
7. [Phase 6: Puzzle (Weeks 16--17)](#7-phase-6-puzzle-weeks-1617)
8. [Phase 7: Narrative (Weeks 18--19)](#8-phase-7-narrative-weeks-1819)
9. [Phase 8: Boss (Weeks 20--21)](#9-phase-8-boss-weeks-2021)
10. [Phase 9: Polish (Weeks 22--24)](#10-phase-9-polish-weeks-2224)
11. [Dependency Map](#11-dependency-map)
12. [Risk Assessment](#12-risk-assessment)
13. [Definition of Done](#13-definition-of-done)

---

## 1. Slice Scope Definition

### Setting

The vertical slice takes place in a portion of the Festival Capital city of Nkran. The playable area covers:

- **The Grand Market:** Open-air market district with crowds, stalls, and rooftops. Used for traversal, social stealth, and exploration.
- **The Palace Approach:** A multi-level fortified zone leading to the palace. Used for stealth infiltration and combat encounters.
- **The Storyteller's Chamber:** An interior puzzle room within the palace. Used for the folklore puzzle.
- **The Palace Courtyard:** An arena space for the boss encounter.
- **Resistance Hideout:** A small interior hub for narrative scenes.

**Approximate playable area:** 400m x 400m exterior + 3 interiors.

### Playable Content Scope

| Element | Scope |
|---------|-------|
| **Playtime** | 30--45 minutes of directed content |
| **Combat encounters** | 3 encounters (2 standard, 1 boss) |
| **Stealth section** | 1 infiltration sequence (Palace Approach) |
| **Traversal segment** | Market rooftop parkour route + 4 web-swing points |
| **Puzzle** | 1 Adinkra symbol puzzle (Storyteller's Chamber) |
| **Narrative** | 1 dialogue scene with Ama Serwaa (1 choice), 1 Anansi narration moment |
| **Boss** | Captain of the Mask (2 phases, simplified from full 3-phase design) |
| **Abilities** | Silken Path (Web Pull + Web Swing) and Spider Sense (Pulse mode) |
| **Enemy types** | Palace Guards (melee + shield variant) |
| **NPCs** | Ama Serwaa (dialogue only), Anansi (narration VO), Auntie Efua (quest giver, placeholder) |

### What Is Excluded

- Open-world exploration beyond the slice boundaries
- Side quests and secondary NPCs
- Gear/upgrade systems (loadout is fixed for the slice)
- Full combo system (reduced to 3 combos: LLL, LLH, HH)
- Borrowed Voice, Trick Mirror, Story Bind abilities
- Spirit-world overlay
- Multiple enemy types (only Palace Guards)
- Save/load (checkpoint-based restart only)
- Full UI (simplified HUD, no menus beyond pause)

### Purpose

The vertical slice must prove that:
1. **Movement feels great** -- traversal and combat are responsive and satisfying
2. **Stealth works** -- the detection system creates genuine tension and rewards clever play
3. **Abilities are fun** -- Silken Path and Spider Sense feel powerful and integrate with other systems
4. **The world is compelling** -- the Festival Capital atmosphere draws the player in
5. **The narrative hook lands** -- players understand the stakes and want to continue

---

## 2. Phase 1: Foundation (Weeks 1--4)

### Week 1: Project Setup and Player Controller

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| UE5 project creation, module structure, plugin setup | Engine Programmer | 16 | None |
| Source control setup (P4 depot, workspace templates) | Engine Programmer | 8 | None |
| CI pipeline: compile stage + basic test stage | Build Engineer | 16 | Project creation |
| `AAnansiPlayerCharacter` class with component stubs | Gameplay Programmer | 16 | Project creation |
| `AAnansiPlayerController` with Enhanced Input setup | Gameplay Programmer | 12 | Player character |
| Input mapping contexts: Exploration, Combat (stubs) | Gameplay Programmer | 8 | Enhanced Input setup |
| GAS setup: `UAnansiAbilitySystemComponent`, `UAnansiAttributeSet` with Health/Stamina | Gameplay Programmer | 16 | Player character |
| Blockout: Market district greybox (BSP/simple meshes) | Level Designer | 24 | Project creation |
| Character mesh import (T-pose, rigged, placeholder textures) | Character Artist | 16 | None |

**Week 1 Deliverable:** Player character spawns in a greyboxed market. Can move (no animation). Input contexts respond. GAS attributes are readable in debug.

### Week 2: Camera and Basic Movement

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Third-person camera: spring arm, collision, orbit | Camera Programmer | 20 | Player controller |
| Camera settings: FOV, distance, lag, collision response | Camera Programmer | 8 | Camera base |
| `UAnansiTraversalComponent`: ground detection, slope handling | Gameplay Programmer | 16 | Player character |
| Basic locomotion: walk, run, sprint, stop/start | Gameplay Programmer | 16 | Traversal component |
| Stamina attribute integration: sprint drains, regen | Gameplay Programmer | 8 | GAS setup, locomotion |
| Locomotion blend space: idle/walk/run/sprint (mocap or hand-keyed) | Animator | 24 | Character mesh |
| Player Animation Blueprint: locomotion state machine | Animator | 16 | Locomotion anims |
| Foot IK prototype (terrain adaptation) | Animator/Programmer | 12 | Anim BP |
| Blockout: Palace Approach greybox | Level Designer | 20 | None |

**Week 2 Deliverable:** Player runs around the market with animated locomotion. Camera follows smoothly. Sprint uses stamina.

### Week 3: Jump, Fall, and Basic Interactions

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Jump: standard, double-jump (gated for later unlock) | Gameplay Programmer | 12 | Locomotion |
| Fall/land system: fall height detection, landing recovery | Gameplay Programmer | 12 | Jump |
| Jump/fall/land animations | Animator | 16 | Anim BP |
| `UAnansiInteractionComponent`: overlap detection, prompt display | Gameplay Programmer | 12 | Player character |
| `AAnansiInteractable` base class: interact interface | Gameplay Programmer | 8 | Interaction component |
| Placeholder interactables in market (crates, doors) | Level Designer | 8 | Interactable base |
| `UAnansiHealthComponent`: damage reception, death state | Gameplay Programmer | 12 | GAS attributes |
| Simple HUD: health bar, stamina bar (UMG, placeholder art) | UI Programmer | 16 | Health/stamina attributes |
| Blockout: Storyteller's Chamber interior, Palace Courtyard | Level Designer | 20 | None |

**Week 3 Deliverable:** Player can jump, fall, land, and interact with placeholder objects. HUD displays health and stamina. Death state exists (respawn at spawn point).

### Week 4: State Machine and Animation Polish

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Player state machine (C++): Exploration, Combat, Traversal, Stealth top-level states | Gameplay Programmer | 24 | All Week 1--3 systems |
| State transition rules and priority system | Gameplay Programmer | 16 | State machine |
| Animation Blueprint: integrate state machine with anim states | Animator | 16 | State machine, all anims |
| Start/stop/turn transition animations | Animator | 16 | Locomotion blend space |
| Blockout: Resistance Hideout interior | Level Designer | 8 | None |
| First greybox art pass: replace BSP with modular kit pieces | Environment Artist | 24 | All blockouts |
| Performance baseline: profile frame times, identify hot spots | Engine Programmer | 8 | All systems running |
| CI: Add functional smoke test (spawn, move to waypoint, no crash) | Build Engineer | 12 | CI pipeline |

**Week 4 Deliverable:** Player character moves through the entire slice greybox with polished transitions. State machine governs behavior. All interiors accessible. First playable milestone internally.

---

## 3. Phase 2: Combat (Weeks 5--7)

### Week 5: Basic Attacks and Combo System

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `UAnansiCombatComponent`: attack state tracking, combo input buffer | Combat Programmer | 24 | State machine |
| Combo graph data asset + editor prototype (can define LLL, LLH, HH) | Combat Programmer + Tools | 20 | Combat component |
| `UAnansiAbility_MeleeAttack` (GAS ability): reads combo graph, plays montage | Combat Programmer | 16 | GAS setup, combo graph |
| Light attack animations (L1, L2, L3) with staff | Animator | 20 | Character mesh |
| Heavy attack animations (H1, H2, charge) with staff | Animator | 16 | Character mesh |
| Anim notifies: `AN_MeleeTraceBegin/End`, `AN_ComboWindowOpen/Close` | Animator | 8 | Attack anims |
| Hit detection: sphere sweep along staff sockets | Combat Programmer | 16 | Anim notifies |
| Staff mesh import + attachment to character skeleton | Character Artist | 8 | Character mesh |

**Week 5 Deliverable:** Player can perform light combo (LLL), light-heavy combo (LLH), and heavy combo (HH) on a test dummy. Hits register with debug visualization.

### Week 6: Damage, Dodge, and Parry

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `UAnansiCombatSubsystem`: damage pipeline implementation | Combat Programmer | 20 | Hit detection |
| `UAnansiStaggerComponent`: stagger bar, stagger-break state | Combat Programmer | 12 | Damage pipeline |
| Dodge ability: tap dodge, directional dodge, i-frames | Combat Programmer | 16 | State machine, GAS |
| Perfect dodge: timing window, slow-motion trigger, counter window | Combat Programmer | 12 | Dodge |
| Parry ability: parry window, success/fail states | Combat Programmer | 16 | State machine, GAS |
| Perfect parry: tighter window, reposition behind enemy | Combat Programmer | 8 | Parry |
| Dodge animations (4 directions + tap) | Animator | 16 | Anim BP |
| Parry animations (attempt, success, fail) | Animator | 12 | Anim BP |
| Hit reaction animations (light, heavy, stagger-break) for player | Animator | 12 | Anim BP |
| Hit-stop implementation (brief freeze on hit) | Combat Programmer | 8 | Damage pipeline |
| Camera shake on hit | Camera Programmer | 4 | Camera system |

**Week 6 Deliverable:** Full player combat moveset functional. Damage applies, stagger works, dodge and parry have correct timing windows. Hit-stop and camera shake provide impact feedback.

### Week 7: Enemy AI -- Palace Guard

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `AAnansiEnemyBase` class with health, stagger, combat components | AI Programmer | 16 | Damage pipeline |
| `AAnansiAIController` with behavior tree, blackboard | AI Programmer | 12 | Enemy base |
| Palace Guard behavior tree: patrol, engage, attack, reposition | AI Programmer | 24 | AI controller |
| Guard melee attacks (3-hit combo, telegraphed heavy swing) | AI Programmer | 12 | BT, combat system |
| Shield variant: blocks light attacks, must be guard-broken with heavy | AI Programmer | 12 | Guard base |
| Guard mesh + placeholder textures | Character Artist | 16 | None |
| Guard animations: locomotion, attacks, hit reactions, death | Animator | 24 | Guard mesh |
| AI perception: sight sense setup, basic detection | AI Programmer | 8 | AI controller |
| `UAnansiAIGroupManager`: attack token system (max 2 attackers) | AI Programmer | 12 | Multiple enemies |
| Place 2 combat encounters in Palace Approach greybox | Level Designer | 8 | Guard AI, blockout |
| Combo finisher + execution move (one each) | Animator + Combat Programmer | 12 | Stagger system |

**Week 7 Deliverable:** Two combat encounters playable in the Palace Approach. Guards patrol, detect player, engage with coordinated attacks. Player can fight, dodge, parry, and execute staggered guards. Group attack tokens prevent unfair swarming.

---

## 4. Phase 3: Traversal (Weeks 8--10)

### Week 8: Parkour Fundamentals

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Auto-vault: detect waist-height obstacles while sprinting, trigger vault | Traversal Programmer | 16 | Traversal component |
| Slide: sprint + crouch input, slide under gaps | Traversal Programmer | 12 | Traversal component |
| Ledge grab: automatic on reaching ledge, hang state | Traversal Programmer | 16 | Traversal component |
| Ledge shimmy, pull-up, drop | Traversal Programmer | 12 | Ledge grab |
| Motion Warping integration: vault alignment, ledge alignment | Traversal Programmer | 12 | Motion Warping plugin |
| Vault animation | Animator | 8 | Anim BP |
| Slide animation | Animator | 8 | Anim BP |
| Ledge animations (grab, hang, shimmy, pull-up, drop) | Animator | 16 | Anim BP |
| Place traversal geometry in Market greybox (ledges, low walls, gaps) | Level Designer | 16 | Parkour systems |

**Week 8 Deliverable:** Player can vault, slide, and grab ledges throughout the Market area. Auto-parkour feels responsive when sprinting through the environment.

### Week 9: Wall Run and Climb

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Wall run: surface detection, horizontal run for 3s, physics | Traversal Programmer | 20 | Traversal component |
| Wall jump: launch off wall during wall run | Traversal Programmer | 8 | Wall run |
| Wall climb: designated climbable surfaces, grip meter | Traversal Programmer | 20 | Traversal component, GAS (grip attribute) |
| Climb jump-off: leap away from wall | Traversal Programmer | 6 | Wall climb |
| Climbable surface tagging system (actor tags + physical material) | Traversal Programmer | 6 | Wall climb |
| Wall run animations (run, start, end, jump-off) | Animator | 16 | Anim BP |
| Climb animations (up, down, left, right, jump-off) | Animator | 16 | Anim BP |
| Hand IK for wall contact and climb surfaces | Animator/Programmer | 12 | Climb anims |
| Tag climbable surfaces in Palace Approach | Level Designer | 8 | Tagging system |
| Design and place rooftop parkour route through Market | Level Designer | 16 | All parkour + climb |

**Week 9 Deliverable:** Player can wall run, wall jump, and climb designated surfaces. A flowing parkour route exists from Market ground level to rooftops and back.

### Week 10: Web Swing (Silken Path -- Traversal)

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `AAnansiSwingPoint` actor: anchor point, max web length, activation range | Traversal Programmer | 8 | Traversal component |
| `UAnansiTraversalSubsystem`: swing point spatial registration and query | Traversal Programmer | 12 | Swing point actor |
| Web swing physics: pendulum motion, speed gain at arc bottom, release momentum | Traversal Programmer | 24 | Swing point system |
| Web attach/release input handling (hold R1 near valid point) | Traversal Programmer | 8 | Swing physics |
| Web line visual: Niagara beam from hand socket to anchor point | VFX Artist | 12 | Swing system |
| Swing animations (attach, swing forward, swing back, release/launch) | Animator | 16 | Anim BP |
| Place 4+ swing points in Market (tree branches, building overhangs) | Level Designer | 8 | Swing point system |
| Chain-swing tuning: smooth transitions between consecutive swing points | Traversal Programmer | 12 | Swing system |
| Traversal integration test: run the full parkour + swing route end-to-end | QA + Level Designer | 8 | All traversal |

**Week 10 Deliverable:** Player can web-swing between anchor points in the Market area. Swinging chains smoothly. The full traversal route (ground parkour to rooftops to web-swing across a gap to climb wall) is playable and fun.

---

## 5. Phase 4: Stealth (Weeks 11--12)

### Week 11: Detection System and Patrol AI

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `UAnansiDetectionComponent` on AI enemies: detection meter, state transitions | Stealth Programmer | 20 | AI perception (Week 7) |
| Detection meter fill rate: distance, lighting, movement, cover multipliers | Stealth Programmer | 16 | Detection component |
| Detection state behaviors: Unaware, Suspicious, Searching, Alert | AI Programmer | 16 | Detection component, BT |
| `UAnansiStealthSubsystem`: zone alert level, shared knowledge | Stealth Programmer | 12 | Detection component |
| Noise system: `FAnansiNoiseEvent`, surface-based noise radius | Stealth Programmer | 12 | Traversal (surface info) |
| Patrol path system: spline-based patrol routes for guards | AI Programmer | 8 | BT patrol state |
| Guard BT: suspicious investigation, searching behavior, return-to-patrol | AI Programmer | 16 | Detection states |
| Detection UI: awareness icon (?, !) above enemy heads, detection arc | UI Programmer | 8 | Detection component |
| Crouch locomotion: reduced speed, reduced noise, lower profile | Gameplay Programmer | 8 | State machine |
| Crouch animations (idle, move, transitions) | Animator | 12 | Anim BP |
| Design stealth layout in Palace Approach (patrol routes, sight lines) | Level Designer | 16 | Patrol paths, detection |

**Week 11 Deliverable:** Guards patrol the Palace Approach on defined routes. Player can be detected through sight and sound. Detection meter fills visually. Guards transition through awareness states with appropriate behavior.

### Week 12: Stealth Actions and Social Stealth

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Silent takedown: approach from behind, prompt, animation, instant kill | Stealth Programmer | 16 | Detection system |
| Takedown motion warping: smooth snap to target position | Stealth Programmer | 8 | Motion Warping |
| Body carry: pick up, reduced speed, hide in designated spots | Stealth Programmer | 12 | Takedown |
| Body discovery: guards finding bodies enter Searching state | AI Programmer | 8 | Shared knowledge |
| Distraction: throw stone (aim arc, impact noise event) | Stealth Programmer | 12 | Noise system |
| Social stealth (simplified): walk near crowd NPC groups to blend, reduced detection | Stealth Programmer | 12 | Detection system |
| Crowd NPC placement in Market (static groups with idle anims) | Level Designer + Animator | 12 | Market blockout |
| Takedown animation (rear approach) | Animator | 8 | Takedown system |
| Body carry animation | Animator | 8 | Body carry system |
| Hiding spots (bushes, carts) placed in Palace Approach | Level Designer | 8 | Body carry |
| Stealth integration test: infiltrate Palace Approach entirely undetected | QA + Level Designer | 8 | All stealth |

**Week 12 Deliverable:** The Palace Approach can be completed entirely through stealth. Player can sneak, takedown guards silently, carry and hide bodies, distract guards with thrown stones, and blend into crowds in the Market area. Full stealth loop proven.

---

## 6. Phase 5: Abilities (Weeks 13--15)

### Week 13: Silken Path -- Web Pull

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Thread Energy attribute (5 segments) + HUD display | Gameplay Programmer + UI | 12 | GAS attributes, HUD |
| Thread Energy regeneration (passive + combat combo bonus) | Gameplay Programmer | 8 | Combo meter |
| `UAnansiAbility_WebPull` (GAS ability): target selection, pull physics | Ability Programmer | 24 | GAS setup, Silken Path |
| Web Pull targeting: lock-on priority, range check (15m), line-of-sight | Ability Programmer | 12 | Web Pull base |
| Web Pull on enemies: yank toward player, stagger on arrival | Ability Programmer | 12 | Combat system integration |
| Web Pull on objects: pull small objects toward player (crate, barrel) | Ability Programmer | 8 | Interaction system |
| Web Pull VFX: thread shoots from wrist, attaches, retracts | VFX Artist | 12 | Niagara |
| Web Pull animation (aim, fire, pull stance) | Animator | 8 | Anim BP |
| Combat integration: Web Pull into combo (pull enemy, follow with attack) | Ability Programmer | 8 | Combo system |

**Week 13 Deliverable:** Player can target and pull enemies and objects with Web Pull. Costs Thread Energy. Integrates with combat flow (pull an enemy, immediately combo).

### Week 14: Spider Sense -- Pulse Mode

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `UAnansiAbility_SpiderSensePulse` (GAS ability): pulse activation, cooldown | Ability Programmer | 12 | GAS setup |
| Pulse visual: expanding ring from player, highlights objects through geometry | VFX Artist + Ability Programmer | 20 | Niagara, post-process |
| Highlight categories: interactive objects (gold), enemies (red), collectibles (white) | Ability Programmer | 12 | Pulse system |
| Highlight rendering: custom stencil + post-process outline material | Rendering Programmer | 16 | UE5 post-process |
| Danger Sense (passive): screen-edge golden thread indicator for off-screen threats | Ability Programmer + UI | 12 | Enemy awareness |
| Tag all interactive objects in slice with highlight category | Level Designer | 8 | Highlight system |
| Spider Sense reveals climbable surfaces (shimmer effect on valid surfaces) | Ability Programmer | 8 | Climb system |
| Spider Sense reveals hidden story fragment (1 placed in Market) | Level Designer + Ability Programmer | 4 | Highlight system |

**Week 14 Deliverable:** Spider Sense pulse highlights all interactive objects, enemies, and collectibles through walls. Danger Sense passively warns of off-screen threats. Climbable surfaces shimmer in Spider Sense. One hidden story fragment discoverable only through Spider Sense.

### Week 15: Ability Integration and Combo Meter

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Combo meter implementation: hit counter, tier thresholds (5/15/30 hits) | Combat Programmer | 12 | Combat system |
| Combo tier effects: damage multiplier, Thread Energy regen bonus | Combat Programmer | 8 | Combo meter, GAS |
| Combo meter HUD element (counter + tier indicator) | UI Programmer | 8 | Combo meter |
| Combo visual flair: golden thread trails on attacks at Tier 3 | VFX Artist | 8 | Niagara, combo tier |
| Ability wheel UI: ability selection (D-pad), active ability indicator | UI Programmer | 12 | Ability system |
| Web Swing ability integration with Thread Energy (free cost after unlock) | Ability Programmer | 4 | Web swing, Thread Energy |
| Thread pickup drops from defeated enemies | Gameplay Programmer | 8 | Loot, Thread Energy |
| Full ability + combat integration test: fight encounter using all abilities | QA | 8 | All Week 13--15 |
| Execution move: available on staggered enemies <25% HP, restores Thread Energy | Combat Programmer + Animator | 12 | Stagger, Thread Energy |

**Week 15 Deliverable:** Combo meter builds during combat, granting damage multipliers and ability energy. Abilities integrate seamlessly with combat: pull enemies, sense threats, swing between encounters, execute for energy. The combat-ability feedback loop is proven.

---

## 7. Phase 6: Puzzle (Weeks 16--17)

### Week 16: Puzzle Framework and Adinkra Puzzle

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `AAnansiPuzzleElement` base class: state (locked/unlocked/solved), interaction | Puzzle Programmer | 12 | Interaction system |
| Adinkra symbol puzzle framework: rotating symbol wheels, match validation | Puzzle Programmer | 20 | Puzzle base |
| Adinkra symbol assets: 8 symbols modeled and textured (flat relief panels) | Environment Artist | 16 | None |
| Puzzle room layout: Storyteller's Chamber interior (symbol wheels on walls + floor) | Level Designer | 16 | Chamber blockout |
| Puzzle solution logic: correct symbol sequence unlocks door | Puzzle Programmer | 8 | Symbol puzzle |
| Spider Sense integration: pulse reveals hidden symbol and connections between panels | Puzzle Programmer | 8 | Spider Sense |
| Puzzle ambient: atmospheric lighting, particle dust, subtle audio cues | VFX Artist + Audio | 8 | Storyteller's Chamber |
| Puzzle feedback: correct symbol lights up, incorrect flashes red, solved plays fanfare | Puzzle Programmer + Audio | 8 | Solution logic |

**Week 16 Deliverable:** The Storyteller's Chamber contains a functional Adinkra symbol puzzle. Player rotates symbol wheels to form the correct sequence. Spider Sense reveals a hidden clue. Solving the puzzle opens a door to a story fragment.

### Week 17: Puzzle Polish and Story Fragment

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Story fragment collectible: pickup interaction, UI display, lore text | Gameplay Programmer + UI | 12 | Interaction, narrative data |
| Story fragment visual: glowing golden object with Niagara particle aura | VFX Artist | 8 | Story fragment actor |
| Puzzle hint system: after 2 minutes, optional hint text appears | Puzzle Programmer + UI | 8 | Puzzle framework |
| Second hint after 5 minutes (more detailed) | Puzzle Programmer | 4 | Hint system |
| Puzzle narrative wrapper: Anansi comments on entering and solving (VO placeholder) | Narrative Designer | 8 | Narrative hooks |
| Place 3 additional story fragments in the slice world (Market, Palace Approach, Courtyard) | Level Designer | 8 | Story fragment system |
| Puzzle QA: test all symbol combinations, edge cases | QA | 8 | Puzzle complete |

**Week 17 Deliverable:** Complete puzzle experience from entry through solution to reward. Hint system assists stuck players. 4 story fragments collectible across the slice. Anansi narration contextualizes the puzzle.

---

## 8. Phase 7: Narrative (Weeks 18--19)

### Week 18: Dialogue System

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| `UAnansiDialogueComponent`: initiate dialogue, manage flow | Narrative Programmer | 16 | Narrative subsystem |
| Dialogue UI: speaker portrait, text display, response wheel (2--4 options) | UI Programmer | 20 | CommonUI |
| `UAnansiDialogueAsset` data structure: nodes, responses, conditions, effects | Narrative Programmer | 12 | Narrative data |
| Dialogue camera: auto-framing for two-character conversations | Camera Programmer | 12 | Camera system |
| Trust meter implementation (Ama Serwaa): value storage, modification | Narrative Programmer | 8 | Narrative subsystem |
| Write dialogue scene: Ama Serwaa in Resistance Hideout (Kweku meets Ama, learns about the palace, one choice point -- trust her plan or suggest an alternative) | Narrative Designer | 16 | Dialogue asset structure |
| Dialogue tone icons: Clever/Direct/Kind/Silent icons in response wheel | UI Artist | 4 | Dialogue UI |
| Response effects: choice modifies Ama trust meter, sets narrative flag | Narrative Programmer | 4 | Trust meter, dialogue |

**Week 18 Deliverable:** Player engages in a dialogue scene with Ama Serwaa. Dialogue wheel presents options with tone icons. One meaningful choice affects Ama's trust meter. Camera frames the conversation naturally.

### Week 19: Narration and Narrative Flow

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Anansi narration system: triggered VO with subtitle display | Narrative Programmer | 12 | Audio, UI |
| Narration triggers: enter Market (intro), enter Palace Approach (tension), solve puzzle (praise/commentary) | Narrative Designer | 8 | Narration system |
| Write Anansi narration lines (4--6 lines, placeholder VO with TTS or temp actor) | Narrative Designer | 8 | Narration system |
| Narrative flow scripting: slice plays as a sequence (Market explore → Hideout dialogue → Palace infiltrate → Puzzle → Boss) with soft gating | Level Designer + Narrative | 16 | All gameplay systems |
| Choice consequence: based on Ama trust, she provides or withholds intel before boss fight (affects boss intro) | Narrative Designer | 8 | Trust meter, boss prep |
| Transition screens: brief Anansi narration during area transitions (text + VO over black) | UI Programmer + Narrative | 8 | Narration system |
| Narrative integration test: play through entire slice following narrative thread | QA | 8 | All narrative |

**Week 19 Deliverable:** The slice has a narrative through-line. Anansi narrates key moments. The Ama dialogue choice has a visible consequence before the boss fight. The experience feels authored, not disconnected.

---

## 9. Phase 8: Boss (Weeks 20--21)

### Week 20: Captain of the Mask -- Phase 1 and 2

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Boss character class: `AAnansiEnemy_Boss` with enhanced health, custom stagger | AI Programmer | 12 | Enemy base |
| Boss AI behavior tree: phase management, attack pattern selection | AI Programmer | 20 | BT system |
| Phase 1: Sword-and-shield melee. Attacks: 3-hit combo, shield bash (unparriable), overhead slam. Summons 2 guard reinforcements at 75% HP. | AI Programmer | 20 | Boss BT |
| Phase 2 (at 50% HP): Dons spirit mask. Gains binding-thread ranged attack (slows player). More aggressive. Guards stop spawning. | AI Programmer | 16 | Boss BT, GAS effects |
| Boss arena setup: Palace Courtyard with boundaries, spectator NPCs (static) | Level Designer | 12 | Courtyard blockout |
| Boss mesh + spirit mask variant | Character Artist | 16 | None |
| Boss animations: unique attack set (6 attacks), hit reactions, phase transition | Animator | 24 | Boss mesh |
| Boss intro cinematic (simple: camera pan, boss draws weapon, Anansi comment) | Level Designer + Camera | 8 | Boss arena |

**Week 20 Deliverable:** Captain of the Mask is fightable with two distinct phases. Phase 1 tests basic combat. Phase 2 introduces binding mechanic. Boss has unique animations and a brief intro.

### Week 21: Boss Polish and Death/Victory

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Boss attack telegraphs: visual wind-up cues, unparriable glow on shield bash | VFX Artist | 8 | Boss attacks |
| Binding-thread VFX: threads wrap around player during bind attack | VFX Artist | 8 | Phase 2 |
| Boss stagger opportunities: clear windows after heavy attacks | AI Programmer | 8 | Boss BT |
| Boss death sequence: defeat animation, mask cracks, collapses | Animator | 8 | Boss health |
| Post-boss narrative beat: Captain reveals coercion, player choice (placeholder -- just dialogue, no branching consequence in slice) | Narrative Designer | 8 | Dialogue system |
| Victory screen / slice-end screen: "End of Vertical Slice" with playtime, stats | UI Programmer | 8 | Boss death |
| Boss tuning: damage values, health pool, attack frequency, stagger thresholds | AI Programmer + Game Designer | 12 | Full boss playable |
| Boss QA: test all attack patterns, phase transitions, edge cases (death during transition, etc.) | QA | 8 | Boss complete |

**Week 21 Deliverable:** Complete boss encounter from intro to victory. Two phases with escalation. Post-boss narrative moment. Boss is challenging but fair, with clear tells and punish windows. Slice has an ending.

---

## 10. Phase 9: Polish (Weeks 22--24)

### Week 22: Art Pass

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Market district art: stall models, cloth canopies, ground textures, vegetation | Environment Artist (x2) | 48 | Market blockout |
| Palace Approach art: fortress walls, torches, banners, stone textures | Environment Artist | 24 | Palace blockout |
| Storyteller's Chamber art: carved walls, symbol panels, atmospheric lighting | Environment Artist | 16 | Chamber blockout |
| Palace Courtyard art: stone floor, pillars, festival decorations | Environment Artist | 16 | Courtyard blockout |
| Resistance Hideout art: underground space, crates, maps on walls, lanterns | Environment Artist | 12 | Hideout blockout |
| Lighting pass: time-of-day (dusk for the slice), interior lighting, mood | Lighting Artist | 20 | Art assets placed |
| Player character texture + material polish | Character Artist | 12 | Character mesh |
| Guard character texture + material polish | Character Artist | 12 | Guard mesh |
| Boss character texture + material polish | Character Artist | 8 | Boss mesh |
| Ama Serwaa mesh + textures (dialogue scene only, no combat) | Character Artist | 16 | None |
| Kente-cloth material: tileable material for architecture accents | Environment Artist | 8 | Material library |
| Sky/atmosphere: golden-hour skybox, volumetric clouds, haze | Environment Artist | 8 | Lighting pass |

**Week 22 Deliverable:** The slice looks representative of final quality. Market feels alive and vibrant. Palace feels oppressive. All characters have polished materials. Lighting sets mood.

### Week 23: Audio and VFX Pass

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| Wwise project setup: bus structure, states, switches | Audio Programmer | 12 | Wwise plugin |
| Player SFX: footsteps (4 surfaces), staff swings, impacts, dodges | Sound Designer | 16 | Animation notifies |
| Enemy SFX: guard footsteps, weapon attacks, armor jingles, death | Sound Designer | 12 | Guard anims |
| Boss SFX: unique attack sounds, mask power-up, binding thread | Sound Designer | 8 | Boss anims |
| Environment SFX: market ambience (crowd murmur, vendor calls), palace ambience (wind, torch crackle) | Sound Designer | 12 | Level art |
| Music: exploration theme (Market), tension theme (Palace Approach), combat theme, boss theme | Composer | 24 | Wwise music system |
| Music state transitions: exploration ↔ combat, combat → boss | Audio Programmer | 8 | Music tracks |
| Combat VFX: hit sparks, stagger burst, parry flash, dodge trail | VFX Artist | 16 | Niagara |
| Ability VFX polish: Spider Sense pulse ring, Web Pull thread, web swing line | VFX Artist | 12 | Ability systems |
| Environment VFX: torch fire, dust motes, spirit-glow on story fragments | VFX Artist | 8 | Level art |
| UI SFX: menu sounds, dialogue blip, combo counter tick, ability activation | Sound Designer | 4 | UI |

**Week 23 Deliverable:** The slice has full audio coverage: music that transitions with gameplay state, spatial SFX for all actions and ambience, and polished VFX on combat, abilities, and environment. The world has atmosphere.

### Week 24: UI, Tuning, and Final QA

| Task | Owner | Est. Hours | Dependencies |
|------|-------|------------|--------------|
| HUD art pass: replace placeholder bars with themed art (golden thread health bar, web-pattern energy) | UI Artist | 12 | HUD layout |
| Pause menu: resume, restart checkpoint, settings (volume sliders, subtitles on/off) | UI Programmer | 12 | CommonUI |
| Subtitle system: timed subtitles for all VO, speaker name, background opacity | UI Programmer | 8 | Narrative system |
| Difficulty tuning (Storyteller preset): widen parry/dodge windows, reduce guard damage, add aim assist to Web Pull | Game Designer | 8 | All combat/ability |
| Full playthrough tuning pass: adjust encounter pacing, enemy health, stamina costs, Thread Energy economy | Game Designer | 16 | All gameplay |
| Bug bash: dedicated QA pass, all team members play and log bugs | QA (all hands) | 24 | All systems |
| Critical bug fixes | All Programmers | 24 | Bug bash |
| Final performance optimization: profile, fix any frame drops below 60 FPS | Engine Programmer | 16 | All content |
| Memory audit: check for leaks, verify streaming budget | Engine Programmer | 8 | All content |
| Build final slice package (Win64 Development, PS5 if available) | Build Engineer | 8 | CI pipeline |
| Stakeholder presentation preparation: capture trailer footage, prepare talking points | Producer + Game Director | 8 | Final build |

**Week 24 Deliverable:** Polished, tuned, bug-fixed vertical slice ready for stakeholder review. Performance validated. Two difficulty settings available. Build packaged and distributable.

---

## 11. Dependency Map

```
Phase 1: Foundation ─────────────────────────────────────────────┐
  [Project Setup] ─> [Player Character] ─> [Movement] ─> [State Machine]
  [Blockout: Market] ──────────────────────────────────────────> │
  [Blockout: Palace] ──────────────────────────────────────────> │
  [Blockout: Chamber] ─────────────────────────────────────────> │
  [Blockout: Courtyard] ───────────────────────────────────────> │
  [GAS Setup] ─────────────────────────────────────────────────> │
                                                                 │
Phase 2: Combat ─────────────────────────────────┐               │
  [Combat Component] <── State Machine           │               │
  [Combo Graph] <── Combat Component             │               │
  [Hit Detection] <── Combo Graph + Anims        │               │
  [Damage Pipeline] <── Hit Detection + GAS      │               │
  [Dodge/Parry] <── State Machine + GAS          │               │
  [Guard AI] <── Damage Pipeline + BT            │               │
  [Encounters] <── Guard AI + Palace Blockout    │               │
                                                 │               │
Phase 3: Traversal ──────────────────┐           │               │
  [Vault/Slide/Ledge] <── Movement   │           │               │
  [Wall Run/Climb] <── Vault/Slide   │           │               │
  [Web Swing] <── Traversal + VFX    │           │               │
                                     │           │               │
Phase 4: Stealth ────────────┐       │           │               │
  [Detection] <── AI Percep  │       │           │               │
  [Takedowns] <── Detection  │       │           │               │
  [Social Stealth] <── Det.  │       │           │               │
                             │       │           │               │
Phase 5: Abilities ──────┐   │       │           │               │
  [Web Pull] <── GAS +   │   │       │           │               │
    Combat + Traversal    │   │       │           │               │
  [Spider Sense] <── GAS  │   │       │           │               │
  [Combo Meter] <── Combat│   │       │           │               │
                          │   │       │           │               │
Phase 6: Puzzle ─────┐    │   │       │           │               │
  [Puzzle FW] <──    │    │   │       │           │               │
    Interact + Sense │    │   │       │           │               │
  [Symbol Puzzle]    │    │   │       │           │               │
                     │    │   │       │           │               │
Phase 7: Narrative ──┤    │   │       │           │               │
  [Dialogue] <──     │    │   │       │           │               │
    Narrative Sub     │    │   │       │           │               │
  [Narration] <──    │    │   │       │           │               │
    Audio + UI       │    │   │       │           │               │
                     │    │   │       │           │               │
Phase 8: Boss ───────┤    │   │       │           │               │
  [Boss AI] <──      │    │   │       │           │               │
    Combat + GAS     │    │   │       │           │               │
  [Boss Polish]      │    │   │       │           │               │
                     │    │   │       │           │               │
Phase 9: Polish ─────┘    │   │       │           │               │
  [Art Pass] <── All Blockouts                                   │
  [Audio Pass] <── All Gameplay                                  │
  [UI Pass] <── All UI Elements                                  │
  [Tuning] <── Everything                                        │
  [QA + Ship] <── Everything                                     │
```

### Critical Path

The critical path through the slice is:

```
Project Setup → Player Character → Movement → State Machine → Combat Component
→ Combo Graph → Hit Detection → Damage Pipeline → Guard AI → Combat Encounters
→ Boss AI → Boss Polish → Tuning → QA → Ship
```

All other systems (traversal, stealth, abilities, puzzle, narrative) are parallel branches that merge into Polish. Delays in Phase 2 (Combat) directly impact the ship date. Stealth, Puzzle, and Narrative have the most schedule flexibility.

---

## 12. Risk Assessment

### High Risk

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| **Web-swing physics feels bad** | Core fantasy broken; traversal pillar fails | Medium | Prototype in Week 2 (greybox pendulum test before full implementation). Reference Spider-Man GDC talks. Budget extra tuning time. Fallback: simplified "zip-to-point" if physics swing can't be made to feel right. |
| **Combat does not feel responsive** | Core gameplay fails. Cannot ship slice. | Medium | Prioritize input latency testing from Week 5. Target <100ms input-to-first-animation-frame. Use animation cancels aggressively. Playtest weekly from Phase 2 onward with external testers. |
| **AI group combat looks dumb** | Combat encounters feel unfair or trivial | High | Attack token system is proven (used in Batman Arkham, Spider-Man). Test with 4+ enemies by Week 7. Budget Week 15 for AI tuning. Worst case: reduce encounter sizes. |
| **Scope creep in boss fight** | Boss takes more time than budgeted | High | Slice boss is explicitly 2 phases, not the full 3-phase design. Phase 2 binding mechanic must be simple (slow debuff, not complex thread physics). Cut Phase 2 entirely if behind schedule; ship a 1-phase boss. |

### Medium Risk

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| **Stealth detection feels unfair** | Players frustrated, stealth pillar untrusted | Medium | Visible detection meter is key. Tune generously for slice (err on the side of too easy). Add debug visualization for detection cones during development. |
| **Puzzle is confusing** | Players stuck, pacing ruined | Medium | Hint system is built in (Week 17). Puzzle is simple for the slice (3 symbols). Playtest with non-gamers to validate comprehension. |
| **Performance issues on PS5** | Cannot demo on target platform | Low | Profile weekly from Week 4. Slice scope is small (400m x 400m). Nanite and Lumen should handle this scale. Fallback: demo on high-end PC. |
| **Animator bottleneck** | Single animator can't deliver all anims on time | High | Prioritize: locomotion → combat → traversal → stealth → boss. Use placeholder/mocap library anims for lower-priority actions. Consider contract animator for Weeks 16--24. |

### Low Risk

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| **Wwise integration issues** | Audio delayed | Low | Wwise UE5 integration is mature. Start setup in Week 22 (not earlier). Placeholder audio via UE5 native sounds until then. |
| **Save system needed for slice** | Not needed; checkpoint restart sufficient | Very Low | Explicitly excluded from slice scope. Simple respawn-at-last-checkpoint is trivial to implement. |
| **Narrative dialogue system over-engineered** | Development time wasted | Low | Slice needs ONE dialogue scene. Build the minimum viable dialogue asset. Full editor is post-slice. |

---

## 13. Definition of Done

The vertical slice is **done** when all of the following criteria are met:

### Functional Criteria

- [ ] Player can traverse the Market area using parkour (vault, slide, ledge grab, wall run, climb) and web-swing
- [ ] Player can engage Palace Guards in melee combat using light combos (LLL), heavy combos (HH), mixed combos (LLH), dodge, and parry
- [ ] Player can use Web Pull to yank enemies and objects
- [ ] Player can use Spider Sense pulse to reveal interactive objects, enemies, and hidden elements
- [ ] Player can infiltrate the Palace Approach using stealth (crouch, silent takedowns, stone distractions, social stealth in crowds)
- [ ] Player can solve the Adinkra symbol puzzle in the Storyteller's Chamber
- [ ] Player can engage in dialogue with Ama Serwaa, making one meaningful choice
- [ ] Player can fight and defeat the Captain of the Mask boss (2 phases)
- [ ] 4 story fragments are collectible across the slice
- [ ] Anansi narrates key moments (minimum 4 narration triggers)
- [ ] The slice plays from start to finish (Market → Hideout → Palace → Puzzle → Boss → End) without crashes

### Quality Criteria

- [ ] Steady 60 FPS on PS5 / Xbox Series X / RTX 2060 PC throughout the slice
- [ ] No game-breaking bugs (crashes, soft locks, progression blockers)
- [ ] All player actions have audio feedback (footsteps, attacks, abilities, UI)
- [ ] Music transitions between exploration, stealth tension, combat, and boss states
- [ ] All environments have representative art (not greybox) with coherent lighting
- [ ] Player character, guards, and boss have polished textures and materials
- [ ] HUD displays health, stamina, Thread Energy, combo meter, detection indicators, and interaction prompts
- [ ] Subtitles display for all voiced content
- [ ] Pause menu functional (resume, restart, volume, subtitles toggle)
- [ ] Two difficulty settings available (Storyteller and Trickster)

### Experience Criteria

- [ ] A first-time player can complete the slice in 30--45 minutes on Trickster difficulty
- [ ] Combat feels responsive: input-to-visual-feedback under 100ms
- [ ] Traversal feels fluid: no stutters, catches, or misaligned animations during parkour chains
- [ ] Stealth is viable: the Palace Approach is completable without being detected
- [ ] The boss fight is beatable without taking damage by a skilled player (all attacks are telegraphed and dodgeable/parriable)
- [ ] At least 3 external playtesters complete the slice and rate the experience positively in post-play survey
- [ ] Stakeholder presentation: the slice is presentable to publishers, investors, or platform partners as evidence of the game's potential

### Documentation Criteria

- [ ] Known issues list documented (no surprises for stakeholders)
- [ ] Playtest feedback summary with action items for full production
- [ ] Performance profile report with per-system breakdown
- [ ] Asset list with final counts (meshes, textures, animations, audio events)
- [ ] Post-mortem notes: what went well, what to improve for full production pipeline

---

## Appendix A: Team Allocation for Vertical Slice

| Role | Count | Phases Active |
|------|-------|---------------|
| **Gameplay Programmer** | 2 | All phases |
| **Combat Programmer** | 1 | Phases 2, 5, 8 |
| **AI Programmer** | 1 | Phases 2, 4, 8 |
| **Traversal Programmer** | 1 | Phases 1, 3, 5 |
| **Stealth/Ability Programmer** | 1 | Phases 4, 5, 6 |
| **Narrative Programmer** | 1 | Phases 7, 6 (puzzle framework) |
| **UI Programmer** | 1 | Phases 1, 3, 5, 7, 9 |
| **Engine Programmer** | 1 | Phases 1, 9 |
| **Camera Programmer** | 0.5 | Phases 1, 2, 7 |
| **Build Engineer** | 0.5 | Phases 1, 9 |
| **Animator** | 2 | All phases |
| **Environment Artist** | 2 | Phases 1 (blockout), 9 (art pass) |
| **Character Artist** | 1 | Phases 1, 2, 7, 8, 9 |
| **VFX Artist** | 1 | Phases 3, 5, 8, 9 |
| **Lighting Artist** | 0.5 | Phase 9 |
| **Sound Designer** | 1 | Phase 9 |
| **Composer** | 0.5 (contract) | Phase 9 |
| **Level Designer** | 2 | All phases |
| **Game Designer** | 1 | All phases (tuning focus in 9) |
| **Narrative Designer** | 1 | Phases 6, 7, 8, 9 |
| **QA** | 1--2 | Phases 7--9 (ramp up), all phases (smoke testing) |
| **Producer** | 1 | All phases |

**Total: ~20 people** for the vertical slice phase.

---

## Appendix B: Weekly Milestone Summary

| Week | Milestone | Gate |
|------|-----------|------|
| 1 | Player spawns and moves in greybox | -- |
| 2 | Animated locomotion + camera | -- |
| 3 | Jump, interact, HUD, health/death | -- |
| 4 | **State machine complete. First Playable.** | Internal review |
| 5 | Basic combo attacks land on test dummy | -- |
| 6 | Dodge, parry, damage pipeline, hit-stop | -- |
| 7 | **Guard encounters playable.** | Playtest #1 |
| 8 | Parkour (vault, slide, ledge) in Market | -- |
| 9 | Wall run + climb routes | -- |
| 10 | **Web swing functional. Traversal complete.** | Playtest #2 |
| 11 | Stealth detection, patrol AI | -- |
| 12 | **Stealth loop complete. Palace infiltratable.** | Playtest #3 |
| 13 | Web Pull ability in combat | -- |
| 14 | Spider Sense pulse | -- |
| 15 | **Abilities integrated. Combat loop polished.** | Playtest #4 |
| 16 | Symbol puzzle functional | -- |
| 17 | **Puzzle complete with hints and reward.** | -- |
| 18 | Dialogue scene with Ama | -- |
| 19 | **Narrative flow connects all sections.** | Playtest #5 |
| 20 | Boss fight Phase 1 + 2 | -- |
| 21 | **Boss fight complete.** | Playtest #6 |
| 22 | Art pass on all environments + characters | -- |
| 23 | Audio + VFX pass | -- |
| 24 | **Vertical Slice DONE. Stakeholder build shipped.** | Final review + stakeholder presentation |

---

*End of Vertical Slice Plan*
