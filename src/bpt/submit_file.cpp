
#include <fstream>
#include <string>
#include <iostream>
#include <cstring>
#include <climits>
#include <iostream>
#include <cstddef>

namespace sjtu {

class exception {
protected:
    const std::string variant = "";
    std::string detail = "";
public:
    exception() {}
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
    virtual std::string what() {
        return variant + " " + detail;
    }
};

class index_out_of_bound : public exception {
/* __________________________ */
};

class runtime_error : public exception {
/* __________________________ */
};

class invalid_iterator : public exception {
/* __________________________ */
};

class container_is_empty : public exception {
/* __________________________ */
};

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

    void print() {
        iterator p = begin();
        while (p != end()) {
            std::cout << *p.iter << ' ';
            ++p;
        }
        std::cout << std::endl;
    }

private:
    T *_data;
    size_t _size;
    size_t _capacity;
    std::allocator<T> _alloc;

};

}

using Ptr = int;

constexpr int MAXBits = 64;
class String {
    char data[MAXBits] = {0};

public:
    String() {}
    String(const std::string &s) {
        for (int i = 0; i < s.length(); ++i) {
            data[i] = s[i];
        }
    }
    bool operator==(const String &other) const {
        return (strcmp(data, other.data) == 0);
    }
    bool operator<(const String &other) const {
        return (strcmp(data, other.data) < 0);
    }
    bool operator>(const String &other) const {
        return (strcmp(data, other.data) > 0);
    }
    friend std::ostream &operator<<(std::ostream &os, const String &obj) {
        os << obj.data;
        return os;
    }
};

struct MyPair {
    String k;
    int v;
public:
    MyPair(const String &key, int value) : k(key), v(value) {}
    MyPair() : k(), v(0) {}

    bool operator==(const MyPair &other) const {
        return k == other.k && v == other.v;
    }
    bool operator<(const MyPair &other) const {
        if (k < other.k) return true;
        if (k > other.k) return false;
        return v < other.v;
    }
    friend std::ostream &operator<<(std::ostream &os, const MyPair &obj) {
        os << '(' << obj.k << ", " << obj.v << ')';
        return os;
    }
};

template <typename keyType, typename valueType, int t, int l>
class BPlusTree {
private:
    class node {
        friend class BPlusTree;
        int key_num = 0;
//        Ptr parent = -1;
        bool son_is_leaf = true;
        keyType keys[2 * t + 2];
        Ptr sons[2 * t + 2] = {-1};
    };

    class LeafNode {
        friend class BPlusTree;
        int key_num = 0;
        Ptr next_leaf = -1;
//        Ptr pre_leaf = -1;
//        Ptr parent = -1;
        keyType keys[2 * l + 2];
        valueType values[2 * l + 2];
    };

    std::fstream file;
    Ptr root;
    bool root_is_leaf;

    // todo 调用前可能要考虑写到垃圾桶里？（如果是写一个全新的节点）（没有垃圾桶的话是写在文件末尾）
    /*
     * 返回写下的那个块的起始位置
     * pos == -1表示在末尾写
     */
    Ptr WriteLeafNode(const LeafNode &tmp, const Ptr &pos);
    Ptr WriteNode(const node &tmp, const Ptr &pos);

    // ! 直接引用传参
    // todo 调用前可能要考虑去缓存找一下
    void ReadNode(node &tmp, const Ptr &pos);
    void ReadLeafNode(LeafNode &tmp, const Ptr &pos);

    void PrintLeafNode(Ptr pos);
    void PrintNode(Ptr pos);

    /*
     * 二分查找keys中key的位置
     * bool返回是否找到key
     * 若找到了，int返回找到的下标
     * 若没找到，int返回key”属于“的那一个key的范围的key的下标
     * e.g. 3 4 6 9 13 若key为2，返回0。若key为3，返回1。若key为7，返回3
     */
    std::pair<int, bool> FindKey(const keyType keys[], int key_num, const keyType &key);

    /*
     * 通过comp函数比较进行二分查找
     * bool返回是否找到key
     * 若找不到key，返回小于目标key的最大的key的下标
     * 若找到key，返回所以key中最小的（第一个）key前面的那一个下标（可能为0）
     * key_num不会等于0
     */
    std::pair<int, bool> FindFirstKey(const keyType keys[], int key_num, const keyType &key, bool (*comp)(const keyType &, const keyType &));

