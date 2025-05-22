// g++ -std=c++20 coinland_service.cpp -o coinland_service.exe -IC:\msys64\mingw64\include -LC:\msys64\mingw64\lib -lzmq
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>

// Haalt naam en gok uit CoinGame-bericht
std::string extract_name_and_guess(const std::string& msg, int& guess_out) {
    size_t start = msg.find("?>");
    if (start == std::string::npos) return "";

    size_t naam_end = msg.find(">", start + 2);
    if (naam_end == std::string::npos) return "";

    std::string naam = msg.substr(start + 2, naam_end - (start + 2));

    size_t gok_start = naam_end + 1;
    size_t gok_end = msg.find(">", gok_start);

    if (gok_start < msg.size() && gok_end != std::string::npos) {
        guess_out = std::stoi(msg.substr(gok_start, gok_end - gok_start));
    }

    return naam;
}

//Haalt naam uit SlotMachine-bericht
std::string extract_name(const std::string& message) {
    size_t pos = message.find("?>");
    if (pos == std::string::npos) return "";
    std::string part = message.substr(pos + 2);
    size_t end = part.find(">");
    return part.substr(0, end);
}

// SlotMachine symbolen
std::vector<std::string> slot_symbols = {"CHERRY", "LEMON", "BELL", "STAR", "DIAMOND"};

// Genereer 3 symbolen
std::vector<std::string> spin_slots() {
    std::vector<std::string> result;
    for (int i = 0; i < 3; ++i) {
        int index = rand() % slot_symbols.size();
        result.push_back(slot_symbols[index]);
    }
    return result;
}

// Bepaal winst uit SlotMachine
std::string determine_slot_outcome(const std::vector<std::string>& result, int& winst) {
    if (result[0] == result[1] && result[1] == result[2]) {
        winst = 10;
        return "Jackpot! Je wint 10 muntjes!";
    } else if (result[0] == result[1] || result[1] == result[2] || result[0] == result[2]) {
        winst = 2;
        return "Goed geprobeerd! Je wint 2 muntjes!";
    } else {
        winst = 0;
        return "Helaas, geen winst deze keer.";
    }
}

int main() {
    zmq::context_t context{1};

    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    // Topics waarop geluisterd wordt
    sub_socket.set(zmq::sockopt::subscribe, "Bjarni>CoinGame?>");
    sub_socket.set(zmq::sockopt::subscribe, "Bjarni>SlotMachine?>");

    std::unordered_map<std::string, int> munten;
    srand(static_cast<unsigned int>(time(nullptr)));

    std::cout << "CoinLand Service actief...\n";

    while (true) {
        zmq::message_t msg;
        auto result = sub_socket.recv(msg, zmq::recv_flags::none);
        if (!result) continue;

        std::string bericht = msg.to_string();
        std::cout << "[Ontvangen] " << bericht << std::endl;

        if (bericht.rfind("Bjarni>CoinGame?>", 0) == 0) {
            int gok = -1;
            std::string naam = extract_name_and_guess(bericht, gok);
            if (naam.empty()) continue;

            if (gok < 1 || gok > 5) {
                std::string fout = "Bjarni>CoinGame!>" + naam + ">Ongeldige gok: \"" +
                std::to_string(gok) + "\". Voer een getal in tussen 1 en 5.";
                push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
                std::cout << "[CoinGame] " << naam << ": Ongeldige gok (" << gok << ")\n";
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
            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
            std::cout << "[CoinGame] " << naam << ": " << feedback << std::endl;

        } else if (bericht.rfind("Bjarni>SlotMachine?>", 0) == 0) {
    std::string naam = extract_name(bericht);
    if (naam.empty()) continue;

    // Check of speler genoeg muntjes heeft om te draaien
    if (munten[naam] < 2) {
        std::string fout = "Bjarni>SlotMachine!>" + naam + ">Niet genoeg muntjes om te draaien (min. 2 nodig). Je hebt " +
                           std::to_string(munten[naam]) + " muntjes.";
        push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
        std::cout << "[SlotMachine] " << naam << ": Niet genoeg muntjes\n";
        continue;
    }

    // Trek 2 muntjes af voor deelname
    munten[naam] -= 2;

    auto result = spin_slots();
    int winst = 0;
    std::string feedback = determine_slot_outcome(result, winst);
    munten[naam] += winst;

    std::string slots = result[0] + " " + result[1] + " " + result[2];
    std::string antwoord = "Bjarni>SlotMachine!>" + naam + ">Spin: " + slots + " => " + feedback +
                           " (-2 inzet, +" + std::to_string(winst) + " winst). Je hebt nu " +
                           std::to_string(munten[naam]) + " muntjes.";
    push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
    std::cout << "[SlotMachine] " << naam << ": " << slots << " => " << feedback << " (netto: " << winst - 2 << ")\n";
}
    }

    return 0;
}
