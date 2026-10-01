# Decisões do KashmirUE

## UE-0001 — Engine definitiva

- **Status:** confirmada.
- **Decisão:** Unreal Engine 5.8.2 é a engine do KashmirAct daqui em diante.
- **Efeito:** Godot não é mais candidato de produção. `C:\Projetos\KashmirAct`
  permanece como arquivo histórico, referência de domínio, dados e testes.

## UE-0003 — Autoridade de gameplay

- **Status:** confirmada e validada para a baseline atual.
- C++ resolve `ActionRequest -> ActionRuntime -> CombatActionDefinition ->
  Delivery -> Target -> Effect -> CombatResult`.
- Blueprints, Animation Blueprints, Control Rig, física, VFX, áudio e câmera
  apresentam ou configuram; não aplicam dano nem substituem o runtime.
- GAS integra atributos, custos, tags, efeitos e tarefas, sem colapsar Delivery,
  Target e Effect em uma única habilidade.
- Na baseline UE1.1, C++ também é a única autoridade de movimento para LMB+RMB;
  o caminho Blueprint que aplicava actor-forward foi removido e validado.

## UE-0004 — Recursos e generalização

- Stamina: ataques físicos e à distância.
- Mana: magia.
- Player, IA, replay e rede futura emitem o mesmo `ActionRequest`.
- Toda abstração fundamental requer ao menos duas implementações em testes.

## UE-0005 — Câmera, facing e referencial de movimento

- **Status:** confirmada e validada.
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
  - direção horizontal de movimento é derivada da Control Rotation/câmera;
  - a implementação usa uma única autoridade nativa em C++.

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

- **Status:** **SUPERSEDED** pela UE-0011 para política de gait.
- **Registro histórico, não vigente:** Sprint por input direto era permitido
  somente quando existia componente frontal positiva no movimento.
- Os princípios data-driven abaixo permanecem vigentes e foram incorporados à
  UE-0011:
  - a magnitude combinada dos eixos é limitada para impedir bônus diagonal;
  - strafe e backpedal podem usar multiplicadores distintos;
  - velocidade final não é hardcoded por personagem;
  - a resolução futura considera dados e modificadores, incluindo:
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
- Os consumidores de movimento devem utilizar a velocidade efetiva resolvida,
  evitando condicionais específicas de conteúdo espalhadas pelo Character.

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

## UE-0011 — Política de gait e autoridade de velocidade

- **Status:** confirmada; implementação atual validada.
- Jog é a gait padrão.
- Walk é a gait de precisão solicitada ao manter Left Alt pressionado.
- Sprint não possui input direto.
- Um Sprint futuro será resolvido por buffs, status ou gameplay effects e deverá
  substituir temporariamente a gait padrão por uma regra explícita.
- C++ de gameplay e configuração data-driven possuem autoridade sobre a
  velocidade física; Blueprint e animação não escrevem `MaxWalkSpeed`.
- Valores validados da baseline: Jog `525`, Walk `215`, conforme
  `DA_PlayerMovement_Default`.

## UE-0012 — Política de input de Dodge e Jump

- **Status:** confirmada e validada.
- Left Shift aciona Dodge por `IA_Dodge`.
- Space aciona `IA_TraversalOrJump`, atualmente resolvido como Jump simples.
- Jump usa o `ACharacter::Jump()` / `CharacterMovement` nativo.
- Não existe air jump, double jump ou air dodge na baseline atual.
- Dodge continua bloqueado durante queda.
- O nome histórico `IA_TraversalOrJump` pode permanecer temporariamente por compatibilidade, mas a mecânica ativa é apenas Jump simples.

## UE-0013 — Política de apresentação de animação

- **Status:** confirmada; baseline atual validada.
- `UKashmirAnimInstance` e `ABP_KashmirCharacter` representam estado produzido
  por gameplay; não tomam decisões de gameplay.
- A locomoção direcional atual usa Blend Spaces 1D dirigidos por `Direction`.
- A costura angular `-180/+180` usa Wrap Input.
- O estado Idle/Locomotion usa `bShouldMove` derivado da velocidade horizontal,
  inclusive durante braking acima do threshold.

## UE-0014 — Política atual de Root Motion

- **Status:** confirmada para a baseline atual; política final pendente.
- A locomoção-base Jog/Walk é in-place.
- O AnimBP consome Root Motion somente de montages.
- Esta decisão não habilita Root Motion global.
- Jump simples permanece sob autoridade do Character Movement.
- Qualquer política futura de Root Motion para Dodge ou escalada será decidida separadamente.

## UE-0015 — Parkour contextual removido do escopo atual

- **Status:** decisão confirmada em 2026-09-24.
- VaultLow, VaultHigh e Mantle não fazem parte do escopo atual do Kashmir.
- Space executa Jump simples e não tenta resolver obstáculos contextuais antes do salto.
- A tentativa de Vault com Motion Warping foi descontinuada antes de entrar na baseline.
- `AM_VaultLow`, Motion Warping e execução de Vault não fazem parte do runtime atual.
- O sistema de Traversal permanece apenas como fundação técnica neutra para possíveis mecânicas futuras.
- `Climb` e `LedgeGrab` ficam reservados para desenvolvimento posterior.
- A futura escalada será tratada como uma mecânica própria de exploração e poderá ser relacionada a trabalhos/profissões como Explorador.
- A futura escalada não deve ser tratada como simples continuação do antigo sistema de parkour.

## UE-0016 — Fronteira e proveniência de assets

- **Status:** confirmada para source control da baseline.
- O repositório inclui somente assets exigidos pelo protótipo validado ou
  aprovados por uma decisão explícita de governança.
- Bibliotecas baixadas, retargets em massa e fontes em `External/` não entram no
  Git por conveniência ou por seleção ampla de diretório.
- Todo novo conjunto precisa registrar origem, licença/termos, data/versão,
  hash, cadeia de derivação/retarget e limite de redistribuição antes do ingest.
- As bibliotecas locais atualmente não rastreadas permanecem preservadas no
  disco; esta decisão não autoriza exclusão.

## UE-0017 — Escopo de Data Validation

- **Status:** confirmada; política operacional vigente.
- Iterações normais usam Data Validation direcionada aos assets e sistemas
  afetados, acompanhada dos testes proporcionais ao risco da mudança.
- Full Content Data Validation não é uma etapa automática de toda passagem de
  desenvolvimento.
- Full Content Data Validation é obrigatória para releases, migrações de assets
  e mudanças capazes de afetar a integridade global dos assets.
- Uma execução já em andamento não deve ser interrompida nem repetida apenas
  pela adoção desta política.
- Evidência deve identificar explicitamente se o escopo validado foi
  direcionado ou global.

## UE-0018 — Classificação e disposição de assets

- **Status:** confirmada; política operacional vigente.
- `USED` → `KEEP IN CONTENT`.
- `UNUSED BUT LICENSED / USEFUL` → `MOVE TO EXTERNAL`.
- `UNKNOWN PROVENANCE` → `QUARANTINE / REVIEW`.
- `OBSOLETE DUPLICATE` → `DELETE ONLY AFTER REFERENCE CHECK`.
- Nenhum asset é movido ou excluído somente pelo nome, aparência ou localização.
- A checagem de referência deve considerar Asset Registry hard/soft references
  e, quando aplicável, Blueprint, código, configuração, carregamento dinâmico e
  cook/package.