    /*
     * 插入（key, value)
     * 若插入后满块，调用SplitLeafNode裂块
     * 若未裂块，返回{key(), -1}，否则返回{要在父节点中新增的key，该key对应的son}
     */
    std::pair<keyType, Ptr> InsertIntoLeafNode(LeafNode target_node, Ptr target_pos, const keyType &key, const valueType &value);

    /*
     * 对（完成插入后的）目标块进行裂块，向右裂块，需更新顺序链接的Ptr，最后写回文件
     * 2 * l + 1 = l + (l + 1)
     * 返回{要在父节点中新增的key，该key对应的son}
     */
    std::pair<keyType, Ptr> SplitLeafNode(LeafNode target_node, Ptr target_pos);

    /*
     * 找到key后调用InsertIntoLeafNode或InsertIntoNode继续插入
     * 根据调用函数的返回值修改本节点的keys和sons
     * 如果满了，调用SplitNode裂块
     * 如果是root，且裂块了，那么裂完要新增一个root并更新root，返回什么不重要
     * 返回{要在父节点中新增的key，该key对应的son}
     */
    std::pair<keyType, Ptr> InsertIntoNode(node target_node, Ptr target_pos, const keyType &key, const valueType &value);

    /*
     * 同SplitLeafNode
     * 区别在于，SplitLeafNode是叶子节点不损失key，只复制一个key到爸爸
     * 而SplitNode是下方node会损失一个key，有一个key被移动到了爸爸
     * 2 * t + 1 = t + 1 + t
     */
    std::pair<keyType, Ptr> SplitNode(node target_node, Ptr target_pos);

    /*
     * 在pos位置的node节点中查找key
     * 若找到key，向前一个key对应的son走，否则向小于key的最大key值对应的son走
     * 若son_is_leaf 返回要找的son的pos
     * 否则递归调用，去儿子里继续找
     */
    Ptr FindinNode(Ptr pos, const keyType &key, bool (*comp)(const keyType &, const keyType &));

    /*
     * 从pos位置的叶子节点开始，找到key对应的所有value（顺序）
     */
    sjtu::vector<valueType> FindinLeafNode(Ptr pos, const keyType &key, bool (*comp)(const keyType &, const keyType &));

    /*
     * pos为需要删除的叶子节点对应的读写位置
     * 删除键值对，如果删的是第一个（上方有相同key）再回溯时需要更新对应key为删除后节点的第一个key
     * 故返回的最后一个参数为keys[1]
     * 如果不需要借儿子或并块，直接写回文件并返回{false, ^}
     * 否则返回{true, LeafNode}即已经删掉该键值对的叶节点
     */
    std::pair<bool, std::pair<LeafNode, keyType>> EraseFromLeafNode(Ptr pos, const keyType &key);

    std::pair<bool, std::pair<node, keyType>> EraseFromNode(Ptr pos, const keyType &key);

    /*
     * 尝试对target_node借儿子 (如果没有哥哥or弟弟，传入Ptr为-1)
     * 若借到了哥哥的儿子（把哥哥的大儿子变成了我的小儿子），return {1, 我的新小儿子}
     * 若借到了弟弟的儿子（把弟弟的小儿子变成了我的大儿子），return {2, 弟弟的新小儿子}
     * 如果借儿子成功，均WriteLeafNode写回
     * 如果都没借到，return {0, keyType()}
     */
    std::pair<int, keyType> BorrowLeafNode(Ptr pos, LeafNode target_node, Ptr elder, Ptr younger);

    std::pair<int, keyType> BorrowNode(Ptr pos, node target_node, Ptr elder, Ptr younger, const keyType &left_fa, const keyType &right_fa);

    /*
     * sign==1表示brother是弟弟（优先弟弟），sign==2表示brother是哥哥
     * 均向左边并块，故不用向上修改key值。右边的块进垃圾桶
     * 记得更新next_leaf
     */
    void MergeLeafNode(Ptr pos, LeafNode target_node, Ptr brother, int sign);

    void MergeNode(Ptr pos, node target_node, Ptr brother, int sign, const keyType &fa);

public:

    explicit BPlusTree(const std::string &file_name);
    ~BPlusTree();

    /*
     * 数据需保证key不重复
     * 如果key已经存在，就什么都不做
     * 没有送养
     * 只有发生裂块才需要更新父节点的keys（新增一个key和son），单纯插入不需要修改父节点的keys
     */
    void insert(const keyType &key, const valueType &value);

