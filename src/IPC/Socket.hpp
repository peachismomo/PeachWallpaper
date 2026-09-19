#pragma once

#include <sys/socket.h>
#include <sys/un.h>

#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace Peach {
class Socket {
  public:
    Socket();
    ~Socket();

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;

    bool Create();
    bool Listen();
    bool Bind();
    bool Accept();
    void Shutdown() noexcept;

    bool Poll();
    std::optional<uint8_t> TakeCommand();

  private:
    void CloseClient() noexcept;
    void CloseServer() noexcept;

    int m_sockfd = -1;
    sockaddr_un m_server_address{};
    std::string m_socket_path;
    bool m_is_bound = false;

    int m_client_fd = -1;
    std::deque<uint8_t> m_pending_commands;
};
} // namespace Peach
