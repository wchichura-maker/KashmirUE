# Kashmir — UE1.1 Complete / UE1.2 Locomotion Baseline

Prototype milestone consolidating the validated UE1.1 gameplay foundation and
the current UE1.2 locomotion baseline. This is not a production-ready game
release.

## VALIDATED

- UE1.1 exploration, camera, Lock-On, target cycling, target-relative orbit,
  LOS policy, A/D Lock-On exit, and eight-direction Dodge foundation.
- Input reconciliation: Left Shift is Dodge; Space is unbound and reserved for
  future Jump/contextual Traversal.
- LMB+RMB forward movement uses horizontal control-camera direction through a
  single native gameplay authority path.
- Jog/Walk gait system: Jog is the default at `525`; Left Alt requests Walk at
  `215`; gameplay C++ and movement data own physical speed.
- `UKashmirAnimInstance` and `ABP_KashmirCharacter` presentation foundation,
  including braking-safe `bShouldMove` semantics.
- Direction-driven one-dimensional Jog and Walk Blend Spaces with Wrap Input at
  the `-180/+180` seam.
- In-place locomotion sequences with the current AnimBP montage-only Root
  Motion consumption policy.
- Cold `KashmirUEEditor` build: 18/18 actions, succeeded.
- Full Content Data Validation: 5,650 assets, zero errors and zero warnings.
- `Kashmir.Foundation`: 6/6 tests succeeded.
- Targeted asset verification: 10/10 checks succeeded.
- UE1.1 and Fix Pass regression: 36/36 headless PIE checks succeeded.
- Retired `IA_Sprint` reference validation: 3/3 checks succeeded.

## PENDING

- Interactive animation cadence calibration; no play rates were changed in
  this milestone.
- Start/Stop animation states.
- Turn In Place refinement.
- Foot IK.
- Final Dodge animation integration.
- Jump.
- Contextual Traversal.
- Motion Warping integration for traversal.
- Linked Anim Layers.
- Final Root Motion policy.
- Buff-driven Sprint gait.

Space remains intentionally unbound. Jump and Traversal are decided future
work and are not implemented by this release.
