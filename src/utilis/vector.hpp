#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {

template<typename T>
class vector {
public:
    class const_iterator;

    class iterator {
    friend class vector<T>;
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::output_iterator_tag;

    private:
        pointer iter;
        pointer start;
        pointer end;

        iterator(pointer p, pointer _st, pointer _ed) : iter(p), start(_st), end(_ed) {}
    public:
        iterator() : iter(nullptr), start(nullptr), end(nullptr) {}
        iterator(const iterator &other) : iter(other.iter), start(other.start), end(other.end) {}

        iterator operator+(const int &n) const {
            return {iter + n, start, end};
        }

        iterator operator-(const int &n) const {
            return {iter - n, start, end};
        }

        int operator-(const iterator &rhs) const {
            pointer tmp = start;
            int cnt = 0, i = -1, j = -1;
            while (tmp++ != end + 1) {
                if (tmp == iter) i = cnt;
                if (tmp == rhs.iter) j = cnt;
                ++cnt;
            }
            if (j == -1) throw invalid_iterator();
            return std::abs(i - j);
        }

        iterator &operator+=(const int &n) {
            iter += n;
            return *this;
        }

        iterator &operator-=(const int &n) {
            iter -= n;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(*this);
            ++iter;
            return tmp;
        }

        iterator &operator++() {
            ++iter;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp(*this);
            --iter;
            return tmp;
        }

        iterator &operator--() {
            --iter;
            return *this;
        }

        T &operator*() const {
            return *iter;
        }

        bool operator==(const iterator &rhs) const {
            return iter == rhs.iter;
        }

        bool operator==(const const_iterator &rhs) const {
            return iter == rhs.iter;
        }

        bool operator!=(const iterator &rhs) const {
            return iter != rhs.iter;
        }

        bool operator!=(const const_iterator &rhs) const {
            return iter != rhs.iter;
        }
    };

    class const_iterator {
        friend class vector<T>;
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = const T *;
        using reference = T &;
        using iterator_category = std::output_iterator_tag;

    private:
        pointer iter;
        pointer start;
        pointer end;
        const_iterator(pointer p, pointer _st, pointer _ed) : iter(p), start(_st), end(_ed) {}

    public:
        const_iterator() : iter(nullptr), start(nullptr), end(nullptr) {}
        const_iterator(const const_iterator &other) : iter(other.iter), start(other.start), end(other.end) {}

        const_iterator operator+(const int &n) const {
            return {iter + n, start, end};
        }

        const_iterator operator-(const int &n) const {
            return {iter - n, start, end};
        }

        int operator-(const const_iterator &rhs) const {
            pointer tmp = start;
            int cnt = 0, i = -1, j = -1;
            while (tmp++ != end + 1) {
                if (tmp == iter) i = cnt;
                if (tmp == rhs.iter) j = cnt;
                ++cnt;
            }
            if (j == -1) throw invalid_iterator();
            return std::abs(i - j);
        }

        const_iterator &operator+=(const int &n) {
            iter += n;
            return *this;
        }

        const_iterator &operator-=(const int &n) {
            iter -= n;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp(*this);
            ++iter;
            return tmp;
        }

        const_iterator &operator++() {
            ++iter;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator tmp(*this);
            --iter;
            return tmp;
        }

        const_iterator &operator--() {
            --iter;
            return *this;
        }

        const T &operator*() const {
            return *iter;
        }

        bool operator==(const iterator &rhs) const {
            return iter == rhs.iter;
        }

        bool operator==(const const_iterator &rhs) const {
            return iter == rhs.iter;
        }

        bool operator!=(const iterator &rhs) const {
            return iter != rhs.iter;
        }

        bool operator!=(const const_iterator &rhs) const {
            return iter != rhs.iter;
        }
    };

    vector() : _size(0),_capacity(2) {
        _data = _alloc.allocate(2);
    }

    // 拷贝时capacity和size均等于other.size
    vector(const vector &other) : _size(other._size),_capacity(other._size) {
        _data = _alloc.allocate(_size);
        T *p = _data, *q = other._data;
        for (int i = 0; i < _size; ++i) {
            p = new (p) T (*q);
            ++p;
            ++q;
        }
    }

    ~vector() {
        T *p = _data;
        for (int i = 0; i < _size; i++) {
            p->~T();
            ++p;
        }
        _alloc.deallocate(_data, _capacity);
    }

    vector &operator=(const vector &other) {
        if (&other == this) return *this;
        this->~vector();
        _size = other._size;
        _capacity = other._size;
        _data = _alloc.allocate(_size);
        T *p = _data, *q = other._data;
        for (int i = 0; i < _size; ++i) {
            p = new (p) T (*q);
            ++p;
            ++q;
        }
        return *this;
    }

