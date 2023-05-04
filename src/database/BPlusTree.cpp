#include "BPlusTree.h"

template class BPlusTree<MyPair<64>, int, 2, 2>;
template class BPlusTree<MyPair<64>, int, 27, 27>;
//template class BPlusTree<String, int, 2, 2>;
//template class BPlusTree<int, int, 2, 2>;

template <typename keyType, typename valueType, int t, int l>
BPlusTree<keyType, valueType, t, l>::BPlusTree(const std::string &file_name_inherit,
                                               const std::string &file_name_node,
                                               const std::string &file_name_leaf)
                                               : file_node(file_name_node), file_leaf(file_name_leaf) {
    file_inherit.open(file_name_inherit, std::ifstream::in | std::fstream::out | std::ifstream::binary);
    if (file_inherit.good()) { // 文件存在 需要读入
        file_inherit.seekg(std::ios::beg);
        file_inherit.read(reinterpret_cast<char *> (&root), sizeof(Ptr));
        file_inherit.read(reinterpret_cast<char *> (&root_is_leaf), sizeof(bool));
    }
    else { // 文件不存在，要创建一个
        root = -1;
        root_is_leaf = true;
        file_inherit.close();
        file_inherit.clear();
        file_inherit.open(file_name_inherit,std::fstream::app | std::fstream::binary);
        file_inherit.close();
        file_inherit.clear();
        file_inherit.open(file_name_inherit,std::fstream::in | std::fstream::out | std::fstream::binary);
    }
}