- Quarentena preserva o arquivo para revisão; não autoriza uso, redistribuição
  ou exclusão.

## UE-0019 — Simplificação de Start/Stop e dinâmica de locomoção

- **Status:** confirmada e validada.
- Estados dedicados de animação `Start` e `Stop` não fazem parte da baseline atual.
- Idle e Locomotion permanecem conectados diretamente.
- A sensação de peso ao iniciar e encerrar movimento deve ser produzida prioritariamente por:
  - aceleração do Character Movement;
  - braking;
  - blending de animação.
- A baseline validada usa:
  - `MaxAcceleration = 1500`;
  - `BrakingDecelerationWalking = 500`;
  - `GroundFriction = 8`.
- Start/Stop dedicado permanece apenas como possível polimento futuro caso o ganho visual justifique a complexidade.
- Turn In Place refinado e animação final de Dodge também ficam fora do caminho crítico atual.

## UE-0020 — Separação entre evidência física, resolução e aplicação

- **Status:** confirmada e validada para a fundação UE2.
- Contato físico não aplica dano diretamente.
- O fluxo autoritativo é:
  `Weapon Contact -> FHitResult -> HitEvidence -> CombatResolver -> EffectResult -> DamageResolver -> EffectApplication`.
- Animação, trace, sweep, física e ragdoll produzem ou apresentam evidência; não são autoridade para alterar Health.
- `FKashmirHitEvidence` preserva dados físicos como ponto de impacto, normal, bone, direção e velocidade.
- `FKashmirCombatResult` representa resultados lógicos de combate.
- `FKashmirEffectResult` carrega magnitude, região atingida, direção de impacto e intensidade física.
- Apenas a camada de aplicação de efeitos pode alterar estado persistente como Health.

## UE-0021 — Regiões semânticas e rastreamento de armas

- **Status:** confirmada e validada para a fundação UE2.
- Gameplay não depende diretamente de nomes de bones específicos do skeleton.
- `UKashmirHitRegionMap` converte bones do skeleton para regiões semânticas `HitRegion.*`.
- Bones desconhecidos em contatos esqueléticos são rejeitados em vez de receber classificação implícita.
- Armas melee usam pontos de contato configuráveis e não regras específicas por tipo de arma.
- O sistema suporta quantidade arbitrária de pontos rastreáveis.
- Para espada, a baseline recomendada de autoria é:
  - `Weapon_Base`;
  - `Weapon_Mid`;
  - `Weapon_Tip`.
- Sweeps físicos produzem evidência de contato e deduplicam um mesmo ator durante uma janela ativa.
- `ImpactStrength` permanece separado de `Damage Magnitude`; velocidade física não determina automaticamente dano.

## UE-0022 — Progression & Identity Foundation v0.1

- **Status:** **DECIDED** em 2026-09-28; implementação global permanece
  majoritariamente **PENDING**.
- **Autoridade normativa:**
  `Docs/Design/Kashmir_Progression_and_Identity_Foundation_v0.1.md`.
- Kashmir usa um `Progression Graph` oculto. O jogador descobre sua identidade
  em jogo e não escolhe antecipadamente uma build completa.
- Character XP, Weapon Mastery, Technique Mastery e Combat Aptitudes
  transferíveis são conceitos separados, com responsabilidades distintas.
- Repetição bruta de input, uso ou cast não concede progressão significativa.
  A ação cria evidência; resultados relevantes de combate/encounter resolvem
  XP e mastery posteriormente.
- Anti-farm deve considerar ameaça, contribuição real e repetição de conteúdo
  trivial; volume de ações, isoladamente, não prova domínio.
- Achievements são estado funcional de gameplay.
- Classes emergem de comportamento, mastery, achievements, afiliações, quests,
  reputação e contexto de mundo. Religião, facção ou outra afiliação cria
  possibilidades, mas não concede classe automaticamente.
- Classes podem abrir novas linhagens de habilidades/técnicas e podem evoluir,
  transformar-se, ser substituídas ou deixar traços permanentes conforme o
  histórico do personagem.
- Cada grande família de arma possui uma `Ultimate Lineage` evolutiva. Ultimate
  não evolui por contagem de casts; resultados relevantes e uma memória de
  feitos históricos podem orientar seus estágios e ramificações.
- Monsters, NPCs, quests, classes, statuses, titles, professions, crafting,
  factions, religions, reputation, exploration, world events, bosses, PvP e
  demais sistemas de progressão devem consultar esta fundação.
- Documento futuro que a contradiga não a substitui silenciosamente: deve
  registrar **CONFLICT** e exigir nova decisão explícita.

## UE-0023 — Weapon Combat Grammar dirigido por TechniqueRequest

- **Status da decisão:** **DECIDED** em 2026-09-28.
- **Status técnico:** contratos e bridge **IMPLEMENTED / PARTIALLY VALIDATED**
  por build e Automation. Bindings físicos finais, assets de estilo e gate
  jogável completo permanecem **PENDING**.
- Combate direcional desenhado pelo mouse é **SUPERSEDED** como modelo de input.
  O gate manual MMB → drag → release está cancelado.
- Combate direcional de armas, combate dirigido por Technique e gramáticas
  específicas por família de arma permanecem **DECIDED**.
- Fluxo autoritativo:
  `TechniqueSlot -> TechniqueRequest -> WeaponCombatStyle -> CombatTechnique -> ActionRequest -> ActionRuntime -> Montage / Physical Trajectory -> WeaponTrace -> HitEvidence -> Combat Resolution`.
- O input não conhece a arma equipada e opera somente por slots lógicos
  configuráveis/rebindáveis.
- Player, AI, Replay e futura Network usam a mesma autoridade de técnica/ação.
- WeaponCombatStyle é data-driven; o core não possui branches `if Sword` ou
  `if Spear`.
- A baseline automatizada prova Sword e uma gramática conceitual contrastante
  de Spear no mesmo resolver. Isso não significa que Spear jogável esteja
  implementada.
- Sword é a primeira referência jogável pretendida, com identidade de
  versatilidade, transitions, combo, counter e mistura de slash/thrust. Spear
  é a segunda gramática conceitual, orientada a reach, spacing, interception e
  comportamento thrust-heavy.
- Os 16 ataques existentes da espada são preservados durante a migração. Ainda
  devem ser inspecionados semanticamente e classificados como Techniques
  distintas, animation variants ou redundâncias; seus montages são preservados
  inicialmente e cada animação não se torna automaticamente uma Skill.
- ActionRequest/Runtime, montage/DefaultSlot, WeaponTrace, HitEvidence,
  Defense, Parry, Clash, Deflect, Stagger e aplicação de estado permanecem
  válidos.
- `AttackDirection` continua relevante para HitEvidence, Block, Parry, Clash,
  Stagger, hit reactions, VFX e avaliação de AI. A intenção direcional vem da
  Technique, enquanto o sweep fornece a evidência física real do contato.
- O mouse permanece prioritariamente associado à câmera, free-look, controle
  corporal e targeting/aim quando aplicável.
- CombatTechnique contém somente hooks/metadados de progressão. Executar uma
  técnica não concede XP diretamente.

### Distinções conceituais vigentes

- **Animation:** representação visual/autoral de uma ação; não é autoridade de
  gameplay ou identidade completa da técnica.
