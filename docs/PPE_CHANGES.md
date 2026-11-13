# PPE — Modifiche rispetto a pret/pokeplatinum

- **Ultimo aggiornamento:** 2026-11-13
- **Allineato a origin/main:** `b2cd286f3` "Fix flag name typo (#1216)" del 21/06/2026.
  Su `ppe` corrisponde a `51a167557`, che ha lo stesso contenuto.
  Al momento `origin/main` è a `62bdaacdc` (06/11/2026), cioè **121 commit più avanti**
  e non ancora importati.
- **Come è stato ricavato:** `git diff b2cd286f3 ppe` e i commit di Enrico Agrusti
  e meisbest. Non si usa `origin/main...ppe` perché la storia è stata riscritta
  (vedi [CLAUDE.md](../CLAUDE.md)).

Legenda: ⚠️ = problema noto o da verificare.

---

## 1. Progressione e livelli

### Level cap progressivo ("hard cap")
- **Cosa cambia:** nuova funzione `GetHardLevelCap()`. Calcola il livello massimo
  in base all'avanzamento della storia: medaglie possedute più alcune variabili
  e flag degli eventi. Il cap va da **10** a inizio gioco fino a **100** dopo il
  Monte Ostile, passando per 16, 21, 27, 30, 36, 37, 42, 45, 47, 52, 54, 58, 59,
  60, 63, 65, 68, 71, 73, 75, 77 e 85.
- **Effetto nel gioco:**
  - In lotta un Pokémon al cap non riceve esperienza. Se la sta per superare, la
    riceve solo fino al cap.
  - La Caramella Rara funziona solo sotto il cap. Al cap compare "Non avrà alcun effetto".
  - Pensione: l'esperienza fatta passeggiando si ferma al cap.
  - Scheda Allenatore: al posto del "Punteggio" compare **"LV CAP"** con il cap attuale.
- **File:** `src/field_battle_data_transfer.c` (funzione `GetHardLevelCap`),
  `include/field_battle_data_transfer.h`, `src/battle/battle_script.c`,
  `src/item_use_pokemon.c`, `src/overlay005/daycare.c`,
  `src/applications/trainer_case/card_text.c`, `src/savedata.c` (nuovo puntatore
  globale `gSaveDataPtr`, usato per leggere il salvataggio da ovunque).
- ⚠️ **Flag 318 controllato due volte:** vale come "→ 85" e come "→ 60". Il controllo
  "85" viene prima, quindi appena il flag 318 si attiva (Saturno al Lago Valore,
  secondo il commento) il cap salta subito a 85 e il ramo "60" non viene mai usato.
  Da verificare.
- **Commit:** `e8b0a26a5` Level cap, `98bb08cdb` Level cap finito, `f6691cc1d`
  (adattamento della scheda allenatore dopo il merge).

### Condividi Esp. sempre attivo
- **Cosa cambia:** cambia il calcolo dell'esperienza in `BtlCmd_CalcExpGain` e
  `BattleScript_GetExpTask`.
- **Effetto nel gioco:** l'esperienza di un Pokémon sconfitto si divide tra **tutti**
  i Pokémon della squadra non esausti e non uova. Chi ha lottato riceve la quota
  intera. Chi non ha lottato riceve metà quota. Lo strumento Condividi Esp. non serve
  più. Il level cap si applica comunque.
- **File:** `src/battle/battle_script.c`
- **Commit:** `7c3bee859` Condividi exp

---

## 2. Lotte

### Niente strumenti contro gli allenatori
- **Cosa cambia:** `BATTLE_TYPE_NO_ITEMS` include anche `BATTLE_TYPE_TRAINER`.
- **Effetto previsto:** nelle lotte contro allenatori la Borsa non si può usare
  (compare "Items can't be used here.").
- ⚠️ **Al momento NON funziona.** Nel merge di giugno upstream ha sostituito il
  controllo in `src/battle/battle_controller_player.c` con `BATTLE_TYPE_FRONTIER_LINK`,
  e `BATTLE_TYPE_NO_ITEMS` non è più usata da nessuna parte. Va riapplicata.
- **File:** `include/constants/battle.h` (`src/battle/battle_controller_player.c` per
  ripristinarla)
- **Commit:** `8191212f0` No items if trainer

### Stile lotta "Fisso" obbligatorio
- **Cosa cambia:** `Options_BattleStyle()` restituisce sempre `SET`. Nel menu
  Opzioni la voce Stile Lotta non si può modificare.
- **Effetto nel gioco:** quando l'avversario manda un nuovo Pokémon, il gioco non
  chiede mai se vuoi cambiare il tuo (regola tipica dei Nuzlocke).
- **File:** `src/game_options.c`, `src/applications/options_menu.c`
- **Commit:** `ff94d873a`

