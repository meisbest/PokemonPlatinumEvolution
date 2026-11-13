# CLAUDE.md — Pokémon Platinum Evolution (PPE)

Istruzioni per Claude Code (e promemoria per chi lavora al progetto).
Lingua di lavoro: **italiano** (messaggi, commit, documentazione).

## Che progetto è

- **Pokémon Platinum Evolution (PPE)** è un fork personale di
  [pret/pokeplatinum](https://github.com/pret/pokeplatinum), la decompilazione di
  Pokémon Platino. Su questo fork si fanno modifiche al gioco (level cap, Guru
  Lotta, negozi, allenatori, ecc.).
- Lo sviluppano due persone: il proprietario del fork e un amico, che lavorano
  entrambi sul branch `ppe` del fork.
- L'elenco delle modifiche rispetto all'originale è in
  [docs/PPE_CHANGES.md](docs/PPE_CHANGES.md). **Va aggiornato a ogni SALVA.**

## Remote e branch

| Remote   | URL                                                    | Uso                                       |
|----------|--------------------------------------------------------|-------------------------------------------|
| `origin` | https://github.com/pret/pokeplatinum                   | **SOLO LETTURA.** Non è nostro. Solo `fetch`. |
| `myfork` | https://github.com/meisbest/PokemonPlatinumEvolution   | Il nostro fork: `pull` e `push`.          |

- **Branch di lavoro: `ppe`.** Tutto il lavoro si fa e si pubblica su `ppe`.
- `myfork/main` è il branch creato automaticamente dal fork su GitHub:
  **non va toccato**, né usato per lavorare.
- Il branch di default del fork su GitHub dovrebbe essere `ppe`. Si imposta da
  *Settings → General → Default branch*.
- I branch locali `level_cap`, `level_cap_test` e `main` sono vecchi branch
  locali. Non usarli per il lavoro corrente.

Flusso di collaborazione:
1. Prima di iniziare: `git pull --no-rebase myfork ppe`.
2. Lavora, poi esegui la procedura **SALVA** (`/salva`).
3. Ogni tanto si importano le novità di pret con la procedura **AGGIORNA DA
   UPSTREAM** (`/aggiorna`).

## Regole di sicurezza (tassative)

1. **Mai fare push su `origin`.** Niente `git push origin ...` e nessun push senza
   remote esplicito. Si fa push solo con `git push myfork ppe`.
2. **Mai `push --force`, `rebase`, `reset --hard`, `commit --amend` su commit già
   pubblicati, `filter-branch`/`filter-repo` o qualsiasi riscrittura della storia**
   senza la conferma esplicita dell'utente, chiesta ogni volta.
3. **Mai committare** `baserom.nds` (128 MB: GitHub rifiuta i file oltre 100 MB),
   la cartella `build/`, altri `*.nds`, file oltre ~50 MB o file personali di
   configurazione.
4. **Controllare sempre `git status` prima di committare.** Aggiungere i file per
   nome o per cartella, mai alla cieca con `git add -A` o `git add .` se nel
   working tree c'è roba che non va committata.
5. Non toccare le stash esistenti (`git stash drop`/`clear`) e non cancellare
   file non tracciati (`git clean`) senza l'ok dell'utente.
6. Niente PR, issue o commenti verso pret/pokeplatinum. Le regole di
   [AGENTS.md](AGENTS.md), che arriva da upstream, valgono per chi contribuisce a pret.
   PPE non invia nulla a pret.

## Situazione nota della storia git (da leggere!)

- A giugno 2026 la storia di `ppe` è stata riscritta con `git filter-branch` per
  togliere `baserom.nds`. Così **tutti** gli hash, compresi quelli dei commit presi da
  upstream, sono diversi da quelli di `origin/main`.
  - Esempio: "Fix flag name typo (#1216)" è `51a167557` su `ppe` e `b2cd286f3` su
    `origin/main`. I due commit hanno **lo stesso identico contenuto** (stesso tree).
  - Per questo git crede che l'ultimo antenato comune sia un commit del 2023
    (`488c510e6`). Un `git merge origin/main` normale darebbe migliaia di conflitti.
- **Correzione una tantum, da fare solo con l'ok dell'utente:** il "merge ponte"
  `git merge -s ours b2cd286f3`. Aggiunge un commit di merge che segna `b2cd286f3`
  come già incluso. Non modifica nessun file e non riscrive la storia. È descritto
  nel passo 0 della procedura AGGIORNA.
- Finché il merge ponte non è fatto, la base per confrontare `ppe` con l'originale
  è `b2cd286f3`, **non** `origin/main...ppe`:
  ```bash
  git diff b2cd286f3 ppe          # le modifiche di PPE
  ```
  Dopo il merge ponte e il primo AGGIORNA, `git diff origin/main...ppe` torna a
  funzionare normalmente.
- In `stash@{0}`...`stash@{3}` ci sono 4 stash del 22/06/2026. Contengono solo
  cambi di permesso dei file (755 → 644), non codice. `stash@{0}` è la copia
  riscritta da filter-branch e `git stash show` non la legge (usare
  `git diff 51a167557 stash@{0}`). Non droppare senza ok.
- `.claude/settings.local.json` è tracciato nel repo anche se è in `.gitignore`.
  È stato committato prima della regola. Toglierlo dal tracking
  (`git rm --cached`) solo se l'utente lo chiede.

## Procedura SALVA (`/salva`)

1. `git status`: guarda cosa è cambiato.
2. **Controllo dei file da escludere**: `baserom.nds`, `build/`, `*.nds`, file
   grandi (`find . -size +50M -not -path './build/*' -not -path './.git/*'`),
   file personali, avanzi di merge (file con `<<<<<<<`). Se c'è qualcosa di dubbio,
   chiedi all'utente.
3. **Aggiorna [docs/PPE_CHANGES.md](docs/PPE_CHANGES.md)** con le modifiche di
   questo salvataggio. Va fatto *prima* del commit, così finisce nello stesso commit.
4. `git add <file/cartelle specifiche>` (compreso `docs/PPE_CHANGES.md`).
5. `git commit` con un **messaggio descrittivo in italiano** (cosa cambia nel gioco,
   non "wip").
6. `git pull --no-rebase myfork ppe`. Prende il lavoro dell'amico. In caso di
   conflitti, fermati e chiedi, come nella procedura AGGIORNA.
7. `git push myfork ppe`.
8. Riporta all'utente: hash del commit, file inclusi, esito di pull e push.

## Procedura AGGIORNA DA UPSTREAM (`/aggiorna`)

0. **Passo una tantum: merge ponte.** Va fatto solo se
   `git merge-base --is-ancestor b2cd286f3 ppe` fallisce, cioè se il ponte non c'è ancora.
   - Verifica: `git merge-base --is-ancestor 51a167557 ppe` (deve riuscire).
   - Mostra all'utente `git diff --stat b2cd286f3 ppe` raggruppato per cartella.
   - **Chiedi l'ok esplicito**, poi esegui
     `git merge -s ours b2cd286f3 -m "Merge ponte: b2cd286f3 (upstream) equivale a 51a167557 (ppe riscritto)"`.
   - Verifica: `git diff HEAD~1 HEAD` deve essere vuoto.
1. **Working tree pulito**: `git status --porcelain` deve essere vuoto. I file
   non tracciati che esistono anche upstream bloccano il merge. Se non è pulito,
   **chiedi all'utente** se committare (SALVA), fare `git stash -u` o, solo per
   avanzi già verificati, ripulire.
2. `git fetch origin`
3. `git merge origin/main`
4. **Se il merge è pulito:**
   - `make update` (aggiorna i subprojects; serve dopo ogni merge da upstream)
   - `make release` (vedi Build: `make` da solo esegue anche `check`, che con un
     gioco modificato fallisce per forza)
   - se la build passa: aggiorna l'intestazione di `docs/PPE_CHANGES.md` (data e
     commit di `origin/main`), committala e fai `git push myfork ppe`.
   - se la build fallisce: **non fare push**. Riporta all'utente l'errore, con
     un'analisi della causa e una proposta di soluzione.
