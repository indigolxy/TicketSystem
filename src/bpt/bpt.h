
#ifndef TICKETSYSTEM_BPT_H
#define TICKETSYSTEM_BPT_H

#include <fstream>
#include "../utilis/vector.hpp"

using Ptr = int;

template <typename keyType, typename valueType, int t>
class BPlusTree {
private:
    class node {
        int key_num = 0;
        Ptr parent = -1;
        bool son_is_leaf = true;
        std::pair<keyType, valueType> keys[2 * t];
        Ptr sons[2 * t] = {-1};
    };

    class LeafNode {
        int key_num = 0;
        Ptr next_leaf = -1;
        Ptr parent = -1;
        std::pair<keyType, valueType> keys[2 * t];
    };

    std::fstream file;
    Ptr root;

public:

    explicit BPlusTree(const std::string &file_name);
    ~BPlusTree();

    void insert(const keyType &key, const valueType &value);

    void remove(const keyType &key, const valueType &value);

    sjtu::vector<valueType> find(const keyType &key);
};

template class BPlusTree<std::string, int, 3>;

#endif //TICKETSYSTEM_BPT_H
