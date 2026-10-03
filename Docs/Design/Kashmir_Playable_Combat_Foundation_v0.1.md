# Kashmir Playable Combat Foundation v0.1

**Status:** DECIDED — normative direction; implementation pending.

**Priority:** the playable combat vertical slice precedes UE2.4 Combat Progression.

## 1. Purpose

Define the minimum gameplay-presentation foundation required before Kashmir expands combat progression. Existing deterministic combat contracts remain authoritative; this document defines how they must become a coherent, responsive, readable playable experience.

## 2. Current Problem

The internal combat architecture is substantially more mature than its visible result. Reported symptoms include locomotion cadence mismatch, foot sliding, weak pelvis participation, torso/lower-body disconnect, sword/body intersections, and discontinuous attack recovery and transitions. These are problems to diagnose, not presumed technical causes.

## 3. Gameplay Feel Targets

- Direct, responsive control with low unintended dead time.
- Readable anticipation, contact, follow-through, recovery, and impact.
- Cohesive motion across weapon, hands, torso, pelvis, legs, facing, and capsule.
- Natural chaining without unrestricted cancellation.
- Compatibility with the validated UE1.1 movement/camera foundation.
- Deterministic gameplay authority independent from presentation.

## 4. Reference Games and What We Take From Each

- **Black Desert Online:** direct control, combat movement, action-camera feel, fluid chaining, and low command lock.
- **Devil May Cry:** continuity, combinability, reduced dead time, and readable individual actions inside fast sequences.
- **Black Myth: Wukong:** body weight, anticipation, weight transfer, follow-through, recovery, impact, and presentation quality.

Kashmir may eventually execute faster than Wukong, but speed must come from authored Techniques, phase timing, movement integration, transitions, chaining, and explicit cancel windows—not only higher montage PlayRate.

## 5. What Kashmir Is Not

Kashmir does not copy these games. This direction does not require extreme aerial combat, exaggerated spectacle, unrestricted cancel freedom, or character-action speed as an isolated goal.

## 6. Existing Combat Architecture To Preserve

Preserve `Technique != Animation != Skill`, generic ActionRuntime, Technique Grammar, Movement Intent, Movement Delivery, WeaponTrace, HitEvidence, CombatResult, CombatOutcome, transition grammar, request buffering, and outcome-conditioned transitions. Sword is the first playable reference, not a reason to hard-code sword semantics into shared contracts.

## 7. Gameplay vs Presentation Authority

Gameplay C++ owns permission, phase, cost, movement intent/delivery, contact resolution, damage, outcomes, and transitions. AnimBP, montages, Control Rig, IK, physics, VFX, audio, and camera present state or help acquire evidence; they do not apply damage or redefine gameplay truth. There is no “100% C++” or “100% Blueprint” goal: use the appropriate layer for authority, authoring, configuration, and iteration.

## 8. Animation Architecture Principles

- CharacterMovement owns base locomotion displacement; locomotion remains in-place unless explicitly reconsidered.
- Animation consumes gameplay state and derived presentation parameters, never raw primary-attribute rules.
- Modular boundaries should support weapon families without duplicating locomotion.
- Evaluate reusable Unreal-native facilities before character-specific fixes.
- Structural automation proves contracts; editor/PIE review proves visual quality.

## 9. Unreal-Native Technology Evaluation Policy

Kashmir creates from scratch what differentiates Kashmir. Generic problems already addressed by Unreal must first be evaluated against native systems and mature samples. Candidates include Game Animation Sample concepts, Motion Matching/Pose Search, Choosers, linked layers, montages/sections/slots, Blend Profiles, inertialization, synchronization, distance/pose warping, Control Rig, IK, specific Motion Warping uses, and GAS. Availability is not adoption: each requires evidence of fit, cost, conflict, redundancy, and benefit.

## 10. Locomotion Quality Requirements

Reduce evident foot sliding and gross stride/cadence mismatch; preserve validated Walk/Jog speeds and controls; keep feet, pelvis, direction, and capsule coherent; and transition among idle, turns/strafe, locomotion, attacks, defense, and recovery without freezes or teleports.

## 11. Combat Body Mechanics Requirements

Attacks must read as whole-body actions. Pelvis/lower-body participation must match Technique and Movement Intent; torso rotation must not appear detached; balance and planted feet must remain credible; procedural presentation must not double-transform base motion.

## 12. Weapon Trajectory Requirements

Weapon trajectory remains Technique-driven and compatible with WeaponTrace evidence. Presentation should avoid gross sword/head/body intersections, preserve stable attachment, and permit hand/weapon alignment without moving damage authority into animation.

## 13. Attack Phases

Conceptual authoring model:

`Anticipation -> Wind-up -> Attack/Acceleration -> Contact Window -> Follow-through -> Recovery`

These are not yet mandated C++ types. Current `Startup / Active / Recovery` remains implemented authority until a separately validated migration.

## 14. Attack Timing / Variable Speed Readiness

