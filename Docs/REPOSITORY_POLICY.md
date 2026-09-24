# Política de repositório e artefatos

## Controle de versão

- Código C++, `Config/`, `Docs`, scripts de validação e `.uproject` pertencem ao
  repositório. `.uasset` e `.umap` entram somente quando necessários para o
  protótipo validado ou aprovados por decisão explícita de governança.
- Não criar commit ou push automaticamente.
- `Binaries/`, `Build/`, `DerivedDataCache/`, `Intermediate/`, `Saved/` e
  arquivos de IDE são gerados e permanecem ignorados.

## Git LFS

Arquivos binários versionados de mapa, assets, modelos, texturas e áudio usam
as regras de `.gitattributes`. Antes do primeiro asset binário, habilitar LFS
na máquina e confirmar que o servidor remoto aceita os ponteiros LFS.

## Assets e proveniência

Nenhum asset de KashmirAct é copiado automaticamente. UE0.2 criará um manifest
com hash, origem, licença, skeleton, formato e destino Unreal. Assets sem essa
proveniência não entram em `Content/`.

### Fronteira validada em 2026-09-23

Entre os assets não rastreados auditados, apenas estes quatro são alcançáveis a
partir do protótipo atual e pertencem à baseline UE1.2:

- `Content/KashmirAct/Characters/Player/Input/IA_Walk.uasset`;
- `Content/KashmirAct/Characters/Player/Animation/ABP_KashmirCharacter.uasset`;
- `Content/KashmirAct/Characters/Player/Animation/Locomotion/BS_Player_Jog_Direction.uasset`;
- `Content/KashmirAct/Characters/Player/Animation/Locomotion/BS_Player_Walk_Direction.uasset`.

Os outros 5.459 assets Unreal não rastreados e todo o conteúdo de `External/`
não são dependências alcançáveis do protótipo nesta auditoria. Permanecem fora
do Git e não devem ser adicionados em lote. Também não serão excluídos nesta
passagem: a cadeia local de aquisição, licença, hash, derivação/retarget e
redistribuição ainda precisa de um manifest de proveniência aprovado.

Reachability do Asset Registry não substitui cook/package audit. Antes de
promover qualquer biblioteca, validar também referências dinâmicas/editor-only
e a composição real do pacote.

As árvores locais `External/`, `Animation/Source/` e `Animation/Retargeted/`
permanecem ignoradas pelo Git. Exceções destinadas à baseline devem ser
selecionadas e adicionadas por caminho exato, nunca por inclusão ampla dessas
árvores. O asset intermediário/deprecated `BS_Player_Locomotion.uasset` também
permanece preservado localmente e ignorado até uma decisão explícita de
retenção ou descarte.

## Política de validação

- Durante iteração normal, executar validação direcionada aos sistemas, assets
  e dependências afetados, além dos testes de regressão proporcionais ao risco.
- Não executar Full Content Data Validation automaticamente em toda passagem de
  desenvolvimento.
- Executar Full Content Data Validation em preparação de release, migração de
  assets ou mudança que possa afetar a integridade global dos assets.
- Uma validação completa já em andamento não deve ser interrompida nem
  duplicada por causa desta política.
- Registrar claramente se a evidência é direcionada ou global; nenhuma delas
  substitui build, testes funcionais ou inspeção interativa quando aplicáveis.

## Evidência

Logs e capturas de validação ficam em `Saved/Validation/` localmente. Relatórios
reprodutíveis pequenos podem ser promovidos a `Docs/evidence/`; mídia grande só
entra no repositório via LFS após decisão explícita.
