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
#include <new>

template <typename T, typename U>
using Hash_Map = std::unordered_map<T, U>;

template <typename T, typename U>
using Ordered_Map = std::map<T, U>;

template <typename T>
using Hash_Set = std::unordered_set<T>;

template <typename T>
using Ordered_Set = std::set<T>;

using Dynamic_String = std::string;

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using Deque = std::deque<T>;

template <typename T>
using Stack = std::stack<T>;



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

template <typename type_t>
struct Packed_Container_Iterator {

    typedef Packed_Container_Iterator<type_t> iterator_t;

    Packed_Container_Iterator() = delete;
    Packed_Container_Iterator(type_t* ptr) : _ptr(ptr) {}

    inline iterator_t& operator ++() {
        _ptr++;
        return *this;
    }
    inline iterator_t& operator --() {
        _ptr--;
        return *this;
    }

    inline type_t& operator *() {
        return *_ptr;
    }
    inline type_t* operator ->() {
        return _ptr;
    }

    inline bool operator ==(const iterator_t& rhs) {
        return _ptr == rhs._ptr;
    }
    inline bool operator !=(const iterator_t& rhs) {
        return _ptr != rhs._ptr;
    }

    inline friend bool operator >(const iterator_t& lhs, const iterator_t& rhs) {
        return lhs._ptr > rhs._ptr;
    }
    inline friend bool operator <(const iterator_t& lhs, const iterator_t& rhs) {
        return lhs._ptr < rhs._ptr;
    }

    inline friend bool operator >=(const iterator_t& lhs, const iterator_t& rhs) {
        return lhs._ptr >= rhs._ptr;
    }
    inline friend bool operator <=(const iterator_t& lhs, const iterator_t& rhs) {
        return lhs._ptr <= rhs._ptr;
    }

    inline friend iterator_t operator +(const iterator_t& lhs, size_t rhs) {
        return lhs._ptr + rhs;
    }
    inline friend iterator_t operator -(const iterator_t& lhs, size_t rhs) {
        return lhs._ptr - rhs;
    }

    type_t* _ptr;
};

//template <typename T, int len>
//using Static_Array = std::array<T, len>;

template <typename type_t, size_t LEN>
struct Static_Array {
    typedef Packed_Container_Iterator<type_t> iterator_t;
    typedef Static_Array<type_t, LEN> container_t;

    Static_Array() {
        memset(_buffer, 0, sizeof(_buffer));
    }

    ~Static_Array() {
       
    }

    inline Static_Array(const std::initializer_list<type_t>& list) {
        *this = list;
    }

    inline Static_Array(const container_t& other) {
        *this = other;
    }

    inline container_t& operator=(const container_t& other) {
        memcpy(_buffer, other._buffer, LEN * sizeof(type_t));

        return *this;
    }

    inline container_t& operator=(const std::initializer_list<type_t>& list) {
        size_t len = list.end() - list.begin();
        if (len > LEN) len = LEN;
        
        memcpy(_buffer, list.begin(), len * sizeof(typet));

        return *this;
    }

    inline const type_t& at(index_t idx) const {
        assert(idx < size() && "Index out of range");
        return _buffer[idx];
    }

    inline type_t& operator[](index_t idx) {
        assert(idx < size() && "Index out of range");
        return _buffer[idx];
    }

    inline static constexpr size_t size() {
        return LEN;
    }

    inline type_t* data() { return _buffer; }

    inline iterator_t begin() const { return (type_t*)_buffer; } 
    inline iterator_t end()   const { return ((type_t*)_buffer) + LEN; }

    inline void fill(byte b) {
        memset(_buffer, (int)b, sizeof(_buffer));
    }

    type_t _buffer[LEN];
};

struct Default_Heap_Allocator {

    static constexpr bool has_limit = false;

    template <typename type_t>
    inline type_t* allocate(size_t n) {
        return (type_t*)malloc(n * sizeof(type_t));
    }

    template <typename type_t>
    inline void construct(type_t* ptr, const type_t& item) {
        new(ptr) type_t(item);
    }
    template <typename type_t>
    inline void construct(type_t* ptr, type_t&& item) {
        new(ptr) type_t(std::move(item));
    }

    template <typename type_t, typename ...args_t>
    inline void construct(type_t* ptr, args_t&& ...args) {
        new(ptr) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate(type_t* ptr) {
        free(ptr);
    }

    template <typename type_t>
    inline void destruct(type_t* ptr) {
        if constexpr (std::is_destructible<type_t>())
            ptr->~type_t();
        else
            (void)ptr;
    }
};

//template <typename type_t>
//using Dynamic_Array = std::vector<type_t>;

template <typename type_t, typename alloc_t = Default_Heap_Allocator>
struct Dynamic_Array {

    typedef Packed_Container_Iterator<type_t> iterator_t;
    typedef Dynamic_Array<type_t, Default_Heap_Allocator> container_t;

    Dynamic_Array() {
        _buffer_head = NULL;
        reserve(1);
    }

    ~Dynamic_Array() {
        this->clear();   
        _allocator.deallocate(_buffer_head);
        _buffer_head = NULL;
    }

    inline Dynamic_Array(const std::initializer_list<type_t>& list) {
        _buffer_head = NULL;
        *this = list;
    }

    inline Dynamic_Array(const container_t& other) {
        _buffer_head = NULL;
        *this = other;
    }

    inline container_t& operator=(const container_t& other) {
        if (_buffer_head) {
            this->clear();
            _allocator.deallocate(_buffer_head);
            _buffer_head = NULL;
        }
        _buffer_head = _allocator.allocate<type_t>(other.capacity());
        _buffer_tail = _buffer_head + other.capacity();
        _buffer_virtual_end = _buffer_head + other.size();
        debug_only(__size = _buffer_virtual_end - _buffer_head);
        debug_only(__capacity = _buffer_tail - _buffer_head);

        memcpy(_buffer_head, other._buffer_head, other.size() * sizeof(type_t));

        return *this;
    }

