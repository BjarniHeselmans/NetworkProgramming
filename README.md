# 🎰 CoinLand Project - CoinGame & SlotMachine over Benternet

Welkom bij **CoinLand**! Dit project bevat twee ZeroMQ-gebaseerde minigames die draaien over het Benternet-netwerk:

- 🎯 **CoinGame** – Raad een getal tussen 1 en 5 en verdien muntjes.
- 🎰 **SlotMachine** – Draai drie symbolen en win muntjes op basis van combinaties.

Beide games delen dezelfde **spelersnaam** en **muntjesbalans**, waardoor er één geïntegreerde spelervaring ontstaat. 

---

## 🌐 Overzicht

Spelers communiceren via ZeroMQ met de services die op het Benternet draaien. De speler:

1. Voert zijn naam in.
2. Stuurt een verzoek naar een van de services.
3. Krijgt een gepersonaliseerd antwoord terug met resultaat én muntjesupdate.

Muntjes worden bijgehouden **per naam**, en bestaan alleen in de service-geheugenruimte (later uitbreidbaar met opslag).

---

## 🧠 CoinGame Flow

```mermaid
flowchart TD
    Start --> NaamInvoer[Speler voert naam in]
    NaamInvoer --> Raad[Raad getal tussen 1 en 5]
    Raad --> SendGuess[Client stuurt Bjarni>CoinGame?>Naam>Gok>]
    SendGuess --> Service[CoinGame-service ontvangt gok]
    Service --> Compare[Genereer random getal en vergelijk]
    Compare --> Resultaat[Correct? Ja/nee + update muntjes]
    Resultaat --> SendBack[Stuur antwoord via Bjarni>CoinGame!>Naam>...>]
    SendBack --> ClientToon[Client toont resultaat aan speler]

## 🎰 SlotMachine Flow
```mermaid
flowchart TD
    Start --> NaamInvoer[Speler voert naam in]
    NaamInvoer --> ClickPlay[Klik op 'Speel!']
    ClickPlay --> SendSpin[Client stuurt Bjarni>SlotMachine?>Naam>]
    SendSpin --> SlotService[SlotMachine-service ontvangt verzoek]
    SlotService --> Rollen[Genereer 3 willekeurige symbolen]
    Rollen --> Evaluatie[Bepaal resultaat: 3x = 10 coins, 2x = 2 coins, anders 0]
    Evaluatie --> SendBack[Stuur antwoord via Bjarni>SlotMachine!>Naam>...>]
    SendBack --> ClientToon[Client toont symbolen en muntjesresultaat]
