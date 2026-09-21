# Fundação de migração

Fonte histórica: `C:\Projetos\KashmirAct`.

## Estratégia

O KashmirUE recomeça a implementação de engine do zero. Não é uma conversão de
projeto Godot. A migração é semântica: preservar comportamentos comprovados e
reescrevê-los em C++/Unreal de forma testável.

| Categoria | Destino |
|---|---|
| Fundação e especificação originais | Referência imutável no KashmirAct |
| Decisões posteriores | Consolidação progressiva em `Docs/` |
| Testes Python | Oráculo temporário e fixtures de paridade |
| Contratos/actions/combat/resources | structs, módulos C++ e Data Assets |
| Tags e condições | Gameplay Tags e queries versionadas |
| Atributos/custos | Attribute Sets, Gameplay Effects e adaptadores Kashmir |
| Godot scenes/GDScript/resources/shaders | Reconstrução Unreal; sem conversão direta |
| FBX, glTF, PNG e CC0 | Reimportação após manifest de licença/origem |
| Worker socket Python | Removido do runtime; Python é editor/offline apenas |

## Regras preservadas

- dano é Effect, não identidade de Attack, arma, animação ou delivery;
- contato produz evidência; só o runtime autoriza resultado;
- animação e física nunca são a autoridade do combate;
- armas fornecem dados e perfis, não executores próprios;
- telemetria observa e candidatos de domínio não promovem poderes sozinhos;
- interações são limitadas, ordenadas e determinísticas.

## Não copiar sem revisão

- contextos Python sem tipo;
- `melee_session.py` e harnesses de laboratório;
- schemas 0.1/1.0 sem migração;
- três cópias do X Bot FBX;
- assets sem proveniência ou direitos de redistribuição confirmados;
- recursos Godot `.res`, `.tres`, `.tscn`, `.import` e `.gdshader`.
