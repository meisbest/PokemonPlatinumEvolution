---
description: Salva il lavoro su PPE (commit + pull + push su myfork/ppe + aggiornamento PPE_CHANGES.md)
argument-hint: "[descrizione facoltativa delle modifiche]"
---

Esegui la **procedura SALVA** descritta in `CLAUDE.md`. Rispetta tutte le regole di sicurezza di quel file.
Descrizione fornita dall'utente (può essere vuota): $ARGUMENTS

Tutti i comandi git vanno eseguiti in WSL nella cartella `~/pokeplatinum`, sul branch `ppe`.
Se il branch corrente non è `ppe`, fermati e chiedi.

1. **Stato:** esegui `git status` e `git diff --stat`. Riassumi all'utente cosa è cambiato.
2. **Controllo esclusioni.** Non devono essere committati:
   - `baserom.nds`, `*.nds`, la cartella `build/`
   - file più grandi di ~50 MB: `find . -size +50M -not -path './build/*' -not -path './.git/*'`
   - file con marcatori di conflitto: `git diff --name-only | xargs grep -l '^<<<<<<< '`
   - file che cambiano solo per i fine riga CRLF o solo per i permessi
     (`git diff --summary` con "mode change 100755 => 100644"), a meno che non siano voluti
   - file personali o temporanei

   Se trovi qualcosa di dubbio, **fermati e chiedi** prima di procedere.
3. **Aggiorna `docs/PPE_CHANGES.md`.** Per ogni modifica di questo salvataggio
   aggiungi o modifica la voce nella sezione giusta (area del gioco): cosa cambia
   rispetto all'originale, i file coinvolti e l'effetto nel gioco se si capisce dal
   codice. Aggiorna la data in testa e la tabella "Storico commit PPE" (l'hash si
   aggiunge dopo il commit, vedi il punto 8).
4. **Add:** `git add` solo dei file e delle cartelle specifiche da salvare, compreso
   `docs/PPE_CHANGES.md`. Non usare `git add -A` né `git add .` se il working tree
   contiene file da escludere. Ricontrolla con `git status` e `git diff --cached --stat`.
5. **Commit:** messaggio **descrittivo in italiano**: una riga di titolo su cosa
   cambia nel gioco, poi, se serve, un elenco puntato. Mai "wip" o messaggi generici.
6. **Pull:** `git pull --no-rebase myfork ppe`. Se ci sono conflitti **non risolverli da solo**: per ogni file spiega
   cosa ha cambiato l'altro, cosa abbiamo cambiato noi e la risoluzione consigliata,
   poi aspetta l'ok.
7. **Push:** `git push myfork ppe`. **Mai** su `origin`, **mai** `--force`.
8. Se lo storico in `PPE_CHANGES.md` ha bisogno dell'hash definitivo, aggiornalo
   ora e committalo con un piccolo commit di documentazione, poi rifai il push.
9. **Report finale:** hash del commit, file inclusi, esito di pull e push, ed
   eventuali file lasciati fuori con il motivo.
