#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

void handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Клиент отключился.\n";
            break;
        }
        std::cout << "Получено сообщение: " << buffer << std::endl;

        // Отправка данных обратно клиенту
        send(client_socket, buffer, bytes_received, 0);
    }
    close(client_socket);
}

int main() {
    const int port = 8080;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Ошибка создания сокета");
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка привязки сокета");
        return EXIT_FAILURE;
    }

    if (listen(server_socket, 10) < 0) {
        perror("Ошибка прослушивания");
        return EXIT_FAILURE;
    }

    std::cout << "Сервер запущен на порту " << port << ". Ожидание подключений...\n";

    std::vector<std::thread> threads;
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0) {
            perror("Ошибка подключения клиента");
            continue;
        }

        std::cout << "Новое подключение от "
                  << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;

        // Обработка клиента в отдельном потоке
        threads.emplace_back(std::thread(handle_client, client_socket));
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    close(server_socket);
    return 0;
}
