# Kashmir Unreal Gameplay / Animation Audit v0.1

**Baseline:** `cfc8f2b8b6173339cdb386b9892841ffe9a0de9d` (`HEAD == origin/main`)

**Method:** source/config/document inspection plus existing structural-test evidence; no build, tests, editor mutation, or PIE.

**Rule:** an enabled plugin does not prove runtime use. Binary details without structural evidence remain unverified.

## Scope and Current Architecture

Inspected the project descriptor/module dependencies, Character, AnimInstance, combat presentation/runtime source, structural tests, identified assets, ROADMAP, DECISIONS, Action Runtime, migration, and design contracts. Existing dirty assets/config and parallel AI, telemetry, editor, Python, stress, and `work/` content were not modified.

- `AKashmirCharacter` composes CharacterMovement, camera/input, locomotion, Dodge/Lock-On/Jump, DirectionalSword, SwordPresentation, MovementDelivery, WeaponTrace, Combatant, and style integration.
- `UKashmirAnimInstance` derives locomotion presentation and carries the resolved sword pose through 13 transient Control Rig inputs. Its neutral-input test hook does not mutate gameplay.
- Structural tests prove the graph around `SM_Locomotion`, `DefaultSlot`, Movement Intent routing, `LayeredBoneBlend`, and `CR_KashmirSword`.
- Sword presentation plays the plan montage at `Plan.PlayRate` and optionally jumps to `MontageSection`; gameplay planning remains upstream.
- ActionRuntime, transition windows, request-buffer lifetime, trace phases, Movement Delivery, and Outcome advance using gameplay-owned seconds.

## Feature Audit

| Feature | Status | Kashmir relevance | Recommended action | Evidence |
|---|---|---:|---|---|
| Game Animation Sample concepts | NOT PRESENT | HIGH | INVESTIGATE | No migrated sample architecture/assets identified. |
| Motion Matching | NOT PRESENT | MEDIUM | PROTOTYPE | No database/schema/runtime usage found. |
| Pose Search | PRESENT BUT UNUSED | MEDIUM | INVESTIGATE | Plugin enabled in current worktree; no source/identified asset usage. |
| Choosers | NOT PRESENT | MEDIUM | DEFER | No plugin/module/source/asset evidence. |
| State Machines | ACTIVE | HIGH | KEEP | `SM_Locomotion` documented and structurally tested. |
| Blend Spaces | ACTIVE | HIGH | KEEP | Directional Walk/Jog Blend Spaces and Wrap Input baseline. |
| Linked Anim Layers | NOT PRESENT | HIGH | PROTOTYPE | ROADMAP pending; no implementation evidence. |
| Animation Layer Interfaces | NOT PRESENT | HIGH | INVESTIGATE | No class/asset/source evidence. |
| Montages | ACTIVE | HIGH | KEEP | Sword montages are referenced by Technique presentation/tests. |
| Montage Sections | PARTIALLY USED | MEDIUM | KEEP | Schema and `Montage_JumpToSection` support; asset population needs editor. |
| Slots | ACTIVE | HIGH | KEEP | Structural tests prove `DefaultSlot` and Movement Intent routes. |
| Blend Profiles | UNKNOWN — NEEDS EDITOR INSPECTION | MEDIUM | INVESTIGATE | Binary transition settings lack textual proof. |
| Inertialization | NOT PRESENT | MEDIUM | PROTOTYPE | No node/source/test evidence. |
| Sync Groups | UNKNOWN — NEEDS EDITOR INSPECTION | MEDIUM | INVESTIGATE | No textual proof; AnimBP metadata is binary. |
| Sync Markers | UNKNOWN — NEEDS EDITOR INSPECTION | MEDIUM | INVESTIGATE | No textual proof; sequence metadata is binary. |
| Distance Matching | NOT PRESENT | MEDIUM | PROTOTYPE | No source/test/identified asset evidence. |
| Stride Warping | NOT PRESENT | HIGH | PROTOTYPE | No active graph evidence; relevant to stride/slide symptoms. |
| Orientation Warping | NOT PRESENT | MEDIUM | PROTOTYPE | No active graph evidence. |
| Pose Warping | NOT PRESENT | MEDIUM | INVESTIGATE | No active graph evidence. |
| Control Rig | ACTIVE | HIGH | KEEP | `CR_KashmirSword`, direct 13-pin bridge, topology, tests. |
| Full Body IK | PRESENT BUT UNUSED | MEDIUM | INVESTIGATE | Plugin enabled; no active FBIK evidence. |
| Foot IK | NOT PRESENT | HIGH | PROTOTYPE | ROADMAP pending; no active implementation. |
| Hand/Weapon IK | PARTIALLY USED | HIGH | INVESTIGATE | Concrete partial evidence: `LeadHandOffset` and `SupportHandOffset` flow through `FKashmirSwordRigAdapter`, transient AnimInstance inputs, and the tested direct Control Rig pin mapping. This proves procedural hand/weapon alignment inputs, not a general-purpose IK solver or reusable IK contract. |
| Motion Warping | NOT PRESENT | LOW | DEFER | ROADMAP excludes it from current runtime; no current use evidence. |
| Animation Curves | UNKNOWN — NEEDS EDITOR INSPECTION | MEDIUM | INVESTIGATE | Sword curve transport was superseded; binary asset curves require editor. |
| Variable montage timing support | PARTIALLY USED | HIGH | PROTOTYPE | Montage PlayRate exists; authoritative windows remain absolute seconds. |
| GAS | PRESENT BUT UNUSED | HIGH | INVESTIGATE | Modules/plugins exist; no ASC/Ability/AttributeSet runtime use found. |
| Gameplay Attributes | NOT PRESENT | HIGH | DEFER | No AttributeSet or primary/derived system. |
| Gameplay Effects | NOT PRESENT | HIGH | DEFER | No GameplayEffect implementation; Kashmir contracts handle effects/results. |
| Gameplay Tags | ACTIVE | HIGH | KEEP | Tags drive contracts, resources, regions, effects, transitions, and tests. |

