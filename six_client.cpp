#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    const char *multicast_group = "239.255.255.250"; // Multicast адрес (должен совпадать с сервером)
    const int port = 12345;

    // Создание UDP сокета
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    // Привязка сокета к локальному адресу и порту
    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(client_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Ошибка привязки сокета");
        close(client_socket);
        return EXIT_FAILURE;
    }

    // Подключение к multicast-группе
    ip_mreq multicast_request{};
    inet_pton(AF_INET, multicast_group, &multicast_request.imr_multiaddr);
    multicast_request.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(client_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) < 0) {
        perror("Ошибка подключения к multicast-группе");
        close(client_socket);
        return EXIT_FAILURE;
    }

    std::cout << "Подключен к multicast-группе " << multicast_group << ":" << port << "\n";

    char buffer[1024];
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));

        // Получение сообщений
        int bytes_received = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
        if (bytes_received < 0) {
            perror("Ошибка получения данных");
            break;
        }

        std::cout << "Получено сообщение: " << buffer << "\n";
    }

    // Отключение от multicast-группы
    if (setsockopt(client_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) < 0) {
        perror("Ошибка отключения от multicast-группы");
    }

    close(client_socket);
    return 0;
}
