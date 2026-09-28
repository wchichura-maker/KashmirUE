# Kashmir — Progression & Identity Foundation v0.1

**Status:** DECIDED — Foundation
**Version:** 0.1
**Scope:** Global progression, identity, combat mastery, class emergence, affiliations, achievements, quests, NPCs, monsters, professions, titles/statuses, skill discovery and ultimate evolution.
**Applies to:** Player characters, NPCs, monsters and all future gameplay systems that participate in progression or identity.

---

## 1. Purpose

This document defines the foundational progression philosophy of **Kashmir**.

The core rule is:

> **The player does not choose a build. The world interprets what the character is becoming.**

Progression in Kashmir must not be reduced to a conventional sequence of:

`level up -> select class -> open skill tree -> copy build -> repeat`

Instead, the game must observe meaningful actions, victories, affiliations, achievements, relationships, discoveries and behavioral patterns, then use that history to unlock techniques, classes, titles, professions, statuses, quests and new capability domains.

This foundation is intentionally cross-system. Future systems for combat, monsters, NPCs, religions, factions, crafting, professions, quests, achievements, titles, classes, magic and world simulation must reference this document rather than invent isolated progression rules.

---

## 2. Global Design Principles

### 2.1 Identity is emergent

A character's identity is a consequence of accumulated behavior and world history.

Examples:

- a player does not simply select **Battle Cleric**;
- a player may become recognized as a Battle Cleric after developing combat ability, maintaining religious affiliation, fulfilling relevant achievements and repeatedly acting in a way compatible with that identity;
- another member of the same religion may instead become a priest, healer, inquisitor, guardian, missionary, oracle or another path according to behavior and circumstances.

Class is therefore not the origin of the character.

**Class is a recognition of what the character has become.**

### 2.2 Progression systems must interact

No future system should assume that these concepts are isolated:

- class;
- weapon mastery;
- technique;
- profession;
- religion;
- faction;
- title;
- status;
- achievement;
- quest;
- reputation;
- social relationship;
- magic affinity;
- crafting history;
- exploration;
- crime;
- leadership;
- combat behavior.

They must be able to participate in the same progression graph.

Example:

`religious affiliation + mace mastery + guard mastery + divine affinity + protection achievements + church quest`

may produce a different result from:

`religious affiliation + healing mastery + blessing mastery + doctrine achievements + social reputation`

even if both characters belong to the same church.

### 2.3 Repetition alone is not progression

Kashmir must not reward raw input repetition.

Forbidden foundation:

`use skill -> gain skill XP`

This creates macro abuse, training-dummy abuse and behavior disconnected from actual gameplay.

Using an action creates **evidence of participation**.

Progression is awarded primarily after meaningful results such as:

- enemy defeated;
- encounter completed;
- boss phase completed;
- dungeon completed;
- valid PvP result;
- quest completed;
- meaningful achievement;
- exploration discovery;
- world event participation;
- profession result;
- social or political accomplishment;
- important narrative event.

---

## 3. Progression Graph

Internally, Kashmir uses a **Progression Graph**, not a conventional visible skill tree.

The graph may contain relationships between:

`Actions -> Results -> XP -> Masteries -> Achievements -> Discoveries -> Techniques -> Classes -> Titles -> Affiliations -> Quests -> New Capability Domains`

The graph is systemic and may branch, converge, mutate or become unavailable according to world state and player history.

The player should not be shown the complete graph.

---

## 4. Player-Facing Progression

The player should primarily see what has already been discovered or what the character is beginning to understand.

Possible player-facing categories:

- Masteries;
- Techniques;
- Discoveries;
- Titles;
- Affiliations;
- Reputation;
- Known Classes;
- Professions;
- Ultimate progression;
- Known affinities;
- Known achievements.

The complete hidden requirements for undiscovered content should not be exposed.

### 4.1 No conventional complete skill tree