## Character, Animation, and Presentation Findings

Character is a broad composition root, but combat work is delegated to components. CharacterMovement remains locomotion authority. The current animation stack is a locomotion State Machine with directional Blend Spaces, `DefaultSlot`, Movement Intent selection between stationary layered and full-body montage routes, then sword Control Rig. Current sword trajectory combines authored montage/base motion with SwordPresentation pose profiles; contact acquisition uses weapon base/mid/tip scene points attached through the mesh/socket chain and WeaponTrace. `LeadHandOffset`/`SupportHandOffset` provide concrete procedural hand-alignment inputs, but textual evidence does not prove a conventional IK solver. Damage remains downstream of evidence resolution. Exact Blueprint defaults, montage blends/notifies, sequence curves/markers, root/pelvis behavior, and current dirty config are not published-baseline proof without editor inspection.

## Current Visual Problems Audit

| Problem | Evidence | Likely area (not proven cause) | Candidate UE tools | Needs PIE? |
|---|---|---|---|---|
| Legs visually too fast / mismatch | Directional 1D Blend Spaces; cadence remains pending. | Sample cadence, speed scaling, sample choice, stride/capsule mismatch. | Blend Spaces, Stride Warping, Distance Matching, Motion Matching evaluation. | Yes |
| Foot sliding | Stationary sword route has bounded foot-lock presentation; no general Foot IK. | Source displacement, blends, gait speed, planted-foot handling. | Foot IK/FBIK, Stride Warping, markers, Distance Matching. | Yes |
| Pelvis participation | Stationary route layers montage from `spine_01`; Control Rig adds offsets. | Layer boundary, base motion, rig distribution, Movement Intent. | Linked Layers, layer review, Control Rig/FBIK. | Yes |
| Torso/pelvis disconnect | Stationary/full-body routing is explicit; visual coherence is unproven. | Bone filters, procedural magnitude, base motion, blends. | Blend Profiles, inertialization, Control Rig. | Yes |
| Sword crosses head/body | Direct rig bridge and attachment are proven; clearance is not. | Base motion, pose profile, hand alignment, arc. | Control Rig, hand IK, authored constraints. | Yes |
| Attack recovery quality | Runtime Recovery is absolute-time; presentation stop/latch paths exist. | Animation tail, montage blend-out, latch, locomotion re-entry. | Blend Profiles, inertialization, markers. | Yes |
| Attack -> attack | Gameplay transition/buffer/outcome contracts validated; visual continuity not. | Montage restart/section, pose and phase synchronization, delivery handoff. | Sections, markers, inertialization, linked layers. | Yes |
| Attack -> locomotion | Presentation polish remains pending. | Blend-out, State Machine re-entry, gait/direction matching. | Inertialization, Blend Profiles, Pose Search evaluation. | Yes |
| Locomotion -> attack | Movement Intent selects body participation after plan resolution. | Entry pose, first-frame mismatch, foot plant, blend-in. | Blend Profiles, inertialization, Chooser/Pose Search prototype. | Yes |

