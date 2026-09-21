# Kashmir Gameplay Data Schema v1

UE0.2 normaliza os schemas legados `0.1` e `1.0` em envelopes `1.0.0`.
O formato permanece engine-neutral e serve tanto à importação Unreal quanto às
fixtures de paridade.

## Envelope comum

```json
{
  "schema_version": "1.0.0",
  "id": "stable_identifier",
  "tags": ["Namespace.Semantic"],
  "data": {}
}
```

## Regras

- IDs são estáveis, não dependem de nome de asset, Actor ou Blueprint.
- Tags usam `FGameplayTag` na Unreal e mantêm namespace semântico.
- `ActionRequest` tem `action_id`, direção e intensidade não negativa.
- `CombatActionDefinition` mantém `delivery`, `target` e `effects` separados.
- Resultados são dados; aplicação de estado ocorre fora da resolução.
- Physical/ranged usa `Resource.Stamina`; magic usa `Resource.Mana`.
- Mudanças futuras exigem migração explícita, nunca alteração silenciosa.

## Arquivos UE0.2

- `Content/KashmirAct/Data/GoldenFixtures/KashmirGoldenFixtures_v1.json`
- `Docs/ASSET_MANIFEST_v1.json`

Esses arquivos são insumos de teste, não catálogos finais de produção.
