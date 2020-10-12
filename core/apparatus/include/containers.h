#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <string>
#include <queue>
#include <stack>
#include <deque>
#include <mutex>


template <typename T>
using Dynamic_Array = std::vector<T>;

template <typename T, typename U>
using Hash_Map = std::unordered_map<T, U>;

template <typename T, typename U>
using Ordered_Map = std::map<T, U>;

template <typename T>
using Hash_Set = std::unordered_set<T>;

template <typename T>
using Ordered_Set = std::set<T>;

using Dynamic_String = std::string;

template <typename T, int len>
using Static_Array = std::array<T, len>;

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using Stack = std::stack<T>;

template <typename T>
using Deque = std::deque<T>;

template<typename T>
struct Thread_Safe_Queue {
    Thread_Safe_Queue() = default;

    Thread_Safe_Queue(Thread_Safe_Queue<T>&& other) noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = std::move(other._queue);
    }
    Thread_Safe_Queue(const Thread_Safe_Queue<T>& other) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = other._queue;
    }

    Thread_Safe_Queue& operator=(const Thread_Safe_Queue& other) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = other._queue;
        return *this;
    }

    virtual ~Thread_Safe_Queue() { }

    size_t size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return {};
        }
        T tmp = _queue.front();
        _queue.pop();
        return tmp;
    }

    std::optional<T> front() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return {};
        }
        return _queue.front();
    }

    void push(const T &item) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(item);
    }

    bool empty() const {
        return _queue.empty();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = Queue<T>();
    }

    Queue<T> _queue;
    mutable std::mutex _mutex;
};