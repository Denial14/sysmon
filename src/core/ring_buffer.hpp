#pragma once
#include <array>
#include <cstddef>
#include <algorithm>
#include <limits>

namespace sysmon::core {

template<typename T, std::size_t N>
class RingBuffer {
public:
    RingBuffer() = default;

    void push(T value) {
        buffer_[head_] = value;
        head_ = (head_ + 1) % N;
        if (size_ < N) ++size_;
    }

    std::size_t size() const { return size_; }

    static constexpr std::size_t capacity() { return N; }

    bool empty() const { return size_ == 0; }

    T at(std::size_t i) const {
        std::size_t start = (head_ + N - size_) % N;
        return buffer_[(start + i) % N];
    }

    T latest() const {
        return size_ == 0 ? T{} : buffer_[(head_ + N - 1) % N];
    }

    T oldest() const {
        return size_ == 0 ? T{} : at(0);
    }

    void clear() {
        head_ = 0;
        size_ = 0;
    }

    T min() const {
        if (size_ == 0) return T{};
        T m = at(0);
        for (std::size_t i = 1; i < size_; ++i) {
            m = std::min(m, at(i));
        }
        return m;
    }

    T max() const {
        if (size_ == 0) return T{};
        T m = at(0);
        for (std::size_t i = 1; i < size_; ++i) {
            m = std::max(m, at(i));
        }
        return m;
    }

private:
    std::array<T, N> buffer_{};
    std::size_t head_ = 0;
    std::size_t size_ = 0;
};
}