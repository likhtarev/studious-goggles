#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    const char *server_ip = "127.0.0.1"; // Локальный сервер
    const int port = 8080;

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Неверный адрес сервера");
        return EXIT_FAILURE;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка подключения к серверу");
        return EXIT_FAILURE;
    }

    std::cout << "Подключено к серверу. Введите сообщение для отправки:\n";

    std::string message;
    char buffer[1024];
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        send(client_socket, message.c_str(), message.size(), 0);
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0) {
            std::cout << "Ответ сервера: " << buffer << std::endl;
        } else {
            std::cout << "Соединение с сервером разорвано.\n";
            break;
        }
    }

    close(client_socket);
    return 0;
}
