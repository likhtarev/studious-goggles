#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

void set_non_blocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
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

    set_non_blocking(server_socket);
    std::cout << "Сервер запущен на порту " << port << ". Ожидание подключений...\n";

    std::vector<int> clients;
    fd_set read_fds;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        int max_fd = server_socket;

        for (int client_socket : clients) {
            FD_SET(client_socket, &read_fds);
            if (client_socket > max_fd) {
                max_fd = client_socket;
            }
        }

        timeval timeout = {1, 0}; // Тайм-аут на select
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            perror("Ошибка select");
            break;
        }

        // Проверка нового подключения
        if (FD_ISSET(server_socket, &read_fds)) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

            if (client_socket >= 0) {
                set_non_blocking(client_socket);
                clients.push_back(client_socket);
                std::cout << "Новое подключение от "
                          << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\n";
            }
        }

        // Обработка сообщений от клиентов
        for (size_t i = 0; i < clients.size(); ++i) {
            int client_socket = clients[i];
            if (FD_ISSET(client_socket, &read_fds)) {
                char buffer[1024];
                std::memset(buffer, 0, sizeof(buffer));
                int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

                if (bytes_received <= 0) {
                    std::cout << "Клиент отключился.\n";
                    close(client_socket);
                    clients.erase(clients.begin() + i);
                    --i;
                } else {
                    std::cout << "Получено сообщение: " << buffer << "\n";

                    // Отправка сообщения всем клиентам
                    std::string message = "Клиент: " + std::string(buffer);
                    for (int other_client : clients) {
                        if (other_client != client_socket) {
                            send(other_client, message.c_str(), message.size(), 0);
                        }
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
