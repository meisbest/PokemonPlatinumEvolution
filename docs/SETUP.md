# Setup da zero su Windows — Pokémon Platinum Evolution (PPE)

Guida passo passo per chi deve iniziare a lavorare su PPE da un PC Windows
senza nulla installato. Alla fine avrai:

- Ubuntu dentro Windows (WSL 2) con tutto il necessario per compilare la ROM;
- VS Code su Windows collegato a Ubuntu, con Claude Code;
- il fork `meisbest/PokemonPlatinumEvolution` clonato sul branch `ppe`;
- la prima ROM compilata.

Le regole del progetto (remote, branch, cosa non committare) sono in
[CLAUDE.md](../CLAUDE.md). La guida ufficiale di pret, in inglese, è
[INSTALL.md](../INSTALL.md).

> **Regola d'oro:** tutto (git, build, Claude Code) si fa **dentro Ubuntu**.
> Windows serve solo a mostrare la finestra di VS Code.

---

## 1. WSL con Ubuntu e dipendenze di build

### WSL 1 o WSL 2? → WSL 2

INSTALL.md descrive entrambe le versioni. Nel nostro caso va usato **WSL 2**,
perché il progetto sta **nella home di Ubuntu** (`~/pokeplatinum`), cioè nel
filesystem Linux:

- WSL 2 è una vera macchina virtuale Linux con un disco ext4 proprio. Leggere e
  scrivere file nella home di Ubuntu è veloce quanto su Linux nativo. Una build
  completa tocca decine di migliaia di file, e qui la differenza si sente.
- WSL 1 traduce ogni accesso ai file in chiamate Windows. È più veloce solo
  quando i file stanno sul disco di Windows (`/mnt/c/...`). È per questo che
  INSTALL.md dice che con WSL 1 il repo va su `C:`. Noi invece non teniamo nulla
  lì: con WSL 1 una build nella home sarebbe molto più lenta.
- Anche pret consiglia WSL 2, e l'estensione WSL di VS Code funziona
  perfettamente con WSL 2.

### Installazione

1. Dal menu Start cerca **Windows PowerShell**, tasto destro →
   **Esegui come amministratore**.
2. Esegui:
   ```powershell
   wsl --install -d Ubuntu
   ```
   Se è la prima volta che installi WSL, Windows chiede di riavviare: fallo.
   Al riavvio si apre Ubuntu e chiede di creare **nome utente e password Linux**
   (la password serve per `sudo`; mentre la scrivi non si vede nulla, è normale).
3. Controlla che Ubuntu sia in versione 2 (sempre da PowerShell):
   ```powershell
   wsl -l -v
   ```
   Nella colonna `VERSION` deve esserci `2`. Se c'è `1`:
   ```powershell
   wsl --set-version Ubuntu 2
   ```
4. Da qui in poi lavora nel terminale **Ubuntu** (menu Start → *Ubuntu*). Il
   prompt deve essere del tipo `nome@PC:~$`, non `PS C:\...>`.
5. Aggiorna il sistema e installa le dipendenze di build:
   ```bash
   sudo apt update && sudo apt upgrade -y
   sudo dpkg --add-architecture i386 && sudo apt update
   sudo apt install bison flex g++ gcc-arm-none-eabi git make ninja-build pkg-config wget python3 xz-utils nasm libc6:i386 libpng-dev
   ```
6. Configura il tuo nome per i commit (usa il tuo utente GitHub e la tua email):
   ```bash
   git config --global user.name "TuoUtenteGitHub"
   git config --global user.email "tua@email"
   ```

---

## 2. VS Code su Windows con l'estensione WSL