## Attack-Speed Readiness Audit

| Boundary | Current evidence | Future risk/question |
|---|---|---|
| ActionRuntime | Startup/Active/Recovery and `Elapsed` are seconds. | Decide which phases scale and how authoritative phase time maps to presentation. |
| Transition rules | `MinElapsed`/`MaxElapsed` compare with runtime elapsed. | Scaling must not accidentally change eligibility or outcome reevaluation. |
| Request Buffer | Lifetime/age use DeltaSeconds; eligibility uses runtime elapsed. | Keep forgiveness distinct from speed scaling and expiry deterministic. |
| Montage | `Montage_Play(..., Plan.PlayRate)` plus optional section jump. | Presentation rate can diverge from gameplay phase time; uniform scaling is insufficient. |
| WeaponTrace | Window follows runtime phase, not animation authority. | Contact must remain aligned when selected phases change speed. |
| Movement Delivery | Advances with DeltaSeconds against action lifetime/spec. | Define distance/duration behavior under scaled execution. |
| Outcome | Finalizes from completed/interrupted/transitioned boundaries and evidence. | Preserve execution/Technique isolation across timing changes. |
| Cancel windows | Runtime phases/windows are authoritative. | Derived speed must not create implicit cancel freedom. |

No solution is selected. Later prototypes may evaluate normalized or phase-local time, curves, markers, scaled authoritative windows, or a hybrid while preserving determinism, testability, future server authority, and `Technique != Animation`.

## Contradictions and Reconciliation

- ROADMAP previously made animation polish non-blocking and placed UE2.4 immediately after UE2.3.19. UE-0035 supersedes that active priority: the playable slice comes first; UE2.4 is deferred, not cancelled.
- The progression foundation's suggested domain sequence remains useful, but its execution order is superseded where it implies immediate progression work.
- Earlier absolute-time decisions remain accurate implementation history. Variable-speed readiness is a future compatibility requirement, not a silent rewrite.
- UE-0003 describes GAS direction, while evidence shows dependencies but no implemented ASC/AttributeSet/GameplayEffect runtime. New docs say “preferred future evaluation.”
- Motion Warping remains outside current runtime and is only a specific-use candidate.
- No active normative claim was found for six attributes, Kashmir LUK, “CHA is renamed LUK,” approved Motion Matching, or “AttackSpeed equals PlayRate.”

## Dependencies, Samples, and Evidence Limits

Current worktree configuration exposes Enhanced Input, GameplayAbilities, GameplayTasks, GameplayTags, Control Rig, FullBodyIK, IKRig, PoseSearch, StateTree, and animation graph/editor dependencies. Because `.uproject` and `Build.cs` have pre-existing local changes, this proves current worktree availability—not adoption or published-baseline activation. No evidence establishes migration of Game Animation Sample or Lyra animation content. This task copied nothing.

## Recommended Vertical-Slice Audit Order

1. Capture editor/PIE evidence for cadence, stride, foot plants, pelvis, and transitions.
2. Inventory exact AnimBP transitions and sequence/montage blends, sections, notifies, curves, markers, skeleton, and retarget metadata.
3. Measure gameplay-time versus montage-time alignment for every active Technique.
4. Prototype the smallest native feature per measured problem; avoid wholesale migration.
5. Validate sword body mechanics and clearance before variable timing.
6. Establish timing synchronization, then attack transitions and locomotion continuity.
7. Add defense/dodge and hit-reaction presentation, then validate the complete `L_CombatTest` flow.

## References

Official Epic sources are centralized in the Playable Combat and Attributes foundation documents. They are evaluation references, not evidence of adoption.
