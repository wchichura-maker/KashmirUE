# KashmirAct — Global Color Bible

**Documento:** `KashmirAct_Global_Color_Bible_v0.1.md`  
**Versão:** 0.1  
**Status:** aprovado como direção visual; paletas e faixas numéricas pendentes de validação técnica em cena real  
**Escopo:** direção cromática global, materiais, iluminação toon e implementação-base na Unreal Engine  
**Uso:** fonte versionável para concept art, personagens, ambientes, VFX, texturas, materiais e lighting

> **Nota de validação:** todos os valores HEX e todas as faixas numéricas deste documento constituem uma **especificação autoral v0.1 do KashmirAct**. Eles ainda precisam ser validados em uma cena real da Unreal Engine, sob exposição, tone mapping, pós-processamento e iluminação representativos do jogo, antes de serem congelados como especificação final.

## 1. Objetivo visual

KashmirAct adota fantasia com linguagem visual de OVA dos anos 1990. **Record of Lodoss War** é a referência principal para atmosfera, desenho, sobriedade cromática e materialidade; **Dragon Ball GT** é referência complementar para clareza gráfica, separação de massas e leitura imediata. Essas obras orientam critérios abstratos, sem copiar personagens, cenários, composições, símbolos, designs ou paletas específicas.

O resultado deve parecer desenhado e colorido de forma deliberada: lineart firme, massas planas, poucos níveis tonais e silhuetas legíveis. Não deve parecer pintura digital, render PBR suavizado ou imagem coberta por filtros.

## 2. Princípios fundamentais

- Usar **flat colors** como linguagem dominante.
- Preservar lineart visível e controlado.
- Usar no máximo três bandas por parte ou material: `Shadow`, `Base` e `Highlight`.
- Fazer a cor `Base` ocupar a maior área visível.
- Reservar `Highlight` principalmente para materiais reflexivos, metal, cabelo e olhos.
- Não inserir manchas escuras, sujeira tonal aleatória, granulação, noise ou variação pictórica na base color.
- Usar outline `near-black`, nunca preto absoluto como padrão global.
- Manter saturação baixa a moderada; controlar saturação junto com `Value` e contraste, nunca isoladamente.
- Produzir albedo sem iluminação, ambient occlusion, highlight ou sombra bakeados.
- Evitar gradientes livres. Quando indispensável para céu, água, névoa ou VFX, o gradiente deve ser amplo, limpo e subordinado às massas gráficas.
- Rugosidade, normal e iluminação podem descrever material, mas não devem apagar a leitura toon nem multiplicar bandas aparentes.

## 3. Sistema tonal de três bandas

| Banda | Função | Área recomendada | Regra |
|---|---|---:|---|
| `Shadow` | Separar planos e dar volume | 15–30% | Forma limpa, borda controlada e sem ruído interno |
| `Base` | Identidade cromática do objeto | 60–85% | Deve permanecer predominante em qualquer condição de luz |
| `Highlight` | Indicar brilho, foco ou material | 0–10% | Opcional; priorizar metal, cabelo, olhos e superfícies reflexivas |

Uma parte pode usar apenas `Base`, ou `Shadow + Base`. As três bandas são um teto, não uma meta. Pele, tecido fosco, madeira e pedra não precisam de highlight permanente. Microdetalhes não justificam bandas adicionais.

### 3.1 Faixas globais provisórias

- Saturação de superfícies comuns: aproximadamente `15–55%` em HSV.
- Saturação de accents e VFX: até aproximadamente `75%`, em área pequena.
- Valor da base: aproximadamente `25–82%`; evitar extremos extensos.
- Diferença de Value entre `Shadow` e `Base`: aproximadamente `12–25` pontos.
- Diferença de Value entre `Base` e `Highlight`: aproximadamente `8–20` pontos.
- Outline global: Value aproximado de `5–12%`, com leve matiz local.

Essas faixas são guardrails v0.1, não substitutos para julgamento visual.

## 4. Outline e lineart

Outline padrão: `#171820`.

- Permitir variações near-black por família: frio `#151923`, quente `#201817`, vegetal `#171C18`.
- Outline externo deve ser mais consistente que linhas internas.
- Linhas internas podem ser mais finas ou usar uma versão escurecida da cor local.
- Preto absoluto (`#000000`) fica reservado a ocorrências mínimas de máxima oclusão ou necessidade gráfica excepcional.
- Não suavizar o lineart com pinceladas, bloom ou iluminação especular contínua.

## 5. Global Color Bible v0.1

