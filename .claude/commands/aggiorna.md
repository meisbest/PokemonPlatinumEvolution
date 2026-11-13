---
description: Importa gli aggiornamenti di pret/pokeplatinum (origin/main) nel branch ppe, compila e pubblica su myfork
---

Esegui la **procedura AGGIORNA DA UPSTREAM** descritta in `CLAUDE.md`. Rispetta tutte le
regole di sicurezza di quel file. Comandi in WSL, cartella `~/pokeplatinum`, branch `ppe`.
Se il branch corrente non è `ppe`, fermati e chiedi.

## 0. Passo una tantum: merge ponte (solo se serve)

Verifica con `git merge-base --is-ancestor b2cd286f3 ppe`:
- se **riesce**, il ponte è già stato fatto: salta al punto 1.
- se **fallisce**, la storia di `ppe` è ancora quella riscritta da filter-branch:
  1. Verifica `git merge-base --is-ancestor 51a167557 ppe` (deve riuscire) e
     `git rev-parse 51a167557^{tree} b2cd286f3^{tree}` (i due tree devono coincidere).
     Se una delle due verifiche fallisce, fermati e spiega.
  2. Mostra all'utente un riepilogo di `git diff --stat b2cd286f3 ppe` raggruppato
     per cartella. Deve contenere solo modifiche PPE.
  3. **Chiedi l'ok esplicito.** Solo dopo esegui:
     `git merge -s ours b2cd286f3 -m "Merge ponte: b2cd286f3 (upstream) equivale a 51a167557 (ppe riscritto)"`
  4. Verifica che `git diff HEAD~1 HEAD` sia vuoto e che
     `git merge-base origin/main ppe` ora sia `b2cd286f3` o un commit successivo.

## 1. Working tree pulito

`git status --porcelain` deve essere vuoto. Anche i file non tracciati contano: se
esistono upstream, bloccano il merge. Se non è pulito, **fermati e chiedi** all'utente se:
- committare (procedura SALVA / `/salva`),
- fare `git stash push -u -m "prima di aggiorna <data>"`,
- oppure, solo per avanzi di upstream già verificati, eliminarli.

Non eliminare nulla senza ok.

## 2. Fetch

```bash
git fetch origin
BASE=$(git merge-base HEAD origin/main)
git log --oneline $BASE..origin/main | wc -l    # quanti commit arrivano
```

## 3. Controlli semantici PRIMA del merge

Un merge senza conflitti **non** garantisce che le modifiche PPE funzionino ancora:
vedi la sezione "Controlli semantici dopo il merge" in `CLAUDE.md`.

1. Leggi la sezione **"Punti di aggancio con upstream"** di `docs/PPE_CHANGES.md`:
   sono i simboli e i file da cui dipendono le nostre modifiche.
2. Analizza le modifiche di upstream:
   ```bash
   git diff --find-renames --diff-filter=RD --name-status $BASE origin/main   # file rinominati/eliminati
   git diff --find-renames --stat $BASE origin/main -- <file di aggancio>
   git log --oneline $BASE..origin/main -S<simbolo>                          # per ogni simbolo di aggancio
   ```
3. Presenta un report con ogni **rinomina, eliminazione, spostamento o riscrittura**
   che tocca un punto di aggancio, l'impatto sulla modifica PPE e una **proposta di
   adattamento**. Se non trovi nulla, scrivilo esplicitamente.
4. Chiedi all'utente se procedere con il merge.

## 4. Merge

```bash
git merge origin/main
```

## 5a. Merge pulito

1. `make update`: aggiorna meson locale, metroskrew e subprojects. È obbligatorio dopo ogni merge.
2. `make release`. Non usare `make` da solo: include `check`, che con un gioco
   modificato fallisce sempre. Se fallisce subito con "generated with an old
   version of meson" (upstream ha aggiornato meson): `make distclean` e poi di
   nuovo `make release`. Per raccogliere tutti gli errori in una sola passata:
   `make setup_release && ninja -C build -k 0 pokeplatinum.us.nds`. Non usare
   `NINJA=...`.
3. Se la build **fallisce**: **niente push**. Riporta l'errore (le righe utili del
   log), un'analisi della causa (di solito un'API rinominata upstream che il codice
   PPE usa ancora) e una proposta di soluzione. Aspetta l'ok prima di modificare
   codice. I controlli semantici del punto 4 vanno fatti comunque.
4. **Controlli semantici DOPO il merge:**
   - **Nomi vecchi rimasti:** per ogni simbolo o file rinominato o eliminato
     upstream, `git grep -n <nome_vecchio>` in tutto il repo, compresi `.s`,
     `.json`, `.order`, `meson.build` e testi.
   - **Ogni voce di `docs/PPE_CHANGES.md` è ancora attiva:** il codice deve esserci
     e anche essere **usato**:
     - le costanti devono essere lette oltre la definizione (`git grep -n <SIMBOLO> -- src include`)
     - le funzioni devono essere chiamate
     - gli ID dei comandi script in `include/data/scripts/scrcmd.h` devono puntare
       ancora alle nostre funzioni
     - lo strumento deve essere in `item_icon.order` e nei negozi
     - l'NPC deve essere nel file eventi
     - i campi JSON degli allenatori devono essere letti da `trainerproc`
   - Report per voce: ✅ attiva / ⚠️ presente ma non usata / ❌ mancante o rotta.
   - Proponi gli adattamenti meccanici e **applicali solo dopo l'ok**. Se upstream
     ha cambiato la **logica** su cui si basa una modifica, **fermati e spiega** le
     alternative.
5. Aggiorna `docs/PPE_CHANGES.md`: intestazione (data, commit di `origin/main`),
   eventuali punti di aggancio rinominati e stato delle voci.
6. Dai l'**elenco delle aree da testare in gioco** in base a cosa è cambiato.
7. Commit (procedura SALVA) e `git push myfork ppe`.

## 5b. Conflitti

- **NON risolverli da solo. NON eseguire `git merge --abort`.**
- Elenca i file: `git diff --name-only --diff-filter=U`
- Per **ogni** file in conflitto riporta:
  - **cosa ha cambiato upstream**: `git log --oneline HEAD..MERGE_HEAD -- <file>` e
    `git diff $(git merge-base HEAD MERGE_HEAD) MERGE_HEAD -- <file>`
  - **cosa avevamo cambiato noi**: `git diff $(git merge-base HEAD MERGE_HEAD) HEAD -- <file>`
  - **risoluzione consigliata**, con il motivo
- Conflitti banali, come file eliminati upstream che noi non avevamo modificato
  o file rinominati upstream senza nostre modifiche: proponili come
  **"da accettare"**, ma **aspetta l'ok**.
- Alla fine ricorda i comandi per completare:
  ```bash
  git add <file risolti>        # oppure: git rm <file eliminati>
  git merge --continue
  make update && make release
  ```
  Dopo, riprendi dal punto 5a.4 (controlli semantici DOPO il merge, aree da
  testare) prima del `git push myfork ppe`.
