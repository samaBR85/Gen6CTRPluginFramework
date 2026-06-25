# Gen6CTRPluginFrameworkOverhauled — v0.3.3

[English](README.md) · **Português**

Uma caixinha de superpoderes para **Pokémon X, Y, Omega Ruby e Alpha Sapphire** no Nintendo 3DS.

## O que é isto?

É um plugin `.3gx` que você coloca no seu 3DS e que adiciona um menu por cima do jogo de Pokémon — gerar qualquer
Pokémon, comprar em qualquer lugar, editar seu time, ler o adversário no meio da batalha, jogar alguns mini-jogos, e
cem pequenas conveniências no meio do caminho.

O pulo do gato: **foi feito pra ser entendido.** Cada recurso tem nome em linguagem simples, cada opção tem um botão
de info que explica o que faz, e existe um guia de 22 páginas *dentro* do plugin que te conduz por tudo. Se você ama
esses jogos mas nunca mexeu com homebrew, é exatamente pra você.

Você abre tudo com **SELECT**, e o menu aparece por cima do jogo:

<p align="center">
  <img src="Screenshots/v0.3.3/export/01_menu.png" width="330" alt="O menu do plugin sobre Pokémon Alpha Sapphire — lista de recursos em cima, botões embaixo" />
</p>

> 🎯 **Atenção — esta versão só tem conteúdo para Pokémon Alpha Sapphire** (Title ID `000400000011C500`). O
> framework é feito para X, Y, Omega Ruby e Alpha Sapphire, mas até agora só o **Alpha Sapphire** foi desenvolvido.
> **X, Y e Omega Ruby já estão a caminho!**

## Feito por um jogador curioso, com o Claude

Vou ser franco: **não sou programador.** Sou um jogador curioso — alguém bom em testar, cutucar as coisas e pensar
bastante num problema e em como resolvê-lo. Cada recurso aqui foi construído em bate-bola com o Claude, e não tenho a
menor vergonha de dizer: foi exatamente essa colaboração que me deixou *materializar* as coisas que eu vivia
desejando que existissem enquanto jogava.