As tabelas abaixo definem `Shadow / Base / Highlight`. O highlight é opcional conforme o material. Os códigos devem ser tratados em sRGB na autoria; a implementação precisa conferir a conversão para linear dentro da Unreal.

### 5.1 Seres e elementos orgânicos

| Família | Shadow | Base | Highlight | Observação |
|---|---|---|---|---|
| Pele humana — clara | `#B97868` | `#D99A82` | `#EDB9A0` | Evitar rosa saturado e brilho cosmético |
| Pele humana — média | `#89594C` | `#B97862` | `#D59A7E` | Preservar subtom quente controlado |
| Pele humana — escura | `#4B3532` | `#765047` | `#A06F5C` | Não deslocar shadow para cinza morto |
| Orc — oliva | `#3D4936` | `#667052` | `#899171` | Highlight raro; usar em face/foco |
| Orc — verde terroso | `#35433A` | `#52624D` | `#76806A` | Saturação contida, sem verde neon |
| Cabelo — preto | `#171922` | `#2B2D38` | `#555565` | Highlight em mechas grandes |
| Cabelo — castanho | `#382823` | `#624238` | `#916556` | Não pintar fios individuais |
| Cabelo — loiro | `#79643F` | `#B4975A` | `#D8C07C` | Evitar amarelo puro |
| Cabelo — ruivo | `#63352A` | `#98513A` | `#C97750` | Accent quente sob controle |
| Cabelo — branco/cinza | `#626671` | `#9A9DA4` | `#D2D1CC` | Manter separação do aço |
| Osso | `#83775F` | `#B5A98A` | `#D2C7A8` | Marfim envelhecido, não branco puro |

### 5.2 Materiais manufaturados

| Família | Shadow | Base | Highlight | Observação |
|---|---|---|---|---|
| Couro — marrom | `#3E2D27` | `#6B4938` | `#8E6750` | Highlight mínimo, bordas amplas |
| Couro — escuro | `#252426` | `#3B3735` | `#5C5149` | Não confundir com ferro |
| Ferro | `#343841` | `#565C64` | `#858B90` | Frio, opaco, irregularidade só em formas grandes |
| Aço | `#3D4652` | `#71808C` | `#B7C1C5` | Highlight estreito e gráfico |
| Bronze | `#5A3D2A` | `#916541` | `#C09561` | Menos amarelo que ouro |
| Ouro | `#755426` | `#B78A3C` | `#E1C06A` | Usar em área pequena e de alto status |
| Madeira — clara | `#5B4230` | `#8B6748` | `#AF8962` | Veios simplificados, sem noise |
| Madeira — escura | `#342923` | `#554137` | `#795D49` | Highlight geralmente dispensável |
| Tecido — cru | `#777064` | `#A59C89` | `#C7BEA8` | Fosco; highlight apenas contextual |
| Tecido — vermelho | `#662E32` | `#9B4244` | `#C5655D` | Ver regras especiais de vermelho |
| Tecido — azul | `#2F4055` | `#4C6580` | `#7188A0` | Azul dessaturado, leitura nobre |
| Tecido — verde | `#384839` | `#59634B` | `#7D8262` | Separar de vegetação por Value |
| Tecido — violeta | `#49394E` | `#6B5271` | `#907395` | Reservar a facções/personagens definidos |

### 5.3 Ambiente natural

| Família | Shadow | Base | Highlight | Observação |
|---|---|---|---|---|
| Pedra — cinza fria | `#3E434A` | `#62676B` | `#898B88` | Planos grandes, sem pontilhado |
| Pedra — quente | `#51453C` | `#776757` | `#9A8974` | Ruína, vila e terreno seco |
| Pedra — ardósia | `#303943` | `#485562` | `#687785` | Boa para arquitetura fria/noturna |
| Vegetação — floresta | `#26392F` | `#425744` | `#68745A` | Highlight agrupado, não folha a folha |
| Vegetação — seca | `#555038` | `#7A704A` | `#9C9061` | Não saturar amarelos |
| Terreno — terra | `#49392F` | `#6D5541` | `#91735A` | Base ampla, variação por formas |
| Terreno — areia | `#74664D` | `#A18D68` | `#C5AF84` | Evitar areia luminosa demais |
| Terreno — lama | `#34332E` | `#504A3D` | `#706551` | Highlight somente quando molhada |
| Água — profunda | `#243D4B` | `#315C6B` | `#6F96A0` | Highlight em lâminas, não ruído especular |
| Água — rasa | `#355B5D` | `#4F7B75` | `#89AAA0` | Fundo legível, transparência controlada |

