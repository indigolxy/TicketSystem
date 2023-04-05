#include "bpt.h"

template <typename keyType, typename valueType, int t>
BPlusTree<keyType, valueType, t>::BPlusTree(const std::string &file_name) {
    file.open(file_name,std::fstream::app | std::fstream::binary);
    file.close();
    file.clear();
    file.open(file_name,std::fstream::in | std::fstream::out | std::fstream::binary);
    root = -1;
}

template <typename keyType, typename valueType, int t>
BPlusTree<keyType, valueType, t>::~BPlusTree() {
    file.close();
    file.clear();
}

