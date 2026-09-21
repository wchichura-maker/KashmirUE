# Política de repositório e artefatos

## Controle de versão

- Código C++, `Config/`, `Docs/`, scripts de validação, `.uproject`, `.uasset`
  e `.umap` pertencem ao repositório.
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

## Evidência

Logs e capturas de validação ficam em `Saved/Validation/` localmente. Relatórios
reprodutíveis pequenos podem ser promovidos a `Docs/evidence/`; mídia grande só
entra no repositório via LFS após decisão explícita.