- **Technique:** ação marcial executável com identidade, direção, timing,
  custo, regras de contato, runtime e metadados próprios. Uma Technique pode
  possuir múltiplas animation variants.
- **Skill/Ability:** capacidade descoberta ou aprendida que pode conceder,
  modificar, substituir ou criar Techniques.
- **Ultimate:** Technique especializada vinculada a uma Ultimate Lineage
  evolutiva; não é um executor paralelo ao combat core.
- **TechniqueSlot:** slot lógico e rebindável de input. Não identifica uma
  Technique permanente.
- **Technique:** ação resolvida para o slot pelo `WeaponCombatStyle` e pelo
  contexto atual. A regra é `TechniqueSlot1 -> current style binding ->
  resolved Technique`, nunca `Slot1 = CrossSlash` de forma fixa.

## UE-0024 — Base Motion define a família cinemática da Technique

- **Status da decisão:** **DECIDED** em 2026-09-29.
- **Status técnico:** **IMPLEMENTED / AUTOMATION VALIDATED** para o Slot2 da
  baseline ativa de Sword.
- A Base Motion define a família cinemática principal da apresentação de uma
  Technique. A identidade completa da Technique não se reduz à Animation, mas
  sua apresentação deve permanecer semanticamente compatível com a cinemática
  da animação-base.
- Control Rig e apresentação procedural podem variar amplitude, altura,
  inclinação, offsets de mão, body lean, abertura, postura e pequenas variações
  direcionais dentro de uma família compatível. Não devem reclassificar uma
  base para uma família incompatível, como `Horizontal -> Thrust` ou
  `Rising -> Horizontal` puro.
- `TechniqueFamily` é a autoridade atual para a família cinemática autorada.
  `AttackShape` continua descrevendo a forma física ampla (`Slash`, `Thrust`,
  `Sweep`, etc.); não será expandido ou duplicado somente para representar
  orientação cinemática.
- O Slot2 de Sword é
  `Technique.Sword.Diagonal.Rising.RightToLeft`, com
  `TechniqueFamily=DiagonalRising`, `AttackDirection=RightToLeft` e
  `AttackShape=Slash`.
- O Slot2 preserva `ActionId=Sword.Direct.Left` como identificador operacional
  legado, além do logical slot, `AM_KashmirSword_HorizontalB`, ActionRuntime,
  WeaponTrace, damage e timing. Renomear o ActionId ampliaria o contrato sem
  benefício para esta correção semântica.

## UE-0025 — Movement Intent define a participação corporal da Base Motion

- **Status da decisão:** **DECIDED** em 2026-09-30.
- **Status técnico:** **IMPLEMENTED / AUTOMATION VALIDATED / PIE VALIDATED** para
  `Stationary` e `FullBody`.
- `MovementIntent` é metadata da `CombatTechnique` e define como a Base Motion
  participa da apresentação corporal. Ele não decide gameplay, dano, contato,
  deslocamento ou Root Motion.
- `Stationary` preserva a locomotion/base pose no lower body e aplica a montage
  de `spine_01` para cima por `Layered Blend per Bone`. Esta é a rota validada
  para os Slots 1–4, Quick e Wide.
- `FullBody` permite que a montage participe do corpo inteiro, sem o mask
  upper-body da rota Stationary. O `CR_KashmirSword` continua downstream e pode
  aplicar apresentação procedural depois de ambas as rotas.
- `FullBody` não significa Root Motion. O AnimBP permanece em
  `Root Motion From Montages Only`, e qualquer deslocamento autorado continua
  sujeito à política de Root Motion vigente e a decisões futuras específicas.
- O fluxo de apresentação é
  `Technique -> SwordActionPlan -> SwordPresentationComponent -> transient AnimInstance snapshot -> AnimGraph route`.
  Não há Actor lookup durante pose evaluation nem branch por TechniqueId,
  Quick, Wide ou Sword.
- `Technique.Sword.Test.FullBody` é uma definição de prova não vinculada a input;
  reutiliza autoridade, timing, damage e WeaponTrace existentes somente para
  demonstrar a seleção da rota FullBody.
- Step, Lunge, Advance, Retreat, Pivot e Airborne permanecem **PENDING** e não
  fazem parte do enum v0.1.
- Esta decisão é distinta de `UE-0024`: Kinematic Family descreve compatibilidade
  cinemática; Movement Intent descreve participação lower-body/full-body.
- PIE confirmou a separação física: `Stationary` manteve pés em aproximadamente
  `0.1 uu` e pelvis em `0.2–0.4 uu`; `FullBody` permitiu aproximadamente
  `39.3/46.7 uu` nos pés e `13.9 uu` na pelvis. O root permaneceu em `0` nas
  duas rotas; trace, dano `-24`, recovery e retorno `false -> true -> false`
  permaneceram corretos.
- Na UE 5.8, `FAnimNode_BlendListByBool` seleciona child `1` para `false` e
  child `0` para `true`. O asset e os testes estruturais preservam explicitamente
  essa semântica: `false -> LayeredBoneBlend` e `true -> DefaultSlot` raw.

## UE-0026 — Movement Delivery define deslocamento autorado pela Technique

- **Status da decisão:** **DECIDED** em 2026-09-30.
- **Status técnico:** contrato e propagação até o ActionPlan **IMPLEMENTED /
  AUTOMATION VALIDATED**. A execução física posterior é regida por UE-0027.
- `MovementIntent` controla a participação corporal da Base Motion na
  apresentação. `MovementDelivery` controla uma solicitação autorada de
  deslocamento real do personagem. Nenhum dos dois implica Root Motion.
- `EKashmirMovementDelivery` começa apenas com `None` e
  `ControlledTranslation`. `FKashmirTechniqueMovementSpec` contém delivery,
  distância, duração e direção lógica relativa; v0.1 expõe somente `Forward`.
- Todas as Techniques atualmente vinculadas/jogáveis permanecem com
  `Delivery=None`. `Technique.Sword.Test.StepForward` existe como definição
  persistente sem binding e usa `ControlledTranslation`; não existe StepForward
  jogável nesta etapa.
- O spec percorre `CombatTechniqueDefinition -> SwordActionPlan` sem alterar
  MovementIntent, montage, damage, WeaponTrace, HitEvidence, hurtboxes ou
  CombatResult.
- O futuro executor deve operar sobre CharacterMovement/movimento autorizado e
  acompanhar o estado Startup/Active/Recovery do ActionRuntime existente, sem
  criar uma segunda máquina de fases. Server authority, prediction, replay,
  collision validation e novas direções permanecem pendentes.
- `Stationary + ControlledTranslation` é uma combinação válida; `FullBody` não
  implica deslocamento. Participação corporal e translação mundial permanecem
  contratos ortogonais.

## UE-0027 — Movement Delivery Runtime usa CharacterMovement com sweep

- **Status da decisão:** **DECIDED** em 2026-09-30.
- **Status técnico:** **IMPLEMENTED / AUTOMATION VALIDATED / PIE VALIDATED**.
- `UKashmirMovementDeliveryComponent` executa `ControlledTranslation` como
  deslocamento de gameplay. A animação continua sendo somente apresentação;
  Root Motion e `MovementIntent` não autorizam nem dirigem o deslocamento.
