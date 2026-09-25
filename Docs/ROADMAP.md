# Roadmap de desenvolvimento — KashmirUE

Engine: Unreal Engine 5.8.2. A escolha de engine está encerrada.

Cada fase é um bloco completo: inspeção única, implementação integral e uma
validação consolidada ao final. Não há commit ou push automático.

## UE0 — Fundação e paridade

### UE0.1 — Bootstrap C++

- Projeto UE 5.8.2, módulos C++, tags e plugins-base.
- Validação de build, Data Validation e Automation.
- Política para arquivos gerados, assets, artefatos e Git/LFS.

### UE0.2 — Manifest e fixtures

- Manifestar assets do KashmirAct: hash, origem, licença, skeleton e destino.
- Consolidar schemas e exportar fixtures de ouro dos 328 testes atuais.
- Fixar contratos C++ e um leitor comum de fixtures.

### UE0.3 — Núcleo C++ de ações

- IDs, eventos, ActionRequest, ActionRuntime, transições e recursos.
- Stamina/mana, custos transacionais e regras de cancelamento.
- Paridade com o oráculo Python para entradas válidas e inválidas.

## UE1 — Personagem e locomoção

### UE1.1 — Personagem canônico — CONCLUÍDO

Baseline implementada e validada:

- [x] `ACharacter`, Character Movement e Enhanced Input.
- [x] Exploração canônica: `W/S` forward/backpedal relativo ao facing, `A/D`
  turn e `Q/E` strafe, com diagonal normalizada.
- [x] LMB free-look sem alterar o corpo; RMB controla câmera e Yaw corporal.
- [x] Free-look preserva o offset e só inicia recenter após parada completa e
  novo movimento frontal; mouse cancela o retorno, que usa o menor arco e uma
  velocidade independente de Camera Rotation Lag.
- [x] LMB + RMB move para frente pela direção horizontal da câmera/control
  rotation, com autoridade única em C++.
- [x] Zoom suave, collision avoidance, camera lag e configuração data-driven de
  movimento/câmera.
- [x] Lock-On com aquisição do alvo válido mais próximo, rebuild e ciclo por
  distância com wrap-around, distância de aquisição/quebra e linha de visão
  com grace time.
- [x] Câmera e corpo acompanham o alvo; `W/S` aproximam/afastam e `Q/E` orbitam
  relativamente ao target com correção radial.
- [x] `A/D` encerra Lock-On fora de Dodge e preserva a intenção manual.
- [x] Dodge em oito direções, com trajetória fixada; no Lock-On, permanece
  target-relative e preserva o Lock-On durante a execução.
- [x] Mapeamento reconciliado: Left Shift = Dodge; Space executa Jump simples
  via `IA_TraversalOrJump`, usando `ACharacter::Jump()` e Character Movement.
- [x] Jump simples bloqueado durante Dodge; sem double jump, air jump ou air dodge.

### UE1.2 — Pipeline de animação — BASELINE VALIDADA

Implementado e validado:

- [x] Bridge nativa `UKashmirAnimInstance` e `ABP_KashmirCharacter`.
- [x] State Machine `SM_Locomotion` com estados Idle e Locomotion.
- [x] Variáveis de apresentação `GroundSpeed`, `Direction`, `bShouldMove`,
  `bIsFalling`, `bIsDodging`, `bIsLockedOn` e `bIsWalking`.
- [x] `bShouldMove` derivado da velocidade horizontal, permanecendo verdadeiro
  durante braking enquanto `GroundSpeed > 3`.
- [x] `BS_Player_Jog_Direction` e `BS_Player_Walk_Direction` como Blend Spaces
  1D por `Direction`, faixa `-180..180` e Wrap Input na costura angular.
- [x] Jog como gait padrão a `525`; Walk de precisão por Left Alt a `215`,
  conforme `DA_PlayerMovement_Default`.
- [x] Autoridade de velocidade em gameplay C++ e configuração data-driven; o
  AnimBP apenas representa o estado.
- [x] Locomoção-base in-place nas 12 sequências alcançáveis e consumo atual de
  Root Motion pelo AnimBP limitado a montages.
  - [x] Fluxo de Jump integrado ao `SM_Locomotion` com `JumpStart`, `Airborne` e
  `Land`, mantendo a física sob autoridade do Character Movement.
- [x] Transições de aterrissagem validadas para Idle e Locomotion sem congelamento
  visual durante landing em movimento.
- [x] Start/Stop dedicado removido da baseline; Idle e Locomotion permanecem
  conectados diretamente, com sensação de peso produzida pela dinâmica do
  Character Movement e pelo blend de animação.
- [x] Baseline física validada com `MaxAcceleration = 1500`,
  `BrakingDecelerationWalking = 500` e `GroundFriction = 8`.
  - [x] Start/Stop dedicado removido da baseline; Idle e Locomotion permanecem
  conectados diretamente.
