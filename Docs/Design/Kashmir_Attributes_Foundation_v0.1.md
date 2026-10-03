# Kashmir Attributes Foundation v0.1

**Status:** DECIDED design direction; NOT IMPLEMENTED.

## Purpose and Structural Model

Define attribute identity without selecting formulas, values, storage, replication, or implementation. The normative boundary is:

`Primary Attributes -> Derived Parameters -> Gameplay Systems`

A primary attribute may feed many derived parameters. A derived parameter may combine attributes, equipment, weapon, buffs/debuffs, Character State, Technique rules, status, and future systems. Animation must consume suitable derived parameters, not primary-attribute rules.

## Seven Primary Attributes

| Code | Name | v0.1 identity |
|---|---|---|
| STR | Strength / Força | Physical force, power, impact, future weapon/requirement/load interaction. |
| AGI | Agility / Agilidade | Bodily quickness, responsiveness, evasion, mobility/recovery influence, future attack execution speed. |
| VIT | Vitality / Vitalidade | Endurance, health, physical resistance, robustness, physical recovery. |
| INT | Intelligence / Inteligência | Knowledge and intellectual/technical arcane power and control. |
| DEX | Dexterity / Destreza | Precision, fine coordination, Technique, Accuracy/Hit, technical/crafting possibilities, future attack execution speed. |
| WIS | Wisdom / Sabedoria | Perception, intuition, spiritual/divine power, support, healing, future spiritual resistance. |
| CHA | Charisma / Carisma | Presence, social influence, persuasion, deception/bluff, situational intimidation, leadership, and selected presence/conviction-based magic or attacks. |

`CHA` is not renamed `LUK`; Kashmir has no decision to retain `LUK` as a hidden primary attribute.

## Attack Execution Speed Direction

`AttackExecutionSpeed` is a provisional documentation name, not a property or formula. AGI and DEX will participate with equipment, weapon, buffs/debuffs, Character State, and Technique rules:

`Primary Attributes + Equipment + Weapon + Modifiers + State + Technique Rules -> Derived Combat Parameters -> Technique Execution -> Animation Presentation`

This must not become “AGI equals montage PlayRate.” It may affect selected execution phases, authoritative windows, and presentation differently. AnimBP/Montage must never need a rule such as `AGI = 70`.

## References and Boundaries

### Ragnarok Online — community game reference

[iRO Wiki: Stats](https://irowiki.org/wiki/Stats) is a consolidated community reference, not Unreal or Kashmir authority. Kashmir takes structural inspiration from multi-effect primaries, multi-source derived stats, equipment/skill/buff modifiers, and meaningful build distribution. Ragnarok's ASPD illustrates that speed need not depend on AGI alone and may include DEX, weapon/class, and modifiers. Kashmir copies no formulas or values.

### D&D — secondary conceptual gameplay reference

D&D informs only the separation of INT, WIS, and CHA: WIS for perception/intuition and spiritual/support archetypes; CHA for presence, social interaction, persuasion, deception/bluff, and selected presence-based offensive/magical archetypes. Kashmir adopts no D&D rules.

## Preferred Future Framework Evaluation

GAS is the preferred framework to evaluate for primary/derived attributes, resources, buffs, debuffs, status, costs, temporary/permanent modifiers, Gameplay Tags, and applicable future replication/prediction. It is not implemented and is not approval to replace Kashmir Technique, ActionRuntime, HitEvidence, CombatResult, CombatOutcome, or transition semantics.

`GAS / Character State -> Derived Combat Parameters -> Kashmir Technique / Combat Architecture`

## Explicit Non-Decisions

No formulas, values, caps, growth, point cost, class modifier, respec, AttributeSet, ASC, GameplayEffect, save/UI/network implementation, final speed-property name, or timing-scaling solution is selected. No AGI/DEX logic belongs in AnimBP, montage, or Control Rig in this phase.

## Compatibility Requirements

Preserve deterministic ActionRuntime authority, resource transactions, Technique/animation separation, evidence-based resolution, Movement Delivery, request buffering, and outcome-conditioned transitions. Animation timing must be ready for variable execution before attribute-driven speed is enabled.

## References / External Technical Sources

### Official engine references

- [Gameplay Ability System](https://dev.epicgames.com/documentation/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system)
- [Gameplay Attributes and Attribute Sets](https://dev.epicgames.com/documentation/unreal-engine/gameplay-attributes-and-attribute-sets-for-the-gameplay-ability-system-in-unreal-engine)
- [Gameplay Effects](https://dev.epicgames.com/documentation/unreal-engine/gameplay-effects-for-the-gameplay-ability-system-in-unreal-engine)

### Community game reference

- [iRO Wiki: Stats](https://irowiki.org/wiki/Stats)

### Project normative documents

- `Docs/Design/Kashmir_Playable_Combat_Foundation_v0.1.md`
- `Docs/Design/Kashmir_Progression_and_Identity_Foundation_v0.1.md`
- `Docs/DECISIONS.md`
- `Docs/ACTION_RUNTIME_CONTRACT.md`
- `Docs/MIGRATION_FOUNDATION.md`
