#include "Socket.hpp"

#include "ur_log/ur-log.h"

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
constexpr int poll_timeout_ms = 100;
constexpr size_t command_buffer_size = 4;
}

namespace Peach {
Socket::Socket() = default;

Socket::~Socket() {
    Shutdown();
}

bool Socket::Create() {
    const char *runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    if (!runtime_dir || *runtime_dir == '\0') {
        UR_ERROR("XDG_RUNTIME_DIR is not set");
        return false;
    }

    m_socket_path = std::string(runtime_dir) + "/peach-wallpaper.sock";
    if (m_socket_path.size() >= sizeof(m_server_address.sun_path)) {
        UR_ERROR("Unix socket path is too long");
        return false;
    }

    std::memset(&m_server_address, 0, sizeof(m_server_address));
    m_server_address.sun_family = AF_UNIX;
    std::memcpy(m_server_address.sun_path, m_socket_path.c_str(),
                m_socket_path.size() + 1);

    m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (m_sockfd < 0) {
        UR_ERROR("Failed to create socket.");
        return false;
    }

    return true;
}

bool Socket::Bind() {
    if (m_sockfd < 0 || m_socket_path.empty()) {
        UR_ERROR("Cannot bind an uninitialized socket");
        return false;
    }

    const auto address_length = static_cast<socklen_t>(
        offsetof(sockaddr_un, sun_path) + m_socket_path.size() + 1);

    if (bind(m_sockfd, reinterpret_cast<const sockaddr *>(&m_server_address),
             address_length) < 0) {
        UR_ERROR("Bind to socket failed");
        return false;
    }

    m_is_bound = true;
    return true;
}

bool Socket::Listen() {
    if (m_sockfd < 0 || !m_is_bound) {
        UR_ERROR("Cannot listen on an unbound socket");
        return false;
    }

    if (listen(m_sockfd, 5) < 0) {
        UR_ERROR("Failed to listen to socket.");
        return false;
    }

    return true;
}

bool Socket::Accept() {
    if (m_sockfd < 0) {
        UR_ERROR("Cannot accept on an invalid socket");
        return false;
    }

    const int accepted_fd = accept(m_sockfd, nullptr, nullptr);

    if (accepted_fd < 0) {
        UR_ERROR("Failed to accept client socket");
        return false;
    }

    if (m_client_fd >= 0) {
        close(accepted_fd);
        return true;
    }

    m_client_fd = accepted_fd;
    return true;
}

void Socket::Shutdown() noexcept {
    CloseClient();
    CloseServer();
}

bool Socket::Poll() {
    if (m_sockfd < 0) {
        UR_ERROR("Cannot poll an invalid socket");
        return false;
    }

    pollfd fds[2]{
        {m_sockfd, POLLIN, 0},
        {m_client_fd, POLLIN, 0},
    };

    int ready = poll(fds, 2, poll_timeout_ms);

    if (ready < 0) {
        if (errno == EINTR)
            return true;

        UR_ERROR("Failed to poll socket");
        return false;
    }

    if (fds[0].revents & POLLIN) {
        if (!Accept())
            return false;
    }

    if (m_client_fd >= 0 && (fds[1].revents & POLLIN)) {
        uint8_t buffer[command_buffer_size];
        const ssize_t bytes_read =
            read(m_client_fd, buffer, sizeof(buffer));

        if (bytes_read == 0) {
            CloseClient();
        } else if (bytes_read < 0) {
            if (errno != EINTR) {
                UR_ERROR("Failed to read socket command");
                CloseClient();
            }
        } else {
            for (ssize_t index = 0; index < bytes_read; ++index)
                m_pending_commands.push_back(buffer[index]);
        }
    }

    return true;
}

std::optional<uint8_t> Socket::TakeCommand() {
    if (m_pending_commands.empty())
        return std::nullopt;

    const uint8_t command = m_pending_commands.front();
    m_pending_commands.pop_front();
    return command;
}

void Socket::CloseClient() noexcept {
    if (m_client_fd >= 0) {
        close(m_client_fd);
        m_client_fd = -1;
    }
}

void Socket::CloseServer() noexcept {
    if (m_sockfd >= 0) {
        close(m_sockfd);
        m_sockfd = -1;
    }

    if (m_is_bound && !m_socket_path.empty()) {
        unlink(m_socket_path.c_str());
        m_is_bound = false;
    }
}
} // namespace Peach