## 6. Hierarquia cromática da tela

A leitura deve acontecer nesta ordem:

1. Silhueta e Value do personagem ou ameaça principal.
2. Cor primária e rosto/arma/mão que conduz a ação.
3. Accent narrativo, sangue pertinente ou VFX de gameplay.
4. Personagens e objetos secundários.
5. Arquitetura e terreno.
6. Céu, fundo distante e ruído ambiental mínimo.

O ambiente não compete com o foco. Quanto mais distante ou menos interativo, menor a saturação, o contraste local e a densidade de detalhe. Accents saturados não devem se repetir por toda a tela.

## 7. Regra cromática para personagens

Cada personagem usa:

- **Primary:** maior massa cromática; define identidade imediata.
- **Secondary:** sustenta volumes e separa equipamentos.
- **Accent:** pequena área de alta prioridade narrativa.
- **Neutrals:** metal, couro, osso, cabelo e utilitários que estabilizam a composição.

Distribuição inicial recomendada: `Primary 50–65%`, `Secondary 20–30%`, `Accent 5–10%` e `Neutrals` no restante. Percentuais são referência de leitura, não medição obrigatória.

### 7.1 Exemplo — Orc guerreiro

| Papel | Aplicação | Cor-base sugerida |
|---|---|---|
| Primary | Pele oliva | `#667052` |
| Secondary | Tecido vermelho escuro | `#9B4244` |
| Accent | Bronze/ouro de patente | `#B78A3C` |
| Neutral 1 | Couro marrom | `#6B4938` |
| Neutral 2 | Ferro | `#565C64` |
| Outline | Contorno geral | `#171820` |

O rosto continua sendo lido pela pele, não por brilhos. O vermelho identifica a unidade sem dominar o quadro; o metal e o couro permanecem subordinados. Dentes e presas usam osso, nunca branco puro.

## 8. Vermelho e sangue

- Vermelho é cor de alta prioridade: limitar sua área e evitar repetição casual no cenário.
- Vermelho de facção, tecido e interface não deve coincidir exatamente com sangue.
- Sangue fresco: `Shadow #5A1F26`, `Base #8E2830`, `Highlight #B84843`.
- Sangue seco: `Shadow #351F21`, `Base #59282A`; normalmente sem highlight.
- Não usar vermelho puro (`#FF0000`).
- Sangue deve formar manchas e silhuetas gráficas, sem spray procedural fino ou noise.
- O highlight do sangue aparece apenas quando molhado e necessário à leitura.
- Em cenas com sangue narrativamente central, reduzir accents vermelhos concorrentes.

## 9. VFX e magia

VFX podem exceder a saturação dos materiais físicos, mas devem permanecer organizados em massas claras e temporárias.

| Família de magia | Núcleo | Corpo | Borda/sombra | Uso |
|---|---|---|---|---|
| Arcana azul | `#C8E8E8` | `#61A9B5` | `#315D78` | Magia neutra, selo, energia |
| Fogo | `#F2D17A` | `#D27642` | `#7F3830` | Evitar branco central extenso |
| Natureza | `#C1D783` | `#6F9A62` | `#365C46` | Não confundir com vegetação de fundo |
| Profana | `#E3A0C9` | `#955985` | `#4F385C` | Saturação concentrada no evento |
| Sagrada | `#E8DFC0` | `#C5A96B` | `#79684F` | Dourado pálido, sem estourar exposição |

Regras:

- Estruturar o efeito em até três níveis visuais equivalentes a núcleo, corpo e borda.
- Bloom é suporte, não forma principal.
- A silhueta do efeito deve continuar legível com bloom desligado.
- Evitar partículas multicoloridas sem função e ruído de alta frequência.
- A cor comunica escola, perigo e função de gameplay de modo consistente.
- Luz emitida pelo VFX pode afetar o entorno, mas não recolorir toda a paleta nem destruir a base color.

## 10. Comportamento por material