- A entrega começa quando o `ActionRuntime` aceita e inicia a ação (`t=0`). A
  direção horizontal relativa ao yaw do personagem é capturada no início e a
  velocidade solicitada é constante: `Distance / Duration`.
- O componente usa o `UpdatedComponent` do `CharacterMovement` por
  `SafeMoveUpdatedComponent`, sempre com sweep. Colisão pode reduzir a distância
  efetiva e encerra a entrega como `Blocked`; não há teleporte direto nem
  compensação posterior da distância bloqueada.
- A entrega encerra ao completar distância/duração, por cancelamento, bloqueio,
  invalidação ou quando a ação correspondente deixa de estar ativa. Ela usa o
  ciclo de vida do `ActionRuntime` existente e não cria uma segunda máquina de
  Startup/Active/Recovery.
- Cancelamento explícito pertence ao `ActionRuntime`. `CancelCurrentAction()`
  solicita a interrupção à mesma autoridade que aplica `bCancellable` e
  `CancelWindows`; somente um cancelamento aceito encerra MovementDelivery como
  `Cancelled`, fecha trace e interrompe presentation. `CancelGesture()` continua
  limitado à captura legada e não é autoridade de Action.
- `ControlledTranslation.Duration` não pode exceder a duração total da definição
  encontrada na tabela autoritativa do `ActionRuntime`. Um plano impossível é
  rejeitado antes de iniciar com `MovementDurationExceedsActionLifetime`; não há
  truncamento silencioso. `ActionEnded` permanece fallback defensivo para estado
  invalidado ou divergência externa.
- Observabilidade de desenvolvimento expõe estado ativo, distância solicitada e
  efetiva, tempo, velocidade/delta solicitados, bloqueio e motivo de término,
  sem autoridade de gameplay adicional.
- Todas as Techniques vinculadas continuam `Delivery=None`; portanto nenhum
  golpe jogável foi alterado. A autoria persistente de StepForward está
  implementada e validada em PIE, mas seu binding permanece **PENDING**.
  Prediction, server authority, replay e novas direções também permanecem
  **PENDING**.
- Cruzar uma borda sem colisão frontal e entrar em `MOVE_FALLING` não é tratado
  como blocking collision. Uma futura `LedgePolicy`/`GroundSupportPolicy` deverá
  decidir por Technique entre permitir queda, parar na borda ou exigir suporte
  projetado; nenhuma dessas políticas foi implementada.
- `L_CombatTest` não contém blocker apropriado para o branch `Blocked`: o alvo de
  combate usa `QueryOnly`/`Pawn=Ignore`, enquanto o único `Pawn=Block` é a
  geometria de chão. Para PIE, o componente oferece exclusivamente em Editor um
  blocker `RF_Transient`, Development-only, sem asset e sem salvar o mapa.
- PIE validou `Completed`, `Cancelled`, cancelamento recusado, `Blocked`,
  duration guard e `ActionEnded`. No teste final de parede, o blocker solicitado
  e efetivo permaneceu em `(100,0,100)`; `RequestedDistance=80 cm` resultou em
  `ActualDistance=47.3536 cm`, sem teleport, compensação, atravessamento ou
  movimento residual. Slope continua futuro/non-blocking e não integra o gate
  v0.1.

## UE-0028 — StepForward é semântica de Technique, não modo de Movement Delivery

- **Status da decisão:** **DECIDED** em 2026-09-30.
- **Status técnico:** autoria persistente **IMPLEMENTED / AUTOMATION VALIDATED /
  PIE VALIDATED**. A prova transitória anterior permanece **PIE VALIDATED / PUSHED**;
  nenhum binding ou conteúdo jogável persistente foi criado.
- `Technique.Sword.Test.StepForward` é a composição de identidade da Technique,
  Base Motion, `MovementIntent=FullBody`, `MovementDelivery=ControlledTranslation`
  e spec `80 cm / 0.25 s / Forward`. Não existe e não deve existir
  `EKashmirMovementDelivery::StepForward`.
- A definição persistente em `DA_KashmirSword_CombatStyle_Baseline` copia a
  Technique baseline do Slot1
  (`Technique.Sword.Horizontal.LeftToRight`) e preserva seu `ActionId`
  `Sword.Direct.Right`, montage `AM_KashmirSword_HorizontalA`, damage,
  WeaponTrace, HitEvidence e timing de combate. Somente a identidade, o
  Movement Intent e o MovementSpec da nova definição mudam. O sufixo `Test` foi
  preservado porque o mesmo asset já usa a convenção para a Technique persistente
  e não vinculada `Technique.Sword.Test.FullBody`; não houve renome por preferência.
- O delivery inicia em `t=0`, sem offset, easing, curve, startup delay ou
  dependência de Root Motion. Cancelamento continua sob autoridade do
  `ActionRuntime`; colisão pode encerrar o delivery como `Blocked` sem cancelar
  ou corromper automaticamente a Action.
- O Data Asset agora contém oito Techniques e mantém exatamente os quatro
  bindings anteriores dos Slots 1–4. StepForward não aparece em nenhum binding,
  Slot 5 continua ausente e nenhum input foi criado. A fixture lê a definição
  persistente e redireciona somente uma cópia transitória do primeiro binding
  para provar resolução pelo pipeline normal; ela não recria StepForward.
- PIE validou free space (`80 cm`, `0.25 s`, `320 cm/s`, `Completed`), blocker
  (`47.3536 cm`, `Blocked`), cancelamento aceito (`32 cm`, `Interrupted` /
  `Cancelled`), cancelamento recusado e cleanup completo. Trace, montage e
  presentation permaneceram sincronizados; o hit baseline continuou em `-24
  HP`.
- A revalidação da definição persistente carregou o Style real e usou somente
  uma cópia transitória do Style/binding, sem reconstruir StepForward. O plano
  preservou `BaseDamage=24`, `BaseGuardDamage=20`, timing
  `0.180/0.140/0.280` (`0.600 s` total), montage e spec. Free space completou
  aproximadamente `80 cm`, blocker encerrou em `44.8 cm`, cancelamento aceito
  encerrou em aproximadamente `32 cm`, e o pós-PIE permaneceu com `8` Techniques,
  `4` bindings, Slot 5 ausente e nenhum Content/map dirty.
- Automation valida 19/19 checks de StepForward, 152/152 de Combat, 8/8 de
  DirectionalSword e 6/6 de Foundation. Tornar StepForward um binding jogável
  persistente e decidir refinamentos v0.2
  (offset, easing, curve ou política de ledge) permanecem **PENDING**.
- Limitações não bloqueantes: a recusa de `CancelCurrentAction` não devolve o
  reason corretamente via Python; trace/montage exigem ticks reais de animação;
  getters de requested velocity/delta zeram após completion; slope, server
  authority, prediction e replay permanecem futuros.

## UE-0029 — Voluntary/Offensive Translation Grammar compõe direção sobre primitives

- **Status da decisão:** **DECIDED** em 2026-10-01.
- **Status técnico:** contrato v0.1 **IMPLEMENTED / AUTOMATION VALIDATED**;
  target-relative execution, RotationDelivery e novas Techniques permanecem
  **DESIGN ONLY / PENDING**.
- **Reclassificação:** esta decisão permanece válida como a subgramática de
  **Voluntary Combat Translation / Technique Movement** dentro da Combat Motion
  Grammar definida por UE-0030. O nome histórico “Offensive Movement Grammar”
  é preservado na suíte v0.1, sem rename massivo ou quebra de API.
