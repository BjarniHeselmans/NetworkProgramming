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