The player should not receive a full tree containing every future skill, class, requirement, threshold or branch.

This preserves discovery and reduces build-by-spreadsheet behavior.

Community knowledge will still emerge, so hidden progression must also avoid simple deterministic recipes whenever possible.

### 4.2 Diegetic hints are allowed

The world may hint that a path is developing without revealing the exact recipe.

Examples:

- an experienced swordsman notices the player's unusual counter technique;
- a priest comments on the player's repeated protection of allies;
- a faction leader notices the player's strategic behavior;
- a crafting master identifies an emerging specialization.

These hints may lead to quests, mentors, trials, rituals, challenges, discoveries or class recognition.

---

## 5. Experience Architecture

Kashmir separates different forms of progression.

### 5.1 Character XP

Represents broad character advancement.

Primary sources:

- meaningful enemy defeats;
- quests;
- exploration;
- achievements;
- world events;
- dungeons;
- bosses;
- valid PvP;
- major profession accomplishments;
- narrative accomplishments.

Character XP must not be granted simply for pressing combat inputs.

### 5.2 Weapon Mastery XP

Represents mastery of a weapon family or combat grammar.

Examples:

- Sword;
- Greatsword;
- Axe;
- Spear;
- Mace;
- Hammer;
- Dagger;
- Katar;
- Katana;
- Cleaver;
- Fists;
- Knuckles.

Weapon Mastery XP comes from meaningful contribution with that weapon during valid encounters.

### 5.3 Technique Mastery XP

Represents mastery of specific techniques or technique families.

A technique does not gain mastery because it was spammed.

It gains progression when its use contributed meaningfully to a valid result.

### 5.4 General Combat Aptitudes

Weapon-specific mastery is complemented by transferable combat aptitudes.

Initial conceptual aptitude families:

- Cutting;
- Thrusting;
- Impact;
- Precision;
- Guard;
- Counter;
- Mobility;
- Reach;
- Control;
- Tempo;
- Power.

A spear master should not become a sword master automatically, but mastery of thrusting, distance and precision should have some transferable value.

Final progression therefore may depend on:

`Character Aptitude + Weapon Mastery + Technique Mastery`

---

## 6. Anti-Farm Foundation

Progression value must consider context.

A conceptual model may include:

`ProgressValue = BaseReward x Challenge x Contribution x EncounterQuality x Novelty`

Exact formulas are not defined by this document.

### 6.1 Challenge

Trivial enemies should provide sharply reduced progression.

Factors may include:

- level/power difference;
- enemy tier;
- group size;
- equipment disparity;
- encounter difficulty;
- world danger.

### 6.2 Contribution

The system should measure whether an action actually contributed to the result.

Examples:

- damage;
- healing;
- protection;
- guard;
- crowd control;
- positioning;
- interruption;
- status application;
- objective interaction;
- tactical support.

Raw action count is not sufficient.

### 6.3 Repetition / novelty

Repeated farming of identical trivial content should lose progression value.

This should be implemented carefully so legitimate specialization is not punished.

The purpose is to reduce degenerate loops, not to force artificial variety.

### 6.4 Training dummies

Training dummies may be used for learning controls, testing techniques, practicing timing, testing combos, inspecting damage and comparing equipment.

Baseline:

- Character XP: 0;
- Weapon Mastery XP: 0;
- Technique evolution: 0;
- Ultimate evolution: 0.

A future limited familiarity system may be considered, but it must not substitute meaningful progression.

---

## 7. Combat Grammar per Weapon

Directional combat controlled directly by mouse movement is **SUPERSEDED**.

Kashmir retains directional attacks, but the player selects martial techniques rather than drawing weapon trajectories with the mouse.

Input flow:

`Input -> Technique Request -> Weapon Combat Style -> Technique Definition -> Action Runtime -> Authored Attack -> Physical Contact -> Hit Evidence -> Combat Resolution`

The input requests a technique.

The technique defines the physical direction and combat behavior.