- Technique, `MovementIntent`, `MovementDelivery`, direction/reference frame e
  semântica de movimento são conceitos distintos. Technique responde “o que”;
  MovementIntent controla participação corporal; MovementDelivery seleciona o
  primitive físico; direction/reference compõem a direção mundial; Step/Lunge/
  Advance/Retreat/Pivot descrevem intenção de design da Technique.
- `EKashmirMovementDelivery` continua somente `None` e
  `ControlledTranslation`. Não existem deliveries `StepForward`, `Lunge`,
  `Retreat` ou `Pivot`.
- `EKashmirMovementDirection` contém `Forward`, `Backward`, `Left` e `Right`.
  `EKashmirMovementReference` contém `Actor` e `Target`. No frame `Target`, o
  eixo Forward representa direção radial toward, Backward representa away e
  Left/Right representam candidatos tangenciais; essa convenção não autoriza
  execução por si só.
- `EKashmirMovementTargetPolicy` torna a disponibilidade de target explícita:
  `NotRequired`, `Required` ou `ActorFallback`. O runtime v0.1 aceita apenas
  `Actor + Forward + NotRequired`. Target-relative e demais direções já podem ser
  expressos como contrato, mas `IsValid()` os rejeita como ainda não executáveis,
  impedindo que sejam tratados silenciosamente como Actor Forward.
- Translation e rotation têm autoridades separadas. `ControlledTranslation`
  não controla yaw. Um futuro Pivot pode combinar translation tangencial,
  manutenção de facing e uma futura política/entrega de rotação, mas
  `RotationDelivery` não existe nesta versão. Root Motion permanece contrato
  separado e não é consumido pelo MovementSpec.
- Não foi criado `EKashmirTechniqueMovementSemantic`: ainda não há consumidor
  comportamental de AI, progression ou regras que justifique a taxonomia.
  TechniqueId, tags, MovementSpec, timing e cancelabilidade já carregam os dados
  necessários. Uma enum sem consumidor seria apenas metadata decorativa.
- Timing, commitment e cancelabilidade pertencem a cada Technique e ao
  `ActionRuntime`; não haverá regra global como “Lunge sempre não cancelável”.

| Semântica | Objetivo | Translation | Target-relative | Rotation | Commitment esperado | Status |
| --- | --- | --- | --- | --- | --- | --- |
| Step | Reposicionamento curto integrado ao ataque | Sim, tipicamente curta | Opcional | Normalmente não | Baixo a médio, definido pela Technique | StepForward é referência IMPLEMENTED; categoria geral DESIGN ONLY |
| Lunge | Extensão ofensiva explosiva de alcance | Sim | Actor ou Target | Pode preservar facing | Médio a alto, data-driven | DESIGN ONLY |
| Advance | Progressão ofensiva sustentada | Possivelmente contínua/segmentada | Opcional | Separada | Definido pela Technique | DESIGN ONLY |
| Retreat | Reposicionamento espacial para trás; não é Dodge | Sim | Actor Backward ou Target Away | Separada | Definido pela Technique | DESIGN ONLY |
| Pivot | Mudança angular ao redor do target sem pressupor mudança radial | Pode ser tangencial | Normalmente sim | Provavelmente necessária e separada | Definido pela Technique | DESIGN ONLY |

- `Technique.Sword.Test.StepForward` permanece `FullBody`,
  `ControlledTranslation`, `80 cm / 0.25 s`, `Actor + Forward + NotRequired`,
  unbound e não artisticamente aprovado. Nenhum asset, input, montage, damage,
  trace, hurtbox, lock-on, AI ou progression foi alterado.
- Evidência: full editor build; `OffensiveMovementGrammar` 10/10; StepForward
  19/19; MovementDeliveryRuntime 19/19; MovementDelivery 5/5; MovementIntent
  6/6; PlayableGrammar 13/13; Combat 162/162; Foundation 6/6.

## UE-0030 — Combat Motion Grammar Foundation

- **Status da decisão:** **DECIDED** em 2026-10-01.
- **Status técnico:** separações existentes **IMPLEMENTED / AUTOMATION
  VALIDATED**; o capability space e os novos contratos enumerados abaixo são
  **DESIGN ONLY / PENDING**. Nenhuma nova mecânica física foi implementada.
- Combat Motion não é uma enumeração monolítica. Uma Technique compõe contratos
  independentes para semântica da ação, apresentação corporal, translation
  voluntária, futura rotation, combat delivery, defense/evasion, reaction/
  forced motion e timing/cancellation/resources/targeting.
- UE-0029 passa a ser a subgramática autoral de translation voluntária. Seu
  `MovementSpec` continua contendo somente `Delivery`, `Distance`, `Duration`,
  `Direction`, `Reference` e `TargetPolicy`. `MovementDelivery` é primitive
  físico; não é a semântica de Dodge, Cast, Grapple, Knockback ou qualquer
  Technique específica.

### Mapa de responsabilidades

| Conceito | Autoridade existente | Lacuna/contrato futuro |
| --- | --- | --- |
| Body presentation | `EKashmirMovementIntent` | Novos intents somente quando houver consumidor real |
| Voluntary combat translation | `FKashmirTechniqueMovementSpec` + `EKashmirMovementDelivery` + `UKashmirMovementDeliveryComponent` | Executors para novas direções/reference frames, prediction e server authority |
| Rotation | Nenhuma autoridade de Technique; translation atual preserva yaw | `RotationSpec` / `RotationDelivery` ou contrato equivalente |
| Attack semantics | `TechniqueId`, `TechniqueFamily`, `AttackDirection`, `AttackShape`, `TechniqueTags` | Vocabulário adicional somente com regra consumidora |
| Combat delivery | `FKashmirCombatActionDefinition` + `EKashmirDeliveryType` (`Contact`, `Projectile`, `Beam`, `Area`, `Field`, `Grapple`, `Self`, `Target`) | Executors e lifecycle específicos conforme cada domínio |
| Timing, cancellation e resources | `FKashmirActionDefinition` + `FKashmirActionRuntime` | Autoria ampliada sem cadeia hardcoded |
| Defense | Guard/Parry/Deflect/Clash resolvers e Defense Pipeline | Autoria integrada de Techniques defensivas |
| Evasion | Dodge do Character existe como regra própria; não é delivery | Contrato de evasão com hurtbox, invulnerability, perfect window e counters |
| Casting | Tipos genéricos de combat delivery cobrem alguns outputs | Contrato de cast: tipo, charge/channel, mobility, release, target, interruption e resource commitment |
| Ranged | Primitives `Projectile`/`Beam` existem | Autoria e lifecycle ranged completos |
| Aerial | CharacterMovement/Jump determinam estado físico | Contrato combat-air para disponibilidade, launch, gravity/air control e landing transitions |
| Grapple | `EKashmirDeliveryType::Grapple` existe | Lifecycle e autoridade de grab/pull/shove/throw/takedown |
| Reaction | `FKashmirHitEvidence`, Stagger e `FKashmirPhysicalReactionResolver` | Integração de presentation/recovery por família de reação |
| Forced motion | Resultados de combate já podem expressar `Displace`; reação expõe direction/intensity | `ForcedMovementResponse` ou contrato equivalente, separado de voluntary translation |
| Technique transitions | `FKashmirTransitionRule`, `TransitionTo()` e `GetTransitionOptions()` existem no `ActionRuntime`; o Sword runtime atual usa rules vazias | Technique Transition Grammar autorável e contextual |

