#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    const char *multicast_group = "239.255.255.250"; // Multicast адрес (должен быть в диапазоне 224.0.0.0 – 239.255.255.255)
    const int port = 12345;

    // Создание UDP сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    // Настройка адреса для отправки
    sockaddr_in multicast_addr{};
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(port);
    inet_pton(AF_INET, multicast_group, &multicast_addr.sin_addr);

    std::cout << "Сервер готов к отправке сообщений в группу " << multicast_group << ":" << port << "\n";

    while (true) {
        std::string message;
        std::cout << "Введите сообщение для отправки (или 'exit' для выхода): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        // Отправка сообщения в multicast группу
        int bytes_sent = sendto(server_socket, message.c_str(), message.size(), 0,
                                (struct sockaddr *)&multicast_addr, sizeof(multicast_addr));
        if (bytes_sent < 0) {
            perror("Ошибка отправки сообщения");
        } else {
            std::cout << "Сообщение отправлено: " << message << "\n";
        }
    }

    close(server_socket);
    return 0;
}