### Allenatori: EV, natura e abilità personalizzabili
- **Cosa cambia:** ogni Pokémon degli allenatori ha 3 campi nuovi nei JSON:
  - `ev_flags`: per ogni statistica `true`/`false`. I 510 EV si dividono in parti
    uguali tra le statistiche attive, al massimo 255 ciascuna.
  - `nature`: per esempio `"NATURE_ADAMANT"`, oppure `null` per lasciarla casuale.
  - `ability`: `0` o `1` per lo slot abilità, oppure `null` per il default.
  - Anche il tool di build `trainerproc` legge i nuovi campi, e le struct
    `TrainerMon*` hanno `evFlags`, `nature` e `abilitySlot`.
- **Stato dei dati:** tutti i **927 file** in `res/trainers/data/` hanno i nuovi
  campi, ma con i valori di default (tutto `false`/`null`). Quindi per ora nel gioco
  **non cambia nulla**: c'è l'infrastruttura, i valori vanno ancora decisi.
- **File:** `include/struct_defs/trainer_data.h`, `src/trainer_data.c`,
  `tools/dataproc/src/trainerproc.c`, `res/trainers/data/*.json`
- **Commit:** `c384801d5` Aggiunta EVs nemici, `852161878` Nature e abilità decise a mano

---

## 3. Personaggi e script (Pratopoli)

### Guru Lotta (nuovo NPC)
- **Cosa cambia:** nella casa est di Pratopoli c'è un nuovo NPC
  `PASTORIA_CITY_EAST_HOUSE_GURU_2` (aspetto da scienziato, posizione x=4, z=3)
  con il menu "Guru's Dojo":
  - **Cambia Abilità**: passa all'altra abilità della specie, se ne ha due.
  - **Cambia Natura**: scegli la natura da un elenco. Il PID viene rigenerato
    mantenendo sesso, slot abilità e cromaticità.
  - **Imposta IV** (0–31 per statistica)
  - **Imposta EV**: aggiunge o toglie EV, con limite di 252 per statistica e 510 in totale.
  - **Azzera EV** (gratis)
- **Prezzi:** il servizio è disponibile da **4 medaglie**. Il prezzo scende a ogni
  medaglia in più:
  - Abilità e IV: 10.000 − 2.000 per medaglia oltre la 4ª
  - Natura: 15.000 − 3.000 per medaglia oltre la 4ª
  - EV: 35.000 − 5.000 per medaglia oltre la 4ª
  - Dopo aver battuto la Lega è tutto **gratis**.
- **Nuovi comandi script:** sostituiscono comandi inutilizzati dell'originale, con
  gli stessi ID:

  | ID | Prima | Ora |
  |----|-------|-----|
  | 0x09C | `ScrCmd_Unused_09C` | `ScrCmd_GetGuruPrice` |
  | 0x09D | `ScrCmd_Unused_09D` | `ScrCmd_CheckAbilitySwap` |
  | 0x09E | `ScrCmd_Unused_09E` | `ScrCmd_ApplyAbilitySwap` |
  | 0x09F | `ScrCmd_Unused_09F` | `ScrCmd_CheckNatureSwap` |
  | 0x0A0 | `ScrCmd_Dummy0A0` | `ScrCmd_ApplyNatureSwap` |
  | 0x23F | `ScrCmd_Dummy23F` | `ScrCmd_ResetEVs` |
  | 0x240 | `ScrCmd_Dummy240` | `ScrCmd_SetIV` |
  | 0x241 | `ScrCmd_Dummy241` | `ScrCmd_ModifyEV` |
  | 0x242 | `ScrCmd_Dummy242` | `ScrCmd_ProcessGuruMoney` |

- **File:** `res/field/events/events_pastoria_city_east_house.json`,
  `res/field/scripts/scripts_pastoria_city_east_house.s` (macro `GuruCmd_*` e
  script `_GuruScript`), `res/text/pastoria_city_east_house.json` (testi in inglese),
  `include/data/scripts/scrcmd.h`, `src/scrcmd.c`, `src/scrcmd_party.c`
- **Commit:** `ff94d873a` Guru lotta

### Insegnamosse: soldi invece della Squama Cuore
- **Cosa cambia:** l'Insegnamosse di Pratopoli non chiede più una Squama Cuore ma
  **5.000 ₽** per mossa. Usa il comando `ProcessGuruMoney`.
- **File:** `res/field/scripts/scripts_pastoria_city_east_house.s`,
  `res/text/pastoria_city_east_house.json`
- **Commit:** `ab7c49e63` Ricordamosse a 5000

### MN usabili dalla Borsa
- **Cosa cambia:** quando uno script controlla se un Pokémon conosce Taglio, Forza,
  Spaccaroccia, Cascata o Scalaroccia, basta avere la **MN corrispondente nella
  Borsa**: lo script crede che il primo Pokémon della squadra conosca la mossa.
  Volo, Surf e Scacciabruma non sono incluse e funzionano come nell'originale.
- **Effetto nel gioco:** non serve un "HM slave" per gli ostacoli sul campo.
- **File:** `src/scrcmd_party.c` (`ScrCmd_CheckPartyMonHasMove`,
  `ScrCmd_FindPartySlotWithMove`)
- **Commit:** `ff94d873a`

---

## 4. Strumenti e negozi

