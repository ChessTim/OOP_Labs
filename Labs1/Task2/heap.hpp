#pragma once
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <class T, class Before = std::less<T>>
class BinaryHeap {
public:
    explicit BinaryHeap(Before before = Before()) : _before(std::move(before)) {}

    void push(T value) {
        _data.push_back(std::move(value));
        siftUp(_data.size() - 1);
    }

    const T& top() const {
        if (_data.empty()) {
            throw std::out_of_range("BinaryHeap: куча пуста");
        }
        return _data.front();
    }

    T pop() {
        if (_data.empty()) {
            throw std::out_of_range("BinaryHeap: куча пуста");
        }
        T result = std::move(_data.front());
        if (_data.size() > 1) {
            _data.front() = std::move(_data.back());
        }
        _data.pop_back();
        if (!_data.empty()) {
            siftDown(0);
        }
        return result;
    }

    std::size_t size() const { return _data.size(); }

    bool empty() const { return _data.empty(); }

    bool isValid() const {
        for (std::size_t i = 1; i < _data.size(); ++i) {
            if (_before(_data[i], _data[(i - 1) / 2])) {
                return false;
            }
        }
        return true;
    }

private:
    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;
            if (!_before(_data[i], _data[parent])) {
                break;
            }
            std::swap(_data[i], _data[parent]);
            i = parent;
        }
    }

    void siftDown(std::size_t i) {
        const std::size_t n = _data.size();
        while (true) {
            std::size_t left = 2 * i + 1;
            std::size_t right = left + 1;
            std::size_t best = i;
            if (left < n && _before(_data[left], _data[best])) {
                best = left;
            }
            if (right < n && _before(_data[right], _data[best])) {
                best = right;
            }
            if (best == i) {
                break;
            }
            std::swap(_data[i], _data[best]);
            i = best;
        }
    }

    std::vector<T> _data;
    Before _before;
};