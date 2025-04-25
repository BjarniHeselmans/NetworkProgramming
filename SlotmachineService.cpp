#include <zmq.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unordered_map>

// Hulpfunctie: haal naam uit bericht
std::string extract_name(const std::string& message) {
    std::size_t pos = message.find("?>");
    if (pos != std::string::npos) {
        std::string part = message.substr(pos + 2);
        std::size_t end = part.find(">");
        return part.substr(0, end);
    }
    return "";
}

// Symbolen voor de slotmachine
std::vector<std::string> slot_symbols = {"🍒", "🍋", "🔔", "⭐", "💎"};

// Genereer 3 willekeurige symbolen
std::vector<std::string> spin_slots() {
    std::vector<std::string> result;
    for (int i = 0; i < 3; ++i) {
        int index = rand() % slot_symbols.size();
        result.push_back(slot_symbols[index]);
    }
    return result;
}

// Bepaal winst/verlies op basis van resultaat
std::string determine_outcome(const std::vector<std::string>& result) {
    if (result[0] == result[1] && result[1] == result[2]) {
        return "Jackpot! Je wint 10 muntjes!";
    } else if (result[0] == result[1] || result[1] == result[2] || result[0] == result[2]) {
        return "Goed geprobeerd! Je wint 2 muntjes!";
    } else {
        return "Helaas, geen winst deze keer.";
    }
}

int main() {
    zmq::context_t context{1};

    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    std::string topic = "Bjarni>SlotMachine?>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());

    std::cout << "🎰 SlotMachine Service actief..." << std::endl;
    srand(static_cast<unsigned int>(time(nullptr)));

    std::unordered_map<std::string, int> munten;

    while (true) {
        zmq::message_t request;
        sub_socket.recv(request, zmq::recv_flags::none);

        std::string bericht = request.to_string();
        std::string naam = extract_name(bericht);

        if (naam.empty()) {
            std::cerr << "⚠️ Ongeldig bericht ontvangen: " << bericht << std::endl;
            continue;
        }

        std::vector<std::string> result = spin_slots();
        std::string slots_output = result[0] + " " + result[1] + " " + result[2];
        std::string outcome = determine_outcome(result);

        // Update munten
        if (outcome.find("10") != std::string::npos) {
            munten[naam] += 10;
        } else if (outcome.find("2") != std::string::npos) {
            munten[naam] += 2;
        }

        std::string antwoord = "Bjarni>SlotMachine!>" + naam + ">Slots: " + slots_output + " => " + outcome +
                               " Je hebt nu " + std::to_string(munten[naam]) + " muntjes.";

        push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
        std::cout << "🎮 Speler: " << naam << " | " << antwoord << std::endl;
    }

    return 0;
}
