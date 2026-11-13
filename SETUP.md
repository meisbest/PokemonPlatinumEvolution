# Setup di PPE (Pokémon Platinum Evolution)

Guida rapida per chi lavora al fork PPE. Per i dettagli (pacchetti da
installare, debug, ecc.) vedi [INSTALL.md](INSTALL.md) di upstream. Per le
regole di lavoro e le procedure vedi [CLAUDE.md](CLAUDE.md).

## 1. Ambiente

- Windows con **WSL Ubuntu**. Tutto (git, build, editor) va eseguito **dentro WSL**.
- In VS Code apri la cartella con l'estensione **WSL** (*Connect to WSL* →
  `~/pokeplatinum`), **non** dal percorso `\\wsl.localhost\...`. Il Git per Windows
  rovina i permessi degli script e i fine riga.
- Installa le dipendenze elencate in [INSTALL.md](INSTALL.md) (sezione Linux).

## 2. Clone

Il progetto usa questi nomi per i remote: `myfork` = il nostro fork (push e pull),
`origin` = pret (solo fetch, **mai push**). Il clone chiama `origin` il fork,
quindi va rinominato:

```bash
cd ~
git clone https://github.com/meisbest/PokemonPlatinumEvolution.git pokeplatinum
cd pokeplatinum
git remote rename origin myfork
git remote add origin https://github.com/pret/pokeplatinum.git
git fetch origin
git checkout ppe          # segue myfork/ppe
git remote -v             # controlla: myfork = fork, origin = pret
```

## 3. Build

```bash
make update      # scarica meson locale, metroskrew e subprojects
make release     # → build/pokeplatinum.us.nds
```

- **Non** usare `make` da solo: esegue anche il controllo SHA1 contro la ROM
  originale, che con un gioco modificato fallisce sempre.
- `baserom.nds` resta solo in locale e non va mai committata.

## 4. Problemi frequenti

- **Dopo `make update` la build fallisce subito** con `... generated with an old
  version of meson`: upstream ha aggiornato meson. Esegui `make distclean`, che
  cancella solo `build/`, poi `make release`.
- **`nitrogfx: LF line endings aren't supported`** su un file `.pal`: le palette
  devono essere CRLF sul disco. `rm <file>.pal && git checkout -- <file>.pal`.
- **`Permission denied`** su uno script in `tools/`: `chmod +x <script>`. Se
  il problema è nel repo: `git update-index --chmod=+x <script>`.

Altri casi: tabella "Troubleshooting noto" in [CLAUDE.md](CLAUDE.md).
