#include "bpt.h"

template <typename keyType, typename valueType, int t, int l>
BPlusTree<keyType, valueType, t, l>::BPlusTree(const std::string &file_name) {
    file.open(file_name,std::fstream::app | std::fstream::binary);
    file.close();
    file.clear();
    file.open(file_name,std::fstream::in | std::fstream::out | std::fstream::binary);
    root = -1;
}

template <typename keyType, typename valueType, int t, int l>
BPlusTree<keyType, valueType, t, l>::~BPlusTree() {
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
        int mid = (lf + rt) / 2;
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
    if (target_node.key_num < 2 * l - 1) {
        WriteLeafNode(target_node, target_pos);
        return {keyType(), -1};
    }
    // 2. target_node满了，需裂块
    return SplitLeafNode(target_node, target_pos);
}

template <typename keyType, typename valueType, int t, int l>
std::pair<keyType, Ptr> BPlusTree<keyType, valueType, t, l>::SplitLeafNode(LeafNode target_node, Ptr target_pos) {
    LeafNode tmp;
    tmp.key_num = l - 1;
    for (int i = 1; i <= l - 1; ++i) {
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
    for (int i = target_node.key_num; i > tmp.first; ++i) {
        target_node.keys[i + 1] = target_node.keys[i];
        target_node.sons[i + 1] = target_node.sons[i];
    }
    target_node.keys[tmp.first + 1] = res.first;
    target_node.sons[tmp.first + 1] = res.second;
    ++target_node.key_num;

    // 若target_node未满，可以直接写入文件并结束
    if (target_node.key_num < 2 * t - 1) {
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
    tmp.key_num = t - 1;
    for (int i = 1; i <= t - 1; ++i) {
        tmp.keys[i] = target_node.keys[i + t];
        tmp.sons[i] = target_node.sons[i + t];
    }
    tmp.sons[0] = target_node.sons[t];
    tmp.son_is_leaf = target_node.son_is_leaf;
    Ptr tmp_pos = WriteNode(tmp, -1);

    target_node.key_num = t - 1;
    WriteNode(target_node, target_pos);
    return {target_node.keys[t], tmp_pos};
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::insert(const keyType &key, const valueType &value) {
    // 是空树
    if (root == -1) {
        LeafNode tmp;
        tmp.keys[1] = key;
        tmp.key_num = 1;
        WriteLeafNode(tmp, 0);
        root = 0;
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

template class BPlusTree<std::pair<std::string, int>, int, 3, 5>;
template class BPlusTree<std::string, int, 3, 5>;
