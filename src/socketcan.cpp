#include "can_telemetry/socketcan.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>

#ifdef __linux__
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace can_telemetry {

int RunSocketCan(const std::string& interface_name, const FrameHandler& handler) {
#ifdef __linux__
    const int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd < 0) {
        std::perror("socket");
        return -1;
    }

    struct ifreq request {};
    if (interface_name.size() >= sizeof(request.ifr_name)) {
        std::fprintf(stderr, "CAN interface name is too long\n");
        close(socket_fd);
        return -1;
    }
    std::memcpy(request.ifr_name, interface_name.c_str(), interface_name.size() + 1U);
    if (ioctl(socket_fd, SIOCGIFINDEX, &request) < 0) {
        std::perror("SIOCGIFINDEX");
        close(socket_fd);
        return -1;
    }

    struct sockaddr_can address {};
    address.can_family = AF_CAN;
    address.can_ifindex = request.ifr_ifindex;
    if (bind(socket_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        std::perror("bind");
        close(socket_fd);
        return -1;
    }

    while (true) {
        struct can_frame raw_frame {};
        const ssize_t bytes_read = read(socket_fd, &raw_frame, sizeof(raw_frame));
        if (bytes_read < 0 && errno == EINTR) {
            continue;
        }
        if (bytes_read != static_cast<ssize_t>(sizeof(raw_frame))) {
            std::perror("read");
            close(socket_fd);
            return -1;
        }

        CanFrame frame;
        frame.id = raw_frame.can_id & CAN_EFF_MASK;
        frame.size = raw_frame.can_dlc > 8U ? 8U : raw_frame.can_dlc;
        for (std::size_t index = 0; index < frame.size; ++index) {
            frame.data[index] = raw_frame.data[index];
        }
        handler(frame);
    }
#else
    (void)interface_name;
    (void)handler;
    std::fprintf(stderr, "SocketCAN is only available on Linux\n");
    return -1;
#endif
}

}  // namespace can_telemetry