- [x] Peso de locomoção validado por Character Movement com
  `MaxAcceleration = 1500`, `BrakingDecelerationWalking = 500` e
  `GroundFriction = 8`.

Pendente:

- [ ] Calibração interativa de cadência/play rate direcional.
- [ ] Política final de Root Motion.
- [ ] Linked Anim Layers.
- [ ] Gait de Sprint dirigido por buffs/status, sem input direto.

### Polimento futuro de animação

Não bloqueia o avanço para combate:

- [ ] Turn In Place refinado.
- [ ] Animação final de Dodge.
- [ ] Foot IK.
- [ ] Start/Stop dedicado somente se futuramente houver ganho visual suficiente
  para justificar a complexidade adicional.


### Futuro de exploração — fora da baseline atual

- VaultLow, VaultHigh, Mantle e parkour contextual foram removidos do escopo atual.
- Space permanece dedicado ao Jump simples.
- Motion Warping não faz parte do runtime atual.
- Climb e LedgeGrab permanecem reservados para um sistema futuro de exploração.
- A futura escalada será tratada como mecânica própria e poderá se relacionar a
  trabalhos/profissões como Explorador, sem depender da antiga implementação de Vault.
## UE2 — Combate corpo a corpo autoritativo

### UE2.1 — Resolução generalizada — FUNDAÇÃO VALIDADA

Implementado e validado:

- [x] `CombatResolver` genérico separando Delivery, Target, Effect e CombatResult.
- [x] `FKashmirEffectResult` detalhado por alvo e efeito.
- [x] `FKashmirHitEvidence` como contrato entre contato físico e gameplay.
- [x] Conversão de `FHitResult` para evidência semântica.
- [x] `UKashmirHitRegionMap` data-driven para converter bones em `HitRegion.*`.
- [x] Pontos de contato de arma independentes do tipo específico de arma.
- [x] Construção determinística de sweep segments entre frames.
- [x] Sweep físico real no `UWorld`, com deduplicação por ator durante a janela ativa.
- [x] Velocidade, direção e intensidade física do ponto de contato preservadas.
- [x] `MeleeHitProcessor` conectando trace físico, HitEvidence e CombatResolver.
- [x] `DamageResolver` numérico separado da física e da aplicação em Health.
- [x] `EffectApplication` como única camada atual autorizada a alterar Health.
- [x] Damage, Heal e clamp de Health cobertos por Automation Tests.

Pendente para concluir UE2.1:

- [ ] Definições data-driven de ataques/armas fornecendo BaseDamage e multiplicadores.
- [ ] Identidade runtime estável de entidades, sem depender de nome de Actor.
- [ ] Integração com GAS/Attribute Set como adaptador de estado.
- [ ] Generalização da aplicação para Control, Modify, Displace, Create,
  Destroy e Information.

### UE2.2 — Evidência, anatomia e defesa

- Hurtboxes e Physics Asset por regiões semânticas.
- Block geométrico.
- Parry temporal.
- Clash / deflect.
- Stagger e guard break.
- Reação física parcial e integração futura com ragdoll/Physics Control.

### UE2.3 — Espada direcional

- Mouse drag parametriza família, direção, intensidade e curvatura.
- Montages autorados definem antecipação, janela ativa e recuperação.
- Control Rig/FBIK ajusta mãos, pés e alvo com limites anatômicos.

## UE3 — Generalização de combate

- Espada de uma/mãos, machado, lança e desarmado sobre o mesmo runtime.
- IA em StateTree emitindo ActionRequest compartilhado.
- Cinco ou mais bots em stress reproduzível.
- Telemetria, comportamento, padrões e candidatos de domínio.

## UE4 — Avaliação visual e operação

- Câmeras de gameplay, lateral, weapon-cam, topo e câmera lenta.
- Teste manual de leitura, responsividade, clipping, foot locking e reações.
- Automation, Functional Tests, Data Validation, Gauntlet, Visual Logger,
  Gameplay Debugger, Rewind Debugger e Unreal Insights.
- Só depois: VFX, áudio, câmera de impacto, magia e projéteis visuais.

## UE5 — Futuro após combate local aprovado

- Autoridade servidor/cliente, prediction e reconciliation.
- Replays, persistência e compatibilidade de schema.
- Magia, projéteis, tiro/arremesso, mundo e sistemas P1+.

## Histórico de reconciliação

O bloco antigo que mantinha UE1.1 como “EM ANDAMENTO” foi substituído em
2026-09-23 após a validação consolidada. A regra histórica de Sprint frontal
também foi superseded: não existe Sprint por input direto; uma futura gait de
Sprint dependerá de buffs/status e de decisão própria. UE0.1, UE0.2 e UE0.3
permanecem concluídos conforme seus relatórios de fundação.
