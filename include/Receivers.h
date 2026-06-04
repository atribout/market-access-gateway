#include <fstream>
#include <print>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class PcapReceiver {
    std::ifstream file;
public:
    explicit PcapReceiver(const std::string& filename) : file(filename, std::ios::binary) {
        if (!file.is_open()) {
            std::println(stderr, "[PCAP] Failed to open");
        }
    }
    
    inline const char* receive(size_t& len) {
        return nullptr;
    }
};

class UdpMulticastReceiver {
private:
    const int sockfd;
    static constexpr int BATCH_SIZE = 32;
    static constexpr int BUF_LEN = 1024;

    struct mmsghdr msgs[BATCH_SIZE];
    struct iovec iovecs[BATCH_SIZE];
    char buffers[BATCH_SIZE][BUF_LEN];

    int current_batch_size = 0;
    int current_msg_idx = 0;

public:
    explicit UdpMulticastReceiver(uint16_t port) : sockfd(socket(AF_INET, SOCK_DGRAM, 0)) {
        
        if(sockfd < 0)  {
            std::println(stderr, "Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        struct sockaddr_in servaddr{};
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);

        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            std::println(stderr, "Bind failed");
            exit(EXIT_FAILURE);
        }  

        for (int i = 0; i < BATCH_SIZE; i++) {
            iovecs[i].iov_base = buffers[i];
            iovecs[i].iov_len = BUF_LEN;
            
            msgs[i] = mmsghdr{};
            msgs[i].msg_hdr.msg_iov = &iovecs[i];
            msgs[i].msg_hdr.msg_iovlen = 1;
        }
    }

    ~UdpMulticastReceiver() {
        if (sockfd > 0) close(sockfd);
    }

    UdpMulticastReceiver(const UdpMulticastReceiver&) = delete;
    UdpMulticastReceiver& operator=(const UdpMulticastReceiver&) = delete;
    UdpMulticastReceiver(UdpMulticastReceiver&&) = delete;
    UdpMulticastReceiver& operator=(UdpMulticastReceiver&&) = delete;
    
    inline const char* receive(size_t& out_len) {
        if (current_msg_idx >= current_batch_size) {
            current_batch_size = recvmmsg(sockfd, msgs, BATCH_SIZE, MSG_DONTWAIT, NULL);
            if (current_batch_size <= 0) {
                current_batch_size = 0;
                out_len = 0;
                return nullptr;
            }
            current_msg_idx = 0;
        }

        out_len = msgs[current_msg_idx].msg_len;
        return buffers[current_msg_idx++];
    }
};