    // 访问指定位置元素（检查下标越界）
    T &at(const size_t &pos) {
        if (pos < 0 || pos >= _size) throw index_out_of_bound();
        return *(_data + pos);
    }
    const T &at(const size_t &pos) const {
        if (pos < 0 || pos >= _size) throw index_out_of_bound();
        return *(_data + pos);
    }

    T &operator[](const size_t &pos) {
        return this->at(pos);
    }
    const T &operator[](const size_t &pos) const {
        return this->at(pos);
    }

    const T &front() const {
        if (_size == 0) throw container_is_empty();
        return *_data;
    }

    const T &back() const {
        if (_size == 0) throw container_is_empty();
        return *(_data + _size - 1);
    }

    iterator begin() {
        return {_data, _data, (_data + _size)};
    }

    const_iterator cbegin() const {
        return {_data, _data, _data + _size};
    }

    iterator end() {
        return {_data + _size, _data, _data + _size};

    }

    const_iterator cend() const {
        return {_data + _size, _data, _data + _size};
    }

    bool empty() const {
        return _size == 0;
    }

    size_t size() const {
        return _size;
    }

    void clear() {
        T *p = _data;
        while (_size--) {
            p->~T();
            ++p;
        }
    }

    iterator insert(iterator pos, const T &value) {
        if (_size < _capacity - 1) {
            T *p = _data + _size;
            p = new (p) T (*(p - 1));
            --p;
            while (p != pos.iter) {
                *p = *(p - 1);
                --p;
            }
            *p = value;
            ++_size;
            return pos;
        }
        _capacity *= 2;
        T *new_data = _alloc.allocate(_capacity);
        T *tmp = new_data;
        T *p = _data;
        T *target = nullptr;
        int i = 0;
        while (p != pos.iter) {
            tmp = new (tmp) T (*p);
            p->~T();
            ++tmp;
            ++p;
            ++i;
        }
        tmp = new (tmp) T (value);
        target = tmp;
        ++tmp;
        while (i < _size) {
            tmp = new (tmp) T (*p);
            p->~T();
            ++tmp;
            ++p;
            ++i;
        }
        _alloc.deallocate(_data, _capacity / 2);
        _data = new_data;
        ++_size;
        return {target, _data, _data + _size};
    }

    iterator insert(const size_t &ind, const T &value) {
        if (ind > _size) throw index_out_of_bound();
        return this->insert({_data + ind, _data, _data + _size}, value);
    }

    iterator erase(iterator pos) {
        if (_size > _capacity / 4) {
            T *p = pos.iter;
            --_size;
            while (p != _data + _size) {
                *p = *(p + 1);
                ++p;
            }
            p->~T();
            return pos;
        }
        size_t pre_capacity = _capacity;
        _capacity /= 2;
        T *new_data = _alloc.allocate(_capacity);
        T *tmp = new_data;
        T *p = _data;
        T *target = nullptr;
        int i = 0;
        while (p != pos.iter) {
            tmp = new (tmp) T (*p);
            p->~T();
            ++tmp;
            ++p;
            ++i;
        }
        target = tmp;
        p->~T();
        ++p;
        ++i;
        while (i < _size) {
            tmp = new (tmp) T (*p);
            p->~T();
            ++tmp;
            ++p;
            ++i;
        }
        _alloc.deallocate(_data, pre_capacity);
        _data = new_data;
        --_size;
        return {target, _data, _data + _size};
    }

    iterator erase(const size_t &ind) {
        if (ind >= _size) throw index_out_of_bound();
        return this->erase({_data + ind, _data, _data + _size});
    }

    void push_back(const T &value) {
        if (_size < _capacity - 1) {
            T *p = _data + _size;
            p = new (p) T (value);
            ++_size;
            return;
        }
        _capacity *= 2;
        T *new_data = _alloc.allocate(_capacity);
        T *tmp = new_data;
        T *p = _data;
        for (int i = 0; i < _size; ++i) {
            tmp = new (tmp) T (*p);
            p->~T();
            ++tmp;
            ++p;
        }
        tmp = new (tmp) T (value);
        _alloc.deallocate(_data, _capacity / 2);
        _data = new_data;
        ++_size;
    }

    void pop_back() {
        if (_size == 0) throw container_is_empty();
        if (_size > _capacity / 4) {
            --_size;
            (_data + _size)->~T();
            return;
        }
        size_t pre_capacity = _capacity;
        _capacity /= 2;
        T *new_data = _alloc.allocate(_capacity);
        T *tmp = new_data;
        T *p = _data;
        int i = 0;
        --_size;
        while (i < _size) {
            tmp = new (tmp) T (*p);
            p->~T();
            ++tmp;
            ++p;
            ++i;
        }
        p->~T();
        _alloc.deallocate(_data, pre_capacity);
        _data = new_data;
    }

private:
    T *_data;
    size_t _size;
    size_t _capacity;
    std::allocator<T> _alloc;

};

}

#endif
