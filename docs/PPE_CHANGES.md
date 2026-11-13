# PPE — Modifiche rispetto a pret/pokeplatinum

- **Ultimo aggiornamento:** 2026-11-13
- **Allineato a origin/main:** `62bdaacdc` "tools: Make scripts and events easier to
  work with by hand (#1332)" del 06/11/2026. Importato con il primo `/aggiorna` del 2026-11-13.
- **Come vedere le modifiche PPE:** `git diff origin/main...ppe`, con tre puntini.
  Funziona dal merge ponte del 2026-11-13 in poi: vedi [CLAUDE.md](../CLAUDE.md).

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
  `src/applications/trainer_case/card_text.c`. Il salvataggio si legge con
  `SaveData_Ptr()` di upstream. Fino al 2026-11-13 c'era un nostro puntatore
  globale `gSaveDataPtr` in `src/savedata.c`, ora tolto: `savedata.c` è come upstream.
- **Condizioni (dal più avanzato):**

  | Cap | Condizione |
  |-----|------------|
  | 100 | `VAR_STARK_MOUNTAIN_ROOM_1_STATE ≥ 1`: Martes e Giove battuti nella Stanza 1 del Monte Ostile |
  | 85 | `VAR_FIGHT_AREA_STATE ≥ 2`: Corrado e Vulcano battuti nell'Area Lotta |
  | 77 | `SystemFlag_CheckGameCompleted`: Lega battuta (Hall of Fame) |
  | 75 | `VAR_RIVAL_BEAT_SUNYSHORE_GYM ≥ 1`: rivale al Centro Pokémon della Lega |
  | 73 | medaglia Faro (Volkner) |
  | 71 | `VAR_DISTORTION_WORLD_PROGRESS ≥ 10` (Cyrus, Mondo Distorto) |
  | 68 | `VAR_SPEAR_PILLAR_STATE ≥ 2` / `VAR_GALACTIC_HQ_CONTROL_ROOM_STATE ≥ 1` |
  | 65 | `VAR_GALACTIC_HQ_4F_STATE ≥ 1` / medaglia Ghiacciolo |
  | 63 | `VAR_LAKE_ACUITY_STATE ≥ 1`: Martes al Lago Verità |
  | 60 | `FLAG_DEFEATED_COMMANDER_SATURN_VALOR_CAVERN`: Saturno al Lago Valore |
  | 59 | medaglia Miniera (Byron) |
  | 58 / 54 | `VAR_CANALAVE_CITY_STATE ≥ 1` / `VAR_CELESTIC_TOWN_STATE ≥ 1` |
  | 52 | medaglia Palude (Wake) |
  | 47 | `VAR_PASTORIA_CITY_STATE ≥ 2`: rivale a Pratopoli |
  | 45 | medaglia Roccia (Maylene) |
  | 42 | `VAR_ROUTE_209_GATE_TO_HEARTHOME_CITY_STATE ≥ 2`: rivale a Cuoripoli |
  | 37 | medaglia Reliquia (Fantina) |
  | 36 | `VAR_ETERNA_CITY_STATE ≥ 3` (Giove) |
  | 30 | medaglia Foresta (Gardenia) |
  | 27 | `VAR_VALLEY_WINDWORKS_STATE ≥ 2` (Martes) |
  | 21 | medaglia Carbone (Roark) |
  | 16 | `VAR_ROUTE_203_RIVAL_STATE ≥ 1`: rivale al Percorso 203 |
  | 10 | default |

- **Corretto il 2026-11-13:**
  - La condizione dell'85 usava per errore il flag 318, cioè Saturno al Lago
    Valore: il cap saltava a 85 a metà gioco.
  - La condizione del 77 usava `0x40F4` (`VAR_DUMMY_0x40F4`), una variabile che il
    gioco non imposta mai.
  - Tutti i numeri grezzi sono stati sostituiti con i nomi di `generated/vars_flags.h`.
    Se upstream rinomina una variabile, la build fallisce invece di rompere il cap in silenzio.
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
- **Cosa cambia:** `BATTLE_TYPE_NO_ITEMS` include anche `BATTLE_TYPE_TRAINER`, e il
  controllo della Borsa in lotta (`case PLAYER_INPUT_ITEM` in
  `battle_controller_player.c`) usa `BATTLE_TYPE_NO_ITEMS`.
- **Effetto nel gioco:** in tutte le lotte contro allenatori, comprese doppie, in
  coppia e rivincite, premendo Borsa compare "Items can't be used here.". Le lotte
  contro Pokémon selvatici non cambiano, quindi le Poké Ball si usano normalmente.
  Gli strumenti usati dall'IA avversaria non sono toccati.
- **Storia:** nel merge di giugno upstream aveva sostituito il controllo con
  `BATTLE_TYPE_FRONTIER_LINK` e la modifica non era più attiva. È stata ripristinata
  il 2026-11-13.
- **File:** `include/constants/battle.h`, `src/battle/battle_controller_player.c`
- **Commit:** `8191212f0` No items if trainer, ripristino del 2026-11-13

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
    Il PID viene corretto solo nella parte alta (a passi di 256), così sesso e slot
    abilità restano quelli dell'originale. Fino al 2026-11-13 si cambiava il byte
    basso: un Pokémon di un'allenatrice di specie 50/50 poteva diventare maschio e
    l'abilità poteva cambiare.
  - `ability`: `0` o `1` per lo slot abilità, oppure `null` per il default. Con `1`
    su una specie che ha una sola abilità si usa quella (prima del 2026-11-13 il
    Pokémon restava senza abilità). Le abilità tengono conto della forma.
  - Il tool di build `trainerproc` (funzione `PackParty`) legge i campi, e le struct
    `TrainerMon*` hanno `evFlags`, `nature` e `abilitySlot`, prima di `item`.
- **Stato dei dati:** tutti i **927 file** in `res/trainers/data/` hanno i nuovi
  campi, ma con i valori di default (tutto `false`/`null`). Quindi per ora nel gioco
  **non cambia nulla**: c'è l'infrastruttura, i valori vanno ancora decisi.
- **Verificato il 2026-11-13**, dopo la riscrittura di `trainerproc`:
  1. valori di prova nel JSON di Roark (attack+speed, `NATURE_ADAMANT`, slot 1);
  2. decodifica di `build/res/trainers/trpoke.narc`, risultato `evFlags=0x0A nature=3 abilitySlot=1`;
  3. JSON ripristinato.
- **File:** `include/struct_defs/trainer_data.h`, `src/trainer_data.c`,
  `tools/dataproc/src/trainerproc.c`, `res/trainers/data/*.json`
- **Commit:** `c384801d5` Aggiunta EVs nemici, `852161878` Nature e abilità decise a
  mano; riportato sulla nuova API di dataproc nel merge del 2026-11-13.

---

## 3. Personaggi e script (Pratopoli)

### Guru Lotta (nuovo NPC)
- **Cosa cambia:** nella casa est di Pratopoli c'è un nuovo NPC `LOCALID_GURU_2`
  (aspetto da scienziato, posizione x=4, z=3, rivolto a sud, script `_GuruScript`)
  con il menu "Guru's Dojo":
  - **Cambia Abilità**: passa all'altra abilità della specie, se ne ha due. Il PID
    viene rigenerato con il bit 0 giusto (stessa natura, sesso e cromaticità), così
    l'abilità resta anche dopo l'evoluzione. Dal 2026-11-13: prima veniva scritta
    solo `MON_DATA_ABILITY` e l'evoluzione la riportava a quella originale.
  - **Cambia Natura**: scegli la natura da un elenco. Il PID viene rigenerato
    mantenendo sesso, abilità attuale e cromaticità.
  - Effetto collaterale noto: cambiare il PID può cambiare l'evoluzione di Wurmple
    (Silcoon/Cascoon dipende dal PID).
  - Se il servizio è gratis (Campione), il cambio natura ha un testo suo con il nome
    della natura scelta (`PastoriaCityEastHouse_Text_GuruChampionFreeNature`). Prima
    riusava il testo dell'abilità.
  - Fino al 2026-11-13, premere B nella lista delle nature e poi confermare (o
    essere Campione) **bloccava il gioco**: lo script non riconosceva `MENU_CANCEL`
    e chiedeva una natura inesistente. Ora B torna al menu e il comando C rifiuta
    nature `>= NATURE_COUNT`.
  - **Imposta IV** (0–31 per statistica)
  - **Imposta EV**: aggiunge o toglie EV, con limite di 252 per statistica e 510 in totale
    (`MAX_EVS_ALL_STATS`). 252 e non 255 (`MAX_EVS_SINGLE_STAT`) è voluto: oltre 252 non si guadagna nulla.
  - Nello script le statistiche sono indicate con `STAT_*`.
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

  `0x242` (`ScrCmd_ProcessGuruMoney`) è stato tolto il 2026-11-13: i soldi si gestiscono
  con i comandi upstream `CheckMoney2` / `RemoveMoney2`, e lo slot è tornato `ScrCmd_Dummy242`.

- **File:** `res/field/maps/pastoria_city_east_house/events.json` (NPC),
  `res/field/maps/pastoria_city_east_house/scripts.s` (script `_GuruScript`),
  `res/text/pastoria_city_east_house.json` (testi in inglese, ID
  `PastoriaCityEastHouse_Text_Guru*`), `include/data/scripts/scrcmd.h` (tabella dei
  comandi), `asm/macros/scrcmd.inc` (macro `GetGuruPrice`, `CheckAbilitySwap`,
  `ApplyAbilitySwap`, `CheckNatureSwap`, `ApplyNatureSwap`, `ResetEVs`, `SetIV`,
  `ModifyEV`, al posto delle vecchie macro `ScrCmd_Unused_09C`…`Dummy0A0` e
  `Dummy23F`…`Dummy241`), `include/constants/scrcmd.h` (costanti `GURU_*`, blocco PPE
  in fondo), `include/scrcmd_party.h` (prototipi), `src/scrcmd_party.c` (codice).
  `src/scrcmd.c` è identico a upstream. Le funzioni `ScrCmd_Unused_09C`…`09F`,
  `ScrCmd_Dummy0A0` e `ScrCmd_Dummy23F`…`241` di upstream restano nel codice ma non
  sono più nella tabella: si lasciano lì apposta, per non allargare il diff.
- **Commit:** `ff94d873a` Guru lotta. Adattato nel merge del 2026-11-13 ai nuovi
  percorsi e al nuovo formato degli eventi (script per etichetta, direzione `DIR_*`,
  id `LOCALID_*`) e ai nuovi nomi dei suoni (`SE_CONFIRM_sseq_3`). Dal 2026-11-13 il
  suono di conferma usa `PlaySE` invece di `PlayFanfare`, che metteva in pausa la musica
  senza il `WaitFanfare` che la fa ripartire.

### Insegnamosse: soldi invece della Squama Cuore
- **Cosa cambia:** l'Insegnamosse di Pratopoli non chiede più una Squama Cuore ma
  **5.000 ₽** per mossa. Usa i comandi upstream `GoToIfNotEnoughMoney` e `RemoveMoney`.
- **File:** `res/field/maps/pastoria_city_east_house/scripts.s`,
  `res/text/pastoria_city_east_house.json`
- **Commit:** `ab7c49e63` Ricordamosse a 5000

### MN usabili dalla Borsa
- **Cosa cambia:** quando uno script controlla se un Pokémon conosce Taglio, Forza,
  Spaccaroccia, Cascata o Scalaroccia, basta avere la **MN corrispondente nella
  Borsa**: lo script crede che un Pokémon della squadra conosca la mossa. Se qualcuno
  la conosce davvero, la usa lui; altrimenti la usa il primo Pokémon che non è un
  Uovo (fino al 2026-11-13 era sempre lo slot 0, anche se era un Uovo).
  Volo, Surf e Scacciabruma non sono incluse e funzionano come nell'originale.
- **Effetto nel gioco:** non serve un "HM slave" per gli ostacoli sul campo.
- **File:** `src/scrcmd_party.c` (`HMFromBag_GetItemForMove`, `ScrCmd_CheckPartyMonHasMove`,
  `ScrCmd_FindPartySlotWithMove`)
- **Commit:** `ff94d873a`

---

## 4. Strumenti e negozi

### Nuovo strumento: Tenda (`ITEM_TENT`)
- **Cosa cambia:** nuovo strumento curativo nella tasca Medicine, costo 500 ₽.
  Cura PS, PP e stati e rianima **tutta la squadra**. Ha un'icona nuova.
  Passa per il percorso della Cenere Sacra (`reviveAll`). Il menu squadra è stato
  modificato perché la cura venga applicata a tutti i Pokémon presenti, non solo
  agli esausti, con una protezione sul conteggio dei PS per chi è già al massimo.
- **Dove si compra:** in tutti i Pokémon Market fin dall'inizio.
- **File:** `res/items/data/tent.json` (con `battleUseCategory`), `res/items/icons/tent.png`
  e `tent.pal` (nel repo in LF, sul disco in CRLF come tutte le palette:
  `*.pal eol=crlf`), `res/items/icons/meson.build`, `res/items/item_icon.order`,
  `generated/items.txt`, `include/data/mart_items.h`,
  `src/applications/party_menu/callbacks.c` (`IsFullPartyHealItem`,
  `FindNextPresentMember`, `FindNextReviveAllTarget`, usati da `PartyMenuCB_HandleSacredAsh`)
- **Commit:** `e623f2622` Tenda fatta. Riportata nel merge del 2026-11-13 da
  `unk_02084B70.c`, eliminato upstream, a `callbacks.c`.

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

- `.gitignore`: in fondo c'è un blocco `# PPE` con `*.nds` e `.claude/settings.local.json`.
- `.claude/settings.local.json` era stato committato prima della regola. Dal
  2026-11-13 non è più tracciato e ognuno tiene il proprio in locale.
- Aggiunti `CLAUDE.md`, `docs/PPE_CHANGES.md` e i comandi `/salva` e
  `/aggiorna` in `.claude/commands/`.
- `docs/SETUP.md`: guida da zero su Windows per un nuovo collaboratore (WSL 2,
  VS Code con estensione WSL e Claude Code, `gh`, clone con i remote giusti, prima
  build, uso quotidiano, troubleshooting). Sostituisce il vecchio `SETUP.md` in root.
- Submodule `subprojects/narc` aggiunto (`f6691cc1d`).
- `.gitattributes`: in fondo c'è un blocco PPE che forza LF per `*.template`,
  `*.patch`, `*.adoc`, `*.csv` e `Makefile`. Non include `*.order` e `*.txt`,
  perché alcuni file upstream di quei tipi sono in CRLF. Non tocca `*.pal`, che upstream
  forza in CRLF.
- Il 2026-11-13 il "rumore da Windows" accumulato (permessi 755 → 644 e fine riga CRLF
  su ~100 file) è stato riportato alla versione upstream con il commit
  `d454b1d66` "Normalizza fine riga e permessi".
- Da origin/main `62bdaacdc` meson è alla 1.12.0. Dopo `make update` può servire
  `make distclean` (vedi Troubleshooting in CLAUDE.md).

---

## Punti di aggancio con upstream

Simboli, file e formati di upstream da cui dipendono le modifiche PPE. Servono
per i **controlli semantici** di `/aggiorna`: se upstream rinomina, sposta o
riscrive uno di questi, la modifica corrispondente va verificata.
**Aggiorna questa tabella quando aggiungi o cambi una modifica.**
Ultima verifica: 2026-11-13, su origin/main `62bdaacdc`.

| Modifica PPE | Simboli upstream usati / modificati | File |
|--------------|-------------------------------------|------|
| Level cap | `VarsFlags_GetVarAddress`, `VarsFlags_CheckFlag`, `SaveData_GetVarsFlags`, `SaveData_GetTrainerInfo`, `TrainerInfo_HasBadge`, `BADGE_ID_*`, `SystemFlag_CheckGameCompleted`, `generated/vars_flags.h` (`VAR_STARK_MOUNTAIN_ROOM_1_STATE`, `VAR_FIGHT_AREA_STATE`, `VAR_RIVAL_BEAT_SUNYSHORE_GYM`, `VAR_DISTORTION_WORLD_PROGRESS`, `VAR_SPEAR_PILLAR_STATE`, `VAR_GALACTIC_HQ_*`, `VAR_LAKE_ACUITY_STATE`, `FLAG_DEFEATED_COMMANDER_SATURN_VALOR_CAVERN`, `VAR_CANALAVE_CITY_STATE`, `VAR_CELESTIC_TOWN_STATE`, `VAR_PASTORIA_CITY_STATE`, `VAR_ROUTE_209_GATE_TO_HEARTHOME_CITY_STATE`, `VAR_ETERNA_CITY_STATE`, `VAR_VALLEY_WINDWORKS_STATE`, `VAR_ROUTE_203_RIVAL_STATE`) e **il significato** dei loro valori negli script delle mappe, `Pokemon_GetSpeciesBaseExpAt`, `ITEM_PARAM_LEVEL_UP`, `Pokemon_CheckItemEffects`, `Pokemon_ApplyItemEffects`, `Daycare_MoveToPartyFromDaycareMon`, `BoxPokemon_GiveExperience`, `ov5_021E63E0`, `DaycareMon_GetSteps`, `SaveData_Ptr` | `src/field_battle_data_transfer.c`, `include/field_battle_data_transfer.h`, `src/item_use_pokemon.c`, `src/overlay005/daycare.c`, `src/battle/battle_script.c`, `src/applications/trainer_case/card_text.c` |
| Level cap in battaglia + Condividi Esp. | `BtlCmd_CalcExpGain`, `BattleScript_GetExpTask`, `sideGetExpMask`, `gainedExp`/`sharedExp`, `HOLD_EFFECT_EXP_SHARE`, `MON_DATA_IS_EGG` | `src/battle/battle_script.c` |
| Level cap nella Scheda Allenatore | `TrainerCard_DrawFrontText`, `TRAINER_CARD_WINDOW_SCORE`, `TrainerCard_DrawNumber`, `TRAINER_CARD_WINDOW_PARTIAL_WIDTH`, `Font_CalcStringWidth` | `src/applications/trainer_case/card_text.c` |
| Niente strumenti contro allenatori | `BATTLE_TYPE_NO_ITEMS`, `BATTLE_TYPE_TRAINER`, il controllo in `case PLAYER_INPUT_ITEM`. Upstream lo riscrive facilmente: ricontrollare dopo ogni merge che usi ancora `BATTLE_TYPE_NO_ITEMS` | `include/constants/battle.h`, `src/battle/battle_controller_player.c` |
| Stile lotta Fisso, testo veloce | `Options_Init`, `Options_BattleStyle`, `OPTIONS_BATTLE_STYLE_SET`, `OPTIONS_TEXT_SPEED_FAST`, `ProcessMainInput`, `ENTRY_BATTLE_STYLE`, `ENTRY_CLOSE` | `src/game_options.c`, `src/applications/options_menu.c` |
| EV, natura e abilità degli allenatori | `TrainerMonBase`/`WithMoves`/`WithItem`/`WithMovesAndItem` (layout: i nostri campi **prima di** `item`, perché `PackParty` copia fino a `offsetof(item)`), `TrainerData_BuildParty`, `Pokemon_InitWith`, il valore di `genderMod` (120/136) e la regola sesso = byte basso del PID, `SpeciesData_GetFormValue`, `ABILITY_NONE`, `NATURE_COUNT`, `SPECIES_DATA_ABILITY_1/2`, `MON_DATA_*_EV`; in trainerproc: `PackParty`, `LOOKUP_ENUM`, `dp_try`, `dp_missing`, `dp_type_of`, `DP_STRING`/`DP_INT`, `dp_bool`, `dp_lookup`, `dp_u8`; formato JSON degli allenatori | `include/struct_defs/trainer_data.h`, `src/trainer_data.c`, `tools/dataproc/src/trainerproc.c`, `tools/dataproc/lib/include/dataproc.h`, `res/trainers/data/*.json` |
| Guru Lotta: comandi script | ID `0x09C`–`0x0A0` e `0x23F`–`0x241` in `scrcmd.h` (se upstream li rinomina o li **riusa**, il Guru si rompe), `ScriptContext_GetVar`/`GetVarPointer`/`ReadHalfWord`, `FieldSystem_GetScriptMemberPtr`, `SCRIPT_MANAGER_STR_TEMPLATE`, `StringTemplate_SetAbilityName`, `LCRNG_Next`, `Pokemon_CalcAbility` (abilità dal bit 0 del PID: se upstream cambia questa regola, va rivisto `Guru_SetPersonality`), `SpeciesData_GetFormValue`, `Pokemon_GetNature`, `Pokemon_GetNatureOf`, `Pokemon_IsPersonalityShiny`, `SpeciesData_FromMonSpecies`/`GetGenderOf`/`Free`, `ABILITY_NONE`, `STAT_*` e l'ordine di `MON_DATA_HP_EV`…`SPDEF_EV` / `MON_DATA_HP_IV`…`SPDEF_IV` (uguale a `STAT_*`), `MAX_IVS_SINGLE_STAT`, `MAX_EVS_ALL_STATS`, `NATURE_COUNT`, `TrainerInfo_IsMainStoryCleared`, `TrainerInfo_BadgeCount` | `include/data/scripts/scrcmd.h`, `include/scrcmd_party.h`, `include/constants/scrcmd.h`, `src/scrcmd_party.c` |
| Guru Lotta: script, testi, NPC | le nostre macro in `asm/macros/scrcmd.inc` (al posto di `ScrCmd_Unused_09C`…`Dummy0A0`, `Dummy23F`…`Dummy241`: se upstream le rinomina, conflitto lì) con le costanti `SCRCMD_*`, comandi upstream `CheckMoney2`, `RemoveMoney2`, `GoToIfNotEnoughMoney`, `RemoveMoney`, `InitLocalTextListMenu`, `WaitABPadPress`, `AddListMenuEntry`, `ShowYesNoMenu`, `GoToIfLt`, `PlaySE`, costanti `MENU_YES`, `MENU_CANCEL`, `PARTY_SLOT_NONE`, `STAT_*`, nomi dei suoni (`SE_CONFIRM_sseq_3`), label `PastoriaCityEastHouse_*` e `_GuruScript`, `ScriptEntry`, `FLAG_TALKED_TO_PASTORIA_CITY_EAST_HOUSE_MOVE_MANIAC`, ID testi `PastoriaCityEastHouse_Text_*` (i nostri sono `..._Text_Guru*`), formato `events.json` (id `LOCALID_*`, `"script"` per etichetta, `"initial_dir"` = `DIR_*`) | `res/field/maps/pastoria_city_east_house/scripts.s`, `res/field/maps/pastoria_city_east_house/events.json`, `res/text/pastoria_city_east_house.json`, `asm/macros/scrcmd.inc` |
| MN dalla Borsa | `ScrCmd_CheckPartyMonHasMove`, `ScrCmd_FindPartySlotWithMove`, `Bag_GetItemQuantity`, `SaveData_GetBag`, `ITEM_HM01`–`ITEM_HM08`, `ITEM_NONE`, uso dello slot in `res/field/scripts/field_moves.s` (`BufferPartyMonNickname`, `PlayHMCutIn`), `MOVE_CUT`/`STRENGTH`/`ROCK_SMASH`/`WATERFALL`/`ROCK_CLIMB` | `src/scrcmd_party.c` |
| Tenda | schema JSON degli strumenti (`itemUseParams`, `fieldUseFunc`, `ITEM_USE_FUNC_HEALING`, `battleUseCategory`/`BATTLE_USE_CATEGORY_*`), `ITEM_PARAM_REVIVE_ALL`, `ITEM_PARAM_HEAL_SLEEP`, `CheckItemSacredAsh`, `PartyMenuCB_HandleSacredAsh`, `FindNextFaintedMember`, `SACREDASH_STATE_*`, `HEAP_ID_PARTY_MENU`, formato palette JASC (CRLF, `nitrogfx`) | `res/items/data/tent.json`, `res/items/icons/*`, `res/items/icons/meson.build`, `res/items/item_icon.order`, `generated/items.txt`, `src/applications/party_menu/callbacks.c` |
| Negozi e prezzi | `PokeMartCommonItems`, `HearthomeMartSpecialties`, `PastoriaMartSpecialties`, `PokemonLeagueMartSpecialties`, `ITEM_*`, campo `price` dei JSON | `include/data/mart_items.h`, `res/items/data/*.json` |
| Blocco PPE in `.gitignore` e `.gitattributes` | file di upstream, i nostri blocchi sono in fondo. `.gitattributes`: non aggiungere pattern già regolati da upstream (`*.pal`) | `.gitignore`, `.gitattributes` |
| Nature colorate | `DrawSkillsPageWindows`, `SUMMARY_TEXT_RED`/`BLUE`/`BLACK`, `SUMMARY_WINDOW_*`, `monData.nature` | `src/applications/pokemon_summary_screen/window.c` |

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
| 2026-11-13 | `4da947f79` | Enrico | Documentazione di gestione (CLAUDE.md, PPE_CHANGES.md, /salva, /aggiorna) |
| 2026-11-13 | `783cb676c` | Enrico | Controlli semantici, settings.local.json non tracciato |
| 2026-11-13 | `61d6d28a0` | Enrico | Merge ponte con `b2cd286f3` (nessun file cambiato) |
| 2026-11-13 | `d454b1d66` | Enrico | Normalizza fine riga e permessi, blocco LF in `.gitattributes` |
| 2026-11-13 | `491f0364e` | Enrico | Merge di origin/main `62bdaacdc` (121 commit), con Tenda, trainerproc e Guru adattati |
| 2026-11-13 | `b8951696d` | Enrico | Level cap: condizioni 85 e 77 corrette, nomi al posto dei numeri. Borsa bloccata contro gli allenatori |
| 2026-11-13 | `7e99c688a` | Enrico | Guida di setup per collaboratori su Windows (`docs/SETUP.md`) |
| 2026-11-13 | `0062765cd` | Enrico | Guru: cambio abilità che resta dopo l'evoluzione |
| 2026-11-13 | `fa9b9fc7b` | Enrico | Guru e level cap: funzioni e comandi upstream al posto dei nostri doppioni |
| 2026-11-13 | `24b75cc95` | Enrico | Guru: B nella lista delle nature non blocca più il gioco |
| 2026-11-13 | `121e1d2d3` | Enrico | Guru: macro con nome, costanti, testi con nome, `PlaySE` |
| 2026-11-13 | `3711abbad` | Enrico | Guru: testo corretto per il cambio natura gratuito |
| 2026-11-13 | `c2c3cd096` | Enrico | MN dalla Borsa: mai un Uovo come Pokémon che usa la mossa |
| 2026-11-13 | `07271878b` | Enrico | Allenatori: la natura dal JSON non cambia più sesso e abilità |
| 2026-11-13 | (questo) | Enrico | Allenatori: `ability: 1` su specie con una sola abilità |
