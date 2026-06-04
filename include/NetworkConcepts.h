#pragma once
#include <concepts>
#include <cstddef>
#include "Messages.h"

template<typename T>
concept PacketReceiverConcept = requires(T t, size_t& len) {
    { t.receive(len) } -> std::same_as<const char*>;
};

template<typename T>
concept MessageParserConcept = requires(T t, const char* data, size_t len, QueueItem* slot) {
    { t.parse(data, len, slot) } -> std::same_as<bool>;
};