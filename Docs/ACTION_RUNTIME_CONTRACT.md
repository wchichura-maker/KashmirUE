# UE0.3 — Action Runtime Contract

The action runtime is deterministic game logic. Animation, montage notifies, hitboxes, physics, AI, input and visual presentation consume it; they do not advance or alter the authoritative phase.

An action has startup, active and recovery durations. A valid request pays its whole start cost atomically before startup. Costs never refund on cancellation or transition. `Resource.Stamina` and `Resource.Mana` use the same generic transaction contract.

Cancellation requires an explicit cancel window. A replacement action pays first; a failed replacement leaves the source untouched. Transitions are data rules filtered by source action, time window, required tags, blocked tags and priority. Same-priority ties use action ID deterministically. UE0.3 uses tag predicates only; richer combat predicates remain later-layer work.

Runtime events have deterministic local IDs and are observation-only.
