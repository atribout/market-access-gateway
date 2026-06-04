#pragma once
#include <vector>
#include <cstdint>
#include "Order.h"

class OrderPool {
private:
    std::vector<Order> store;
    int32_t freeHead;

public:
    explicit OrderPool(size_t size) : store(size) {
        for(size_t i = 0; i < size - 1; ++i) {
            store[i].next = static_cast<int32_t> (i + 1);
        }
        store [size - 1].next = -1;

        freeHead= 0;
    }

    template<typename... Args>
    int32_t allocate(Args&&... args) {
        if (freeHead == -1) [[unlikely]] return -1;

        int32_t idx = freeHead;
        freeHead = store[idx].next;

        new (&store[idx]) Order(std::forward<Args>(args)...);

        return idx;
    }

    void deallocate(int32_t idx) {
        store[idx].next = freeHead;
        freeHead = idx;
    }

    inline Order& get(int32_t idx) {
        return store[idx];
    }
};