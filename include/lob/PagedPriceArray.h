#include <cstdint>
#include <array>
#include <vector>

template <typename T, size_t MaxPrice, size_t PreallocatedPagesCount = 1024>
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

    std::vector<Page> pagePool;
    size_t nextFreePage{0};

    std::array<Page*, NUM_PAGES> pages;
public:
    PagedPriceArray() : pagePool(PreallocatedPagesCount) {}

    T& operator[](uint32_t price) noexcept {
        size_t pageIdx = price >> PAGE_SHIFT;
        size_t offset = price & PAGE_MASK;

        if (!pages[pageIdx]) [[unlikely]] {
            pages[pageIdx] = &pagePool[nextFreePage++];
        }
        return pages[pageIdx]->levels[offset];
    }
};