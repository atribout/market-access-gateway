#pragma once

#include <span>
#include <cstdint>
#include <stdexcept>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

class BinaryItchReceiver {
private:

    // RAII
    struct FileDescriptor {
        int fd = -1;

        explicit FileDescriptor(const char* filepath) : fd(open(filepath, O_RDONLY)) {
            if (fd < 0) throw std::runtime_error("Cannot open Binary file");
        }

        ~FileDescriptor() {
            if (fd >= 0) close(fd);
        }

        int get() const noexcept { return fd; }
    };

    FileDescriptor file;
    size_t fileSize{0};
    const uint8_t* mappedData{nullptr};
    size_t currentOffset{0};
public:
    BinaryItchReceiver(const char* filepath) : file(filepath) {
        struct stat sb;
        if (fstat(file.get(), &sb) < 0) throw std::runtime_error("fstat failed");
        fileSize = sb.st_size;

        mappedData = static_cast<const uint8_t*>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, file.get(), 0));
        if (mappedData == MAP_FAILED) throw std::runtime_error("mmap failed");
    }

    ~BinaryItchReceiver() {
        if (mappedData != MAP_FAILED) munmap(const_cast<uint8_t*>(mappedData), fileSize);
    }

    std::span<const uint8_t> receive() {
        if (currentOffset + 2 > fileSize) return {};

        uint16_t msgLength = std::byteswap(*reinterpret_cast<const uint16_t*>(mappedData + currentOffset));
        currentOffset += 2;

        if (currentOffset + msgLength > fileSize) return {};

        const uint8_t* payloadPtr = mappedData + currentOffset;

        currentOffset += msgLength;

        return {payloadPtr, msgLength};
    }
};