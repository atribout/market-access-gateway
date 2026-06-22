#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <memory>

template <typename T, size_t MaxPrice, size_t GlobalPageCount = 32768>
class PagedPriceArray {
    static_assert(MaxPrice > 0, "MaxPrice must be greater than 0.");
private:
    static constexpr size_t PAGE_SHIFT{12uz};
    static constexpr size_t PAGE_SIZE = 1uz << PAGE_SHIFT;
    static constexpr size_t PAGE_MASK = PAGE_SIZE - 1uz;
    static constexpr size_t NUM_PAGES = (MaxPrice / PAGE_SIZE) + 1uz;

    struct Page {
        std::array<T, PAGE_SIZE> levels{};
    };

    static inline std::vector<Page> pagePool{GlobalPageCount};
    static inline size_t nextFreePage{0uz};

    static inline std::vector<std::unique_ptr<Page>> fallbackPool;

    std::array<Page*, NUM_PAGES> pages{};

    __attribute__((noinline)) Page* allocatePage(size_t pageIdx) noexcept {
        Page* newPage;
        if (nextFreePage < GlobalPageCount) [[likely]] {
            newPage = &pagePool[nextFreePage++];
        } 
        else {
            newPage = new Page();
            fallbackPool.emplace_back(newPage);
        }
        pages[pageIdx] = newPage;
        return newPage;
    }

public:
    PagedPriceArray() = default;

    T& operator[](uint32_t price) noexcept {
        size_t pageIdx = price >> PAGE_SHIFT;
        size_t offset = price & PAGE_MASK;

        Page* p = pages[pageIdx];

        if (!p) [[unlikely]] {
            p = allocatePage(pageIdx);
        }

        if (!pages[pageIdx]) [[unlikely]] {
            if (nextFreePage < GlobalPageCount) {
                pages[pageIdx] = &pagePool[nextFreePage++];
            }
            else {
                pages[pageIdx] = new Page();
            }
        }
            
        return p->levels[offset];
    }
};