### 7.1 Technique input abstraction

Code must not depend on literal keyboard keys.

Use abstract inputs such as:

- TechniqueSlot1;
- TechniqueSlot2;
- TechniqueSlot3;
- TechniqueSlot4;
- TechniqueSlot5;
- contextual modifiers where appropriate.

The player may rebind these inputs.

A slot may perform different techniques depending on equipped weapon, weapon combat style, stance, class, discovered techniques and current combat state.

### 7.2 Weapon identity

Each weapon family must have its own combat grammar and skill lineage.

| Weapon family | Primary identity |
|---|---|
| Sword | versatility, transitions, counters |
| Greatsword | reach, commitment, half-sword possibilities |
| Axe | cleave, hooks, execution |
| Spear | reach, spacing, interception |
| Mace | impact, armor trauma, guard pressure |
| Hammer | momentum, commitment, guard collapse |
| Dagger | precision, mobility, weak-point pressure |
| Katar | close thrusting, punching mechanics, counters |
| Katana | timing, draw techniques, precision, counter |
| Cleaver | chopping, close-range pressure |
| Fists | tempo, combinations, mobility |
| Knuckles | impact, body pressure, guard disruption |

These are foundation identities, not final movesets.

---

## 8. Technique Discovery

Techniques should be discovered through multiple conditions.

A discovery may depend on combinations of:

- mastery;
- achievements;
- encounter history;
- combat behavior;
- mentor interaction;
- quest state;
- class;
- affiliation;
- reputation;
- equipment;
- world discovery;
- title;
- status;
- enemy type;
- historical event.

Avoid simplistic requirements such as:

`Use Slash 100 times -> unlock Slash II`

Preferred model:

`Mastery + meaningful behavior + context + achievement -> discovery`

Multiple routes to the same discovery may exist.

---

## 9. Ultimate Lineage

Every major weapon combat grammar must have a specific **Ultimate Lineage**.

An ultimate is not one static endgame skill.

It is an ability that can evolve over the lifetime of the character.

Conceptual progression:

`Seed -> Awakened -> Mastered -> Transcendent -> Legendary/Mythic manifestation`

Names and number of stages are not final.

### 9.1 Ultimate evolution is qualitative

Ultimate progression must not be limited to more damage, shorter cooldown or larger radius.

Evolution may change:

- attack structure;
- targeting;
- timing;
- movement;
- interaction with guard;
- interaction with armor;
- interaction with multiple targets;
- status effects;
- combo behavior;
- counter behavior;
- execution behavior;
- battlefield control;
- audiovisual identity.

### 9.2 Ultimate branches by character history

Two users of the same weapon may evolve the same initial ultimate differently.

The branch may depend on mastery, achievements, class identity and legendary events.

Identical starting equipment does not imply identical endgame combat identity.

### 9.3 Ultimate XP is not granted by casting

Forbidden:

`Cast Ultimate -> Ultimate XP`

Ultimate evolution must depend on meaningful accomplishments such as dangerous bosses, difficult duels, world events, protection of allies, legendary enemies or class-defining achievements.

### 9.4 Ultimate Memory

The system should support a persistent **Ultimate Memory** or equivalent history.

It may record extraordinary moments such as:

- first legendary enemy defeated;
- dragon slain;
- impossible counter achieved;
- boss enrage interrupted;
- group protected;
- city defended;
- powerful player defeated;
- major quest resolved.

These memories may affect future ultimate evolution.

---

## 10. Achievements as Functional Progression

Achievements are not merely trophies.

They may participate directly in:

- technique discovery;
- class emergence;
- ultimate evolution;
- titles;
- statuses;
- faction progression;
- religion;
- NPC reactions;
- quests;
- professions;
- world recognition.

Achievements must be treated as gameplay state.

---

## 11. Class Emergence

Classes are not primarily selected from a menu.

Classes emerge when the Progression Graph recognizes sufficient identity evidence.

