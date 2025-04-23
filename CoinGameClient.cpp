// g++ CoinGameClient.cpp -o CoinGameClient.exe -IC:\msys64\mingw64\include -LC:\msys64\mingw64\lib -lzmq

#include <iostream>
#include <string>
#include <zmq.hpp>

int main() {
    zmq::context_t context{1};

    // PUSH socket om berichten te versturen
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    // SUB socket om antwoorden te ontvangen
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    std::cout << "Welkom bij CoinGame!" << std::endl;

    std::string naam;
    std::cout << "Voer je naam in: ";
    std::getline(std::cin, naam);

    // Abonneer op specifiek antwoord voor deze speler
    std::string topic = "Bjarni>CoinGame!>" + naam + ">";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());

    while (true) {
        int gok;
        std::cout << "\nRaad een getal tussen 1 en 5: ";
        std::cin >> gok;

        // Flush cin buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Stuur gok naar de server
        std::string bericht = "Bjarni>CoinGame?>" + naam + ">" + std::to_string(gok) + ">";
        push_socket.send(zmq::buffer(bericht), zmq::send_flags::none);

        // Wacht op het antwoord van de server
        zmq::message_t reply;
        sub_socket.recv(reply, zmq::recv_flags::none);
        std::string antwoord = reply.to_string();

        // Toon enkel het laatste deel van het antwoord
        std::size_t laatste_haakje = antwoord.rfind(">");
        std::string feedback = antwoord.substr(topic.length(), laatste_haakje - topic.length());
        std::cout << "Server: " << feedback << std::endl;

        // Vraag of speler wil doorgaan
        std::string keuze;
        std::cout << "\nWil je nog een keer spelen? (N = nog eens, Q = stoppen): ";
        std::getline(std::cin, keuze);

        if (keuze == "Q" || keuze == "q") {
            std::cout << "Bedankt voor het spelen, " << naam << "! Tot de volgende keer." << std::endl;
            break;
        }
    }

    return 0;
}