    inline container_t& operator=(const std::initializer_list<type_t>& list) {
        if (_buffer_head) {
            this->clear();
            _allocator.deallocate(_buffer_head);
            _buffer_head = NULL;
        }
        size_t len = list.end() - list.begin();
        _buffer_head = _allocator.allocate<type_t>(len);
        _buffer_tail = _buffer_head + len;
        _buffer_virtual_end = _buffer_head + len;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
        debug_only(__capacity = _buffer_tail - _buffer_head);

        for (size_t i = 0; i < len; i++) {
            _buffer_head[i] = list.begin()[i];
        }

        return *this;
    }

    inline const type_t& at(index_t idx) const {
        assert(idx < size() && "Index out of range");
        return _buffer_head[idx];
    }

    inline type_t& operator[](index_t idx) {
        assert(idx < size() && "Index out of range");
        return _buffer_head[idx];
    }

    inline size_t size() const {
        return _buffer_virtual_end - _buffer_head;
    }

    inline type_t* data() { return _buffer_head; }

    inline iterator_t begin() const { return _buffer_head; } 
    inline iterator_t end()   const { return _buffer_virtual_end; } 

    inline size_t capacity() const {
        return _buffer_tail - _buffer_head;
    }

    inline void reserve(size_t num) {
        if (capacity() >= num) return;

        type_t* new_buffer = _allocator.allocate<type_t>(num);
        type_t* new_tail = new_buffer + num;
        type_t* new_virtual_end = new_buffer + size();

        if (_buffer_head) {
            //memcpy(new_buffer, _buffer_head, size() * sizeof(type_t));

            for (size_t i = 0; i < size(); i++) {
                _allocator.construct(new_buffer + i, std::move(_buffer_head[i]));
            
                _allocator.destruct(_buffer_head + i);            
            }

            _allocator.deallocate(_buffer_head);
            _buffer_head = NULL;
        }

        _buffer_head = new_buffer;
        _buffer_tail = new_tail;
        _buffer_virtual_end = new_virtual_end;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
        debug_only(__capacity = _buffer_tail - _buffer_head);
    }

    inline void resize(size_t num) {
        if (size() > num) {
            for (size_t i = num; i < size(); i++) {
                _allocator.destruct(_buffer_head + i);
            }
        } else {
            reserve(num);
            for (size_t i = size(); i < num; i++) {
                _allocator.construct(_buffer_head + i, std::move(type_t()));
            }
        }   
        _buffer_virtual_end = _buffer_head + num;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
    }

    inline void push_back(const type_t& item) {
        if (size() + 1 > capacity()) {
            reserve(capacity() + capacity());
        }

        _allocator.construct(_buffer_head + size(), item);
        _buffer_virtual_end++;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
    }
    inline void push_back(type_t&& item) {
        if (size() + 1 > capacity()) {
            reserve(capacity() + capacity());
        }

        _allocator.construct(_buffer_head + size(), std::move(item));
        _buffer_virtual_end++;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
    }

    template <typename ...args_t>
    inline void emplace_back(args_t&& ...args) {
        if (size() + 1 > capacity()) {
            reserve(capacity() + capacity());
        }

        _allocator.construct(_buffer_head + size(), std::forward<args_t>(args)...);
        _buffer_virtual_end++;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
    }

    inline void pop_back() {
        assert(size() > 0 && "Cannot pop_back empty dynamic array");
        _allocator.destruct(_buffer_virtual_end - 1);
        _buffer_virtual_end--;
        debug_only(__size--);
    }

    inline void erase(iterator_t it) {
        assert(it >= begin() && it < end() && "Invalid iterator");
        
        _allocator.destruct(it._ptr);

        memmove(it._ptr, it._ptr + 1, _buffer_virtual_end - (it._ptr + 1));

        _buffer_virtual_end--;
        debug_only(__size--);
    }

    inline void erase(iterator_t erase_begin, iterator_t erase_end) {
        assert(erase_begin >= begin() && erase_begin <  end() && "Invalid iterator");
        assert(erase_end   >= begin() && erase_end   <= end() && "Invalid iterator");
        if (erase_begin == erase_end) {
            erase(erase_begin);
            return;
        }
        
        if (erase_end < erase_begin) {
            iterator_t temp = erase_begin;
            erase_begin = erase_end;
            erase_end = temp;
        }
        
        for (iterator_t it = erase_begin; it < erase_end; ++it) {
            _allocator.destruct(it._ptr);
        }
        
        if (erase_end == end()) {
            _buffer_virtual_end = erase_begin._ptr;
            debug_only(__size = _buffer_virtual_end - _buffer_head);
            return;

            
        }

        memmove(erase_begin._ptr, erase_end._ptr, _buffer_virtual_end - erase_end._ptr);

        _buffer_virtual_end -= (erase_end - erase_begin)._ptr;
        debug_only(__size = _buffer_virtual_end - _buffer_head);
    }

    inline void clear() {
        for (type_t* ptr = _buffer_head; ptr < _buffer_virtual_end; ptr++) {
            _allocator.destruct(ptr);
        }
        _buffer_virtual_end = _buffer_head;
        debug_only(__size = 0);
    }

    alloc_t _allocator;
    type_t* _buffer_head = NULL;
    type_t* _buffer_tail = NULL;
    type_t* _buffer_virtual_end = NULL;

    debug_only(size_t __size = 0);
    debug_only(size_t __capacity = 0);
};