Possible evidence includes:

- combat mastery;
- magical mastery;
- religion;
- faction affiliation;
- achievements;
- quests;
- profession;
- reputation;
- titles;
- social behavior;
- leadership;
- exploration;
- crafting;
- crime;
- world events.

### 11.1 Religion example

Two characters affiliated with the same church may evolve differently.

Character A:

- high faith;
- strong mace mastery;
- high guard mastery;
- combat achievements;
- protection achievements;
- divine affinity.

Possible emergence:

`Battle Cleric`

Character B:

- high faith;
- healing mastery;
- blessing mastery;
- doctrine;
- support achievements.

Possible emergence:

`Priest`

Character C:

- high faith;
- leadership;
- doctrine;
- diplomacy;
- religious reputation.

Possible emergence:

`High Priest / Bishop / equivalent`

The religion provides context and possibility.

The player's behavior determines the realized path.

### 11.2 Classes unlock new capability domains

A class should not merely add stat bonuses.

A class may unlock entirely new technique lineages or systemic interactions.

Example after Battle Cleric emergence:

- Consecrated Strike;
- Guardian Prayer;
- Judgment Impact;
- Martyr Guard;
- Divine Counter;
- other hybrid combat/divine abilities.

### 11.3 Classes can evolve

Classes may have further emergent branches.

Example:

`Battle Cleric -> Templar / Inquisitor / Crusader / Divine Guardian / War Saint / other religion-specific identities`

This must depend on subsequent behavior, not on selecting a branch from a conventional tree.

### 11.4 Classes may be replaced, corrupted or transformed

Class identity is not necessarily permanent.

Major events may cause:

- class evolution;
- class replacement;
- class corruption;
- class loss;
- excommunication;
- conversion;
- ascension;
- hybridization;
- unique class emergence.

Previous progression may leave permanent traces where appropriate.

---

## 12. Affiliations, Religion and Factions

Joining an organization must not automatically grant a class.

Affiliation creates:

- context;
- reputation;
- quests;
- mentors;
- techniques;
- equipment opportunities;
- social consequences;
- possible class paths;
- possible achievements;
- possible enemies.

The realized identity still depends on player history.

The same principle applies to churches, guilds, nations, criminal organizations, military orders, academies, crafting organizations and secret societies.

---

## 13. Titles and Statuses

Titles and statuses must represent meaningful state.

They may affect:

- NPC dialogue;
- access;
- prices;
- reputation;
- quests;
- faction reactions;
- intimidation;
- trust;
- religion;
- political standing;
- technique discovery;
- class emergence.

Examples include Dragon Slayer, Heretic, Oathbreaker, Protector, Master Smith, Royal Duelist, Excommunicated, Saint, Wanted and Champion.

These should not exist only for cosmetic display.

---

## 14. Quests

Quests are part of the Progression Graph.

A quest may:

- test an emerging identity;
- confirm a class;
- unlock a technique;
- provide a mentor;
- evolve an ultimate;
- change affiliation;
- change religion;
- grant or remove status;
- generate an achievement;
- transform reputation;
- reveal hidden progression possibilities.

Quests should be able to react to the player's actual history.

---

## 15. NPCs

NPC systems must be designed to read relevant identity state.

NPC behavior may consider:

- class;
- reputation;
- religion;
- faction;
- achievements;
- titles;
- profession;
- crimes;
- relationships;
- combat reputation;
- crafting reputation;
- world events;
- prior quests;
- unique discoveries.

NPCs may also recognize emerging paths, offer specialized mentorship, challenge the player, refuse service, generate quests, alter dialogue, become rivals, become followers or trigger class/technique discoveries.

NPCs should participate in progression, not merely deliver static quests.

---

## 16. Monsters and Enemies

Enemy design must support progression quality.

Each enemy may provide data relevant to:

- threat tier;
- combat style;
- resistances;
- weaknesses;
- intelligence;
- rarity;
- faction;
- ecosystem role;
- boss status;
- unique achievements;
- mastery opportunities;
- progression value.

Defeating different enemy behaviors should be able to reward different mastery evidence.

---

## 17. Professions and Crafting

The same philosophy applies outside combat.

A profession should not be merely:

`craft item -> gain profession XP`

Meaningful crafting progression may depend on:

- item quality;
- difficulty;
- material rarity;
- experimentation;
- commissioned work;
- unique discoveries;
- economic relevance;
- masterwork achievements;
- repair/restoration challenges;
- NPC mentorship;
- crafted item performance in the world.

Profession identities may also emerge, such as Swordsmith, Armorsmith, Runecrafter, Alchemist, Siege Engineer, Royal Artisan or Legendary Craftsman.

Crafting, combat and class systems must be able to intersect.

---

## 18. Hidden and Unique Paths

Kashmir should support hidden progression paths.

Not all content must be visible from the beginning.

Some classes, techniques, titles or ultimate evolutions may depend on rare combinations of:

- world events;
- achievements;
- behavior;
- affiliations;
- unique NPC relationships;
- exploration;
- combat mastery;
- crafting mastery;
- historical state.

Some discoveries may be rare or unique without requiring arbitrary randomness.

The system should favor **causal rarity** over purely random rarity.

---

## 19. World Recognition

A major progression event should feel like recognition by the world.

Examples:

- a class is recognized;
- a religion acknowledges the player;
- a weapon technique becomes legendary;
- an NPC names a new style;
- a title spreads through a region;
- an ultimate evolves after a historic event.

Progression should produce world consequences wherever feasible.

---

## 20. Architecture Principles

The implementation must preserve the existing Kashmir engineering principles:

- modular composition;
- reusable state machines;
- data-driven behavior;
- contracts/interfaces;
- low coupling;
- minimal hardcoding;
- dependency inversion where appropriate;
- future server-authoritative compatibility;
- deterministic tests;
- reusable content definitions;
- no character-specific logic in core systems.

### 20.1 Suggested core abstractions

Future implementation may introduce concepts such as:

- `UCombatTechniqueDefinition`
- `UWeaponCombatStyle`
- `FTechniqueSlot`
- `FMasteryChannel`
- `FProgressionEvent`
- `FProgressionEvidence`
- `FProgressionReward`
- `FDiscoveryCondition`
- `FDiscoveryResult`
- `FClassEmergenceCondition`
- `FClassIdentityState`
- `FAchievementDefinition`
- `FAffiliationState`
- `FUltimateLineage`
- `FUltimateMemory`

Exact names are not yet implementation commitments.

### 20.2 Data, not subclass explosion

Avoid architectures such as SwordComponent, AxeComponent, SpearComponent, HammerComponent when the distinction can be expressed through reusable data and composition.

Weapon identity should primarily come from:

`Combat Style + Technique Definitions + Mastery Channels + Transition Rules + Resource Rules + Animation/Contact Profiles`

---

## 21. Relationship to Existing Combat Architecture

Existing authoritative combat flow remains valid:

`ActionRequest -> ActionRuntime -> CombatActionDefinition -> Delivery -> Target -> Effect -> CombatResult`

Weapon combat grammar should integrate above and around this flow.

A technique request should eventually produce an authoritative action request.

Animation must not become damage authority.

Physics and traces produce evidence.

Combat resolution decides the result.

Persistent state application remains separate.

---

## 22. Superseded Decision

The previous concept of controlling sword attack direction by dragging the mouse is:

**SUPERSEDED**

Reason:

- conflicts with camera control;
- increases input fatigue;
- complicates lock-on;
- complicates authored animation quality;
- creates unnecessary continuous-motion interpretation;
- scales poorly across many weapon families.

Preserved from the previous idea:

- attacks remain directional;
- contact velocity remains relevant;
- attack direction remains relevant;
- parry/guard/clash may use direction;
- authored weapon trajectory remains physically meaningful;
- attack direction may still influence hit evidence and resolution.

The direction comes from the selected technique and performed motion, not from raw mouse drawing.

---

## 23. Current Decisions

The following are **DECIDED**:

1. Progression is identity-driven and emergent.
2. The player does not select a conventional complete build.
3. The complete progression graph is hidden.
4. Raw action repetition does not grant meaningful mastery.
5. Meaningful encounter results are the main source of combat progression.
6. Character XP, weapon mastery, technique mastery and general aptitudes are distinct concepts.
7. Anti-farm rules must account for threat, contribution and repetitive trivial content.
8. Training dummies do not provide normal progression.
9. Directional attacks remain, but mouse-drawn attack direction is superseded.
10. Each weapon family has its own combat grammar.
11. Weapon input uses abstract technique slots, not hardcoded keyboard keys.
12. Techniques may be discovered through multi-condition progression.
13. Achievements are functional progression state.
14. Classes emerge from character history.
15. Affiliations create possibilities but do not automatically grant classes.
16. Classes may unlock new capability lineages.
17. Classes may evolve, transform or be replaced.
18. Every major weapon family has an evolving Ultimate Lineage.
19. Ultimate progression depends on meaningful achievements, not cast count.
20. Ultimate Memory or equivalent persistent legendary history should be supported.
21. Titles and statuses may affect gameplay and world recognition.
22. Quests, NPCs, monsters, crafting and professions must integrate with the same progression foundation.
23. Future systems must not reinvent isolated progression rules without referencing this foundation.

---

## 24. Pending Design Decisions

The following remain **PENDING**:

- exact XP formulas;
- exact anti-farm decay model;
- exact mastery scale;
- exact terminology shown to players;
- maximum number of active/equipped techniques;
- whether some discoveries are account-wide;
- exact class rarity model;
- whether some classes can be world-unique;
- exact ultimate stage count;
- ultimate resource/cooldown architecture;
- exact achievement taxonomy;
- exact profession progression rules;
- exact NPC recognition implementation;
- exact hidden-path discoverability rules;
- exact transfer coefficients between general aptitudes and weapon mastery;
- exact PvP progression safeguards;
- exact party contribution distribution.

These must be validated through prototypes before becoming final rules.

---

## 25. Roadmap Impact

The previous directional-sword roadmap should be replaced by a broader **Weapon Combat Grammar & Progression** phase.

Suggested future sequence:

1. Technique Definition
2. Weapon Combat Style
3. Technique Input Slots
4. Technique Transition / Combo Graph
5. Mastery Channels
6. Progression Evidence
7. Progression Reward Resolution
8. Discovery Conditions
9. Achievement Integration
10. Ultimate Lineage Foundation
11. Sword reference implementation
12. Contrasting second weapon implementation
13. Cross-weapon aptitude transfer
14. Class Emergence hooks
15. Affiliation hooks
16. NPC / Quest integration
17. Extended weapon families

At least two sufficiently different weapon families must validate any generic combat-grammar architecture before it is considered proven.

---

## 26. Foundation Rule for Future Documents

Every future design document involving any of the following must explicitly reference this foundation:

- monsters;
- NPCs;
- classes;
- professions;
- statuses;
- titles;
- abilities;
- magic;
- weapons;
- quests;
- achievements;
- religions;
- factions;
- reputation;
- crafting;
- exploration;
- world events;
- bosses;
- PvP;
- progression.

If a future proposal conflicts with this foundation, the conflict must be explicitly marked and resolved through a new decision rather than silently overriding this document.

---

## Final Design Statement

> **Kashmir progression is the history of a character becoming something. Actions create evidence, meaningful outcomes create progression, the world recognizes patterns, and that recognition opens new possibilities. The player should discover an identity through play rather than select a finished build in advance.**