### Capability space

- **REFERENCE / DESIGN SPACE:** Locomotion; Voluntary Combat Translation;
  Rotation; Attack Motion; Defense; Evasion; Counter; Casting; Ranged; Aerial;
  Grapple; Forced Motion; Reaction; Transformation/Summoned Motion.
- Registrar uma família não declara implementação. Cada capacidade deve reutilizar
  contratos existentes ou justificar um contrato novo com consumidor real.
- Evasion pode compor `ControlledTranslation`, mas possui semântica própria
  (invulnerability, hurtbox policy, timing perfeito, cancel e counter). Portanto
  `Dodge`/`Roll` não entram em `EKashmirMovementDelivery`.
- Casting não é “montage + projectile”. O output `Projectile` ou `Beam` já cabe
  em combat delivery, mas charge, channel, mobility, release, interruption,
  targeting e resource commitment exigem lifecycle próprio.
- Aerial não é `MovementDirection=Up`: envolve movement mode, gravity, launch,
  air control, disponibilidade de ações e transição de landing.
- Forced movement é consequência involuntária de resultado de combate.
  Knockback, pull, launch e throw displacement não reutilizam automaticamente
  o runtime de movimento voluntário de uma Technique.
- Translation authority permanece diferente de rotation authority. O executor
  atual não altera actor yaw nem control rotation; Root Motion também permanece
  independente.

### Transition e ritmo

- A direção futura é `Action -> Transition Window -> qualquer Technique
  compatível`, não uma cadeia hardcoded `Attack1 -> Attack2 -> Attack3`.
  Compatibilidade poderá considerar weapon family, Technique atual, pose/state,
  grounded/airborne, target distance/angle, contact result, resources, ação
  anterior, input e progression/unlocks.
- `FKashmirTransitionRule` já fornece um substrate genérico de janela/tags, mas
  não constitui ainda a Technique Transition Grammar e o Sword runtime atual
  passa uma lista vazia de rules.
- Intenção de feeling: biomecânica fornece coerência; fantasia fornece magnitude
  e velocidade; baixo tempo morto e transições rápidas sem perder legibilidade.
  Black Myth: Wukong é referência de peso/leitura; Kashmir pode ser mais rápido;
  DMC e Black Desert são referências de continuidade/combinabilidade. Isso não
  autoriza tuning nesta decisão.
- Permanecem pendentes e fora deste escopo: pernas excessivamente rápidas em
  locomotion/combate, destaque do problema nos ataques 2 e 4, torso twist alto,
  pouca participação da pelvis e recovery corporal rápido.

### Evidência e limites

- `Kashmir.Combat.CombatMotionGrammar` valida 12/12 separações estruturais:
  voluntary translation vs MovementIntent/rotation/forced movement; ausência de
  Evasion/Casting no delivery; independência de Root Motion; preservação de
  StepForward; gate target-relative; autoridades existentes; runtime genérico;
  e ausência de mega-enum sem consumidor.
- Build `KashmirUEEditor` passou. Regressões: OffensiveMovementGrammar 10/10;
  StepForward 19/19; MovementDeliveryRuntime 19/19; filtro MovementDelivery
  24/24 (19 Runtime + 5 base); MovementIntent 6/6; PlayableGrammar 13/13;
  Combat 174/174; Foundation 6/6.
- `Technique.Sword.Test.StepForward` permanece o primeiro exemplo validado de
  Voluntary Combat Translation: `FullBody`, `ControlledTranslation`, `80 cm /
  0.25 s`, `Actor + Forward + NotRequired`, unbound.
- Não foram implementados RotationDelivery, Evasion, Casting, Ranged gameplay,
  Aerial combat, Grapple lifecycle, ForcedMovementResponse, Transition Grammar,
  novas Techniques, animations ou inputs.

## UE-0031 — Technique Transition Grammar é uma camada acima do ActionRuntime

- **Status da decisão:** **DECIDED** em 2026-10-01.
- **Status técnico:** v0.1 **IMPLEMENTED / AUTOMATION VALIDATED / PIE
  VALIDATED**; a primeira regra está persistida no Style baseline. Input
  buffering e condições por resultado de combate permanecem **PENDING**.
- Technique transition, Action transition e cancelamento são operações
  semanticamente distintas. Uma Technique transition aceita emite
  `Transitioned(A)` seguido por `Started(B)`; cancelamento continua emitindo
  `Interrupted(A)` e continua regido por `bCancellable`/`CancelWindows`.
- `TechniqueId != ActionId`. Duas Techniques podem compartilhar o mesmo
  `ActionId`, Base Motion ou montage e ainda exigir MovementSpec, MovementIntent
  ou apresentação procedural diferentes. O `ActionRuntime` não recebe
  conhecimento de `TechniqueId`.
- `UKashmirWeaponCombatStyle` é owner de `FKashmirTechniqueTransitionRule` e da
  elegibilidade `FromTechniqueId + ToTechniqueId + Elapsed + ContextTags`.
  Regras usam `MinElapsed`, `MaxElapsed`, tags required/blocked e prioridade,
  com ordenação determinística e destinos deduplicados.
- `ActionRuntime::Elapsed` é o relógio autoritativo. A janela não usa tempo de
  montage, notify, normalized animation time ou Tick visual.
- `UKashmirDirectionalSwordComponent` é o orquestrador transacional. Ele resolve
  B completamente, valida MovementDelivery/presentation e executa preflight de
  action/resources antes de permitir que o runtime faça a troca inferior.
- O `ActionRuntime` continua uma primitive genérica. A extensão
  `CanTransitionTo(Request, Rule, Context)` permite preflight sem mutação, e o
  overload correspondente de `TransitionTo` continua responsável por pagamento
  atômico e eventos. Nenhuma branch por Sword Technique existe no runtime.
- Após commit bem-sucedido do runtime, o orquestrador fecha o WeaponTrace A e
  limpa seu hit-set, encerra o MovementDelivery A como `Transitioned`, encerra a
  presentation A, substitui `ActivePlan` por B e inicia delivery, presentation e
  a futura janela Active de B. O commit inferior ocorre primeiro para que falha
  de recurso não destrua A.
- Sword presentation passa a distinguir também `TechniqueId`, evitando que duas
  Techniques com o mesmo `ActionId` sejam sincronizadas como a mesma identidade.
- Rules vazias preservam exatamente a rota anterior de cancel/reject. Não existe
  input buffer v0.1: requests antes/depois da janela continuam no comportamento
  anterior. OnHit, OnMiss, OnBlock, OnParry e OnGuardBreak não participam da
  elegibilidade v0.1.
- O Style baseline persiste exatamente uma regra: Slot1
  `Technique.Sword.Horizontal.LeftToRight` para Slot2
  `Technique.Sword.Diagonal.Rising.RightToLeft`, prioridade zero, sem tags, na
  janela inclusiva `[0.320, 0.470] s`. A janela começa no fim de Active
  (`0.180 + 0.140`) e cobre os primeiros `0.150 s` da Recovery real (`0.280 s`).