Future execution speed may alter selected phases without uniformly compressing the animation. Architecture must not permanently depend on “a Technique always lasts X absolute seconds.” Traces, contact windows, transitions, buffers, Outcome, and cancel windows must remain synchronized with actual execution when speed changes. v0.1 selects no formula, normalized-time scheme, marker scheme, curve policy, or Time Stretch Curve.

## 15. Attack-to-Attack Transition Requirements

Preserve eligibility, outcome, buffer, cancellation, and serial boundaries. A-to-B presentation should avoid hard cuts, pose discontinuity, foot jumps, duplicated movement, and stale procedural state. Faster chaining must not accidentally widen authoritative windows.

## 16. Attack-to-Locomotion Requirements

Recovery must restore the correct stationary, Walk, Jog, directional, and Lock-On presentation without blocking movement. Locomotion-to-attack must respect Movement Intent without transient lower-body contradiction.

## 17. Dodge / Defense Presentation

Validated eight-way Dodge and Lock-On rules remain unchanged. Improve readability and continuity without adding air dodge, unrestricted cancels, new invulnerability rules, or animation-owned gameplay lockouts.

## 18. Hit Reaction / Impact Presentation

Contact already flows through WeaponTrace, HitEvidence, CombatResult, and CombatOutcome. Add readable feedback driven by proven results while keeping damage/outcome authority outside presentation. Heavy reaction, hit stop, camera, VFX, and audio need bounded decisions.

## 19. Weapon-Family Modularity

Sword establishes the first quality bar. Locomotion, phase synchronization, transition, IK, and presentation interfaces must remain reusable by spear, axe, unarmed, and later families. Family kinematics belong in data/assets or bounded modules.

## 20. Vertical Slice Acceptance Criteria

In `L_CombatTest`, demonstrate:

`Idle -> locomotion -> turns/strafe -> Attack A -> Attack B -> Technique chaining -> Dodge/defense -> reposition -> attack -> actual contact -> reaction/feedback -> recovery -> locomotion`

Minimum bar: body reads as a unit; pelvis participation is appropriate; gross leg-speed discrepancy and foot sliding are reduced; weapon avoids gross head/body intersections caused by presentation; transitions do not read as hard cuts; impact is legible; control remains responsive; current deterministic/observable architecture remains intact. Final production metrics are deferred.

## 21. Explicit Non-Goals

- Combat Progression, attributes, mastery, class, affiliation, or quests.
- Mandatory Motion Matching or a universal animation rewrite.
- New weapon families, aerial combat, unrestricted cancellation, or networking.
- Traversal or Motion Warping as a general foundation.
- Undiagnosed cadence/play-rate tuning or bulk external asset import.

## 22. References / External Technical Sources

### Official engine references

- [Game Animation Sample](https://dev.epicgames.com/documentation/unreal-engine/game-animation-sample-project-in-unreal-engine)
- [Motion Matching](https://dev.epicgames.com/documentation/unreal-engine/motion-matching-in-unreal-engine)
- [Locomotion](https://dev.epicgames.com/documentation/unreal-engine/locomotion-in-unreal-engine)
- [Pose / Stride Warping](https://dev.epicgames.com/documentation/unreal-engine/pose-warping-in-unreal-engine)
- [Distance Matching](https://dev.epicgames.com/documentation/unreal-engine/distance-matching-in-unreal-engine)
- [Animation Sync Groups](https://dev.epicgames.com/documentation/unreal-engine/animation-sync-groups-in-unreal-engine)
- [Animation Blueprint Linking](https://dev.epicgames.com/documentation/unreal-engine/animation-blueprint-linking-in-unreal-engine)
- [Animation in Lyra](https://dev.epicgames.com/documentation/unreal-engine/animation-in-lyra-sample-game-in-unreal-engine)
- [Control Rig](https://dev.epicgames.com/documentation/unreal-engine/control-rig-in-unreal-engine)
- [Control Rig Full Body IK](https://dev.epicgames.com/documentation/unreal-engine/control-rig-full-body-ik-in-unreal-engine)
- [Motion Warping](https://dev.epicgames.com/documentation/unreal-engine/motion-warping-in-unreal-engine)

### Project normative documents

- `Docs/DECISIONS.md`
- `Docs/ROADMAP.md`
- `Docs/ACTION_RUNTIME_CONTRACT.md`
- `Docs/Design/Kashmir_Attributes_Foundation_v0.1.md`
- `Docs/Design/Kashmir_Progression_and_Identity_Foundation_v0.1.md`

## 23. Open Questions

- Refine the current locomotion graph, partially migrate native-sample concepts, or use a bounded hybrid?
- Which phases scale with future `AttackExecutionSpeed`, and which preserve authored readability?
- How should authoritative phase time map to presentation without making animation authoritative?
- Which IK/warping tools solve measured problems without fighting current Control Rig presentation?
- What minimum reaction, audio, VFX, camera, and hit-stop set proves readable impact?