1. Installa [VS Code](https://code.visualstudio.com/) **su Windows**, normalmente.
2. In VS Code apri le estensioni (`Ctrl+Shift+X`), cerca **WSL** (di Microsoft) e
   installala.
3. Il progetto lo apri **sempre dal terminale Ubuntu**, dopo averlo clonato
   (passo 4):
   ```bash
   cd ~/pokeplatinum
   code .
   ```
   La prima volta VS Code installa da solo un piccolo "server" dentro Ubuntu.
4. **Verifica:** in basso a sinistra, sul riquadro colorato, deve esserci
   **`WSL: Ubuntu`**. Se non c'è, stai lavorando lato Windows: chiudi e riapri con
   `code .` da Ubuntu.
5. **Estensioni nel lato WSL.** Le estensioni che lavorano sul codice vanno
   installate *dentro* Ubuntu. Nella lista estensioni, per ognuna premi
   **"Install in WSL: Ubuntu"**. Installa così almeno:
   - **Claude Code** (di Anthropic). Al primo avvio chiede di fare login con il
     tuo account Claude.
   - C/C++ (Microsoft), facoltativa ma comoda.

   Il terminale integrato di VS Code (`Ctrl+ò` o *Terminal → New Terminal*) deve
   aprire `bash` in Ubuntu, con il prompt `nome@PC:~/pokeplatinum$`.

### Regola: mai Git per Windows, mai `\\wsl.localhost`

- **Non installare né usare Git per Windows** (né GitHub Desktop, né il git di
  VS Code lato Windows) su questo repo.
- **Non aprire il progetto da Esplora risorse** con il percorso
  `\\wsl.localhost\Ubuntu\home\...` (né da `\\wsl$`), e non aprirlo in VS Code
  con *File → Open Folder* di Windows.

Perché:

- **Fine riga:** gli strumenti Windows salvano spesso i file con fine riga CRLF.
  Git vede ogni riga come cambiata e i diff diventano enormi e illeggibili. Gli
  script `.sh` con CRLF poi non partono più in Linux.
- **Permessi:** Windows non conosce il bit di esecuzione. Git per Windows e le
  modifiche fatte da Windows fanno perdere il permesso `755` agli script in
  `tools/`, e poi la build fallisce con `Permission denied`. Ci è già successo:
  vedi le stash del 22/06/2026 in CLAUDE.md.
- **Velocità e affidabilità:** `\\wsl.localhost` è una condivisione di rete. Git e
  la build su quel percorso sono lentissimi e possono dare errori strani di lock
  o di file non trovati.
- **Due git sulla stessa cartella** (Windows e Linux) vedono lo stato in modo
  diverso e si confondono a vicenda.

Con `code .` da Ubuntu e `WSL: Ubuntu` in basso a sinistra, VS Code usa il git e
i file di Linux e questi problemi non esistono.

---

## 3. Login a GitHub con `gh`

1. **Accetta l'invito come collaboratore.** Il proprietario del fork ti invia un
   invito su GitHub. Lo trovi nell'email o su
   <https://github.com/meisbest/PokemonPlatinumEvolution/invitations>. Finché
   non lo accetti, il push viene rifiutato.
2. Nel terminale Ubuntu:
   ```bash
   sudo apt install gh
   gh auth login
   ```
   Rispondi così:
   - *Where do you use GitHub?* → `GitHub.com`
   - *Preferred protocol* → `HTTPS`
   - *Authenticate Git with your GitHub credentials?* → `Yes`
   - *How would you like to authenticate?* → `Login with a web browser`.
     Copia il codice di 8 caratteri, apri il link nel browser di Windows, incolla
     il codice e autorizza.
3. Collega git alle credenziali di `gh`:
   ```bash
   gh auth setup-git
   ```
4. Verifica:
   ```bash
   gh auth status
   ```

---

## 4. Clone del fork, branch `ppe` e remote `origin`

```bash
cd ~
git clone https://github.com/meisbest/PokemonPlatinumEvolution pokeplatinum
cd pokeplatinum
git checkout ppe
```

Il clone chiama `origin` il nostro fork. Per il progetto invece `origin` è pret
e il fork si chiama `myfork` (vedi CLAUDE.md, e così si aspettano `/salva` e
`/aggiorna`). Sistema i nomi:

```bash
git remote rename origin myfork
git remote add origin https://github.com/pret/pokeplatinum
git fetch origin
git branch --set-upstream-to=myfork/ppe ppe
```

Verifica con `git remote -v`. Deve risultare:

```
myfork  https://github.com/meisbest/PokemonPlatinumEvolution (fetch)
myfork  https://github.com/meisbest/PokemonPlatinumEvolution (push)
origin  https://github.com/pret/pokeplatinum (fetch)
origin  https://github.com/pret/pokeplatinum (push)
```

`origin` (pret) è **solo lettura**: da lì si fa solo `fetch`, **mai push**. Si
pubblica solo con `git push myfork ppe`. Il branch `main` del fork non si usa.

Ora apri il progetto in VS Code: `code .` (vedi il passo 2).

---

## 5. Prima build

Dal terminale Ubuntu, nella cartella `~/pokeplatinum`:

```bash
make update     # scarica meson locale, metroskrew e i subprojects
make release    # compila la ROM
```

La prima volta ci vuole parecchio. Il risultato è `build/pokeplatinum.us.nds`,
che puoi aprire con un emulatore DS da Windows (in Esplora risorse: `\\wsl.localhost\Ubuntu\home\<utente>\pokeplatinum\build\`,
**solo per copiare la ROM**, non per modificare file).

Note:

- Non usare `make` da solo: esegue anche il controllo SHA1 contro la ROM
  originale, che con un gioco modificato fallisce sempre. È normale.
- Meson è quello locale scaricato da `make update` (in `subprojects/meson-*`),
  non quello di sistema. Non serve installarlo con apt.

### `baserom.nds` serve? No, per la build no

La build **non usa** `baserom.nds`: tutto il gioco (codice e dati) è nel repo.
Il file è letto solo da uno strumento di debug facoltativo,
`tools/asmdiff/asmdiff.sh`, che confronta il codice compilato con quello della
ROM originale. Per lavorare su PPE non ti serve.

Se un giorno ti servisse (per `asmdiff`):

- usa **il dump della tua cartuccia** di Pokémon Platino (USA). Mettilo nella
  root del repo come `baserom.nds`;
- **non va mai condiviso** (è una ROM protetta da copyright) e **non va mai
  committato**. È già escluso da `.gitignore` (`*.nds`), e comunque pesa 128 MB:
  GitHub rifiuta i file oltre 100 MB e un commit con dentro la ROM blocca tutti i
  push successivi.

---

## 6. Uso quotidiano

Apri VS Code (`cd ~/pokeplatinum && code .` dal terminale Ubuntu) e il pannello
di Claude Code. I comandi del progetto sono in `.claude/commands/`.

**Prima di iniziare** a lavorare, prendi il lavoro dell'altro:

```bash
git pull --no-rebase myfork ppe
```

**`/salva`** — quando hai finito una modifica (o a fine sessione). Claude Code:

1. guarda cosa è cambiato ed esclude ROM, `build/`, file enormi e avanzi di merge;
2. aggiorna [docs/PPE_CHANGES.md](PPE_CHANGES.md);
3. fa commit con un messaggio descrittivo in italiano;
4. fa `pull` da `myfork/ppe` e poi `push`.

Puoi aggiungere una descrizione: `/salva aggiunto negozio a Giubilopoli`.

**`/aggiorna`** — ogni tanto, per importare le novità di pret. Claude Code
controlla prima cosa cambia upstream e se tocca le nostre modifiche, poi chiede
l'ok, fa il merge, compila e verifica che le modifiche PPE funzionino ancora.
Alla fine ti dà l'elenco delle cose da provare in gioco.

### Se Claude Code si ferma per un conflitto

È voluto: Claude Code **non risolve i conflitti da solo** e non annulla il merge.

1. Leggi la spiegazione. Per ogni file in conflitto ti dice cosa ha cambiato
   l'altro (l'amico o pret), cosa avevi cambiato tu e quale risoluzione consiglia.
2. Rispondi con la tua scelta (per esempio "ok, procedi come proponi", oppure
   "tieni la nostra versione in quel file"). Se non sei sicuro, fermati e senti
   l'altro collaboratore prima di decidere.
3. Claude Code applica le risoluzioni e completa con
   `git add` → `git merge --continue`, poi build e push.

Cose da **non** fare durante un conflitto: `git merge --abort`, `git reset --hard`,
`git push --force`, cancellare file a caso. Se ti trovi in uno stato strano, chiedi
prima di toccare qualcosa.

---

## 7. Troubleshooting

| Problema | Soluzione |
|----------|-----------|
| Il terminale di VS Code apre **PowerShell** (`PS C:\...>`) invece di Ubuntu | La finestra non è collegata a WSL: in basso a sinistra manca `WSL: Ubuntu`. Chiudi VS Code e riaprilo con `code .` dal terminale **Ubuntu**. Se sei già in `WSL: Ubuntu` ma il terminale è PowerShell, scegli `bash` dal menu `+ ⌄` del pannello terminale. |
| Il comando `code` non esiste in Ubuntu | Riapri il terminale Ubuntu dopo aver installato VS Code. Se ancora non va, verifica che VS Code sia installato con l'opzione "Add to PATH" (attiva di default). |
| `Permission denied` su uno script (es. `get_metroskrew.sh`) | Lo script ha perso il permesso di esecuzione, di solito per una modifica da Windows. `chmod +x tools/devtools/get_metroskrew.sh`. Per vedere quali file sono coinvolti: `git diff --summary` (righe `mode change 100755 => 100644`). Per ripristinarli in git: `git update-index --chmod=+x <file>`, oppure `git checkout -- <file>` se non li avevi modificati. |
| Errori di meson (`meson: command not found`, versione sbagliata, `ERROR: ... subproject`) | Esegui `make update` e poi `make release`. Succede sempre dopo un merge da upstream. Non installare meson con apt: si usa quello in `subprojects/meson-<versione>`, con la versione indicata da `MESON_VER` nel Makefile. |
| Dopo `make update`: `... generated with an old version of meson` | Upstream ha aggiornato meson. Esegui `make distclean` (cancella solo `build/`), poi `make release`. |
| Diff enormi in cui righe identiche risultano tutte cambiate (CRLF) | Il file è stato salvato con fine riga Windows. Non committarlo. Se non avevi modifiche vere: `git checkout -- <file>`. Se le avevi: in VS Code (lato WSL) clicca `CRLF` nella barra in basso, scegli `LF` e salva. Poi controlla che non stai usando Git per Windows o `\\wsl.localhost`. **Eccezione:** le palette `.pal` devono essere CRLF sul disco (vedi riga sotto). |
| `nitrogfx: LF line endings aren't supported` su un `.pal` | Le palette JASC devono essere CRLF sul disco (lo impone `.gitattributes`). Il file è stato creato o salvato con un editor: `rm <file>.pal && git checkout -- <file>.pal`. |
| `/bin/bash^M: bad interpreter` | Stesso problema CRLF su uno script `.sh`: ripristinalo con `git checkout -- <file>`. |
| Il push viene rifiutato con `403` / `Permission denied` | Non hai accettato l'invito da collaboratore, oppure `gh` non è collegato a git: rifai `gh auth status` e `gh auth setup-git`. |
| Il push viene rifiutato per un file > 100 MB | Quasi sempre è `baserom.nds` finito in un commit. Fermati e chiedi: non riscrivere la storia da solo. |
| Build lentissima | Il repo è su `/mnt/c/...` oppure Ubuntu è in WSL 1. Il repo deve stare in `~/pokeplatinum` e `wsl -l -v` deve dire `2`. |