    /*
     * 如果不存在这一key，什么都不做
     * 有借儿子和并块
     * 如果删的是某一叶节点的第一个key，要修改上方对应key值
     */
    void remove(const keyType &key);

    /*
     * 通过Compare比较key值进行查找
     * 可能有多个value对应“在Compare意义下的”同一个key
     * 利用叶子节点的顺序链接找到所有value
     * （得到的value是升序的）
     */
    sjtu::vector<valueType> find(const keyType &key, bool (*comp)(const keyType &, const keyType &));

    /*
     * 用于调试，输出整颗树
     * 每个结点一行，输出keys和sons or values
     */
    void print();
};

template <typename keyType, typename valueType, int t, int l>
BPlusTree<keyType, valueType, t, l>::BPlusTree(const std::string &file_name) {
    file.open(file_name, std::ifstream::in | std::fstream::out | std::ifstream::binary);
    if (file.good()) { // 文件存在 需要读入
        file.seekg(std::ios::beg);
        file.read(reinterpret_cast<char *> (&root), sizeof(Ptr));
        file.read(reinterpret_cast<char *> (&root_is_leaf), sizeof(bool));
    }
    else { // 文件不存在，要创建一个
        root = -1;
        root_is_leaf = true;
        file.close();
        file.clear();
        file.open(file_name,std::fstream::app | std::fstream::binary);
        file.close();
        file.clear();
        file.open(file_name,std::fstream::in | std::fstream::out | std::fstream::binary);
    }
}

template <typename keyType, typename valueType, int t, int l>
BPlusTree<keyType, valueType, t, l>::~BPlusTree() {
    file.seekp(std::ios::beg);
    file.write(reinterpret_cast<const char *> (&root), sizeof(Ptr));
    file.write(reinterpret_cast<const char *> (&root_is_leaf), sizeof(bool));
    file.close();
    file.clear();
}

template <typename keyType, typename valueType, int t, int l>
Ptr BPlusTree<keyType, valueType, t, l>::WriteLeafNode(const BPlusTree::LeafNode &tmp, const Ptr &pos) {
    if (pos == -1) {
        file.seekp(0, std::ios::end);
        Ptr ans = file.tellp();
        file.write(reinterpret_cast<const char *> (&tmp), sizeof(LeafNode));
        return ans;
    }
    file.seekp(pos);
    file.write(reinterpret_cast<const char *> (&tmp), sizeof(LeafNode));
    return pos;
}