template <typename keyType, typename valueType, int t, int l>
BPlusTree<keyType, valueType, t, l>::~BPlusTree() {
    file_inherit.seekp(std::ios::beg);
    file_inherit.write(reinterpret_cast<const char *> (&root), sizeof(Ptr));
    file_inherit.write(reinterpret_cast<const char *> (&root_is_leaf), sizeof(bool));
    file_inherit.close();
    file_inherit.clear();
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
    if (tmp.second) return {keyType(), -2};
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
        file_leaf.WritePage(target_node, target_pos);
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
    Ptr tmp_pos = file_leaf.WritePage(tmp, -1);

    target_node.key_num = l;
    target_node.next_leaf = tmp_pos;
    file_leaf.WritePage(target_node, target_pos);
    return {tmp.keys[1], tmp_pos};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<keyType, Ptr> BPlusTree<keyType, valueType, t, l>::InsertIntoNode(node target_node, Ptr target_pos, const keyType &key, const valueType &value) {
    std::pair<int, bool> tmp = FindKey(target_node.keys, target_node.key_num, key);
    std::pair<keyType, Ptr> res;
    Ptr next_pos = target_node.sons[tmp.first];
    if (target_node.son_is_leaf) {
        LeafNode next_node = file_leaf.ReadPage(next_pos);
        res = InsertIntoLeafNode(next_node, next_pos, key, value);
    }
    else {
        node next_node = file_node.ReadPage(next_pos);
        res = InsertIntoNode(next_node, next_pos, key, value);
    }
    if (res.second < 0) return res;

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
        file_node.WritePage(target_node, target_pos);
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
    root = file_node.WritePage(new_root, -1);
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
    Ptr tmp_pos = file_node.WritePage(tmp, -1);

    target_node.key_num = t;
    file_node.WritePage(target_node, target_pos);
    return {target_node.keys[t + 1], tmp_pos};
}

template <typename keyType, typename valueType, int t, int l>
bool BPlusTree<keyType, valueType, t, l>::insert(const keyType &key, const valueType &value) {
    // 是空树
    if (root == -1) {
        LeafNode tmp;
        tmp.keys[1] = key;
        tmp.values[1] = value;
        tmp.key_num = 1;
        root = file_leaf.WritePage(tmp, -1);
        root_is_leaf = true;
        return true;
    }

    if (root_is_leaf) {
        LeafNode target_root = file_leaf.ReadPage(root);
        std::pair<keyType, Ptr> res = InsertIntoLeafNode(target_root, root, key, value);
        if (res.second == -1) return true;
        if (res.second == -2) return false;
        node new_root;
        new_root.key_num = 1;
        new_root.son_is_leaf = true;
        new_root.keys[1] = res.first;
        new_root.sons[0] = root;
        new_root.sons[1] = res.second;
        root = file_node.WritePage(new_root, -1);
        root_is_leaf = false;
        return true;
    }

    node target = file_node.ReadPage(root);
    if (InsertIntoNode(target, root, key, value).second == -2) return false;
    else return true;
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::PrintLeafNode(Ptr pos) {
    LeafNode tmp = file_leaf.ReadPage(pos);
    for (int i = 1; i <= tmp.key_num; ++i) {
        std::cout << " (" << tmp.keys[i] << ", " << tmp.values[i] << ") ";
    }
    if (tmp.next_leaf > 0) {
        std::cout << " --> ";
        tmp = file_leaf.ReadPage(tmp.next_leaf);
        for (int i = 1; i <= tmp.key_num; ++i) {
            std::cout << " (" << tmp.keys[i] << ", " << tmp.values[i] << ") ";
        }
    }
    std::cout << std::endl;
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::PrintNode(Ptr pos) {
    node tmp = file_node.ReadPage(pos);
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
    node tmp = file_node.ReadPage(pos);
    int res = FindFirstKey(tmp.keys, tmp.key_num, key, comp).first;
    if (tmp.son_is_leaf) return tmp.sons[res];
    return FindinNode(tmp.sons[res], key, comp);
}

template <typename keyType, typename valueType, int t, int l>
sjtu::vector<valueType> BPlusTree<keyType, valueType, t, l>::FindinLeafNode(Ptr pos, const keyType &key, bool (*comp)(const keyType &, const keyType &)) {
    LeafNode tmp = file_leaf.ReadPage(pos);
    sjtu::vector<valueType> ans;
    std::pair<int, bool> res = FindFirstKey(tmp.keys, tmp.key_num, key, comp);
    if (!res.second) {
        // 找不到 可能在后一个叶子中
        int nxt = tmp.next_leaf;
        if (nxt == -1) return ans;
        tmp = file_leaf.ReadPage(nxt);
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
            tmp = file_leaf.ReadPage(nxt);
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
std::pair<int, std::pair<typename BPlusTree<keyType, valueType, t, l>::LeafNode, keyType>> BPlusTree<keyType, valueType, t, l>::EraseFromLeafNode(Ptr pos, const keyType &key) {
    LeafNode tmp = file_leaf.ReadPage(pos);
    std::pair<int, bool> res = FindKey(tmp.keys, tmp.key_num, key);
    // 找不到：什么都不做
    if (!res.second) return {-1, {tmp, keyType()}};
    // 找到了：删掉
    for (int i = res.first; i < tmp.key_num; ++i) {
        tmp.keys[i] = tmp.keys[i + 1];
        tmp.values[i] = tmp.values[i + 1];
    }
    --tmp.key_num;

    // 如果key_num符合要求，不需要进一步处理，可以直接写回文件
    if (tmp.key_num > l - 1) {
        file_leaf.WritePage(tmp, pos);
        return {0, {tmp, tmp.keys[1]}};
    }
    // 否则后续需要借儿子或者并块，返回true（后续再写回文件）
    return {1, {tmp, tmp.keys[1]}};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, std::pair<typename BPlusTree<keyType, valueType, t, l>::node, keyType>> BPlusTree<keyType, valueType, t, l>::EraseFromNode(Ptr pos, const keyType &key) {
    node tmp = file_node.ReadPage(pos);
    std::pair<int, bool> find_res = FindKey(tmp.keys, tmp.key_num, key);
    // ! find_res.first可能为0！elder也可能为0!
    Ptr next = tmp.sons[find_res.first];
    Ptr elder = (find_res.first > 0) ? tmp.sons[find_res.first - 1] : -1;
    Ptr younger = (find_res.first < tmp.key_num) ? tmp.sons[find_res.first + 1] : -1;
    int delete_index = 0;
    keyType res_key;
    if (tmp.son_is_leaf) {
        std::pair<int, std::pair<LeafNode, keyType>> res = EraseFromLeafNode(next, key);
        if (find_res.second) tmp.keys[find_res.first] = res.second.second;
        if (res.first <= 0) {
            if (find_res.second) file_node.WritePage(tmp, pos);
            return {res.first, {tmp, res.second.second}};
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
        std::pair<int, std::pair<node, keyType>> res = EraseFromNode(next, key);
        if (find_res.second) tmp.keys[find_res.first] = res.second.second;
        if (res.first <= 0) {
            if (find_res.second) file_node.WritePage(tmp, pos);
            return {res.first, {tmp, res.second.second}};
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
        file_node.DumpPage(root); // root进垃圾桶
        // ! 换根 重置root_is_leaf
        if (delete_index == find_res.first + 1) root = next;
        else root = elder;
        if (tmp.son_is_leaf) root_is_leaf = true;
        return {0, {tmp, res_key}};
    }
    if (delete_index > 0) {
        --tmp.key_num;
        for (int i = delete_index; i <= tmp.key_num; ++i) {
            tmp.keys[i] = tmp.keys[i + 1];
            tmp.sons[i] = tmp.sons[i + 1];
        }
    }
    if (tmp.key_num > t - 1) {
        file_node.WritePage(tmp, pos);
        return {0, {tmp, res_key}};
    }
    if (pos == root) {
        file_node.WritePage(tmp, pos);
        return {1, {tmp, res_key}};
    }
    return {1, {tmp, res_key}};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, keyType> BPlusTree<keyType, valueType, t, l>::BorrowLeafNode(Ptr pos, BPlusTree::LeafNode target_node, Ptr elder, Ptr younger) {
    if (elder != -1) {
        LeafNode tmp = file_leaf.ReadPage(elder);
        if (tmp.key_num > l) {
            --tmp.key_num;
            ++target_node.key_num;
            for (int i = target_node.key_num; i > 1; --i) {
                target_node.keys[i] = target_node.keys[i - 1];
                target_node.values[i] = target_node.values[i - 1];
            }
            target_node.keys[1] = tmp.keys[tmp.key_num + 1];
            target_node.values[1] = tmp.values[tmp.key_num + 1];
            file_leaf.WritePage(tmp, elder);
            file_leaf.WritePage(target_node, pos);
            return {1, target_node.keys[1]};
        }
    }
    if (younger != -1) {
        LeafNode tmp = file_leaf.ReadPage(younger);
        if (tmp.key_num > l) {
            --tmp.key_num;
            ++target_node.key_num;
            target_node.keys[target_node.key_num] = tmp.keys[1];
            target_node.values[target_node.key_num] = tmp.values[1];
            for (int i = 1; i <= tmp.key_num; ++i) {
                tmp.keys[i] = tmp.keys[i + 1];
                tmp.values[i] = tmp.values[i + 1];
            }
            file_leaf.WritePage(tmp, younger);
            file_leaf.WritePage(target_node, pos);
            return {2, tmp.keys[1]};
        }
    }
    return {0, keyType()};
}

template <typename keyType, typename valueType, int t, int l>
std::pair<int, keyType> BPlusTree<keyType, valueType, t, l>::BorrowNode(Ptr pos, BPlusTree::node target_node, Ptr elder, Ptr younger, const keyType &left_fa, const keyType &right_fa) {
    if (elder != -1) {
        node tmp = file_node.ReadPage(elder);
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

            file_node.WritePage(tmp, elder);
            file_node.WritePage(target_node, pos);
            return {1, tmp.keys[tmp.key_num + 1]};
        }
    }
    if (younger != -1) {
        node tmp = file_node.ReadPage(younger);
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
            file_node.WritePage(tmp, younger);
            file_node.WritePage(target_node, pos);
            return {2, res};
        }
    }
    return {0, keyType()};
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::MergeLeafNode(Ptr pos, BPlusTree::LeafNode target_node, Ptr brother, int sign) {
    LeafNode tmp = file_leaf.ReadPage(brother);
    if (sign == 1) {
        target_node.key_num = 2 * l - 1;
        for (int i = l; i < 2 * l; ++i) {
            target_node.keys[i] = tmp.keys[i - l + 1];
            target_node.values[i] = tmp.values[i - l + 1];
        }
        target_node.next_leaf = tmp.next_leaf;
        file_leaf.WritePage(target_node, pos);
        file_leaf.DumpPage(brother); // tmp(brother)进垃圾桶
    }
    else {
        tmp.key_num = 2 * l - 1;
        for (int i = l + 1; i < 2 * l; ++i) {
            tmp.keys[i] = target_node.keys[i - l];
            tmp.values[i] = target_node.values[i - l];
        }
        tmp.next_leaf = target_node.next_leaf;
        file_leaf.WritePage(tmp, brother);
        file_leaf.DumpPage(pos); // target_node进垃圾桶
    }
}

template <typename keyType, typename valueType, int t, int l>
void BPlusTree<keyType, valueType, t, l>::MergeNode(Ptr pos, BPlusTree::node target_node, Ptr brother, int sign, const keyType &fa) {
    node tmp = file_node.ReadPage(brother);
    if (sign == 1) {
        target_node.key_num = 2 * t;
        target_node.keys[t] = fa;
        target_node.sons[t] = tmp.sons[0];
        for (int i = t + 1; i <= 2 * t; ++i) {
            target_node.keys[i] = tmp.keys[i - t];
            target_node.sons[i] = tmp.sons[i - t];
        }
        file_node.WritePage(target_node, pos);
        file_node.DumpPage(brother);
    }
    else {
        tmp.key_num = 2 * t;
        tmp.keys[t + 1] = fa;
        tmp.sons[t + 1] = target_node.sons[0];
        for (int i = t + 2; i <= 2 * t; ++i) {
            tmp.keys[i] = target_node.keys[i - t - 1];
            tmp.sons[i] = target_node.sons[i - t - 1];
        }
        file_node.WritePage(tmp, brother);
        file_node.DumpPage(pos);
    }
}

template <typename keyType, typename valueType, int t, int l>
bool BPlusTree<keyType, valueType, t, l>::remove(const keyType &key) {
    if (root == -1) return false;
    if (root_is_leaf) {
        LeafNode tmp = file_leaf.ReadPage(root);
        std::pair<int, bool> res = FindKey(tmp.keys, tmp.key_num, key);
        if (!res.second) return false;
        --tmp.key_num;
        if (tmp.key_num == 0) {
            file_leaf.DumpPage(root);
            root = -1;
            return true;
        }
        for (int i = res.first; i <= tmp.key_num; ++i) {
            tmp.keys[i] = tmp.keys[i + 1];
            tmp.values[i] = tmp.values[i + 1];
        }
        file_leaf.WritePage(tmp, root);
        return true;
    }
    if (EraseFromNode(root, key).first == -1) return false;
    return true;
}

template <typename keyType, typename valueType, int t, int l>
std::pair<bool, valueType> BPlusTree<keyType, valueType, t, l>::FindModify(const keyType &key, bool need_modify, const valueType &value) {
    // 空树
    if (root == -1) return {false, valueType()};
    if (root_is_leaf) return FindModifyInLeafNode(root, key, need_modify, value);
    return FindModifyInLeafNode(FindModifyInNode(root, key), key, need_modify, value);
}

template <typename keyType, typename valueType, int t, int l>
std::pair<bool, valueType> BPlusTree<keyType, valueType, t, l>::FindModifyInLeafNode(Ptr pos, const keyType &key, bool need_modify, const valueType &value) {
    LeafNode tmp = file_leaf.ReadPage(pos);
    std::pair<int, bool> res = FindKey(tmp.keys, tmp.key_num, key);
    if (!res.second) return {false, valueType()}; // 没找到
    if (!need_modify) return {true, tmp.values[res.first]};
    tmp.values[res.first] = value;
    file_leaf.WritePage(tmp, pos);
    return {true, value};
}

template <typename keyType, typename valueType, int t, int l>
Ptr BPlusTree<keyType, valueType, t, l>::FindModifyInNode(Ptr pos, const keyType &key) {
    node tmp = file_node.ReadPage(pos);
    int res = FindKey(tmp.keys, tmp.key_num, key).first;
    if (tmp.son_is_leaf) return tmp.sons[res];
    return FindinNode(tmp.sons[res], key);
}