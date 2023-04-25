
#ifndef TICKETSYSTEM_BPT_H
#define TICKETSYSTEM_BPT_H

#include <fstream>
#include "../utilis/vector.hpp"
#include <string>

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
    friend std::ostream &operator<<(std::ostream &os, const String &obj) {
        os << obj.data;
        return os;
    }
};

struct MyPair {
    int k;
    int v;
public:
    MyPair(int key, int value) : k(key), v(value) {}
    MyPair() : k(0), v(0) {}

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
     * p为上方出现过该key的node的读写位置（若无，则为-1）
     * 删除键值对，如有必要(p != -1)，修改p位置的节点的keys为删除本key后接着的下一个key
     * 如果不需要借儿子或并块，直接写回文件并返回{false, ^}
     * 否则返回{true, LeafNode}即已经删掉该键值对的叶节点
     */
    std::pair<bool, LeafNode> EraseFromLeafNode(Ptr pos, Ptr p, const keyType &key);

    std::pair<bool, node> EraseFromNode(Ptr pos, Ptr p, const keyType &key);

    /*
     * 尝试对target_node借儿子 (如果没有哥哥or弟弟，传入Ptr为-1)
     * 若借到了哥哥的儿子（把哥哥的大儿子变成了我的小儿子），return {1, 我的新小儿子}
     * 若借到了弟弟的儿子（把弟弟的小儿子变成了我的大儿子），return {2, 弟弟的新小儿子}
     * 如果借儿子成功，均WriteLeafNode写回
     * 如果都没借到，return {0, keyType()}
     */
    std::pair<int, keyType> BorrowLeafNode(Ptr pos, LeafNode target_node, Ptr elder, Ptr younger);

    std::pair<int, keyType> BorrowNode(Ptr pos, node target_node, Ptr elder, Ptr younger, const keyType &fa);

    /*
     * sign==1表示brother是弟弟（优先弟弟），sign==2表示brother是哥哥
     * 均向左边并块，故不用向上修改key值。右边的块进垃圾桶
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

#endif //TICKETSYSTEM_BPT_H