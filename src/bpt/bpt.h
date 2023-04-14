
#ifndef TICKETSYSTEM_BPT_H
#define TICKETSYSTEM_BPT_H

#include <fstream>
#include "../utilis/vector.hpp"

using Ptr = int;

template <typename keyType, typename valueType, int t, int l>
class BPlusTree {
private:
    class node {
        friend class BPlusTree;
        int key_num = 0;
//        Ptr parent = -1;
        bool son_is_leaf = true;
        keyType keys[2 * t];
        Ptr sons[2 * t] = {-1};
    };

    class LeafNode {
        friend class BPlusTree;
        int key_num = 0;
        Ptr next_leaf = -1;
//        Ptr pre_leaf = -1;
//        Ptr parent = -1;
        keyType keys[2 * l];
        valueType values[2 * l];
    };

    std::fstream file;
    Ptr root;
    bool root_is_leaf = true;

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

    /*
     * 二分查找keys中key的位置
     * bool返回是否找到了这一键值对
     * 若找到了，int返回找到的下标
     * 若没找到，int返回key”属于“的那一个key的范围的key的下标
     * e.g. 3 4 6 9 13 若key为2，返回0。若key为3，返回1。若key为7，返回3
     */
    std::pair<int, bool> FindKey(const keyType keys[], int key_num, const keyType &key);

    /*
     * 插入（key, value)
     * 若插入后满块，调用SplitLeafNode裂块
     * 若未裂块，返回{key(), -1}，否则返回{要在父节点中新增的key，该key对应的son}
     */
    std::pair<keyType, Ptr> InsertIntoLeafNode(LeafNode target_node, Ptr target_pos, const keyType &key, const valueType &value);

    /*
     * 对（完成插入后的）目标块进行裂块，向右裂块，需更新顺序链接的Ptr，最后写回文件
     * 2 * l - 1 = l + (l - 1)
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
     * 2 * t - 1 = (t - 1) + 1 + (t - 1)
     */
    std::pair<keyType, Ptr> SplitNode(node target_node, Ptr target_pos);

public:

    explicit BPlusTree(const std::string &file_name);
    ~BPlusTree();

    /*
     * 数据需保证key不重复
     * 如果key已经存在，就什么都不做
     * 没有送养
     * 只有发生裂块才需要更新父节点的keys（新增一个key和son），单纯插入不修改父节点的keys
     */
    void insert(const keyType &key, const valueType &value);

    void remove(const keyType &key, const valueType &value);

    sjtu::vector<valueType> find(const keyType &key);
};

#endif //TICKETSYSTEM_BPT_H