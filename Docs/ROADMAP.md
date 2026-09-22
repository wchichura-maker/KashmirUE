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

### UE1.1 — Personagem canônico — EM ANDAMENTO

- `ACharacter`, Character Movement e Enhanced Input.
- Câmera third-person independente do facing.
- Controle de exploração inspirado em MMO/WoW:
  - `W/S` forward/backpedal;
  - `A/D` turn;
  - `Q/E` strafe;
  - LMB free-look;
  - RMB câmera + facing;
  - LMB + RMB forward relativo à câmera.
- Free-look com offset persistente e recenter contextual após parada + novo
  movimento frontal.
- Velocidade de recenter separada de Camera Rotation Lag.
- Zoom suave, collision avoidance e camera lag.
- Movimento diagonal normalizado.
- Configuração data-driven de velocidade e dinâmica.
- Sprint condicionado à componente frontal.
- Aquisição básica de alvo de lock-on implementada e validada.
- Pendente:
  - comportamento corporal durante lock-on;
  - movimento durante lock-on;
  - câmera de lock-on;
  - esquiva em oito direções;
  - validação consolidada do UE1.1.
- [x] ACharacter + Character Movement.
- [x] Enhanced Input.
- [x] W/S forward/backpedal.
- [x] A/D turn.
- [x] Q/E strafe.
- [x] LMB free-look.
- [x] RMB câmera + facing.
- [x] LMB + RMB movimento frontal pela câmera.
- [x] Recenter contextual pelo menor arco.
- [x] Movimento data-driven.
- [x] Sprint frontal.
- [x] Lock-On por proximidade.
- [x] câmera acompanhando target.
- [x] facing corporal de Lock-On.
- [x] movimento relativo ao target.
- [x] órbita com correção radial.
- [x] distância de aquisição/quebra.
- [x] linha de visão com grace time.
- [x] ciclo de targets por Tab.
- [x] saída de Lock-On por A/D.
- [x] dodge em oito direções.
- [x] Integração validada entre exploração, Lock-On e dodge.

### UE1.2 — Pipeline de animação

- Manny/Quinn como skeleton inicial; segunda malha por IK Rig/Retargeter.
- Animation Blueprint, starts/stops, turn-in-place, foot IK e root-motion policy.
- Linked Anim Layers por postura/arma.

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

## Estado operacional atual

UE0.1, UE0.2 e UE0.3 estão concluídos e possuem validação consolidada.

### UE1.1 — Personagem canônico — CONCLUÍDO

- [x] ACharacter + Character Movement.
- [x] Enhanced Input.
- [x] Política completa de exploração.
- [x] Free-look e recenter contextual.
- [x] Sprint frontal.
- [x] Lock-On com aquisição, ciclo, distância e LOS.
- [x] Facing e câmera orientados ao target.
- [x] Movimento target-relative e órbita.
- [x] Saída de Lock-On por A/D.
- [x] Dodge em oito direções.
- [x] Integração validada entre exploração, Lock-On e dodge.