Porque é daí que tudo isto veio — **necessidades reais, descobertas jogando.** Eu estava no meio de uma run, batia
numa fricção e pensava "tinha que ter um jeito de…", e aí a gente ia construir. Aos poucos despejei um pouco da minha
personalidade em cada recurso conforme ele tomava forma. **Parabéns aos criadores** de tudo em que isto se apoia —
estar sobre os ombros de gigantes não é exagero aqui (veja os [Créditos](#-créditos)).

## Um passeio guiado

Aqui vai o plugin mais ou menos na ordem em que você o encontraria. Nada abaixo é leitura obrigatória — é só um
passeio amigável pelos cômodos.

### Tudo começa com um botão

Aperte **SELECT** e o menu abre por cima do jogo. Mova com o **D-Pad**, escolha com **A**, volte com **B**. A tela de
toque de baixo tem os botões grandes — **Favoritos**, **Busca**, o **Game Guide** e o **App Guide**, **ActionReplay**
e **Tools**. Destaque qualquer coisa e aperte **X** pra ver a nota de info, ou **Y** pra fixar nos Favoritos. Essa é
a língua inteira do menu.

### Gere e capture qualquer Pokémon

O **Wild Pokémon Spawner** é uma lista filtrada ao vivo de todas as espécies. A tela de cima mostra os resultados; a
de baixo é um hub de filtros — refine por nome, número da Dex, geração, tipo ou pelo que você já tem.

<p align="center">
  <img src="Screenshots/v0.3.3/export/02_spawner_finder.png" width="330" alt="Wild Pokémon Spawner — resultados em cima, filtros embaixo" />
  <img src="Screenshots/v0.3.3/export/03_spawner_filtered.png" width="330" alt="Spawner filtrado só pros tipo Fogo da 6ª geração" />
</p>

Abra qualquer resultado pra ver a ficha completa — sprite, tipos, habilidades (incluindo a Oculta), stats base e os
quatro golpes que ele sabe no seu nível — então defina forma, nível e Normal/Shiny e gere-o na grama.

<p align="center">
  <img src="Screenshots/v0.3.3/export/04_spawner_detail.png" width="330" alt="Ficha do Pyroar com stats e ajustes de spawn" />
  <img src="Screenshots/v0.3.3/export/05_respawn_legendary.png" width="330" alt="Respawn Legendary — escolha um lendário e mande-o de volta pro lugar dele" />
</p>

Derrubou um lendário, ou viu um fugir? O **Respawn Legendary** lista todos com a localização real e manda o que você
escolher de volta pro lugar onde você o encontrou.

### Carregue um Poké Mart na bag

O **PokéMart Anywhere** transforma o "adicionar item" numa loja de verdade. Escolha um modo: **FREE** adiciona
qualquer coisa, qualquer quantidade, de graça — ou **PAY**, um Poké Mart real onde a lista filtra pro que você
realmente compra e cada item custa o seu dinheiro.

<p align="center">
  <img src="Screenshots/v0.3.3/export/06_pokemart_modes.png" width="330" alt="PokéMart Anywhere — escolhendo FREE ou PAY" />
  <img src="Screenshots/v0.3.3/export/07_pokemart_pay.png" width="330" alt="Modo PAY mostrando os itens com preços" />
</p>

Compre na hora, ou monte um **carrinho** e revise tudo no **Checkout** antes de pagar — nunca dá pra gastar a mais. E
ordene a lista inteira por nome, preço, tipo ou quantos você tem.

<p align="center">
  <img src="Screenshots/v0.3.3/export/08_pokemart_checkout.png" width="330" alt="O carrinho e o checkout, com o total correndo contra o seu dinheiro" />
</p>

### Construa e reconstrua nas suas boxes

O **PC Box ++** é seu armazenamento mostrado como uma grade de sprites, igual ao PC do jogo. Mova com o D-Pad, troque
de box com L/R, e **mova (X)**, **clone (Y)** ou **busque (START)** direto na grade.

<p align="center">
  <img src="Screenshots/v0.3.3/export/09_pcbox_grid.png" width="330" alt="Grade visual do PC Box ++ com a carta do Pokémon selecionado embaixo" />
  <img src="Screenshots/v0.3.3/export/10_pcbox_editor.png" width="330" alt="O editor por abas — Main, Stats, Moves, Origins, Misc" />
</p>

Aperte **A** pra abrir o editor, onde os detalhes se dividem em abas — Main, Stats, Moves, Origins, Misc. Cada campo
(espécie, IVs/EVs, golpes, habilidade, item, Poké Ball e mais) é escolhido numa lista visual — sem digitar, sem
teclado.

<p align="center">
  <img src="Screenshots/v0.3.3/export/11_pcbox_movepicker.png" width="330" alt="Escolhendo um golpe numa lista buscável e filtrada por tipo" />
</p>

### Conheça seu time num relance

O **View Party Summary** dispõe seu time como cartas com os números reais e ocultos — stats, IVs, EVs, nature,
habilidade, item e golpes. Deslize um seletor sobre um stat e aperte **A** pra pular pro companheiro com o valor mais
alto (ou mais baixo); marquinhas ▲/▼ sinalizam o melhor e o pior do seu time.

<p align="center">
  <img src="Screenshots/v0.3.3/export/12_party_card.png" width="330" alt="Carta de um membro do time com stats, nature, habilidade e golpes" />
</p>

### Vença a batalha

As **In-Battle Tools** são o que você usa no meio da luta. O **Enemy Helper** é uma carta de treinador pro inimigo —
explica a habilidade, o item e os golpes dele, lista os tipos que o vencem, e compara os seis stats dele com o seu
Pokémon ativo (e até diz se você já tem a espécie).

<p align="center">
  <img src="Screenshots/v0.3.3/export/14_enemy_details.png" width="330" alt="Enemy Helper — os detalhes do inimigo e o item segurado" />
  <img src="Screenshots/v0.3.3/export/15_enemy_compare.png" width="330" alt="Enemy Helper — aba Compare, os stats do inimigo vs os seus" />
</p>

O **Change Party Stats** é um editor visual do seu próprio time, ali no meio da luta — cure, ajuste HP/PP, status,
item, golpes, ou multiplique a EXP sem nunca sair da batalha. E o **Display Enemy Stats** sobrepõe os dados ocultos
do oponente na tela de cima.

<p align="center">
  <img src="Screenshots/v0.3.3/export/13_party_stats.png" width="330" alt="Change Party Stats — editando seu time no meio da batalha" />
  <img src="Screenshots/v0.3.3/export/16_enemy_overlay.png" width="420" alt="Overlay do Display Enemy Stats lendo o inimigo durante a batalha" />
</p>

### Dê um tempo — o Mini Game Corner

Quando você quer só jogar, tem um fliperama de sete jogos. Escolha um na grade; a tela de baixo tem um interruptor
**FREE / PAY** (FREE você fica com o que ganha, seu dinheiro nunca muda; PAY coloca Pokédólares de verdade em jogo).

<p align="center">
  <img src="Screenshots/v0.3.3/export/17_mini_hub.png" width="330" alt="O hub do Mini Game Corner com sete tiles de jogos" />
  <img src="Screenshots/v0.3.3/export/18_mini_toptrumps.png" width="330" alt="Top Trumps — um duelo de stats, vencido" />
</p>

Abra loot boxes, gire uma roleta de prêmios, puxe os slots, aposte num duelo de stats, chute maior-ou-menor, role um
desafiante selvagem pro seu próximo encontro, ou gere um time aleatório inteiro numa box vazia.

<p align="center">
  <img src="Screenshots/v0.3.3/export/19_mini_lootbox.png" width="250" alt="Revelação de prêmio do Loot Box" />
  <img src="Screenshots/v0.3.3/export/20_mini_wheel.png" width="250" alt="Spin the Wheel acertando um multiplicador alto" />
  <img src="Screenshots/v0.3.3/export/22_mini_teamgen.png" width="250" alt="Team Generator pré-visualizando seis Pokémon aleatórios" />
</p>

### Suavize a jornada longa

As ferramentas de **Overworld & Quality of Life** tiram a dor de uma run longa — texto rápido, andar rápido,
teletransporte, atravessar paredes, ovos instantâneos e mais. E um **HUD** pequeno e configurável pode manter seu
dinheiro, relógio, posição no mapa e o status do líder na tela enquanto você joga.

<p align="center">
  <img src="Screenshots/v0.3.3/export/24_qol_movement.png" width="330" alt="Opções de Movement & Travel" />
  <img src="Screenshots/v0.3.3/export/25_hud_ingame.png" width="330" alt="O HUD sobreposto no overworld" />
</p>

### Deixe do seu jeito

Vinte e cinco **temas** de cor repaginam a interface inteira — menus, botões, até o teclado do jogo — com uma amostra
de cor ao lado de cada nome e sua escolha lembrada entre as sessões. Fixe os recursos que mais usa nos **Favoritos**
(duas colunas, arraste pra reordenar), e remapeie as teclas do menu em **Tools › Hotkeys**.

<p align="center">
  <img src="Screenshots/v0.3.3/export/26_themes.png" width="330" alt="O seletor de temas com amostras de cor" />
  <img src="Screenshots/v0.3.3/export/27_appguide.png" width="330" alt="O leitor do App Guide embutido" />
</p>

E se você se sentir perdido, o **App Guide** (22 páginas, escrito como uma aventura guiada em vez de um manual) e uma
**nota de info (i) em cada função** estão sempre a um botão de distância.

## 📜 Como cresceu

Uma breve história, do mais antigo ao mais novo — sem detalhe, só o formato dela:

- **v0.2.x** — a grande repaginada de UX: menus reorganizados, Favoritos, avisos (toasts), o HUD e os primeiros temas.
- **v0.3.0** — os localizadores de tela dupla (Wild Pokémon Spawner, Respawn Legendary) e o App Guide.
- **v0.3.1** — **PokéMart Anywhere** (uma loja real na bag com preços, carrinho e checkout) + ordenar a bag.
- **v0.3.2** — capturar a própria UI do plugin nas screenshots; favoritos/cheats/hotkeys sobrevivem a uma atualização.
- **v0.3.3** — **In-Battle Tools** (Enemy Helper, Change Party Stats), o editor visual **PC Box ++**, e o
  **Mini Game Corner**.

## 📥 Instalando
1. Atualize para o [Luma3DS](https://github.com/LumaTeam/Luma3DS/releases/latest) mais recente.
2. Baixe a [versão](https://github.com/samaBR85/Gen6CTRPluginFramework/releases/latest) mais recente.
3. Extraia o `.zip` na **raiz do seu cartão SD**, mantendo a estrutura de pastas. Ele adiciona duas pastas:
   - `luma/` — o plugin em si, em `luma/plugins/000400000011C500/Gen6CTRPluginFramework.3gx` (junto com o App Guide e o Game Guide embutidos).
   - `Gen6CTRPluginFramework/` — os dados do plugin, incluindo os **arquivos de idioma** (Inglês, Francês, Alemão, Italiano, Japonês, Espanhol). O plugin carrega o idioma daqui, então não pule essa pasta. (Seus `Theme.txt` e `HUD.txt` são criados nessa pasta automaticamente no primeiro uso.)
4. Garanta que `Gen6CTRPluginFramework.3gx` seja o único arquivo `.3gx` do título.

> **Nota:** O pacote de idioma precisa ficar dentro da pasta `Gen6CTRPluginFramework` na **raiz do SD**. Garanta que o caminho seja exatamente:
> `SD:/Gen6CTRPluginFramework/Language/<Idioma>.txt`
> (por exemplo `SD:/Gen6CTRPluginFramework/Language/English.txt`).

## 🔧 Compilando
1. Requer o `devkitPro`.
2. Abra `C:/devkitPro/msys2` e rode `msys2_shell.bat`.
3. Adicione os repositórios ThePixellizerOSS (cole e rode):
   ```sh
   if ! grep -Fxq "[thepixellizeross-lib]" /etc/pacman.conf; then echo -e "\n[thepixellizeross-lib]\nServer = https://thepixellizeross.gitlab.io/packages/any\nSigLevel = Optional" | tee -a /etc/pacman.conf > /dev/null; fi; if ! grep -Fxq "[thepixellizeross-win]" /etc/pacman.conf; then echo -e "\n[thepixellizeross-win]\nServer = https://thepixellizeross.gitlab.io/packages/x86_64/win\nSigLevel = Optional" | tee -a /etc/pacman.conf > /dev/null; fi
   ```
4. Rode `pacman -Sy` e confirme que os bancos ThePixellizerOSS aparecem.
5. Rode `Release.bat` na pasta do plugin.

## 🙏 Créditos

Este projeto se apoia numa longa linha de trabalho voluntário — do primeiríssimo ancestral até este fork — e
**cada pedacinho merece reconhecimento.** Sem o esforço doado livremente por esta comunidade, nada disto existiria.

**A linhagem do plugin**
- **Baseado em** [Gen 6 CTRPluginFramework](https://github.com/biometrix76/Gen6CTRPluginFramework) por
  [biometrix76](https://github.com/biometrix76) — construído sobre o
  [Alolan CTRPluginFramework](https://github.com/biometrix76/alolanctrpluginframework/releases/latest)
  e uma continuação do abandonado
  [Multi-Pokémon Framework](https://github.com/semaj14/Multi-PokemonFramework) e
  [seus contribuidores](https://github.com/semaj14/Multi-PokemonFramework/blob/main/Credits.md).

**Fundações & ferramentas** (preservadas do upstream)
- [ThePixellizerOSS](https://gitlab.com/thepixellizeross) et al. — o 3gxtool e o CTRPluginFramework usados pra compilar plugins
- [PKHeX](https://github.com/kwsch/PKHeX/) (kwsch) et al. — banco de dados, documentação, exemplos e código
- [AnalogMan151](https://github.com/AnalogMan151) — a base ultraSuMoFramework do Alolan CTRPluginFramework
- [dragonfyre173](https://github.com/dragonfyre173) — o overlay de visualização de dados em jogo
- [JourneyOver](https://github.com/JourneyOver) et al. — o extenso [banco de códigos ActionReplay](https://github.com/JourneyOver/CTRPF-AR-CHEAT-CODES)
- [Alexander Hartmann](https://github.com/Hartie95) — a base de XY & ORAS deste plugin

**Fontes de imagem & dados** (para o Spawner, o localizador de itens e os dados da Pokédex)
- **Sprites de Pokémon** — os sprites do Spawner e os ícones de Lendários são reduzidos do conjunto de sprites X/Y do
  [Pokémon Database](https://pokemondb.net).
- **Ícones de itens / TM / HM** — do repositório [PokéAPI sprites](https://github.com/PokeAPI/sprites).
- **Dados de Pokédex, tipo, habilidade & golpe** — [Pokémon Showdown](https://github.com/smogon/pokemon-showdown) e
  [PokéAPI](https://pokeapi.co); **nomes de itens** do [PKHeX](https://github.com/kwsch/PKHeX/) (kwsch).
- Todas as imagens e nomes de Pokémon são **© Nintendo / Game Freak / The Pokémon Company.** Esses espelhos da
  comunidade são usados apenas pra construir esta ferramenta de fã, gratuita e não comercial.

**O Game Guide embutido** — o guia do Professor Oak Challenge
- **Mewlax** ([u/mewlax84](https://www.reddit.com/user/mewlax84), Instagram [@pokemewlax](https://www.instagram.com/pokemewlax/),
  X [@Mewlax1](https://twitter.com/Mewlax1)) — autor do guia de ORAS, compartilhado pela comunidade
  [r/ProfessorOak](https://www.reddit.com/r/ProfessorOak/).
- **Chamale** — inspirou primeiro o Professor Oak Challenge lá em 2018.
- **Johnstone** e **Chaotic Meatball** — por ajudarem a comunidade r/ProfessorOak a crescer.
- **Dynamite** — pela ordem das O-Powers; **Likemeon** — pela dica de chaining na Granite Cave.

**Este fork**
- Fork, repaginação e adições da v0.3.0 → v0.3.3 por [samaBR85](https://github.com/samaBR85), feitos em colaboração
  com o **Claude** (Anthropic).

## Licença
Licenciado sob **GNU GPL-3.0**, herdada do upstream. Veja [LICENSE](LICENSE).