- Evidência PIE: requests em `0.22 s` e `0.53 s` foram recusados; `0.35 s`,
  `0.4333 s` e `0.450 s` foram aceitos. A troca emitiu `Transitioned(A) ->
  Started(B)` sem `Interrupted(A)`; B reiniciou em Startup/elapsed zero e
  completou sua duração de `0.600 s`.
- Com o pawn imóvel e o alvo temporariamente em alcance somente durante PIE,
  Health passou `100 -> 76 -> 52`, com 24 damage por Technique. A geração de
  trace passou `7 -> 8`, e B atingiu novamente o mesmo alvo, provando fechamento
  de A, limpeza do hit-set e nova contact identity. Slot2 standalone passou e
  Slot5 permaneceu safe-unbound.
- Nenhum T-pose ou resíduo foi observado no frame capturado. Smoothness temporal,
  blend e polish visual contínuo permanecem **UNVERIFIED / PENDING** porque não
  houve validação por vídeo contínuo.
- Evidência: full `KashmirUEEditor` build; TechniqueTransitionGrammar 9/9;
  CombatMotionGrammar 12/12; OffensiveMovementGrammar 10/10; StepForward 19/19;
  MovementDeliveryRuntime 19/19; MovementDelivery 5/5; MovementIntent 6/6;
  PlayableGrammar 13/13; DirectionalSword 19/19; Combat 183/183; Foundation 6/6.
- Futuro: input buffering/early queue, combat-outcome conditions, graph editing,
  aerial/cast/counter transitions, Ultimate phase orchestration e transitions
  geradas/desbloqueadas por progression.

## UE-0032 — Technique Request Buffer preserva intenção, não execução resolvida

- **Status da decisão:** **DECIDED** em 2026-10-01.
- **Status técnico:** v0.1 **IMPLEMENTED / AUTOMATION VALIDATED / PIE VALIDATED**.
- `UKashmirDirectionalSwordComponent` é owner do lifecycle do buffer.
  `ActionRuntime` permanece sem `TechniqueId`, pending request ou branches de
  arma; seu `Elapsed` continua sendo o relógio autoritativo da Transition Window.
- O buffer guarda uma `FKashmirTechniqueRequest`, Style, Technique destino,
  identidade da execução fonte, elapsed de captura, idade e lifetime. Ele não
  guarda `ActionId` como identidade da intenção, montage, animation, combo step
  ou um `FKashmirTechniqueActionPlan` resolvido como autoridade final.
- Existe somente um pending request. A política é `latest valid request wins`;
  repetir a mesma request renova sua idade. Não existe FIFO, array ou combo queue.
- O lifetime default é `0.20 s`: suficiente para uma request em aproximadamente
  `0.20–0.25 s` alcançar a abertura em `0.320 s`, mas curto demais para uma
  intenção no início sobreviver quase toda a Action. A idade acumula o mesmo
  `DeltaSeconds` determinístico passado a `AdvanceRuntime`, separadamente de
  `ActionRuntime::Elapsed`.
- Expiração precede elegibilidade: se `PendingAge > Lifetime`, o pending é limpo
  antes de verificar se a Transition Window acabou de abrir. Um frame longo não
  pode ressuscitar intenção expirada. Em PIE, uma request capturada no primeiro
  instante bufferable, seguida de `AdvanceRuntime(0.21)`, levou o source elapsed
  a `0.330`, limpou como `Expired` e não iniciou B.
- A execução fonte usa serial local monotônico do DirectionalSword, mais
  `SourceActionId` e `SourceTechniqueId`. Isso impede vazamento entre reinícios,
  Techniques que compartilham ActionId e Actions posteriores não relacionadas.
- Buffering só ocorre quando há Technique ativa, a request resolve, existe regra
  `ActiveTechnique -> RequestedTechnique`, tags atuais passam pelo resolver do
  Style, a janela ainda está no futuro, cabe no lifetime e abre antes do fim da
  Action fonte. Não há previsão de tags futuras.
- Dentro da janela, a transition é imediata. Depois de `MaxElapsed`, a request é
  rejeitada e não é carregada adiante. Requests cedo demais para caber no buffer
  preservam o comportamento anterior de cancel/reject; o buffer não vira um novo
  gate global.
- No consumo, a request é resolvida novamente pelo WeaponCombatStyle e pelo
  profile. Falha de resource/preflight descarta o pending como `ResourceFailure`,
  mantém A intacta e não executa fallback oculto para `TryCancel()`.
- Pending é limpo por consumo, expiração, término/mudança da execução fonte,
  transition para outro destino, cancelamento explícito, invalidation, janela
  perdida, troca/reset de profile ou EndPlay.
- A infraestrutura preserva `Player`, `AI`, `Replay` e `Network` sem branches por
  Source. Slot5 e StepForward continuam unbound; o asset baseline e a janela
  persistente `[0.320, 0.470] s` não foram alterados.
- Observabilidade development-only expõe presença, Slot, TechniqueId, Source,
  idade, elapsed de captura, lifetime e clear reason determinístico.
- Evidência: full `KashmirUEEditor` build; TechniqueRequestBuffer 7/7 cobrindo as
  25 propriedades requeridas; TechniqueTransitionGrammar 9/9;
  CombatMotionGrammar 12/12; OffensiveMovementGrammar 10/10; StepForward 19/19;
  MovementDeliveryRuntime 19/19; MovementDelivery 5/5; MovementIntent 6/6;
  PlayableGrammar 13/13; DirectionalSword 19/19; Combat 190/190; Foundation 6/6.
- Evidência PIE: probes before-window em `0.22`, `0.24` e `0.30` permaneceram
  pending e foram consumidos automaticamente em `0.320`; o limite inicial
  bufferable foi `0.120`, enquanto `0.06` e `0.10` não criaram pending. Requests
  em `0.36` e `0.44` transicionaram imediatamente; `0.50` retornou
  `TechniqueTransitionWindowMissed`. Repetição em `0.22 -> 0.27` preservou um
  único pending, renovou age/captured elapsed e terminou como `Consumed`; Slot5
  e os casos expired/stale permaneceram seguros.
- Evidência de combate PIE: a sequência real reduziu health `100 -> 76 -> 52`,
  com `24` damage por hit, e abriu trace generations `23 -> 24 -> 25` no mesmo
  target. O pawn permaneceu em `(0, 0, 98.1501)`; posição do target e health
  foram alterações somente de PIE, sem persistência em asset/mapa.
- Limitação conhecida não bloqueante: o diagnóstico do Character ainda descreve
  uma request aceita no buffer com a Action fonte/como iniciada antes de B
  realmente iniciar. Gameplay está correto; semântica de observabilidade e
  request-result fica para trabalho futuro. Polimento visual contínuo da
  transition também não foi validado.
- Futuro: semântica de logging/request-result, polimento visual contínuo,
  buffering condicionado a resultados de combate, prediction/network/rollback,
  AI behavior e graph/queue de combos permanecem fora do v0.1.

## UE-0033 — Combat Outcome Evidence é escopo de execução, não de contato

- **Status da decisão:** **DECIDED** em 2026-10-01.
- **Status técnico:** v0.1 **IMPLEMENTED / AUTOMATION VALIDATED / PIE
  VALIDATED**.
