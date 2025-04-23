// g++ CoinGameService.cpp -o CoinGameService.exe -IC:\msys64\mingw64\include -LC:\msys64\mingw64\lib -lzmq

#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <zmq.hpp>

// Helper om naam en gok uit bericht te halen
std::pair<std::string, int> parse_message(const std::string& message) {
    // Verwacht formaat: Bjarni>CoinGame?>[NAAM]>[GOK]>
    size_t naam_start = message.find("?>");
    if (naam_start == std::string::npos) return {"", -1};
    size_t naam_end = message.find(">", naam_start + 2);
    std::string naam = message.substr(naam_start + 2, naam_end - (naam_start + 2));

    size_t gok_start = naam_end + 1;
    size_t gok_end = message.find(">", gok_start);
    int gok = std::stoi(message.substr(gok_start, gok_end - gok_start));

    return {naam, gok};
}

int main() {
    zmq::context_t context{1};

    // SUB socket voor inkomende gokjes
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    // Luister naar specifiek topic
    std::string topic = "Bjarni>CoinGame?>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.size());

    // PUSH socket om antwoorden te sturen
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    std::unordered_map<std::string, int> munten;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5);

    std::cout << "CoinGame Service gestart...\n";

    while (true) {
        zmq::message_t msg;
        sub_socket.recv(msg, zmq::recv_flags::none);

        std::string bericht = msg.to_string();
        auto [naam, gok] = parse_message(bericht);

        if (naam.empty() || gok < 1 || gok > 5) {
            std::cerr << "Ongeldig bericht ontvangen: " << bericht << std::endl;
            continue;
        }

        int juist_getal = dis(gen);
        std::string antwoord;

        if (gok == juist_getal) {
            munten[naam] += 1;
            antwoord = "Bjarni>CoinGame!>" + naam + ">Correct! Je hebt een muntje gewonnen. Totaal: " + std::to_string(munten[naam]) + ">";
        } else {
            antwoord = "Bjarni>CoinGame!>" + naam + ">Fout! Het juiste getal was " + std::to_string(juist_getal) + ". Je hebt " + std::to_string(munten[naam]) + " muntjes.>";
        }

        push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
        std::cout << "[" << naam << "] gokte " << gok << ", juist was " << juist_getal << " => " << antwoord << "\n";
    }

    return 0;
}
