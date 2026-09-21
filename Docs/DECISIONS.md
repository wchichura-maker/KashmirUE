# Decisões do KashmirUE

## UE-0001 — Engine definitiva

- **Status:** confirmada.
- **Decisão:** Unreal Engine 5.8.2 é a engine do KashmirAct daqui em diante.
- **Efeito:** Godot não é mais candidato de produção. `C:\Projetos\KashmirAct`
  permanece como arquivo histórico, referência de domínio, dados e testes.

## UE-0002 — Recomeço técnico com migração semântica

- **Status:** confirmada.
- **Decisão:** o executável Unreal começa limpo, em C++ e assets Unreal nativos.
- **Migra:** decisões, contratos, regras, schemas, fixtures, testes e assets
  licenciados/importáveis.
- **Não migra diretamente:** GDScript, cenas, resources, import metadata,
  shaders Godot, IPC Python e soluções visuais provisórias.

## UE-0003 — Autoridade de gameplay

- C++ resolve `ActionRequest -> ActionRuntime -> CombatActionDefinition ->
  Delivery -> Target -> Effect -> CombatResult`.
- Blueprints, Animation Blueprints, Control Rig, física, VFX, áudio e câmera
  apresentam ou configuram; não aplicam dano nem substituem o runtime.
- GAS integra atributos, custos, tags, efeitos e tarefas, sem colapsar Delivery,
  Target e Effect em uma única habilidade.

## UE-0004 — Recursos e generalização

- Stamina: ataques físicos e à distância.
- Mana: magia.
- Player, IA, replay e rede futura emitem o mesmo `ActionRequest`.
- Toda abstração fundamental requer ao menos duas implementações em testes.