- `FKashmirHitEvidence`, defense resolution, `FKashmirCombatResult` e
  `FKashmirCombatantApplicationResult` continuam sendo as verdades do contato.
  `FKashmirCombatExecutionOutcome` é somente o resumo acumulado do que esses
  contratos provaram durante uma execução iniciada.
- O contrato e `FKashmirCombatOutcomeAccumulator` são genéricos. Na integração
  v0.1, `UKashmirDirectionalSwordComponent` possui `current + last finalized`
  porque já orquestra Technique, ActionRuntime, trace, delivery e presentation.
  `WeaponTrace` permanece acquisition-only e `ActionRuntime` permanece outcome-
  agnostic.
- A identidade reutiliza o serial monotônico existente do DirectionalSword mais
  `TechniqueId` e `ActionId`. `ActionId` isolado é insuficiente: duas Techniques
  com o mesmo ActionId recebem outcomes separados.
- Um contato entra no acumulador somente depois de `DirectionalMeleeResolver`
  resolver e `CombatantComponent::ApplyResolvedMelee` concluir. `ContactCount`
  conta resultados autoritativos processados, nunca sweeps; `UniqueTargetCount`
  conta `TargetId` distintos e não cresce com contatos repetidos no mesmo alvo.
- Damage usa exclusivamente `FKashmirEffectApplicationResult::AppliedMagnitude`
  para efeitos de resolução `Damage` realmente aplicados. Não usa BaseDamage,
  magnitude solicitada nem diferença inferida de health. Block, parry e guard
  break vêm das flags explícitas de `FKashmirDefensePipelineResult`.
- Outcome ativo expõe serial, Technique, Action, contatos, alvos únicos, flags
  suportadas, dano aplicado total, último TargetId e último CombatResult. O
  finalized acrescenta `Completed`, `Interrupted` ou `Transitioned`. Não existe
  histórico, persistência ou replicação no v0.1.
- Completion e cancel finalizam o current. Transition bem-sucedida finaliza A
  como `Transitioned` e começa B vazio, inclusive com ActionId compartilhado e
  consumo de request buffered. Request pending não cria outcome. Preflight/
  transition falha preserva A e seu evidence intactos.
- Uma execução finalizada com `ContactCount == 0` é a base objetiva para futuro
  `OnMiss`, mas nenhuma condição de transition por outcome foi implementada.
- Evidência PIE: uma execução real acumulou `1` contato, `1` alvo único e `24`
  damage aplicado, finalizando como `Completed`; a execução seguinte começou
  vazia e acumulou evidência própria. Uma execução fora de alcance finalizou
  `Completed` com zero contatos, sem implementar `OnMiss`.
- Transition imediata finalizou A como `Transitioned` e iniciou B com novo serial
  e outcome vazio. A prova buffered foi determinística via `ActionRuntime`: em
  aproximadamente `0.220 s`, B ficou pending sem criar outcome; ao cruzar a
  janela, o pending foi `Consumed`, A finalizou `Transitioned` e B iniciou vazio,
  depois acumulando seu próprio contato real e `24` damage. Request em `0.500 s`,
  após a janela `[0.320, 0.470] s`, foi recusada com
  `TechniqueTransitionWindowMissed` e preservou A sem falsa finalização.
- O log `Technique slot 2 started ActionId=Sword.Direct.Right` ainda aparece no
  request buffered antes do início real de B; os estados autoritativos confirmam
  que é somente um logging/observability wart conhecido. Nenhuma mudança
  persistente de asset ou mapa foi necessária para a validação PIE.
- Evidência final automatizada: full `KashmirUEEditor Win64 Development` build;
  `Kashmir.Combat.CombatOutcomeEvidence` 8/8;
  `Kashmir.Combat.TechniqueRequestBuffer` 7/7;
  `Kashmir.Combat.TechniqueTransitionGrammar` 9/9;
  `Kashmir.Combat.DirectionalSword` 19/19; `Kashmir.Combat` 198/198; e
  `Kashmir.Foundation` 6/6.
- Futuro: condições OnHit/OnMiss/OnBlock/OnParry, progression, mastery,
  achievements, AI, telemetry, networking/replication/prediction/rollback e
  histórico persistente permanecem fora do v0.1. Block, parry, guard break e
  `Interrupted` permanecem automation-covered neste checkpoint.

## UE-0034 — Technique transitions podem exigir fatos positivos do Outcome

- **Status da decisão:** **DECIDED** em 2026-10-01.
- **Status técnico:** v0.1 **IMPLEMENTED / AUTOMATION VALIDATED / PIE VALIDATED**.
- `FKashmirTechniqueTransitionRule::RequiredOutcomeFacts` é um bitmask de fatos
  positivos monotônicos: `HadContact`, `AppliedDamage`, `Blocked`, `Parried` e
  `GuardBroken`. Zero preserva a grammar anterior; múltiplos bits usam AND e OR
  é authorado como múltiplas rules. Bits desconhecidos são inválidos.
- A grammar recebe somente `FKashmirCombatOutcomeFacts`, uma view reduzida sem
  TargetId, CombatResult, totals ou counts. O accumulator continua apenas
  agregando evidence; `ActionRuntime` e `WeaponTrace` permanecem Outcome-
  agnostic.
- `WeaponCombatStyle` distingue `EligibleNow`, `FutureWindowReachable`,
  `OutcomePending`, `WindowMissed` e ausência de regra. Somente rules com fatos
  satisfeitos competem por resolução imediata; prioridade, tie-break lexical e
  dedupe por destino permanecem determinísticos. Requirements participam da
  equivalência semântica de rules.
- `DirectionalSwordComponent` fornece snapshot somente quando serial,
  TechniqueId e ActionId coincidem com a execução atual. Requests podem ser
  buffered antes da evidence e são reavaliadas por `AdvanceRuntime`; pending não
  congela uma rule nem cria Outcome. Na janela, requirements ainda ausentes
  produzem `OutcomePending` até evidence, fechamento ou expiração.
- Expiration continua precedendo eligibility. Outcome falso durante execução
  ativa significa apenas evidence ainda não observada e nunca é interpretado
  como Miss. NoContact, predicates negativos, thresholds e target predicates
  permanecem fora do v0.1.
- A regra persistente Slot1 -> Slot2 foi validada em PIE sem Outcome positivo:
  A serial 1 transitou para B serial 2 em elapsed 0.3553 e A finalizou como
  `Transitioned`. Isso confirma que requirements zero preservam o legado.
- A proof condicionada usou somente uma rule transient PIE A -> B, sem salvar
  package. Com `AppliedDamage` ausente, o request em elapsed 0.220 permaneceu
  pending ao abrir a janela e expirou sem iniciar B quando evidence não surgiu.
  Com janela transient [0.120, 0.300], o request único em elapsed 0.130688 foi
  consumido após contato real em `HitRegion.Torso`/`spine_04`: A serial 4
  finalizou `Transitioned` com 1 contato e 24 damage; B iniciou automaticamente
  com serial 5 e Outcome limpo. Nenhum asset/mapa foi alterado.
- Evidência: `Kashmir.Combat.OutcomeConditionedTransitions` 9/9;
  TechniqueTransitionGrammar 9/9; TechniqueRequestBuffer 7/7;
  CombatOutcomeEvidence 8/8; DirectionalSword 19/19; Combat 207/207;
  Foundation 6/6; full `KashmirUEEditor Win64 Development` build passa.
