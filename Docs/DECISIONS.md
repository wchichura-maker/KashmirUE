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
- Câmera, facing corporal e direção de movimento são conceitos independentes.
- O modo padrão de exploração segue controle inspirado em World of Warcraft/MMO clássico.

### Movimento

- `W`: movimento para frente relativo ao facing atual do personagem.
- `S`: backpedal relativo ao facing atual.
- `A/D`: giram o personagem para esquerda/direita.
- `A/D` também deslocam a orientação da câmera pelo mesmo delta de Yaw,
  preservando qualquer offset angular atual entre câmera e personagem.
- `Q/E`: strafe lateral sem alterar o facing.
- Movimento diagonal tem magnitude limitada para impedir bônus de velocidade.

### Controle da câmera por mouse

- Mouse sem botão pressionado não orbita a câmera.
- LMB + arrastar:
  - gira somente a câmera;
  - não altera o facing do personagem;
  - cria free-look temporário.
- RMB + arrastar:
  - gira a câmera;
  - o personagem acompanha o Yaw da câmera.
- LMB + RMB:
  - personagem caminha para frente;
  - direção horizontal de movimento é derivada da direção da câmera.

### Retorno da câmera

- Soltar LMB não recentraliza imediatamente a câmera.
- Continuar o movimento após free-look não força recentralização.
- `A/D` durante esse estado giram personagem e câmera juntos, preservando o
  offset relativo atual.
- O recenter automático só pode iniciar após:
  1. o jogador encerrar completamente o movimento;
  2. iniciar novamente movimento frontal positivo.
- `S`, `Q` ou `E` após a parada não iniciam recenter.
- LMB ou RMB durante o recenter cancelam imediatamente o retorno automático.
- A velocidade de recenter é independente da responsividade normal da câmera.

### Orientação automática

- `CharacterMovement` não usa `Orient Rotation to Movement` no modo padrão.
- A direção corporal não deve ser inferida automaticamente da velocidade.

## UE-0006 — Política de câmera UE1.1

- **Status:** confirmada.
- A câmera third-person usa Spring Arm e é independente do facing quando o
  jogador utiliza free-look.
- Deve oferecer:
  - rotação horizontal e vertical;
  - zoom por roda do mouse;
  - distância configurável;
  - interpolação suave de zoom;
  - collision test;
  - Camera Lag;
  - Camera Rotation Lag;
  - recenter automático opcional e independente.

- `CameraRotationLagSpeed` controla a resposta/suavização normal da câmera.
- `CameraRecenterSpeed` controla exclusivamente o retorno automático da câmera.
- Esses parâmetros não devem ser tratados como a mesma coisa.
- Estados futuros como lock-on, mira e modos de combate podem aplicar políticas
  próprias sem destruir o comportamento-base de exploração.

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

## UE-0008 — Hierarquia e locomoção de Lock-On

- **Status:** confirmada.
- Lock-On é um estado de controle, não um bloqueio global de inputs.
- A câmera acompanha continuamente o target enquanto o Lock-On permanece válido.
- O corpo tende a orientar-se para o target.
- A locomoção durante Lock-On usa uma base relativa ao alvo:
  - `W`: aproxima.
  - `S`: afasta.
  - `Q`: órbita para esquerda.
  - `E`: órbita para direita.
- A órbita utiliza correção radial para preservar aproximadamente a distância atual do alvo.
- Alterações voluntárias de distância por `W/S` redefinem o raio orbital de referência.
- `A/D` representam intenção explícita de abandonar o estado de Lock-On:
  - o Lock-On é encerrado imediatamente;
  - a mesma entrada continua sendo processada;
  - o personagem passa diretamente ao giro normal de exploração.
- O sistema deve priorizar intenção manual do jogador sobre assistências automáticas.

## UE-0009 — Seleção e ciclo de alvos no Lock-On

- **Status:** confirmada.
- `Tab` é a única ação primária de Lock-On.
- Sem alvo atual:
  - seleciona o alvo válido mais próximo.
- Com Lock-On ativo:
  - percorre ciclicamente todos os alvos válidos dentro de `LockOnAcquireDistance`.
- A lista de candidatos é reconstruída dinamicamente a cada acionamento.
- Apenas entram no ciclo alvos que:
  - possuem `KashmirLockOnTargetComponent`;
  - aceitam Lock-On;
  - estão dentro da distância de aquisição;
  - possuem linha de visão válida.
- A ordem atual é determinística por distância:
  - mais próximo → mais distante.
- O ciclo possui wrap-around:
  - último alvo → primeiro alvo.
- `A/D` continuam sendo intenção explícita de sair do Lock-On.

## UE-0010 — Dodge direcional

- **Status:** confirmada.
- O dodge usa uma direção determinada no instante em que é iniciado.
- Durante o dodge, a trajetória não é alterada por novos inputs de movimento ou giro.
- O sistema suporta oito direções:
  - frente;
  - trás;
  - esquerda;
  - direita;
  - quatro diagonais.
- Sem Lock-On:
  - a base direcional é o facing do personagem;
  - sem input direcional, o fallback é para frente.
- Com Lock-On:
  - a base direcional é relativa ao target;
  - `W` aproxima;
  - `S` afasta;
  - `Q/E` deslocam lateralmente em relação ao alvo;
  - diagonais combinam os eixos;
  - sem input direcional, o fallback é dodge para trás, afastando-se do alvo.
- O Lock-On permanece ativo durante o dodge.
- A/D não cancelam Lock-On enquanto o dodge está ativo.
- Velocidade e duração são data-driven por `UKashmirMovementConfig`.
- A implementação atual usa `CharacterMovement` e velocidade controlada.
- Animation Montage, Root Motion, i-frames e recovery pertencem à evolução posterior do sistema.