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
- [x] Mapeamento reconciliado: Left Shift = Dodge; Space sem binding e reservado
  para o futuro intent de Jump/Traversal.

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

Pendente:

- [ ] Calibração interativa de cadência/play rate direcional.
- [ ] Start / Stop.
- [ ] Turn In Place refinado.
- [ ] Foot IK.
- [ ] Política final de Root Motion.
- [ ] Linked Anim Layers.
- [ ] Animação final de Dodge.
- [ ] Fluxo de animação de Jump.
- [ ] Traversal contextual.
- [ ] Integração de Motion Warping para traversal.
- [ ] Gait de Sprint dirigido por buffs/status, sem input direto.

## UE2 — Combate corpo a corpo autoritativo

### UE2.1 — Resolução generalizada

- Delivery, Target, Effect e CombatResult em C++.
- Damage, heal, control, modify, displace, create, destroy e information.
- GAS como adaptador de atributos/efeitos, não como segundo resolver.

### UE2.2 — Evidência, anatomia e defesa

- Sweeps/traces produzem HitEvidence deduplicado.
- Hurtboxes por região; lâmina, ponta, haste, punhos e pés.
- Corte, perfuração, impacto, block geométrico, parry temporal, clash, deflect,
  stagger, guard break e recuperação.

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
