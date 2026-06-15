#pragma once
#include <concepts>
#include <span>
#include "Messages.h"

template<typename T>
concept PacketReceiverConcept = requires(T t) {
    { t.receive() } -> std::same_as<std::span<const uint8_t>>;
};

template<typename T>
concept MessageParserConcept = requires(T t, std::span<const uint8_t> payload, QueueItem* slot) {
    { t.parse(payload, slot) } -> std::same_as<bool>;
};