### Nuovo strumento: Tenda (`ITEM_TENT`)
- **Cosa cambia:** nuovo strumento curativo nella tasca Medicine, costo 500 ₽.
  Cura PS, PP e stati e rianima **tutta la squadra**. Ha un'icona nuova.
  Il menu squadra è stato modificato perché la cura venga applicata a tutti i
  Pokémon presenti, non solo agli esausti.
- **Dove si compra:** in tutti i Pokémon Market fin dall'inizio.
- **File:** `res/items/data/tent.json`, `res/items/icons/tent.png`/`.pal`,
  `res/items/icons/meson.build`, `res/items/item_icon.order`, `generated/items.txt`,
  `include/data/mart_items.h`, `src/applications/party_menu/unk_02084B70.c`
- **Commit:** `e623f2622` Tenda fatta

### Merce nei negozi
- **Market comuni:** Tenda (da 0 medaglie) e **Caramella Rara** (da 3 medaglie).
- **Cuoripoli (Hearthome):** tutte le 9 pietre evolutive (Pietrasolare, Pietralunare,
  Pietrafocaia, Pietratuono, Pietraidrica, Pietrafoglia, Pietrabrillo, Pietraoscura,
  Pietralbore).
- **Pratopoli (Pastoria):** Coccio Rosso, Blu, Giallo e Verde.
- **Lega Pokémon:** Elisir Max e Revitalizzante Max.
- **File:** `include/data/mart_items.h`
- **Commit:** `5ed53d14f` Added items to marts, `e623f2622`

### Prezzi modificati
| Strumento | Originale | PPE |
|-----------|-----------|-----|
| Caramella Rara | 4.800 | **1.000** |
| Pietre evolutive (9) | 2.100 | **4.000** |
| Cocci (4) | 200 | **100** |
| Elisir Max | 4.500 | **5.000** |
| Revitalizzante Max | 4.000 | **3.000** |

- **File:** `res/items/data/*.json`
- **Commit:** `f6691cc1d` (prezzi riportati nel nuovo formato JSON dopo il merge di giugno)

---

## 5. Interfaccia

### Nature colorate nel riepilogo Pokémon
- **Effetto nel gioco:** nella pagina statistiche la statistica aumentata dalla
  natura è in **rosso**, quella diminuita in **blu**. Con nature neutre restano nere.
- **File:** `src/applications/pokemon_summary_screen/window.c`
- **Commit:** `6611df2f8` Nature colorate

### Opzioni predefinite
- Velocità testo di default: **Veloce** (era Normale).
- Stile lotta: **Fisso**, bloccato (vedi Lotte).
- **File:** `src/game_options.c`

---

## 6. Build e repository (non toccano il gioco)

- `.gitignore`: aggiunti `*.nds` e `.claude/settings.local.json`.
- `.claude/settings.local.json` è stato committato prima della regola, quindi è ancora tracciato.
- Submodule `subprojects/narc` aggiunto (`f6691cc1d`).
- **Rumore da Windows, senza effetti sul gioco:**
  - Circa 60 script in `tools/`, `.github/` e `replace.sh` hanno perso il permesso di
    esecuzione (755 → 644, commit `b160815e5`). `get_metroskrew.sh` è stato ripristinato
    il 2026-11-13.
  - 14 file binari in `res/trainers/classes/` cambiano solo nel modo.
  - Il commit `bb3ac00e1` (wip, 25/06/2026) ha convertito in CRLF `Makefile`, molti
    `.order` in `res/`, i template di `tools/dataproc/data/`, le patch yyjson e la doc
    di nitroarc. Contenuto identico, cambiano solo i fine riga.

---

## Storico commit PPE

| Data | Commit | Autore | Descrizione |
|------|--------|--------|-------------|
| 2026-02-20 | `b160815e5` | Enrico | Base (prima versione level cap, permessi file) |
| 2026-02-23 | `e8b0a26a5` | Enrico | Level cap |
| 2026-02-23 | `98bb08cdb` | Enrico | Level cap finito |
| 2026-02-23 | `8191212f0` | Enrico | Niente strumenti contro allenatori |
| 2026-03-14 | `ff94d873a` | Enrico | Guru Lotta, MN da Borsa, opzioni |
| 2026-03-14 | `7c3bee859` | Enrico | Condividi Esp. globale |
| 2026-03-14 | `6611df2f8` | Enrico | Nature colorate |
| 2026-03-14 | `ab7c49e63` | Enrico | Insegnamosse a 5.000 ₽ |
| 2026-06-22 | `5ed53d14f` | Enrico | Strumenti nei negozi |
| 2026-06-22 | `75d52ff64` | Enrico | Merge da origin/main (fino a #1216) |
| 2026-06-22 | `f6691cc1d` | Enrico | Adattamento al merge: strumenti e scheda allenatore |
| 2026-06-22 | `c384801d5` | Enrico | EV allenatori |
| 2026-06-22 | `852161878` | Enrico | Nature e abilità allenatori |
| 2026-06-25 | `839c9b4c9` | Enrico | `.gitignore` (`*.nds`) |
| 2026-06-25 | `bb3ac00e1` | meisbest | wip (solo fine riga CRLF) |
| 2026-11-13 | `e623f2622` | Enrico | Tenda |
