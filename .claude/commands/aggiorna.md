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

## 2. Fetch e merge

```bash
git fetch origin
git log --oneline HEAD..origin/main | wc -l    # quanti commit arrivano
git merge origin/main
```

## 3a. Merge pulito

1. `make update`: aggiorna meson locale, metroskrew e subprojects. È obbligatorio dopo ogni merge.
2. `make release`. Non usare `make` da solo: include `check`, che con un gioco
   modificato fallisce sempre.
3. Se la build **passa**: aggiorna l'intestazione di `docs/PPE_CHANGES.md` (data,
   commit di `origin/main` allineato) e controlla che le modifiche PPE elencate
   siano ancora presenti dopo il merge, come nel caso di `BATTLE_TYPE_NO_ITEMS`.
   Committa e fai `git push myfork ppe`.
4. Se la build **fallisce**: **niente push**. Riporta l'errore (le righe utili del
   log), un'analisi della causa (di solito un'API rinominata upstream che il codice
   PPE usa ancora) e una proposta di soluzione. Aspetta l'ok prima di modificare codice.

## 3b. Conflitti

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
  git push myfork ppe
  ```
