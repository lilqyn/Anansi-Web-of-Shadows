# Anansi: Web of Shadows -- Game Design Document

**Version:** 1.0  
**Date:** 2026-04-10  
**Engine:** Unreal Engine 5 (C++ / Blueprints)  
**Genre:** Third-Person Action-Adventure  
**Platforms:** PC, PlayStation 5, Xbox Series X|S  
**Target Rating:** T (Teen)  
**Development Team Size:** 40--60 core  
**Estimated Development Timeline:** 30 months (vertical slice at month 6)

---

## Table of Contents

1. [Game Overview](#1-game-overview)
2. [Story Synopsis](#2-story-synopsis)
3. [Characters](#3-characters)
4. [Core Gameplay Loop](#4-core-gameplay-loop)
5. [Combat System](#5-combat-system)
6. [Stealth System](#6-stealth-system)
7. [Traversal System](#7-traversal-system)
8. [Ability System](#8-ability-system)
9. [Puzzle Design](#9-puzzle-design)
10. [Narrative and Choice System](#10-narrative-and-choice-system)
11. [Enemy Types](#11-enemy-types)
12. [Boss Design](#12-boss-design)
13. [World Regions](#13-world-regions)
14. [Progression Systems](#14-progression-systems)
15. [HUD and UI Design](#15-hud-and-ui-design)
16. [Accessibility Features](#16-accessibility-features)

---

## 1. Game Overview

### Concept

Anansi: Web of Shadows is a third-person action-adventure game steeped in Akan folklore and set in a mythic vision of Ghana. The player controls Kweku, a young trickster apprentice who is marked by the spider spirit Anansi after the murder of the village storyteller shatters the boundary between the mortal world and the spirit realm. Kweku must journey across six distinct regions to restore balance, unravel a conspiracy led by the Masked Ruler, and decide whether to wield trickster power for liberation or domination.

### Tone and Aesthetic

- **Visual tone:** Vibrant West African color palettes layered with eerie spirit-world desaturation. Kente-cloth geometry woven into architecture, UI elements, and spell effects. Dense jungles, red-earth savannas, coastal markets, and crumbling golden palaces.
- **Narrative tone:** Oral-tradition storytelling -- the game is framed as a story being told. Humor and wit balance dark themes of oppression, identity, and the cost of power. Anansi himself is an unreliable narrator.
- **Audio tone:** Griot-inspired narration, talking-drum percussion combat music, highlife guitar exploration themes, spirit-world ambient drones.

### Pillars

1. **Trickster Fantasy** -- Every problem has a clever solution. Brute force is an option, never the only option.
2. **Living Folklore** -- The world breathes myth. Stories are literal power. Proverbs unlock doors.
3. **Consequence and Voice** -- Player choices reshape relationships, regions, and the ending.
4. **Fluid Movement** -- Traversal feels as good as combat. Moving through the world is a joy.

### Reference Games

- God of War (2018) -- weighty combat, narrative integration
- Spider-Man (Insomniac) -- traversal joy, web-swinging
- Sekiro -- parry/counter depth, posture system inspiration
- Dishonored -- stealth/creative ability use
- Hades -- narrative progression through repeated storytelling

---

## 2. Story Synopsis

### Premise

In the world of Anansi: Web of Shadows, stories are not metaphors -- they are the threads that hold reality together. Every village has a storyteller who maintains the boundary between the mortal world and Asamando (the spirit realm). When stories are lost or corrupted, the boundary frays.

### Act I -- The Torn Thread

Kweku is a 19-year-old apprentice to Nana Adwoa, the storyteller of Nkran village. One night, masked soldiers storm the village and murder Nana Adwoa mid-ritual. The half-finished story tears the boundary wide open. Spirit creatures flood into the mortal world. In the chaos, a golden spider descends from the rafters and bites Kweku on the wrist, leaving a glowing spider mark.

Anansi's voice fills Kweku's mind: "She was telling my story. Now you must finish it -- or everything unravels."

Kweku discovers the attack was ordered by the Masked Ruler, a figure who has seized control of the Asante Confederacy and is systematically killing storytellers to harvest their story-threads for a ritual that would let him control both worlds.

### Act II -- The Journey

Kweku travels across six regions, seeking surviving storytellers, gathering torn story fragments, and building alliances. Along the way he discovers:

- The Masked Ruler is **Nana Bekoe**, a former storyteller who was denied the role of Grand Storyteller and turned to forbidden spirit-binding.
- Anansi's "gift" is not altruistic -- the spider spirit is using Kweku to reassemble a story that would give Anansi himself dominion over reality.
- Ama Serwaa, Kweku's childhood friend turned resistance fighter, has her own agenda tied to her mother's disappearance.

### Act III -- The Final Thread

Kweku reaches the Golden City of Kumasi, Nana Bekoe's seat of power. He must choose:

- **The Weaver's Path** -- Complete Anansi's story, granting Anansi dominion but banishing the spirit threat permanently.
- **The Breaker's Path** -- Destroy the story entirely, severing both worlds permanently and ending all spirit magic, including Kweku's own powers.
- **The Trickster's Path** -- Rewrite the story's ending, tricking both Anansi and Nana Bekoe, preserving the boundary but at great personal cost.

The ending achieved depends on accumulated trust scores with key NPCs, story fragments collected, and critical choices made throughout the game.

---

## 3. Characters

### Kweku (Protagonist)

- **Age:** 19
- **Role:** Trickster apprentice, reluctant hero
- **Personality:** Quick-witted, empathetic, occasionally reckless. Uses humor to deflect fear. Deeply loyal but struggles with the temptation of power.
- **Visual design:** Lean build, spider-mark on left wrist that glows during ability use. Wears a storyteller's sash that accumulates collected story fragments as visible adornments. Hair styled in traditional Akan braids.
- **Combat style:** Agile, evasive. Fights with a carved wooden staff (Obi's Staff) that channels spirit energy. No shield -- relies on dodges, counters, and trickster misdirection.
- **Arc:** From frightened apprentice to confident trickster who must decide what stories are worth telling.

### Ama Serwaa

- **Age:** 20
- **Role:** Resistance fighter, companion, potential romantic interest
- **Personality:** Fierce, pragmatic, guarded. Carries anger about her mother's disappearance that sometimes clouds her judgment.
- **Visual design:** Athletic build, short-cropped hair, scarification marks on her arms (resistance symbols). Wears lightweight leather armor with kente accents.
- **Combat style:** Dual machetes, aggressive, combo-heavy. In joint encounters she fights alongside the player.
- **Arc:** Learns that revenge without understanding perpetuates cycles. Her trust meter determines whether she stands with Kweku in the finale or pursues her own path.

### Anansi

- **Role:** Mentor, narrator, trickster god, secondary antagonist
- **Personality:** Charming, manipulative, genuinely fond of Kweku but ultimately self-serving. Speaks in proverbs and riddles. His narration frames the entire game -- when he lies, the world visually glitches.
- **Manifestation:** Appears as a golden spider on Kweku's shoulder, occasionally as a shadowy human figure with eight limbs, or as a disembodied voice. Never fully trustworthy.
- **Role in gameplay:** Provides hints disguised as stories. His approval/disapproval shifts based on player choices. High Anansi trust unlocks the Weaver's Path ending and upgrades trickster abilities. Low trust weakens his gifts but opens the Breaker's Path.

### Nana Bekoe / The Masked Ruler (Antagonist)

- **Age:** 55
- **Role:** Primary antagonist, corrupted storyteller
- **Personality:** Brilliant, bitter, genuinely believes he is saving the world by controlling it. Views storytellers as irresponsible stewards of dangerous power.
- **Visual design:** Tall, gaunt figure in ornate golden robes. Wears the Mask of Ananse Ntentan -- a golden spider-web mask that channels stolen story-threads. Without the mask, his face shows the cost of spirit-binding: cracked skin leaking golden light.
- **Combat style:** Fights with story-thread constructs -- summoning mythic creatures from stolen tales, binding the player's movement, rewriting the arena mid-fight.
- **Arc:** Tragic villain. Optional story fragments reveal his backstory: a gifted boy denied his calling, watching incompetent storytellers nearly destroy the boundary through negligence. His methods are monstrous but his fear is legitimate.

### Key NPCs

| Name | Role | Region | Function |
|------|------|--------|----------|
| **Okomfo Yaw** | Priest of Nyame | Sacred Grove | Teaches spiritual mechanics, provides lore |
| **Auntie Efua** | Market queen | Festival Capital | Fence, information broker, side-quest hub |
| **Kofi Blackhand** | Blacksmith | Coastal Stronghold | Weapon upgrades, gear crafting |
| **Maame Water** | River spirit | River Delta | Ambiguous ally, puzzle gatekeeper |
| **Opanyin Mensah** | Elder storyteller | Mountain Refuge | Story fragment translator, lore deepening |
| **Akosua** | Child spirit | Spirit Crossing | Guide in spirit-world sections, emotional anchor |
| **Captain Owusu** | Deserter soldier | Festival Capital | Optional ally, military intel, combat trainer |

---

## 4. Core Gameplay Loop

### Macro Loop (Region Level)

```
Arrive in Region
    |
    v
Explore / Discover threats and allies
    |
    v
Build trust with local NPCs (dialogue, side quests)
    |
    v
Infiltrate enemy stronghold (stealth/combat choice)
    |
    v
Solve folklore puzzle to access story fragment
    |
    v
Confront region boss
    |
    v
Make narrative choice that reshapes the region
    |
    v
Unlock new ability or upgrade --> next region
```

### Micro Loop (Moment to Moment)

```
Traverse environment (parkour, swing, climb)
    |
    v
Encounter enemies or puzzle
    |
    +---> Combat: engage with melee, abilities, environment
    |
    +---> Stealth: bypass, distract, silent takedown
    |
    +---> Puzzle: observe, experiment, solve with abilities
    |
    v
Gain experience / story threads / resources
    |
    v
Upgrade abilities, gear, or unlock dialogue options
```

### Session Flow (2-Hour Play Session Target)

A typical 2-hour session should include: one traversal/exploration segment (20 min), one major combat encounter or stealth infiltration (20 min), one puzzle (15 min), one narrative scene (10 min), and free exploration/side content (remaining time). Pacing alternates between tension and release.

---

## 5. Combat System

### Philosophy

Combat should feel like a dance -- rhythmic, responsive, and rewarding mastery. Kweku is not a tank; he is fast, clever, and uses the environment. Every encounter should have multiple valid approaches.

### Controls (Default Gamepad)

| Input | Action |
|-------|--------|
| Square / X | Light attack |
| Triangle / Y | Heavy attack |
| Circle / B | Dodge (direction + input) |
| L1 / LB | Parry (timing window) |
| R1 / RB | Ability quick-use |
| R2 / RT | Ability modifier (hold + face button) |
| L2 / LT | Lock-on toggle |
| D-Pad | Ability selection |
| L3 | Sprint |

### Attack System

**Light Attacks**
- Fast, low damage, low stamina cost
- Chain up to 5 hits in a basic combo
- Purpose: building combo meter, interrupting enemies, safe poke damage

**Heavy Attacks**
- Slow, high damage, moderate stamina cost
- Can be charged (hold button) for increased damage and guard-break properties
- Purpose: finishing combos, breaking enemy guard, stagger damage

**Combo System**
- Combos are built from sequences of light (L) and heavy (H) inputs
- Base combos: LLL, LLH, LHL, HH, HLH
- Additional combos unlocked through progression
- Combo finishers trigger unique animations with bonus effects (knockback, stagger, area damage)
- **Combo Meter:** Builds with consecutive hits without taking damage. Higher combo tiers grant damage multipliers and ability energy regeneration:
  - Tier 1 (5 hits): 1.1x damage
  - Tier 2 (15 hits): 1.25x damage, +10% ability energy regen
  - Tier 3 (30 hits): 1.5x damage, +25% ability energy regen, visual flair (golden threads trail attacks)
- Meter resets on taking damage or after 4 seconds without landing a hit

### Dodge System

- **Tap dodge:** Quick sidestep, brief i-frames (0.15s), low stamina cost
- **Directional dodge:** Longer roll in input direction, extended i-frames (0.25s), moderate stamina cost
- **Perfect dodge:** Dodge within 0.2s of an incoming attack -- triggers slow-motion window (0.8s) allowing a guaranteed counter-attack. Visual cue: spider-sense threads flash gold before enemy attacks
- Dodge cancels out of any attack animation after the first 0.1s of the attack

### Parry / Counter System

- **Parry window:** 0.2s from L1/LB press
- **Successful parry:** Staggers the attacker, opens a 1.0s counter window, restores a small amount of stamina
- **Perfect parry:** Parry within the first 0.1s -- greater stagger, Kweku automatically repositions behind the enemy (spider-reflex animation), counter deals 2x damage
- **Failed parry:** Kweku takes full damage with a brief recovery stagger
- **Unparriable attacks:** Indicated by red glow. Must be dodged. Heavy enemies and bosses use these frequently.

### Stamina

- **Max stamina:** 100 (base), upgradeable to 150
- **Costs:** Light attack (5), Heavy attack (15), Charged heavy (25), Tap dodge (10), Directional dodge (20), Sprint (drains 8/s)
- **Regeneration:** 15/s when not attacking or sprinting. Pauses for 0.5s after any stamina expenditure.
- **Depleted state:** When stamina hits 0, Kweku enters a 1.5s exhaustion state (cannot dodge or attack, reduced movement speed). Punishes over-aggression.

### Damage Types

| Type | Source | Effect |
|------|--------|--------|
| **Physical** | Melee attacks, environmental | Standard damage, affected by armor |
| **Spirit** | Trickster abilities, spirit enemies | Bypasses physical armor, damages spirit bar |
| **Fire** | Environmental hazards, ritual beasts | Damage over time (3s burn), destroys web constructs |
| **Binding** | Story-thread attacks, traps | Slows movement, restricts ability use |
| **Ancestral** | Boss attacks, sacred sites | Cannot be reduced below 50% mitigation, ignores i-frames on unparriable moves |

### Enemy Interaction

- **Stagger system:** Enemies have a stagger bar beneath their health. Attacks fill it. When full, the enemy is staggered for 2s, taking 1.5x damage. Heavy attacks and parry counters fill it faster.
- **Environmental interaction:** Kick enemies into hazards, use web to pull objects onto them, trigger traps.
- **Execution moves:** Available on staggered enemies below 25% health. Cinematic finishers that restore ability energy.

---

## 6. Stealth System

### Philosophy

Stealth is always a viable path. Most combat encounters can be bypassed or trivialized through stealth. The system rewards patience and creative ability use, but does not punish players who prefer direct combat.

### Detection Model

**Detection States (Per Enemy)**

| State | Icon | Behavior |
|-------|------|----------|
| **Unaware** | None | Normal patrol, idle animations, conversations |
| **Suspicious** | Yellow ? | Heard a noise or glimpsed movement. Investigates the stimulus location. Returns to patrol after 15s. |
| **Searching** | Orange ? | Found evidence (body, broken object). Actively searches the area. Calls nearby allies. Searching lasts 30s. |
| **Alert** | Red ! | Spotted the player. Engages in combat. Alerts all enemies within earshot. |
| **Alarmed** | Red !! | Alarm raised. All enemies in the zone enter combat state. Reinforcements may spawn. |

**Detection Meter**
- Visible arc around enemy awareness icon
- Fills based on proximity, visibility, noise, and time
- Drains when stimulus is removed
- Fill rate modifiers: darkness (-50%), foliage (-40%), movement speed (crouch -30%, still -60%), height advantage (-25%)

### Alert Levels (Zone-Wide)

| Level | Trigger | Effect |
|-------|---------|--------|
| **Calm** | Default | Normal patrols, relaxed |
| **Cautious** | 2+ suspicious events | Patrol routes tighten, enemies look around more |
| **High Alert** | Player spotted once, then escaped | Patrols doubled, torches lit, some paths blocked |
| **Lockdown** | Alarm raised | All enemies searching, gates closed, reinforcements called |

Alert levels decay one step every 90s if no new events occur (except Lockdown, which requires 180s).

### Stealth Mechanics

**Movement**
- Crouch: Reduces noise, lowers profile, slower movement
- Crouch-sprint: Faster crouched movement, moderate noise
- Prone: Not supported (does not fit character fantasy)

**Cover**
- Automatic lean against walls when crouching near them
- Peek around corners (camera shift)
- No snap-to-cover system -- movement is fluid, not sticky

**Noise**
- Walking on different surfaces generates different noise radii (stone: 5m, wood: 7m, water: 10m, grass: 3m)
- Sprinting doubles noise radius
- Crouch-walking halves noise radius
- Environmental noise (waterfalls, crowds, machinery) masks player noise within their radius

**Distractions**
- Throw stones (unlimited, manual aim): Creates noise at impact point, draws one nearby enemy to investigate
- Borrowed Voice ability: Mimics a voice to lure specific enemies (see Abilities)
- Environmental: Knock over objects, release caged animals, trigger market stall collapses
- Enemies investigate distractions for 10s before returning. A second distraction at the same spot makes them suspicious.

**Silent Takedowns**
- Approach unaware enemy from behind within 2m
- Prompt appears: Hold Square/X for silent takedown (1.5s animation)
- Takedown is silent if no other enemy has line-of-sight within 15m
- Takedown varieties: rear (default), above (from ledge), below (pull off ledge), corner (around wall), web (yank with Silken Path from 8m)
- Bodies can be carried (reduced movement speed) and hidden in designated spots (bushes, carts, dark alcoves)
- Enemies who discover bodies enter Searching state

**Social Stealth**
- In populated areas (markets, festivals, palace grounds), Kweku can blend into crowds
- Blend triggers: Walk (don't run) near groups of 3+ NPCs. Kweku adopts contextual animations (browsing wares, carrying goods, chatting).
- Detection rate reduced by 80% while blending
- Broken by: sprinting, attacking, using abilities visibly, bumping into guards repeatedly
- Trick Mirror ability enhances social stealth (see Abilities)

### Stealth Rating

End-of-area stealth rating (Ghost, Shadow, Unseen, Detected) affects XP bonus but does not gate content.

---

## 7. Traversal System

### Philosophy

Movement should feel liberating and expressive. Kweku is part spider -- he clings, climbs, swings, and flows through environments in ways that feel superhuman but grounded. Traversal is not just transportation; it opens combat, stealth, and puzzle options.

### Parkour

**Auto-parkour:** When sprinting, Kweku automatically vaults low obstacles, slides under gaps, and mantles ledges. The system reads geometry ahead and selects contextual animations.

**Manual parkour moves:**

| Move | Input | Description |
|------|-------|-------------|
| Vault | Sprint + toward obstacle | Vault over waist-height objects |
| Slide | Sprint + Crouch | Slide under low gaps, momentum preserved |
| Wall run | Sprint + toward wall + Jump | Horizontal wall run for up to 3s / 8m |
| Wall jump | Jump during wall run | Launch off wall, can chain to opposite wall |
| Tic-tac | Sprint toward wall at angle | Single-step wall bounce to reach higher ledge |
| Ledge grab | Automatic on reaching ledge | Hang, shimmy, pull up, or drop |
| Beam balance | Automatic on narrow surfaces | Walk/run along beams, branches, ropes |
| Swing pole | Interact near horizontal pole | Swing and launch with momentum |

### Wall Climbing

- Kweku can climb designated climbable surfaces (rough stone, wood, woven structures, giant trees)
- Climbable surfaces have subtle visual tells: spider-silk shimmer effect visible in Spider Sense
- Climbing uses a stamina-like grip meter (separate from combat stamina): 10s of continuous climbing before grip weakens
- Grip meter recharges on flat surfaces and ledges
- Climbing speed: moderate. Can be upgraded.
- Jump-off: Press jump while climbing to leap away from the wall

### Web-Swinging (Silken Path)

- Unlocked after Act I
- Contextual swing points in the environment (tree branches, building overhangs, bridge supports, spirit anchors)
- Press and hold R1/RB while airborne near a swing point to attach a web line
- Physics-based pendulum swing: speed builds at the bottom of the arc
- Release at the optimal point for maximum launch distance/height
- Chain swings for fluid traversal across large spaces
- Web lines are visible as golden spirit threads
- Cannot swing in enclosed spaces (threads need anchor points above)

### Spirit Thread Paths

- In spirit-world areas or when using Spider Sense, hidden spirit-thread paths become visible
- These are pre-placed traversal routes through otherwise impassable terrain
- Walking on spirit threads: Kweku balances on glowing golden lines suspended in space
- Spirit threads can be temporary (disappear after use) or permanent
- Some puzzles involve creating or rerouting spirit threads

### Traversal Upgrades

| Upgrade | Effect | Unlock |
|---------|--------|--------|
| Extended Wall Run | +2s wall run duration | Story progression (Region 2) |
| Double Jump | Air jump, resets wall run | Story progression (Region 3) |
| Web Zip | Quick zip to swing point (no pendulum) | Ability upgrade |
| Thread Dash | Short air dash in any direction | Ability upgrade |
| Spider Cling | Hang from ceilings for 5s | Story progression (Region 4) |

---

## 8. Ability System

### Overview

Kweku's trickster abilities are powered by **Anansi's Thread** -- a spiritual energy resource. Thread energy is displayed as a segmented bar (5 segments, each representing one ability charge). Thread regenerates slowly over time (1 segment per 30s), faster during combat combos, and can be restored by collecting story-thread pickups from defeated enemies.

### Ability 1: Silken Path

**Description:** Kweku projects lines of spirit-silk to interact with the environment and enemies at range.

**Mechanics:**
- **Web Swing:** Attach to overhead anchor points for traversal (see Traversal System)
- **Web Pull:** Target an object or enemy and yank them toward Kweku (or Kweku toward heavy objects). Range: 15m.
- **Web Trap:** Fire a web at a surface to create a sticky trap. Enemies who walk through it are immobilized for 3s. Lasts 30s or until triggered. Max 3 active traps.
- **Web Tether:** Connect two points with a thread. Can create tripwires, bind enemies to objects, or create traversal shortcuts.

**Energy Cost:** Swing (free after unlock), Pull (0.5 segment), Trap (1 segment), Tether (1 segment)

**Upgrade Path:**
| Tier | Upgrade | Effect |
|------|---------|--------|
| 1 | Reinforced Thread | Pull works on heavier enemies, trap duration +2s |
| 2 | Thread Network | Traps chain -- triggering one triggers all within 5m |
| 3 | Spider's Reach | Pull range +10m, can pull two targets simultaneously |
| 4 | Golden Weave | Traps deal spirit damage, tethers cannot be broken by normal enemies |

### Ability 2: Borrowed Voice

**Description:** Kweku mimics any voice he has heard, projecting it to a targeted location to deceive enemies and solve puzzles.

**Mechanics:**
- **Voice Lure:** Project a heard voice (guard captain's order, animal call, civilian cry) to a targeted point within 20m. Enemies within earshot investigate.
- **Command Mimic:** At higher tiers, mimic authority figures to issue false orders to soldiers (stand down, change patrol, open gate).
- **Puzzle Interaction:** Certain doors, locks, and spirit barriers require specific voices to open. Kweku must first hear the voice (eavesdrop, find a recording) then reproduce it.
- **Conversation Infiltration:** In dialogue, unlock persuasion options by mimicking voices the NPC trusts.

**Energy Cost:** Voice Lure (0.5 segment), Command Mimic (1 segment), Puzzle Interaction (free, contextual)

**Upgrade Path:**
| Tier | Upgrade | Effect |
|------|---------|--------|
| 1 | Echo Memory | Voices are remembered permanently (no need to re-hear) |
| 2 | Chorus | Project two different voices simultaneously |
| 3 | Commanding Tone | Command Mimic works on elite enemies, longer compliance |
| 4 | Voice of the Ancestors | Mimic spirit voices, unlock ancestral dialogue options |

### Ability 3: Spider Sense

**Description:** Kweku's heightened awareness reveals hidden information in the environment.

**Mechanics:**
- **Pulse (Tap):** Brief pulse highlights interactive objects (gold), enemies (red), allies (blue), and collectibles (white) through walls for 3s. Range: 30m.
- **Sustained (Hold):** Sustained vision mode. World desaturates, all highlighted elements remain visible. Reveals hidden paths, climbable surfaces, trap locations, and enemy patrol routes as ghostly after-images. Drains energy while active.
- **Danger Sense (Passive):** Subtle visual cue (golden threads at screen edge) when an off-screen enemy is about to attack. Always active, no energy cost.
- **Story Sight:** In sustained mode, story fragments and lore objects glow with readable Adinkra symbols.

**Energy Cost:** Pulse (free, 5s cooldown), Sustained (0.5 segment per 5s), Danger Sense (free)

**Upgrade Path:**
| Tier | Upgrade | Effect |
|------|---------|--------|
| 1 | Lingering Pulse | Pulse highlights persist for 6s, range +10m |
| 2 | Thread Sight | See enemy awareness states and detection cones in sustained mode |
| 3 | Fate Threads | See interactive dialogue outcomes before choosing (brief preview) |
| 4 | Anansi's Eye | Sustained mode reveals hidden boss weak points and secret rooms |

### Ability 4: Trick Mirror

**Description:** Kweku creates illusory duplicates of himself or other characters to deceive enemies.

**Mechanics:**
- **Decoy:** Create a stationary illusion of Kweku at his current position. Kweku becomes semi-transparent for 3s. Enemies target the decoy. Decoy disperses when hit or after 8s.
- **Swap:** While decoy is active, press ability button again to instantly swap positions with the decoy. Useful for repositioning in combat and bypassing barriers.
- **Phantom Walk:** Create an illusion that walks a designated path (place start and end point). Useful for distracting patrols.
- **Mirror Mask:** Temporarily take on the appearance of a knocked-out enemy. Lasts 20s or until performing a suspicious action. Enhances social stealth.

**Energy Cost:** Decoy (1 segment), Swap (free if decoy is active), Phantom Walk (1.5 segments), Mirror Mask (2 segments)

**Upgrade Path:**
| Tier | Upgrade | Effect |
|------|---------|--------|
| 1 | Persistent Image | Decoy lasts 15s, can perform idle animations |
| 2 | Explosive Dispersal | Decoy deals spirit damage when destroyed |
| 3 | Double Trouble | Two simultaneous decoys |
| 4 | Story Mirror | Mirror Mask copies the target's voice (combines with Borrowed Voice) |

### Ability 5: Story Bind

**Description:** Kweku manipulates the narrative threads of reality, briefly rewriting the rules of an encounter.

**Mechanics:**
- **Thread Freeze:** Freeze a single enemy in narrative stasis for 4s. They are intangible and invulnerable but completely immobilized. Removes them from a fight temporarily.
- **Rewind Thread:** After taking a fatal blow, automatically rewinds 3s of time (once per encounter, passive). Kweku returns to his position 3s ago with the health he had then.
- **Story Snare:** In a targeted area (5m radius), all enemies are slowed to 50% speed for 6s. Kweku moves at normal speed.
- **Narrative Collapse (Ultimate):** Once per major encounter (boss fights, climactic battles). Kweku tears reality, creating a 10s window where the arena restructures -- platforms shift, hazards appear, enemy patterns reset. The specific changes are contextual to the encounter.

**Energy Cost:** Thread Freeze (1 segment), Rewind Thread (3 segments, auto-triggered), Story Snare (2 segments), Narrative Collapse (full bar, 5 segments)

**Upgrade Path:**
| Tier | Upgrade | Effect |
|------|---------|--------|
| 1 | Lingering Stasis | Thread Freeze lasts 6s |
| 2 | Temporal Echo | Rewind Thread also creates a decoy at the death position |
| 3 | Deep Snare | Story Snare also disables enemy abilities |
| 4 | Anansi's Loom | Narrative Collapse lasts 15s and Kweku gains 2x damage during it |

---

## 9. Puzzle Design

### Philosophy

Puzzles are woven into the world's mythology. They should feel like deciphering an ancient culture's wisdom, not arbitrary video-game locks. Every puzzle type ties to a trickster ability, encouraging players to think like Anansi.

### Puzzle Type 1: Voice Mimicry Puzzles

**Core Mechanic:** Doors, barriers, or mechanisms that respond only to specific spoken phrases or voices.

**Design:**
- Player must first find the required voice (eavesdrop on a conversation, find a spirit echo, listen to a storytelling stone)
- Navigate to the locked mechanism
- Use Borrowed Voice to reproduce the phrase
- Complexity scales: early puzzles require one voice; later puzzles require specific sequences of multiple voices, or combining voices with timing

**Example:** A spirit gate in the River Delta requires the greeting call-and-response between two river spirits. Kweku must find both spirit echoes (one upstream, one downstream) and replay them in sequence at the gate.

### Puzzle Type 2: Symbol Puzzles (Adinkra Puzzles)

**Core Mechanic:** Adinkra symbols (real Akan cultural symbols with specific meanings) are used as a puzzle language throughout the game.

**Design:**
- Rotating symbol wheels that must be aligned to form a meaningful proverb
- Symbol-matching across multiple surfaces (rotate a room to align symbols on walls and floor)
- Symbol logic puzzles: given a set of Adinkra symbols, determine the correct one to complete a sequence based on their meanings
- Spider Sense reveals hidden symbols and connections

**Example:** A tomb door displays three Adinkra symbols representing "return," "learn," and a blank. The room contains carvings telling a story. The answer is the symbol for "wisdom" (Nea Onnim) -- "He who does not know can learn; to return and fetch what you forgot is not taboo."

### Puzzle Type 3: Rhythm Puzzles

**Core Mechanic:** Drumming sequences that must be replicated or completed. Based on actual Akan talking-drum communication patterns.

**Design:**
- Listen to a rhythm pattern, then replay it using environmental drums or Kweku's staff on resonant surfaces
- Progressive: patterns get longer and incorporate rests
- Some puzzles require playing counter-rhythms (complementary patterns) rather than copies
- Tied to the game's soundtrack -- solving rhythm puzzles layers new musical elements into the area's ambient music

**Example:** A sealed chamber in the Mountain Refuge has four drums at cardinal points. A spirit drummer plays a base rhythm. Kweku must play the complementary rhythm on each drum in sequence, unlocking the chamber layer by layer.

### Puzzle Type 4: Memory / Illusion Puzzles

**Core Mechanic:** Trick Mirror and Spider Sense are used to navigate spaces where reality is unreliable.

**Design:**
- Rooms where the layout changes when Kweku looks away (turn around, the door has moved)
- Mirror puzzles: create decoys to stand on pressure plates, then swap positions to reach new areas
- Illusion mazes: Spider Sense reveals the true path through a space filled with illusory walls
- Memory rooms: enter a room, observe it, leave, then re-enter to find it changed -- must identify what changed to find the hidden element

**Example:** In the Spirit Crossing, a corridor loops infinitely. Using Spider Sense reveals that one wall is an illusion. Placing a Trick Mirror decoy at the illusion wall and then swapping to the decoy teleports Kweku through, breaking the loop.

### Puzzle Integration

- Each region introduces one puzzle type prominently, then mixes types in later regions
- Boss arenas incorporate puzzle elements (must solve mid-fight to create openings)
- Optional challenge puzzles in hidden areas reward story fragments and ability upgrades

---

## 10. Narrative and Choice System

### Structure

The narrative is framed as a story being told by Anansi. This allows for:
- Unreliable narration (Anansi embellishes, omits, or lies -- the player can catch him)
- Fourth-wall awareness (Anansi acknowledges player deaths, retries, and choices)
- Tonal flexibility (Anansi's narration shifts from humorous to dark as the story demands)

### Dialogue System

**Dialogue Wheels**
- 2--4 options per dialogue node
- Options are categorized by tone:
  - **Clever** (spider icon): Witty, indirect, manipulative. Builds Anansi trust.
  - **Direct** (fist icon): Honest, blunt, confrontational. Builds Ama Serwaa trust.
  - **Kind** (heart icon): Empathetic, generous, self-sacrificing. Builds NPC trust broadly.
  - **Silent** (mask icon): Say nothing. Sometimes the most powerful choice.
- Options show tone icon and a short preview. Full dialogue plays after selection.

**Branching Depth**
- Main story: 4 major branch points (one per act transition + finale), each with 2--3 options
- Regional stories: Each region has one critical choice affecting the region's fate
- Side quests: Binary outcomes (help or refuse, side with A or B)
- No "game over" from dialogue choices -- all paths continue, but consequences accumulate

### Trust Meters

**Key Trust Meters:**

| Character | Range | High Trust Unlock | Low Trust Consequence |
|-----------|-------|--------------------|-----------------------|
| **Anansi** | -100 to +100 | Weaver's Path ending, upgraded abilities | Abilities cost more energy, Anansi withholds hints |
| **Ama Serwaa** | -100 to +100 | Romance subplot, she fights alongside in finale | She pursues revenge alone, potential boss fight |
| **Okomfo Yaw** | 0 to 100 | Ancestral ability upgrades, lore revelations | Withholds teachings, some shrines inaccessible |
| **The People** | 0 to 100 (aggregate) | NPC assistance in finale, best Trickster Path outcome | Civilians flee from Kweku, hostile world state |

Trust is affected by: dialogue choices, quest outcomes, combat behavior (killing vs. sparing), story fragment discoveries, and ability use (some abilities frighten NPCs).

### Multiple Endings

| Ending | Requirements | Description |
|--------|-------------|-------------|
| **The Weaver's Path** | High Anansi trust (>75), complete Anansi's story | Anansi ascends. Spirits are permanently bound. Kweku becomes the new Grand Storyteller, but under Anansi's influence. Bittersweet. |
| **The Breaker's Path** | Low Anansi trust (<-25), high People trust (>50) | Kweku destroys all story-threads. Magic dies. The world is mundane but free. Kweku loses his powers and lives as an ordinary man. Peaceful but melancholy. |
| **The Trickster's Path** | Balanced Anansi trust (-25 to 75), high Ama trust (>75), 80%+ story fragments collected | Kweku rewrites the ending, tricking both Anansi and Nana Bekoe. The boundary is restored, Kweku becomes a true storyteller -- independent, balanced. Costs him his relationship with Anansi. The "golden" ending. |
| **The Broken Web** | Low trust across all meters, <40% story fragments | Kweku fails. Nana Bekoe completes his ritual. Kweku is trapped in the spirit world. Anansi begins searching for a new apprentice. Dark ending with sequel hook. |

### Story Fragments

- 120 collectible story fragments scattered across the world
- Each is a piece of the Grand Story that Nana Adwoa was telling when she was killed
- Collecting fragments: restores the story, powers up abilities, unlocks lore, shifts ending thresholds
- Fragments are found through: exploration, quest rewards, puzzle solutions, defeating optional bosses, and NPC gifts (high trust)
- At 25/50/75/100% collection, Kweku has "visions" -- playable flashback sequences revealing Nana Bekoe's backstory

---

## 11. Enemy Types

### Human Enemies

**Palace Guards**
- **Tier:** Fodder
- **Behavior:** Patrol in pairs, call for help when one is downed. Predictable attack patterns (telegraphed heavy swing, 3-hit combo). Block light attacks with shields (must be guard-broken or flanked).
- **Stealth vulnerability:** Standard detection, easily distracted, bodies cause alert.

**Masked Zealots**
- **Tier:** Standard
- **Behavior:** Nana Bekoe's elite followers. Fight with dual blades and spirit-infused attacks. Aggressive, will dodge player combos. Use binding attacks (thread-wraps that slow the player). Fight in groups of 3--5, attempt to surround.
- **Stealth vulnerability:** Heightened awareness, can detect Spider Sense use at close range, resist Borrowed Voice (zealous loyalty).

**Mercenaries**
- **Tier:** Standard/Elite (variants)
- **Behavior:** Hired muscle with varied loadouts. Spearmen maintain distance and poke. Brawlers close in with grapples. Archers hold high ground and fire at intervals. They coordinate via callouts -- eliminating the leader disrupts group tactics.
- **Stealth vulnerability:** Standard detection, leaders are high-value stealth targets (removing them degrades group AI).

### Spirit Enemies

**Whisper Spirits**
- **Tier:** Fodder/Swarm
- **Behavior:** Small, fast, translucent. Attack in swarms of 5--8. Individual damage is low. They whisper distracting phrases (minor screen effect). Weak to area attacks and Silken Path traps.
- **Special:** Cannot be stealth-killed. Always aware of Kweku's presence. However, they are non-aggressive until the player enters their territory.

**River Shades**
- **Tier:** Standard
- **Behavior:** Humanoid water spirits found near rivers and in the River Delta region. Attack with extending water-whip limbs (long range). Can submerge and reposition. Weak to spirit damage, resistant to physical.
- **Special:** When "killed," they reform after 15s unless the spawning water source is sealed (puzzle element).

**Ritual Beasts**
- **Tier:** Elite
- **Behavior:** Corrupted animal spirits (giant spiders, serpents, hyenas with too many eyes). Each type has unique mechanics:
  - **Anansi's Brood (spiders):** Web attacks, ceiling ambushes, hatch smaller spiders when staggered.
  - **Sasabonsam (bat-like):** Aerial swoops, screech attack (area stun), grab-and-carry mechanic.
  - **Adze (firefly swarm):** Split into swarm when damaged, reform after 10s, fire-element attacks.
- **Stealth vulnerability:** Cannot be stealth-killed, but can be bypassed or trapped.

**Mythic Guardians**
- **Tier:** Mini-boss
- **Behavior:** Powerful spirit creatures guarding story fragments or sacred sites. Each is unique. Examples:
  - **Stone Ancestor:** Massive stone humanoid. Slow but devastating ground-pound attacks. Must be climbed (Shadow of the Colossus style) to reach weak points.
  - **Dream Weaver:** A spider-spirit that creates illusory duplicates. Only one is real -- Spider Sense reveals it. Attacks with web-constructs.
  - **Drum Sentinel:** Animated ceremonial drum. Attacks via shockwaves timed to drum beats. Must be countered in rhythm. Rhythm puzzle and combat combined.

---

## 12. Boss Design

### Design Philosophy

Each boss should:
1. Test a mechanic introduced in that region
2. Have multiple phases with escalating complexity
3. Be susceptible to creative ability use (not just damage)
4. Tell a story through the fight itself

### Region 1: Festival Capital -- Captain of the Mask (Nana Bekoe's General)

- **Arena:** Palace courtyard during a corrupted festival
- **Phase 1:** Sword-and-shield combat. Tests parry/counter mastery. Calls palace guards for backup.
- **Phase 2:** Dons a spirit mask, gaining binding-thread attacks. Player must use dodge and Silken Path to avoid being immobilized. Guard reinforcements stop.
- **Phase 3:** Arena fills with festival fireworks (environmental hazards). Boss becomes aggressive with unparriable combos. Player must use environment (kick boss into firework launchers).
- **Story beat:** Defeated, the Captain reveals he was coerced -- his family is held hostage. Player choice: kill, spare, or recruit.

### Region 2: Sacred Grove -- The Corrupted Okomfo (False Priest)

- **Arena:** Inside a massive hollow baobab tree, spirit-world bleed
- **Phase 1:** Spirit projectile attacks from range. Player must close distance through platforming (branches, roots). Tests traversal under pressure.
- **Phase 2:** Summons spirit-animal constructs. Player fights constructs while the boss shields himself. Must use Spider Sense to identify the real boss among illusions.
- **Phase 3:** The tree itself animates. Boss merges with the tree. Player must climb the interior while avoiding root attacks, reaching the boss embedded in the canopy.
- **Story beat:** The Okomfo was not corrupted by choice -- a spirit parasite controls him. Player choice: exorcise (risky, might kill him) or sever his connection to the spirit world (safe, but he loses his abilities forever).

### Region 3: River Delta -- Maame Water (River Spirit Queen)

- **Arena:** A flooded temple, water level rises and falls across phases
- **Phase 1:** Water-whip melee attacks on dry platforms. Player must stay on platforms (water damages Kweku). Tests spacing and dodge.
- **Phase 2:** Water rises. Fight becomes aquatic traversal -- using Silken Path to swing between exposed pillars. Boss attacks from below. Tests web-swing combat.
- **Phase 3:** Maame Water reveals her true form -- massive serpentine river spirit. Player rides debris in a rapids-chase sequence, attacking when close enough. QTE-punctuated set piece.
- **Story beat:** Maame Water is Ama Serwaa's mother, transformed by Nana Bekoe as punishment. Choice affects Ama's trust dramatically.

### Region 4: Coastal Stronghold -- The Slaver King (Mercenary Lord)

- **Arena:** A fortress built into sea cliffs, multiple elevation levels
- **Phase 1:** Stealth approach -- player must infiltrate the fortress and reach the Slaver King. Being detected spawns overwhelming reinforcements. Tests stealth mastery.
- **Phase 2:** Direct combat. The Slaver King is a massive brawler with grapple attacks. Arena has destructible elements (walls, floors) that open new routes. Tests all combat skills.
- **Phase 3:** The fortress begins to collapse (he triggers demolition charges). Running fight through crumbling corridors. Tests traversal and combat simultaneously.
- **Story beat:** The Slaver King is selling captured storytellers to Nana Bekoe. Freeing them grants story fragments. Choice: destroy the fortress (kills prisoners who can't be evacuated) or leave it standing (allows future slaving operations).

### Region 5: Mountain Refuge -- The Dream Eater (Ancient Spirit)

- **Arena:** A dreamscape arena that shifts based on the fight
- **Phase 1:** Reality is normal. Dream Eater fights with nightmarish illusion attacks. Player must use Spider Sense to distinguish real attacks from illusions.
- **Phase 2:** The arena fractures into floating islands. Platforming between islands while the Dream Eater creates mirror copies of Kweku. Player must identify and attack the real Dream Eater using Trick Mirror (the real one can't be duplicated).
- **Phase 3:** Fully in the dream world. The rules change -- gravity shifts, time runs differently. Player must use Story Bind to stabilize reality while attacking during stable windows.
- **Story beat:** The Dream Eater is consuming the mountain people's dreams, keeping them docile. It was imprisoned here by a previous storyteller -- Kweku's predecessor's predecessor. Choice: re-imprison (history repeats) or destroy (mountain people wake but lose their dream-based cultural memory).

### Region 6: Golden City of Kumasi -- Nana Bekoe (The Masked Ruler)

- **Arena:** The Grand Story Chamber -- a vast room where story-threads form the walls, floor, and ceiling. The entire arena is narrative construct.
- **Phase 1:** Nana Bekoe wields stolen story-threads. He summons echoes of previously defeated bosses (each lasting 30s, weakened versions). Tests mastery of all combat skills.
- **Phase 2:** Nana Bekoe rewrites reality. The arena shifts between all six regions, each for 20s. He gains elemental attacks matching each region. Player must adapt.
- **Phase 3 (varies by ending path):**
  - *Weaver's Path:* Kweku and Anansi combine power. Player has fully upgraded abilities. Nana Bekoe becomes desperate, unleashing raw story energy. Spectacle fight.
  - *Breaker's Path:* Kweku's abilities fail mid-fight. Must defeat Nana Bekoe with basic combat and environmental interaction alone. Most mechanically challenging.
  - *Trickster's Path:* Kweku must simultaneously fight Nana Bekoe AND resist Anansi's takeover attempt (button-mashing resistance sequences interspersed with combat). Both are enemies.
- **Story beat:** Final choice triggers after Nana Bekoe is defeated. He pleads his case one last time. Then the player commits to their ending path.

---

## 13. World Regions

### Region 1: The Festival Capital -- Nkran (Accra-inspired)

- **Description:** A bustling coastal city in the midst of a grand festival (the Odwira purification ceremony, now corrupted by Nana Bekoe's influence). Colorful market districts, palace grounds, performance stages, crowded streets. The festival provides cover for both social stealth and Nana Bekoe's soldiers.
- **Tone:** Vibrant, chaotic, festive surface hiding oppression beneath
- **Mechanics introduced:** Core combat, basic stealth, social stealth, Silken Path basics
- **Enemies:** Palace guards, mercenaries
- **Collectibles:** 20 story fragments, 8 Adinkra tablets, 5 festival masks (cosmetic)
- **Key locations:** Grand Market, Palace of the Asantehene (corrupted), Festival Stage, Underground Resistance Hideout, Harbor District
- **Boss:** Captain of the Mask

### Region 2: The Sacred Grove -- Bosumtwi (Crater Lake)

- **Description:** A massive ancient forest surrounding a sacred crater lake. Enormous trees with root systems forming natural architecture. Spirit-world bleed is heavy here -- the boundary is nearly transparent. Bioluminescent flora lights the canopy at night.
- **Tone:** Mystical, reverent, unsettling
- **Mechanics introduced:** Advanced traversal (wall climbing, extended wall run), Spider Sense, Adinkra symbol puzzles
- **Enemies:** Whisper spirits, ritual beasts (spiders), corrupted priests
- **Collectibles:** 20 story fragments, 6 spirit masks, 4 ancestral memory stones
- **Key locations:** The Great Baobab, Crater Lake Shore, Okomfo's Temple, Spider Shrine, Ancestor's Walk
- **Boss:** The Corrupted Okomfo

### Region 3: The River Delta -- Volta Crossing

- **Description:** A vast river delta with flooded temples, stilted villages, and waterways choked with spirit-corrupted vegetation. Travel is primarily by boat and web-swing between elevated structures. Rain is constant.
- **Tone:** Melancholic, beautiful, dangerous
- **Mechanics introduced:** Web-swinging mastery, Borrowed Voice, voice mimicry puzzles, water traversal
- **Enemies:** River shades, whisper spirits, mercenary river pirates
- **Collectibles:** 20 story fragments, 5 river spirit offerings, 3 sunken relics
- **Key locations:** Stilted Village of Anloga, Flooded Temple of Nana Asuo, River Market (boat-based), Maame Water's Grotto, Delta Watchtower
- **Boss:** Maame Water

### Region 4: The Coastal Stronghold -- Elmina Fortress (Elmina-inspired)

- **Description:** A fortified coastal settlement built around a massive European-influenced fort repurposed by the Slaver King's mercenaries. Heavy military presence. The ocean crashes against the fortress walls. Cannons and watchtowers create dense stealth challenges.
- **Tone:** Oppressive, claustrophobic, historically resonant
- **Mechanics introduced:** Advanced stealth (infiltration, social stealth mastery), Trick Mirror, group combat tactics
- **Enemies:** Mercenaries (all variants), masked zealots, war beasts
- **Collectibles:** 20 story fragments, 4 prisoner stories (NPC rescue missions), 3 trade route maps (reveal side content)
- **Key locations:** The Fortress, Slave Dungeons (to be liberated), Merchant Quarter, Clifftop Lighthouse, Secret Cove
- **Boss:** The Slaver King

### Region 5: The Mountain Refuge -- Kwahu Plateau

- **Description:** High mountain villages connected by rope bridges and carved switchback paths. A community of refugees, artists, and the last free storytellers. Thin air, stunning vistas, and the ever-present Dream Eater's influence manifesting as a purple haze above the peaks.
- **Tone:** Serene yet anxious, a fragile sanctuary
- **Mechanics introduced:** Story Bind, rhythm puzzles, advanced illusory puzzles, full ability mastery
- **Enemies:** Dream manifestations, Sasabonsam, Nana Bekoe's advance scouts (masked zealots)
- **Collectibles:** 20 story fragments (highest density of significant fragments), 3 storyteller legacies, 5 dream echoes
- **Key locations:** Refugee Village, Storyteller's Archive, Drum Circle (rhythm puzzle hub), Dream Gate, Peak Shrine
- **Boss:** The Dream Eater

### Region 6: The Golden City -- Kumasi (Ashanti Empire capital)

- **Description:** The seat of Nana Bekoe's power. A magnificent golden city now corrupted by stolen story-threads that visibly weave through the architecture. The Grand Story Chamber sits at the city center. Spirit-world and mortal-world have nearly merged. Streets shift, buildings whisper, the sky is threaded with gold.
- **Tone:** Awe-inspiring and terrible, apocalyptic beauty
- **Mechanics introduced:** No new mechanics -- tests mastery of everything. All puzzle types, all enemy types, all abilities required.
- **Enemies:** All types, plus story-constructs (enemies made from stolen narratives)
- **Collectibles:** 20 story fragments (critical final pieces), Nana Bekoe's journal entries
- **Key locations:** Grand Story Chamber, Throne of Threads, The Unraveling (spirit bleed zone), Nana Bekoe's Private Archive, The Final Gate
- **Boss:** Nana Bekoe (The Masked Ruler)

---

## 14. Progression Systems

### Ability Upgrades

- Abilities unlock via story progression (one per region, Regions 1--5)
- Each ability has 4 upgrade tiers (see Ability System section)
- Upgrades are purchased with **Story Thread Points** earned from: story fragments, quest completion, optional bosses, puzzle solutions
- Tier costs: T1 = 3 points, T2 = 6 points, T3 = 10 points, T4 = 15 points
- Players will not have enough points to max all abilities in one playthrough (encourages replay and build choices)

### Combat Upgrades

| Category | Examples | Source |
|----------|----------|--------|
| **New combos** | Unlock LHH finisher, aerial combo string, dodge-counter extensions | Trainers (Captain Owusu) + point cost |
| **Staff upgrades** | Increased base damage, spirit damage infusion, extended reach | Kofi Blackhand (crafting) |
| **Passive skills** | Increased parry window, stamina regen boost, combo meter decay reduction | Story Thread Points |

### Gear

- No loot system. Gear is narrative -- each piece is unique, named, and tied to the story.
- **Staff skins:** Cosmetic staff appearances found as rewards (e.g., Okomfo's Bone Staff, Maame Water's Coral Staff)
- **Storyteller's Sash:** Kweku's primary "armor." Adorned with collected story fragments that visibly hang from it. More fragments = more visual flair + passive stat bonuses:
  - 25%: +10% spirit damage resistance
  - 50%: +10% ability energy regeneration
  - 75%: +15% all damage dealt
  - 100%: Anansi's True Weave -- cosmetic transformation (golden spider-thread patterns across Kweku's body)
- **Talismans:** Equippable items (2 slots) providing passive bonuses. Found through exploration and quests. Examples:
  - Talisman of the Patient Spider: Perfect parry window +0.05s
  - Talisman of the Weaver: Web abilities cost 20% less energy
  - Talisman of the Griot: Story fragments reveal their location at closer range

### Story Fragments as Power Source

Story fragments are simultaneously narrative collectibles, upgrade currency, and visual progression. This triad ensures collecting them always feels rewarding on multiple levels:

1. **Narrative:** Each fragment is a piece of the Grand Story, with readable text and Anansi's commentary
2. **Mechanical:** Converted to Story Thread Points for upgrades
3. **Visual:** Displayed on Kweku's sash, affecting his appearance
4. **Structural:** Collection percentage gates endings and unlocks vision sequences

---

## 15. HUD and UI Design

### HUD Philosophy

Minimal and diegetic where possible. The world communicates information; the HUD confirms it.

### HUD Elements

```
+-----------------------------------------------------------+
|  [Health Bar]                          [Ability Wheel]     |
|  ||||||||||||                          (radial, hidden     |
|  [Stamina Bar]                          until triggered)   |
|  ========                                                  |
|                                                            |
|                                                            |
|                                                            |
|                                                            |
|                                                            |
|                                                            |
|                                                            |
|                                                            |
|  [Combo Meter]        [Interaction    [Thread Energy]      |
|  x15 -----            Prompt]         |||||                |
|                       [Y] Examine     (5 segments)         |
+-----------------------------------------------------------+
```

**Health Bar (Top Left)**
- Golden thread that frays and darkens as health decreases
- No numerical display; segmented into roughly 4 notches for readability
- Pulses red at low health with audio cue
- Regenerates slowly out of combat (first 25% only; rest requires pickups)

**Stamina Bar (Below Health)**
- Thin white bar that depletes and refills fluidly
- Flashes red when depleted (exhaustion state)

**Thread Energy (Bottom Right)**
- 5 golden segments in a spider-web pattern
- Each segment fills independently
- Active ability icon displayed above

**Combo Meter (Bottom Left)**
- Numerical hit counter with tier indicator (I/II/III)
- Builds upward; resets with a shattering animation on hit taken

**Interaction Prompt (Bottom Center)**
- Contextual button prompts for interactions
- Disappears after 2s if not used

**Enemy Indicators (On Enemies)**
- Health bar appears above locked-on enemy
- Stagger bar beneath health bar
- Unparriable attack indicator (red glow)

### UI Screens

**Pause Menu:** Narrative-framed as "pausing the story." Anansi comments on menu access. Tabs: Resume, Abilities, Map, Inventory, Story (codex), Settings.

**Map:** Illustrated parchment-style map. Fog of war reveals as explored. Icons for: quests, collectibles (found/unfound), vendors, fast travel points (storytelling stones).

**Ability Screen:** Web-shaped skill tree. Each ability is a strand, with tiers radiating outward. Locked tiers appear as frayed threads.

**Codex (Story Tab):** Collected lore, character bios, Adinkra symbol meanings, enemy notes. Written in Anansi's voice.

---

## 16. Accessibility Features

### Vision

- **Colorblind modes:** Deuteranopia, protanopia, tritanopia filters affecting HUD, detection indicators, damage types, and environmental highlights
- **HUD scaling:** 75%--200% scaling for all HUD elements
- **Subtitle customization:** Size (small/medium/large/extra-large), background opacity, speaker name color-coding, caption toggle for ambient sounds
- **High-contrast mode:** Outlines on interactive objects, enemies, and traversal surfaces. Adjustable outline color and thickness.
- **Screen narration:** All menu text can be read aloud via TTS

### Hearing

- **Visual audio cues:** Directional indicators for important off-screen sounds (enemy footsteps, alarms, environmental hazards). These are separate from Spider Sense.
- **Rhythm puzzle accessibility:** Visual-only mode for rhythm puzzles (timing markers replace audio cues)
- **Music/SFX/Voice volume independent sliders** (standard)
- **Mono audio option**

### Motor

- **Full button remapping** on controller and keyboard
- **Toggle vs. hold options** for: sprint, crouch, aim, ability sustained mode, lock-on
- **Auto-combo mode:** Pressing attack button rhythmically auto-chains combos without requiring specific L/H sequences
- **Simplified QTE:** QTE sequences can be set to single-button press or fully automatic
- **Aim assist:** Adjustable for web-pull targeting and voice projection targeting
- **One-handed control presets** (left and right variants)

### Cognitive

- **Difficulty settings:**
  - **Storyteller (Easy):** Reduced enemy damage, wider parry/dodge windows, enhanced auto-aim, objective markers always visible, ability energy costs reduced
  - **Trickster (Normal):** Default balance
  - **Spider's Challenge (Hard):** Increased enemy aggression, narrower windows, no auto-aim, ability costs increased
  - **Anansi's Web (Very Hard):** One-hit stagger from any attack, no HUD combat aids, enemies have enhanced AI tactics
  - Individual sliders within each preset (incoming damage, parry window, puzzle hint timer)
- **Navigation assistance:** Optional waypoint guidance (golden thread on ground leading to objective)
- **Puzzle hint system:** After 2 minutes, optional hint appears. After 5 minutes, detailed solution hint. Can be set to immediate, delayed, or off.
- **Quest log clarity:** Current objective always summarized in one sentence. Full context available by expanding.
- **Content warnings:** Optional toggle for content warnings before intense narrative scenes

### Presentation

- **Camera shake:** Adjustable intensity (0--100%)
- **Motion blur:** On/Off
- **Field of view:** 60--120 degrees
- **Camera distance:** Adjustable (close/default/far)
- **Persistent dot reticle:** Optional center-screen dot for motion sickness reduction
- **Photo mode** with full camera control, filters, and character posing

---

*End of Game Design Document*
