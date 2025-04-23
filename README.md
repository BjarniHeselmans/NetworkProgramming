# NetworkProgramming
this exercise makes use of the exercises before in the following repo: **https://github.com/BjarniHeselmans/Benternet-EX1-EX2**

# 🎮 CoinGame

CoinGame is een eenvoudige client-server toepassing waarin spelers via het netwerk een gokspel kunnen spelen om muntjes te verzamelen. De communicatie verloopt via **ZeroMQ** sockets.

## 📦 Bestanden

- `coingame_service.cpp` – De server/service die het spel beheert.
- `coingame_client.cpp` – Een command-line client waarmee een speler kan deelnemen.
- *(optioneel)* `coingame_gui.py` – Een Python GUI-client met Tkinter (voorbeeld).

---

## 🔧 Compilatie en uitvoering

### ✅ Vereisten
- C++ compiler
- ZeroMQ library (`libzmq`)
- Python 3 + pyzmq (voor GUI-optie)

### 🔨 Compilatie (Linux/Mac)
```bash
g++ coingame_service.cpp -o service -lzmq
g++ coingame_client.cpp -o client -lzmq
```

### ▶️ Uitvoeren
Start de client:

```bash
./CoinGameClient
```

### 🎲 Speluitleg

Voer je naam in.

Raad een getal tussen 1 en 5.

De server laat weten of je juist gokte, en hoeveel muntjes je hebt.

Na elke ronde kies je of je **opnieuw speelt (N)** of **afsluit (Q)**.

### 📡 Technische details
PUSH socket stuurt het gokbericht naar "tcp://benternet.pxl-ea-ict.be:24041"

SUB socket ontvangt antwoorden op "tcp://benternet.pxl-ea-ict.be:24042"

De SUB filtert op *Bjarni>CoinGame!>[NAAM]>* zodat enkel jouw antwoorden getoond worden.

### ✨ Voorbeeld
```yaml
Welkom bij CoinGame!
Voer je naam in: Bjarni

Raad een getal tussen 1 en 5: 1
Server: Juist! Je hebt nu 1 muntjes.

Wil je nog een keer spelen? (N = nog eens, Q = stoppen): N
```
