#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    const char *server_ip = "127.0.0.1";
    const int port = 8080;

    // Создание сокета
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    // Настройка адреса сервера
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Неверный адрес сервера");
        close(client_socket);
        return EXIT_FAILURE;
    }

    char buffer[1024];

    while (true) {
        std::cout << "Введите сообщение (или 'exit' для выхода): ";
        std::string message;
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        // Отправка сообщения серверу
        int bytes_sent = sendto(client_socket, message.c_str(), message.size(), 0,
                                (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (bytes_sent < 0) {
            perror("Ошибка при отправке данных");
            continue;
        }

        // Получение ответа от сервера
        sockaddr_in from_addr{};
        socklen_t from_addr_len = sizeof(from_addr);
        std::memset(buffer, 0, sizeof(buffer));

        int bytes_received = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr *)&from_addr, &from_addr_len);
        if (bytes_received < 0) {
            perror("Ошибка при получении данных");
            continue;
        }

        std::cout << "Ответ от сервера: " << buffer << "\n";
    }

    close(client_socket);
    return 0;
}