| Material | Bandas usuais | Especular/highlight | Tratamento |
|---|---:|---|---|
| Pele | 2–3 | Pequeno e seletivo | Shadow amplo; nariz, lábio e olhos recebem detalhe controlado |
| Cabelo | 2–3 | Faixa gráfica | Agrupar em mechas grandes; nada de fios pintados |
| Tecido fosco | 1–2 | Ausente na maioria | Dobras descritas por blocos, não gradientes |
| Couro | 2–3 | Borda curta | Diferenciar couro por Value e brilho, sem textura fotográfica |
| Ferro | 2–3 | Curto e opaco | Contraste médio, reflexo simplificado |
| Aço | 3 | Estreito e claro | Contraste mais alto; formas reflexivas deliberadas |
| Bronze/ouro | 3 | Quente e seletivo | Accent de status; não espalhar pela tela |
| Madeira | 1–2 | Raro | Veios grandes e poucos; sem noise procedural visível |
| Pedra | 1–2 | Ausente, salvo molhada | Quebras por planos grandes e bordas úteis |
| Osso | 2 | Raro | Marfim fosco; contraste menor que metal |
| Vegetação | 1–2 | Agrupado | Folhagem em massas; detalhe por cluster |
| Água | 2–3 | Lâminas gráficas | Reflexo e transparência controlados; sem cintilação aleatória |

## 11. Paletas regionais de exemplo

Estas combinações usam famílias da Global Color Bible e demonstram relação, não criam novas regras.

### Floresta antiga

- Dominantes: vegetação floresta `#425744`, madeira escura `#554137`.
- Suporte: pedra fria `#62676B`, terra `#6D5541`.
- Accent: osso `#B5A98A` ou magia arcana `#61A9B5`.
- Atmosfera: baixa saturação, contraste médio e fundos frios.

### Planalto seco

- Dominantes: areia `#A18D68`, pedra quente `#776757`.
- Suporte: vegetação seca `#7A704A`, couro `#6B4938`.
- Accent: tecido vermelho `#9B4244` em área restrita.
- Atmosfera: Value mais alto, sombras ainda cromáticas e sem amarelo excessivo.

### Fortaleza do norte

- Dominantes: ardósia `#485562`, ferro `#565C64`.
- Suporte: madeira escura `#554137`, tecido azul `#4C6580`.
- Accent: aço `#B7C1C5` e ouro `#B78A3C` apenas em focos.
- Atmosfera: fria, contraste firme e iluminação local rara.

### Pântano profanado

- Dominantes: lama `#504A3D`, água profunda `#315C6B`.
- Suporte: vegetação escura `#26392F`, pedra fria `#3E434A`.
- Accent: magia profana `#955985`.
- Atmosfera: Value baixo, silhuetas preservadas e ausência de névoa que lave toda a cena.

## 12. Arquitetura de materiais proposta — Unreal Engine

### 12.1 `M_Kashmir_Master`

Master Material responsável por:

- receber albedo limpo e autoral;
- selecionar e compor `Shadow / Base / Highlight`;
- controlar thresholds toon e suavidade mínima de transição;
- aplicar resposta por tipo de material;
- integrar tintes globais sem substituir a paleta local;
- expor máscaras de pele, cabelo, metal, olhos, emissivo e accents;
- oferecer debug views para albedo, bandas, Value, saturação e IDs de material.

Parâmetros sugeridos:

| Parâmetro | Tipo | Função |
|---|---|---|
| `BaseColor_Texture` | Texture2D | Albedo sem luz bakeada |
| `Palette_ID` | Scalar/Index | Família cromática autorizada |
| `Material_Type` | Scalar/Enum | Pele, tecido, metal, madeira etc. |
| `Shadow_Threshold` | Scalar | Corte entre shadow e base |
| `Highlight_Threshold` | Scalar | Corte opcional de highlight |
| `Band_Softness` | Scalar | Antialiasing da borda; manter baixo |
| `Shadow_Tint_Strength` | Scalar | Influência cromática global |
| `Highlight_Enable` | Static Bool | Desliga a terceira banda quando desnecessária |
| `Outline_Color` | Vector | Near-black local/global |
| `Accent_Mask` | Texture/Channel | Isola accents para controle |

### 12.2 Material Functions

- `MF_Kashmir_ThreeBandLighting`: quantização e composição das bandas.
- `MF_Kashmir_PaletteLookup`: busca `Shadow / Base / Highlight` por família.
- `MF_Kashmir_MaterialResponse`: resposta específica de pele, tecido, metal etc.
- `MF_Kashmir_Outline`: cor e comportamento de outline.
- `MF_Kashmir_TimeOfDayTint`: influência controlada de dia, tarde e noite.
- `MF_Kashmir_VFXPalette`: famílias autorizadas de magia.
- `MF_Kashmir_Debug`: inspeção de bandas, máscaras e valores.

### 12.3 `MPC_Kashmir_WorldPalette`

Material Parameter Collection global para:

