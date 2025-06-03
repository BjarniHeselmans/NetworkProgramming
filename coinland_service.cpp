// g++ -std=c++20 coinland_service.cpp -o coinland_service.exe -IC:\msys64\mingw64\include -LC:\msys64\mingw64\lib -lzmq -lstdc++fs
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>

// JSON library
#include "json.hpp"
using json = nlohmann::json;

// Persistent opslag
const std::string balance_file = "balances.json";
std::unordered_map<std::string, int> munten;

void load_balances() {
    std::ifstream in(balance_file);
    if (in) {
        json data;
        in >> data;
        for (auto& [k, v] : data.items()) {
            munten[k] = v.get<int>();
        }
    }
}

void save_balances() {
    json data;
    for (const auto& [naam, saldo] : munten) {
        data[naam] = saldo;
    }
    std::ofstream out(balance_file);
    out << data.dump(4);
}

// CoinGame: naam en gok extractie
std::string extract_name_and_guess(const std::string& msg, int& guess_out) {
    size_t start = msg.find("?>");
    if (start == std::string::npos) return "";

    size_t naam_end = msg.find(">", start + 2);
    if (naam_end == std::string::npos) return "";

    std::string naam = msg.substr(start + 2, naam_end - (start + 2));
    size_t gok_start = naam_end + 1;
    size_t gok_end = msg.find(">", gok_start);

    if (gok_start < msg.size() && gok_end != std::string::npos) {
        try {
            guess_out = std::stoi(msg.substr(gok_start, gok_end - gok_start));
        } catch (...) {
            guess_out = -1;
        }
    }

    return naam;
}

// SlotMachine: naam en inzet extractie
std::pair<std::string, int> extract_name_and_bet(const std::string& msg) {
    std::string naam;
    int inzet = 0;
    size_t start = msg.find("?>");
    if (start == std::string::npos) return {"", 0};

    size_t naam_end = msg.find("|", start + 2);
    if (naam_end == std::string::npos) return {"", 0};

    naam = msg.substr(start + 2, naam_end - (start + 2));

    size_t inzet_start = naam_end + 1;
    size_t inzet_end = msg.find(">", inzet_start);
    if (inzet_end == std::string::npos) inzet_end = msg.size();

    try {
        inzet = std::stoi(msg.substr(inzet_start, inzet_end - inzet_start));
    } catch (...) {
        inzet = 0;
    }

    return {naam, inzet};
}

// SlotMachine symbolen
std::vector<std::string> slot_symbols = {"CHERRY", "LEMON", "BELL", "STAR", "DIAMOND"};

std::vector<std::string> spin_slots() {
    std::vector<std::string> result;
    for (int i = 0; i < 3; ++i) {
        int index = rand() % slot_symbols.size();
        result.push_back(slot_symbols[index]);
    }
    return result;
}

std::string determine_slot_outcome(const std::vector<std::string>& result, int inzet, int& winst) {
    if (result[0] == result[1] && result[1] == result[2]) {
        winst = inzet * 3;
        return "Jackpot! 3 dezelfde symbolen!";
    } else if (result[0] == result[1] || result[1] == result[2] || result[0] == result[2]) {
        winst = static_cast<int>(inzet * 1.5);
        return "Goed geprobeerd! 2 dezelfde symbolen.";
    } else {
        winst = 0;
        return "Helaas, geen overeenkomende symbolen.";
    }
}

int main() {
    zmq::context_t context{1};
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};

    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    sub_socket.set(zmq::sockopt::subscribe, "Bjarni>CoinGame?>");
    sub_socket.set(zmq::sockopt::subscribe, "Bjarni>SlotMachine?>");
    sub_socket.set(zmq::sockopt::subscribe, "Bjarni>Sync?>");

    srand(static_cast<unsigned int>(time(nullptr)));
    load_balances();

    std::cout << "🟢 CoinLand Service actief...\n";

    while (true) {
        zmq::message_t msg;
        auto result = sub_socket.recv(msg, zmq::recv_flags::none);
        if (!result) continue;

        std::string bericht = msg.to_string();
        std::cout << "[Ontvangen] " << bericht << std::endl;

        // Sync-aanvraag
        if (bericht.rfind("Bjarni>Sync?>", 0) == 0) {
            // naam volledig extraheren, niet een substring vanaf index 14 met 1 minder teken
            size_t naam_start = std::string("Bjarni>Sync?>").length();
            size_t naam_end = bericht.find(">", naam_start);
            std::string naam = bericht.substr(naam_start, naam_end - naam_start);

            if (munten.find(naam) == munten.end()) munten[naam] = 10;

            std::string antwoord = "Bjarni>Sync!>" + naam + ">Je hebt nog " + std::to_string(munten[naam]) + " muntjes>";
            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
            continue;
        }

        // CoinGame
        if (bericht.rfind("Bjarni>CoinGame?>", 0) == 0) {
            int gok = -1;
            std::string naam = extract_name_and_guess(bericht, gok);
            if (naam.empty()) continue;
            if (munten.find(naam) == munten.end()) munten[naam] = 10;

            if (gok < 1 || gok > 5) {
                std::string fout = "Bjarni>CoinGame!>" + naam + ">Ongeldige gok: \"" +
                                   std::to_string(gok) + "\". Voer een getal in tussen 1 en 5.";
                push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
                continue;
            }

            int juist = 1 + rand() % 5;
            std::string feedback;

            if (gok == juist) {
                munten[naam] += 1;
                feedback = "Correct! Je wint 1 muntje!";
            } else {
                feedback = "Fout! Het juiste getal was " + std::to_string(juist) + ". Geen winst.";
            }

            std::string antwoord = "Bjarni>CoinGame!>" + naam + ">" + feedback +
                                   " Je hebt nu " + std::to_string(munten[naam]) + " muntjes.";
            save_balances();
            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
        }

        // SlotMachine
        else if (bericht.rfind("Bjarni>SlotMachine?>", 0) == 0) {
            auto [naam, inzet] = extract_name_and_bet(bericht);
            if (naam.empty() || inzet <= 0) {
                std::string fout = "Bjarni>SlotMachine!>" + naam + ">Ongeldige inzet. Geef een getal groter dan nul.";
                push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
                continue;
            }

            if (munten[naam] < inzet) {
                std::string fout = "Bjarni>SlotMachine!>" + naam + ">Niet genoeg muntjes. Inzet: " +
                                   std::to_string(inzet) + ", je hebt: " + std::to_string(munten[naam]);
                push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
                continue;
            }

            munten[naam] -= inzet;
            auto result = spin_slots();
            int winst = 0;
            std::string feedback = determine_slot_outcome(result, inzet, winst);
            munten[naam] += winst;

            std::ostringstream antwoord;
            antwoord << "Bjarni>SlotMachine!>" << naam << ">Spin: " << result[0] << " " << result[1] << " " << result[2];
            antwoord << " => " << feedback << " (-" << inzet << ", +" << winst << ")";
            antwoord << ". Je hebt nu " << munten[naam] << " muntjes.";

            save_balances();
            push_socket.send(zmq::buffer(antwoord.str()), zmq::send_flags::none);
        }
    }

    return 0;
}