template <typename keyType, typename valueType, int t, int l>
Ptr BPlusTree<keyType, valueType, t, l>::WriteNode(const BPlusTree::node &tmp, const Ptr &pos) {
    if (pos == -1) {
        file.seekp(0, std::ios::end);
        Ptr ans = file.tellp();
        file.write(reinterpret_cast<const char *> (&tmp), sizeof(node));
        return ans;
    }
    file.seekp(pos);
    file.write(reinterpret_cast<const char *> (&tmp), sizeof(node));
    return pos;
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::ReadNode(BPlusTree::node &tmp, const Ptr &pos) {
    file.seekg(pos);
    file.read(reinterpret_cast<char *> (&tmp), sizeof(node));
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::ReadLeafNode(BPlusTree::LeafNode &tmp, const Ptr &pos) {
    file.seekg(pos);
    file.read(reinterpret_cast<char *> (&tmp), sizeof(LeafNode));
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, bool> BPlusTree<keyType, valueType, t, l>::FindKey(const keyType keys[], int key_num, const keyType &key) {
    if (key_num == 0) return {1, false};
    int lf = 1, rt = key_num;
    while (lf < rt) {
        int mid = (lf + rt) >> 1;
        if (key == keys[mid]) return {mid, true};
        if (key < keys[mid]) rt = mid;
        else lf = mid + 1;
    }
    if (key == keys[lf]) return {lf, true};
    if (key < keys[lf]) return {lf - 1, false};
    return {lf, false};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<keyType, Ptr> BPlusTree<keyType, valueType, t, l>::InsertIntoLeafNode(LeafNode target_node, Ptr target_pos, const keyType &key, const valueType &value) {
    std::pair<int, bool> tmp = FindKey(target_node.keys, target_node.key_num, key);
    // 如果已经存在这一key，什么都不做
    if (tmp.second) return {keyType(), -1};
    ++tmp.first;

    // 插入键值对
    for (int i = target_node.key_num; i >= tmp.first; --i) {
        target_node.keys[i + 1] = target_node.keys[i];
        target_node.values[i + 1] = target_node.values[i];
    }
    target_node.keys[tmp.first] = key;
    target_node.values[tmp.first] = value;
    ++target_node.key_num;

    // 1. target_node未满，直接写回文件，不需要修改上方索引节点
    if (target_node.key_num < 2 * l + 1) {
        WriteLeafNode(target_node, target_pos);
        return {keyType(), -1};
    }
    // 2. target_node满了，需裂块
    return SplitLeafNode(target_node, target_pos);
}

template <typename keyType, typename valueType, int t, int l>
std::pair<keyType, Ptr> BPlusTree<keyType, valueType, t, l>::SplitLeafNode(LeafNode target_node, Ptr target_pos) {
    LeafNode tmp;
    tmp.key_num = l + 1;
    for (int i = 1; i <= l + 1; ++i) {
        tmp.keys[i] = target_node.keys[i + l];
        tmp.values[i] = target_node.values[i + l];
    }
    tmp.next_leaf = target_node.next_leaf;
    Ptr tmp_pos = WriteLeafNode(tmp, -1);

    target_node.key_num = l;
    target_node.next_leaf = tmp_pos;
    WriteLeafNode(target_node, target_pos);
    return {tmp.keys[1], tmp_pos};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<keyType, Ptr> BPlusTree<keyType, valueType, t, l>::InsertIntoNode(node target_node, Ptr target_pos, const keyType &key, const valueType &value) {
    std::pair<int, bool> tmp = FindKey(target_node.keys, target_node.key_num, key);
    std::pair<keyType, Ptr> res;
    Ptr next_pos = target_node.sons[tmp.first];
    if (target_node.son_is_leaf) {
        LeafNode next_node;
        ReadLeafNode(next_node, next_pos);
        res = InsertIntoLeafNode(next_node, next_pos, key, value);
    }
    else {
        node next_node;
        ReadNode(next_node, next_pos);
        res = InsertIntoNode(next_node, next_pos, key, value);
    }
    if (res.second == -1) return res;

    // 在target_node中加入新的key和son
    for (int i = target_node.key_num; i > tmp.first; --i) {
        target_node.keys[i + 1] = target_node.keys[i];
        target_node.sons[i + 1] = target_node.sons[i];
    }
    target_node.keys[tmp.first + 1] = res.first;
    target_node.sons[tmp.first + 1] = res.second;
    ++target_node.key_num;

    // 若target_node未满，可以直接写入文件并结束
    if (target_node.key_num < 2 * t + 1) {
        WriteNode(target_node, target_pos);
        return {keyType(), -1};
    }
    // 若target_node满了，需要裂块
    std::pair<keyType, Ptr> ans = SplitNode(target_node, target_pos);
    if (target_pos != root) return ans;
    node new_root;
    new_root.key_num = 1;
    new_root.son_is_leaf = false;
    new_root.keys[1] = ans.first;
    new_root.sons[0] = target_pos;
    new_root.sons[1] = ans.second;
    root = WriteNode(new_root, -1);
    return {keyType(), -1};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<keyType, Ptr> BPlusTree<keyType, valueType, t, l>::SplitNode(BPlusTree::node target_node, Ptr target_pos) {
    node tmp;
    tmp.key_num = t;
    for (int i = 1; i <= t; ++i) {
        tmp.keys[i] = target_node.keys[i + t + 1];
        tmp.sons[i] = target_node.sons[i + t + 1];
    }
    tmp.sons[0] = target_node.sons[t + 1];
    tmp.son_is_leaf = target_node.son_is_leaf;
    Ptr tmp_pos = WriteNode(tmp, -1);

    target_node.key_num = t;
    WriteNode(target_node, target_pos);
    return {target_node.keys[t + 1], tmp_pos};
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::insert(const keyType &key, const valueType &value) {
    // 是空树
    if (root == -1) {
        LeafNode tmp;
        tmp.keys[1] = key;
        tmp.values[1] = value;
        tmp.key_num = 1;
        root = sizeof(Ptr) + sizeof(bool);
        WriteLeafNode(tmp, root);
        root_is_leaf = true;
        return;
    }

    if (root_is_leaf) {
        LeafNode target_root;
        ReadLeafNode(target_root, root);
        std::pair<keyType, Ptr> res = InsertIntoLeafNode(target_root, root, key, value);
        if (res.second == -1) return;
        node new_root;
        new_root.key_num = 1;
        new_root.son_is_leaf = true;
        new_root.keys[1] = res.first;
        new_root.sons[0] = root;
        new_root.sons[1] = res.second;
        root = WriteNode(new_root, -1);
        root_is_leaf = false;
        return;
    }

    node target;
    ReadNode(target, root);
    InsertIntoNode(target, root, key, value);
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::PrintLeafNode(Ptr pos) {
    LeafNode tmp;
    ReadLeafNode(tmp, pos);
    for (int i = 1; i <= tmp.key_num; ++i) {
        std::cout << " (" << tmp.keys[i] << ", " << tmp.values[i] << ") ";
    }
    if (tmp.next_leaf > 0) {
        std::cout << " --> ";
        ReadLeafNode(tmp, tmp.next_leaf);
        for (int i = 1; i <= tmp.key_num; ++i) {
            std::cout << " (" << tmp.keys[i] << ", " << tmp.values[i] << ") ";
        }
    }
    std::cout << std::endl;
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::PrintNode(Ptr pos) {
    node tmp;
    ReadNode(tmp, pos);
    for (int i = 1; i <= tmp.key_num; ++i) {
        std::cout << " " << tmp.keys[i] << " ";
    }
    std::cout << std::endl;
    if (tmp.son_is_leaf) {
        for (int i = 0; i <= tmp.key_num; ++i) {
            PrintLeafNode(tmp.sons[i]);
        }
    }
    else {
        for (int i = 0; i <= tmp.key_num; ++i) {
            PrintNode(tmp.sons[i]);
        }
    }
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::print() {
    if (root == -1) {
        std::cout << "空树" << std::endl;
        return;
    }
    if (root_is_leaf) {
        std::cout << "只有根" << std::endl;
        PrintLeafNode(root);
        return;
    }
    PrintNode(root);
    std::cout << std::endl;
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, bool> BPlusTree<keyType, valueType, t, l>::FindFirstKey(const keyType *keys, int key_num, const keyType &key, bool (*comp)(const keyType &, const keyType &)) {
    int lf = 1, rt = key_num;
    int ans = 0;
    while (lf < rt) {
        int mid = (lf + rt) >> 1;
        if (!comp(keys[mid], key) && !comp(key, keys[mid])) {
            ans = mid;
            rt = mid;
        }
        else if (comp(keys[mid], key)) {
            lf = mid + 1;
        }
        else {
            rt = mid;
        }
    }
    if (!comp(keys[lf], key) && !comp(key, keys[lf])) ans = lf;
    if (ans != 0) return {ans - 1, true};
    if (comp(key, keys[lf])) return {lf - 1, false};
    return {lf, false};
}

template <typename keyType, typename valueType, int t, int l>
Ptr BPlusTree<keyType, valueType, t, l>::FindinNode(Ptr pos, const keyType &key, bool (*comp)(const keyType &, const keyType &)) {
    node tmp;
    ReadNode(tmp, pos);
    int res = FindFirstKey(tmp.keys, tmp.key_num, key, comp).first;
    if (tmp.son_is_leaf) return tmp.sons[res];
    Ptr ans = FindinNode(tmp.sons[res], key, comp);
    return ans;
}

template <typename keyType, typename valueType, int t, int l>
sjtu::vector<valueType> BPlusTree<keyType, valueType, t, l>::FindinLeafNode(Ptr pos, const keyType &key, bool (*comp)(const keyType &, const keyType &)) {
    LeafNode tmp;
    ReadLeafNode(tmp, pos);
    sjtu::vector<valueType> ans;
    std::pair<int, bool> res = FindFirstKey(tmp.keys, tmp.key_num, key, comp);
    if (!res.second) {
        // 找不到 可能在后一个叶子中
        int nxt = tmp.next_leaf;
        if (nxt == -1) return ans;
        ReadLeafNode(tmp, nxt);
        res = FindFirstKey(tmp.keys, tmp.key_num, key, comp);
        if (!res.second) return ans; // 还是找不到 就寄了
    }
    int p = res.first + 1;
    while (!comp(key, tmp.keys[p]) && !comp(tmp.keys[p], key)) {
        ans.push_back(tmp.values[p]);
        if (p < tmp.key_num) ++p;
        else {
            // 去看下一个叶子节点
            int nxt = tmp.next_leaf;
            if (nxt == -1) return ans;
            ReadLeafNode(tmp, nxt);
            p = 1;
        }
    }
    return ans;
}

template <typename keyType, typename valueType, int t, int l>
sjtu::vector<valueType> BPlusTree<keyType, valueType, t, l>::find(const keyType &key, bool (*comp)(const keyType &, const keyType &)) {
    if (root == -1) return sjtu::vector<valueType>();
    if (root_is_leaf) return FindinLeafNode(root, key, comp);
    return FindinLeafNode(FindinNode(root, key, comp), key, comp);
}

template <typename keyType, typename valueType, int t, int l>
std::pair<bool, std::pair<typename BPlusTree<keyType, valueType, t, l>::LeafNode, keyType>> BPlusTree<keyType, valueType, t, l>::EraseFromLeafNode(Ptr pos, const keyType &key) {
    LeafNode tmp;
    ReadLeafNode(tmp, pos);
    std::pair<int, bool> res = FindKey(tmp.keys, tmp.key_num, key);
    // 找不到：什么都不做
    if (!res.second) return {false, {tmp, keyType()}};
    // 找到了：删掉
    for (int i = res.first; i < tmp.key_num; ++i) {
        tmp.keys[i] = tmp.keys[i + 1];
        tmp.values[i] = tmp.values[i + 1];
    }
    --tmp.key_num;

    // 如果key_num符合要求，不需要进一步处理，可以直接写回文件
    if (tmp.key_num > l - 1) {
        WriteLeafNode(tmp, pos);
        return {false, {tmp, tmp.keys[1]}};
    }
    // 否则后续需要借儿子或者并块，返回true（后续再写回文件）
    return {true, {tmp, tmp.keys[1]}};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<bool, std::pair<typename BPlusTree<keyType, valueType, t, l>::node, keyType>> BPlusTree<keyType, valueType, t, l>::EraseFromNode(Ptr pos, const keyType &key) {
    node tmp;
    ReadNode(tmp, pos);
    std::pair<int, bool> find_res = FindKey(tmp.keys, tmp.key_num, key);
    // ! find_res.first可能为0！elder也可能为0!
    Ptr next = tmp.sons[find_res.first];
    Ptr elder = (find_res.first > 0) ? tmp.sons[find_res.first - 1] : -1;
    Ptr younger = (find_res.first < tmp.key_num) ? tmp.sons[find_res.first + 1] : -1;
    int delete_index = 0;
    keyType res_key;
    if (tmp.son_is_leaf) {
        std::pair<bool, std::pair<LeafNode, keyType>> res = EraseFromLeafNode(next, key);
        if (find_res.second) tmp.keys[find_res.first] = res.second.second;
        if (!res.first) {
            if (find_res.second) WriteNode(tmp, pos);
            return {false, {tmp, res.second.second}};
        }
        res_key = res.second.second;

        std::pair<int, keyType> res2 = BorrowLeafNode(next, res.second.first, elder, younger);
        if (res2.first == 1) {
            // ! 如果是向哥哥借儿子的，find_res.first一定!=0
            tmp.keys[find_res.first] = res2.second;
        }
        else if (res2.first == 2) {
            tmp.keys[find_res.first + 1] = res2.second;
        }
        else {
            if (younger != -1) { // 和弟弟并块
                MergeLeafNode(next, res.second.first, younger, 1);
                delete_index = find_res.first + 1;
            }
            else {
                MergeLeafNode(next, res.second.first, elder, 2);
                delete_index = find_res.first;
            }
        }
    }
    else {
        std::pair<bool, std::pair<node, keyType>> res = EraseFromNode(next, key);
        if (find_res.second) tmp.keys[find_res.first] = res.second.second;
        if (!res.first) {
            if (find_res.second) WriteNode(tmp, pos);
            return {false, {tmp, res.second.second}};
        }
        res_key = res.second.second;

        std::pair<int, keyType> res2 = BorrowNode(next, res.second.first, elder, younger, tmp.keys[find_res.first], tmp.keys[find_res.first + 1]);
        if (res2.first == 1) {
            tmp.keys[find_res.first] = res2.second;
        }
        else if (res2.first == 2) {
            tmp.keys[find_res.first + 1] = res2.second;
        }
        else {
            if (younger != -1) { // 和弟弟并块
                delete_index = find_res.first + 1;
                MergeNode(next, res.second.first, younger, 1, tmp.keys[delete_index]);
            }
            else {
                delete_index = find_res.first;
                MergeNode(next, res.second.first, elder, 2, tmp.keys[delete_index]);
            }
        }
    }
    // 前面发生并块了，删除delete_index对应的key和son
    if (pos == root &&  delete_index > 0 && tmp.key_num == 1) {
        // todo root进垃圾桶
        // ! 换根 重置root_is_leaf
        if (delete_index == find_res.first + 1) root = next;
        else root = elder;
        if (tmp.son_is_leaf) root_is_leaf = true;
        return {false, {tmp, res_key}};
    }
    if (delete_index > 0) {
        --tmp.key_num;
        for (int i = delete_index; i <= tmp.key_num; ++i) {
            tmp.keys[i] = tmp.keys[i + 1];
            tmp.sons[i] = tmp.sons[i + 1];
        }
    }
    if (tmp.key_num > t - 1) {
        WriteNode(tmp, pos);
        return {false, {tmp, res_key}};
    }
    if (pos == root) {
        WriteNode(tmp, pos);
        return {true, {tmp, res_key}};
    }
    return {true, {tmp, res_key}};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, keyType> BPlusTree<keyType, valueType, t, l>::BorrowLeafNode(Ptr pos, BPlusTree::LeafNode target_node, Ptr elder, Ptr younger) {
    if (elder != -1) {
        LeafNode tmp;
        ReadLeafNode(tmp, elder);
        if (tmp.key_num > l) {
            --tmp.key_num;
            ++target_node.key_num;
            for (int i = target_node.key_num; i > 1; --i) {
                target_node.keys[i] = target_node.keys[i - 1];
                target_node.values[i] = target_node.values[i - 1];
            }
            target_node.keys[1] = tmp.keys[tmp.key_num + 1];
            target_node.values[1] = tmp.values[tmp.key_num + 1];
            WriteLeafNode(tmp, elder);
            WriteLeafNode(target_node, pos);
            return {1, target_node.keys[1]};
        }
    }
    if (younger != -1) {
        LeafNode tmp;
        ReadLeafNode(tmp, younger);
        if (tmp.key_num > l) {
            --tmp.key_num;
            ++target_node.key_num;
            target_node.keys[target_node.key_num] = tmp.keys[1];
            target_node.values[target_node.key_num] = tmp.values[1];
            for (int i = 1; i <= tmp.key_num; ++i) {
                tmp.keys[i] = tmp.keys[i + 1];
                tmp.values[i] = tmp.values[i + 1];
            }
            WriteLeafNode(tmp, younger);
            WriteLeafNode(target_node, pos);
            return {2, tmp.keys[1]};
        }
    }
    return {0, keyType()};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, keyType> BPlusTree<keyType, valueType, t, l>::BorrowNode(Ptr pos, BPlusTree::node target_node, Ptr elder, Ptr younger, const keyType &left_fa, const keyType &right_fa) {
    if (elder != -1) {
        node tmp;
        ReadNode(tmp, elder);
        if (tmp.key_num > t) {
            --tmp.key_num;
            ++target_node.key_num;
            for (int i = target_node.key_num; i > 1; --i) {
                target_node.keys[i] = target_node.keys[i - 1];
                target_node.sons[i] = target_node.sons[i - 1];
            }
            target_node.sons[1] = target_node.sons[0];
            target_node.keys[1] = left_fa;
            target_node.sons[0] = tmp.sons[tmp.key_num + 1];

            WriteNode(tmp, elder);
            WriteNode(target_node, pos);
            return {1, tmp.keys[tmp.key_num + 1]};
        }
    }
    if (younger != -1) {
        node tmp;
        ReadNode(tmp, younger);
        if (tmp.key_num > t) {
            --tmp.key_num;
            ++target_node.key_num;
            keyType res = tmp.keys[1];
            target_node.keys[target_node.key_num] = right_fa;
            target_node.sons[target_node.key_num] = tmp.sons[0];
            tmp.sons[0] = tmp.sons[1];
            for (int i = 1; i <= tmp.key_num; ++i) {
                tmp.keys[i] = tmp.keys[i + 1];
                tmp.sons[i] = tmp.sons[i + 1];
            }
            WriteNode(tmp, younger);
            WriteNode(target_node, pos);
            return {2, res};
        }
    }
    return {0, keyType()};
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::MergeLeafNode(Ptr pos, BPlusTree::LeafNode target_node, Ptr brother, int sign) {
    LeafNode tmp;
    ReadLeafNode(tmp, brother);
    if (sign == 1) {
        target_node.key_num = 2 * l - 1;
        for (int i = l; i < 2 * l; ++i) {
            target_node.keys[i] = tmp.keys[i - l + 1];
            target_node.values[i] = tmp.values[i - l + 1];
        }
        target_node.next_leaf = tmp.next_leaf;
        WriteLeafNode(target_node, pos);
        // todo tmp(brother)进垃圾桶
    }
    else {
        tmp.key_num = 2 * l - 1;
        for (int i = l + 1; i < 2 * l; ++i) {
            tmp.keys[i] = target_node.keys[i - l];
            tmp.values[i] = target_node.values[i - l];
        }
        tmp.next_leaf = target_node.next_leaf;
        WriteLeafNode(tmp, brother);
        // todo target_node进垃圾桶
    }
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::MergeNode(Ptr pos, BPlusTree::node target_node, Ptr brother, int sign, const keyType &fa) {
    node tmp;
    ReadNode(tmp, brother);
    if (sign == 1) {
        target_node.key_num = 2 * t;
        target_node.keys[t] = fa;
        target_node.sons[t] = tmp.sons[0];
        for (int i = t + 1; i <= 2 * t; ++i) {
            target_node.keys[i] = tmp.keys[i - t];
            target_node.sons[i] = tmp.sons[i - t];
        }
        WriteNode(target_node, pos);
        // todo tmp进垃圾桶
    }
    else {
        tmp.key_num = 2 * t;
        tmp.keys[t + 1] = fa;
        tmp.sons[t + 1] = target_node.sons[0];
        for (int i = t + 2; i <= 2 * t; ++i) {
            tmp.keys[i] = target_node.keys[i - t - 1];
            tmp.sons[i] = target_node.sons[i - t - 1];
        }
        WriteNode(tmp, brother);
        // todo target_node进垃圾桶
    }
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::remove(const keyType &key) {
    if (root == -1) return;
    if (root_is_leaf) {
        LeafNode tmp;
        ReadLeafNode(tmp, root);
        std::pair<int, bool> res = FindKey(tmp.keys, tmp.key_num, key);
        if (!res.second) return;
        --tmp.key_num;
        if (tmp.key_num == 0) {
            root = -1;
            return;
        }
        for (int i = res.first; i <= tmp.key_num; ++i) {
            tmp.keys[i] = tmp.keys[i + 1];
            tmp.values[i] = tmp.values[i + 1];
        }
        WriteLeafNode(tmp, root);
        return;
    }
    EraseFromNode(root, key);
}

template class BPlusTree<MyPair, int, 2, 2>;
template class BPlusTree<MyPair, int, 27, 27>;
//template class BPlusTree<String, int, 2, 2>;
//template class BPlusTree<int, int, 2, 2>;

bool comp(const MyPair &a, const MyPair &b) {
    return a.k < b.k;
}

int main() {
    /*
    BPlusTree<int, int, 2, 2> a("file");
    int tmp;

    for (int i = 0; i < 20000; ++i) {
        std::cin >> tmp;
        a.insert(tmp, tmp);
    }
    for (int i = 0; i < 19900; ++i) {
        std::cin >> tmp;
        a.remove(tmp);
    }
    for (int i = 0; i < 90; ++i) {
        std::cin >> tmp;
        sjtu::vector<int> ans = a.find(tmp, comp);
        if (ans.empty() || ans[0] != tmp) {
            std::cout << "oops" << tmp << std::endl;
            return 0;
        }
    }
    //    a.find({1, 0}, comp).print();

    */
//    std::cout << sizeof(int) << std::endl;
//    std::cout << sizeof(bool) << std::endl;
//    std::cout << sizeof(MyPair) << std::endl;

    BPlusTree<MyPair, int, 27, 27> b("file");
    int n;
    std::cin >> n;
    std::string instruction;
    std::string key;
    int value;
    for (int i = 1; i <= n; ++i) {
        std::cin >> instruction;
        std::cin >> key;
        if (instruction == "insert") {
            std::cin >> value;
            b.insert({key, value}, value);
        }
        else if (instruction == "delete") {
            std::cin >> value;
            b.remove({key, value});
        }
        else if (instruction == "find") {
            sjtu::vector<int> ans = b.find({key, value}, comp);
            if (ans.empty()) printf("null\n");
            else {
                for (int an : ans) {
                    printf("%d ", an);
                }
                printf("\n");
            }
        }
//        std::cout << i << std::endl;
//        b.print();
    }

    return 0;
}