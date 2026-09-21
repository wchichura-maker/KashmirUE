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

## UE-0005 — Câmera, facing e referencial de movimento

- **Status:** confirmada.
- **Escopo:** UE1.1 — personagem canônico.
- A direção da câmera, a direção do corpo e a direção de movimento são conceitos independentes.
- O modo padrão de exploração segue controle inspirado em MMO/WoW:
  - `W` move para a frente relativa ao facing atual do personagem;
  - `S` executa backpedal sem inverter o facing;
  - `A/D` executam strafe lateral sem girar automaticamente o corpo;
  - orbitar a câmera não altera o facing por si só.
- Segurar RMB permite que o Yaw do corpo acompanhe o Yaw da câmera.
- Soltar RMB devolve independência entre câmera e facing.
- LMB mantém órbita livre da câmera.
- LMB + RMB solicita movimento contínuo para a frente.
- A câmera third-person é centralizada no modo normal; câmera sobre o ombro pode existir futuramente como estado contextual de câmera, não como regra global.
- O `CharacterMovement` não deve orientar automaticamente o personagem para a direção do movimento no modo padrão.

## UE-0006 — Política de câmera UE1.1

- **Status:** confirmada.
- A câmera usa Spring Arm e permanece independente do facing quando RMB não está ativo.
- Deve oferecer:
  - rotação horizontal e vertical;
  - zoom por roda do mouse;
  - distância mínima e máxima configuráveis;
  - interpolação suave de distância;
  - collision test contra geometria;
  - Camera Lag;
  - Camera Rotation Lag.
- A câmera não deve mudar de ombro como efeito colateral da rotação do corpo.
- Estados futuros, como lock-on, mira ou combate específico, podem alterar a política de câmera sem substituir a câmera-base.

## UE-0007 — Política de velocidade e direção

- **Status:** confirmada.
- A magnitude combinada dos eixos de movimento deve ser limitada para impedir bônus de velocidade diagonal.
- Strafe e backpedal podem usar multiplicadores distintos da velocidade frontal.
- Sprint só é permitido quando existe componente frontal positiva no movimento.
- Valores usados durante UE1.1 são parâmetros de protótipo, não regras permanentes.
- Velocidade final não será hardcoded por personagem.
- O sistema definitivo deverá resolver velocidade a partir de dados e modificadores, incluindo:
  - velocidade-base;
  - modo de locomoção;
  - peso/carga;
  - atributos;
  - equipamentos;
  - habilidades;
  - buffs;
  - debuffs;
  - terreno;
  - estados contextuais.
- Os consumidores de movimento devem utilizar a velocidade efetiva resolvida, evitando condicionais específicas de conteúdo espalhadas pelo Character.