5. **Se ci sono conflitti:**
   - **NON risolverli da solo e NON fare `git merge --abort`.**
   - Per ogni file in conflitto (`git diff --name-only --diff-filter=U`) spiega:
     - cosa ha cambiato upstream: `git log --oneline MERGE_HEAD --not HEAD -- <file>` e `git diff HEAD...MERGE_HEAD -- <file>`
     - cosa avevamo cambiato noi: `git diff <merge-base> HEAD -- <file>`
     - la risoluzione consigliata
   - I conflitti banali, come file eliminati upstream che noi non avevamo
     modificato, si possono proporre come "da accettare". Comunque **aspetta l'ok**.
   - Alla fine ricorda i comandi per completare:
     `git add <file>` / `git rm <file>` → `git merge --continue` → poi `make update`,
     `make release` e `git push myfork ppe`.

## Build

Ambiente: WSL Ubuntu, repo in `~/pokeplatinum`. Guida completa: [INSTALL.md](INSTALL.md).

```bash
make release        # compila la ROM: build/pokeplatinum.us.nds
make                # = release + check. Il check SHA1 FALLISCE con un gioco modificato: è normale
make update         # aggiorna meson locale, metroskrew e subprojects (dopo ogni merge da upstream)
make debug          # build con simboli di debug
make clean          # pulisce gli oggetti di build
```

- Meson è **quello locale** in `subprojects/meson-<versione>/meson.py`, scaricato
  dal Makefile, non quello di sistema. Per lanciarlo a mano:
  `python3 subprojects/meson-*/meson.py ...`. Se ci sono più versioni, vale quella
  in `MESON_VER` nel Makefile.
- `baserom.nds` resta solo in locale (è ignorato da `*.nds` in `.gitignore`).

## Troubleshooting noto

| Problema | Soluzione |
|----------|-----------|
| Build fallisce dopo un merge da upstream | `make update`, poi rifai `make release`. |
| `get_metroskrew.sh: Permission denied` | `chmod +x tools/devtools/get_metroskrew.sh`. Per salvarlo in git: `git update-index --chmod=+x tools/devtools/get_metroskrew.sh`. |
| Script in `tools/` che perdono il permesso di esecuzione | Succede quando si modificano i file da Windows. Controlla con `git diff --summary` (righe "mode change 100755 => 100644") e ripristina con `git update-index --chmod=+x <file>`. |
| Commit bloccato da "untracked content" nel submodule `subprojects/narc` | `cd subprojects/narc && git clean -fd && cd -` |
| GitHub rifiuta il push per un file > 100 MB | Quasi sempre è `baserom.nds`. Non riscrivere la storia da solo: fermati e chiedi. |
| Fine riga CRLF nei file (diff enormi con +N/-N righe identiche) | I file sono stati salvati da Windows. Non committarli: ripristinali con `git checkout -- <file>`. |
| `git stash show stash@{0}` dà "not a stash-like commit" | È la stash riscritta da filter-branch: usa `git diff 51a167557 stash@{0}`. |