- `World_ShadowTint`;
- `World_AmbientTint`;
- `World_HighlightTint`;
- `World_SaturationScale`;
- `World_ValueScale`;
- `World_ContrastScale`;
- `TimeOfDay_Blend`;
- `Night_ReadabilityFloor`;
- `VFX_EmissionScale`.

O MPC ajusta o clima como conjunto. Ele não deve recolorir cada asset livremente nem quebrar as relações definidas pela Color Bible.

## 13. Iluminação global

A base de iluminação é:

- um `Directional Light` como fonte principal e legível;
- um `Sky Light` controlado para preenchimento;
- poucas luzes locais, sempre motivadas e com função compositiva;
- exposição e pós-processamento estáveis por situação;
- HDRI e Lumen como recursos técnicos, **não autoritativos** sobre o visual toon.

Lumen, HDRI, reflexos e GI não podem introduzir gradientes excessivos, contaminação cromática, múltiplos highlights, sombras lavadas ou aparência de pintura/render realista. Quando entrarem em conflito com as bandas, a Color Bible e a leitura toon vencem.

## 14. Dia, fim de tarde e noite

### Dia

- Preservar as bases com maior neutralidade.
- Directional Light define sombras limpas; Sky Light evita pretos fechados.
- Não elevar saturação de toda a cena para simular sol.
- Highlights permanecem seletivos.

### Fim de tarde

- Aquecer moderadamente a luz principal e esfriar levemente o preenchimento.
- Manter identidade de pele, tecido, vegetação e materiais; o mundo não vira filtro laranja.
- Controlar vermelho e ouro para não fundirem com a luz quente.
- Alongar sombras sem acrescentar bandas.

### Noite

- Escurecer Value e deslocar o ambiente para azul/ardósia controlado, sem banho azul uniforme.
- Manter a base color reconhecível; preservar separação entre personagem, terreno e fundo.
- Usar `Night_ReadabilityFloor` para impedir colapso de shadows e outlines.
- Luzes locais devem ser poucas, motivadas e gráficas.
- Não transformar cada superfície em material molhado ou altamente especular.
- Accents, olhos e VFX podem conduzir a atenção, sem substituir a leitura da silhueta.

As três condições alteram a iluminação sobre a paleta; não trocam a identidade cromática dos assets.

## 15. Os 10 mandamentos visuais

1. **A base color domina.** A maior parte de cada forma permanece simples, limpa e reconhecível.
2. **Nenhuma parte excede três bandas.** `Shadow / Base / Highlight` são o limite, não a obrigação.
3. **Lineart é estrutura.** Outline near-black e linhas internas controladas sustentam a leitura.
4. **Albedo não contém luz.** Sem AO, sombra, highlight ou iluminação bakeados.
5. **Nada de pintura digital.** Sem pinceladas, manchas tonais, ruído ou modelagem por gradientes livres.
6. **Value vem antes da saturação.** Silhueta, contraste e hierarquia precisam funcionar antes do colorido.
7. **Highlight é privilégio.** Metal, cabelo, olhos e superfícies reflexivas o recebem com intenção.
8. **Accent é raro e narrativo.** Vermelho, ouro e magia não se espalham sem função.
9. **A luz serve à paleta.** HDRI, Lumen, pós-processamento e luzes locais nunca governam o estilo.
10. **Toda decisão deve sobreviver à cena real.** Validar em gameplay na Unreal antes de congelar valores.

## 16. Critérios de validação antes do freeze

Validar a v0.1 em pelo menos uma cena representativa contendo pele humana, orc, cabelo, tecido, couro, ferro, aço, madeira, pedra, vegetação, água, sangue e um VFX. Testar dia, fim de tarde e noite com as mesmas instâncias de material.

Checklist mínimo:

- a base continua predominante em todas as condições;
- nenhuma parte aparenta mais de três bandas;
- outlines não fecham detalhes nem desaparecem;
- personagens se separam do fundo em escala de gameplay;
- vermelho e VFX preservam prioridade sem estourar a tela;
- metais se distinguem de pedra, cabelo e tecido;
- pele humana e orc mantêm identidade sob os três horários;
- albedo isolado permanece limpo e sem iluminação;
- Lumen/HDRI não criam uma estética paralela;
- captura sem pós-processamento confirma a coerência dos materiais.

Somente após essa validação os HEX, thresholds e faixas podem receber status `frozen` em uma versão posterior.

## 17. Controle de versão

| Versão | Status | Alteração |
|---|---|---|
| `0.1` | Direção aprovada; validação em Unreal pendente | Primeira consolidação da direção visual, Global Color Bible, materiais, iluminação e regras de implementação |

