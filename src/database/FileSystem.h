#ifndef TICKETSYSTEM_FILESYSTEM_H
#define TICKETSYSTEM_FILESYSTEM_H

#include <fstream>
#include <unordered_map>
#include "../utilis/vector.hpp"
#include "../utilis/LinkedHashMap.h"

using Ptr = int;

template <typename page, int m>
class FileSystem {
private:
    const int size_of_page = sizeof (page);
    struct node {
        page data;
        Ptr pos;
        node *pre;
        node *nxt;

        node() : data(), pos(-1), pre(nullptr), nxt(nullptr) {}
        node(const page &data_, const Ptr &pos_, node *prev, node *next) : data(data_), pos(pos_), pre(prev), nxt(next) {}
    };
    std::fstream file;
    std::unordered_map<int, node *> Map;
//    LinkedHashMap<node *, m> unordered_map;
    node *head;
    node *tail;
    int size;
    sjtu::vector<Ptr> recycle_bin;
    Ptr end_of_file = 0;

    // * 写回文件，更新size和hash_map，删除最后一个结点
    void CachePop() {
        node *tmp = tail->pre;
        file.seekp(tmp->pos);
        file.write(reinterpret_cast<char *> (&tmp->data), size_of_page);
        Map[tmp->pos / size_of_page] = nullptr;
//        unordered_map.EraseValueAt(tmp->pos / size_of_page);
        tmp->pre->nxt = tail;
        tail->pre = tmp->pre;
        delete tmp;
        --size;
    }

    // * 若cache已满，先pop
    // * 加入在队头，更新size和hash_map
    void CacheAddPage(const page &obj, const Ptr &pos) {
        if (size == m) CachePop();
        node *tmp = new node(obj, pos, head, head->nxt);
        head->nxt->pre = tmp;
        head->nxt = tmp;
        ++size;
        Map[pos / size_of_page] = tmp;
//        unordered_map.SetValueAt(pos / size_of_page, tmp);
    }

    // * 如果已经是队头，什么都不做
    void CacheMoveToFront(node *tmp) {
        if (tmp->pre == head) return;
        tmp->pre->nxt = tmp->nxt;
        tmp->nxt->pre = tmp->pre;
        tmp->nxt = head->nxt;
        tmp->pre = head;
        head->nxt->pre = tmp;
        head->nxt = tmp;
    }

public:
    // * 完成文件继承
    explicit FileSystem(const std::string &file_name) {
        file.open(file_name, std::ifstream::in | std::fstream::out | std::ifstream::binary);
        if (!file.good()) { // 文件不存在，需要新建一个
            file.close();
            file.clear();
            file.open(file_name,std::fstream::app | std::fstream::binary);
            file.close();
            file.clear();
            file.open(file_name,std::fstream::in | std::fstream::out | std::fstream::binary);
        }
        head = new node();
        tail = new node();
        head->nxt = tail;
        tail->pre = head;
        size = 0;
    }


    // * 把缓存中的记录全部写回文件
    ~FileSystem() {
        while (size > 0) {
            CachePop();
        }
        delete head;
        delete tail;
        file.close();
    }


    page ReadPage(Ptr pos) {
        node *tmp = Map[pos / size_of_page];
        if (tmp) {
            CacheMoveToFront(tmp);
            return tmp->data;
        }
//        std::pair<node *, bool> tmp = unordered_map.FindValueAt(pos / size_of_page);
//        if (tmp.second) {
//            CacheMoveToFront(tmp.first);
//            return tmp.first->data;
//        }
        file.seekg(pos);
        page new_page;
        file.read(reinterpret_cast<char *> (&new_page), size_of_page);
        CacheAddPage(new_page, pos);
        return new_page;
    }

    Ptr WritePage(const page &obj, Ptr pos)  {
        if (pos == -1) {
            if (recycle_bin.empty()) {
                pos = end_of_file;
                end_of_file += size_of_page;
            }
            else {
                pos = recycle_bin.back();
                recycle_bin.pop_back();
            }
        }
        node *tmp = Map[pos / size_of_page];
        if (tmp) {
            tmp->data = obj;
            CacheMoveToFront(tmp);
        }
//        std::pair<node *, bool> tmp = unordered_map.FindValueAt(pos / size_of_page);
//        if (tmp.second) {
//            tmp.first->data = obj;
//            CacheMoveToFront(tmp.first);
//        }
        else {
            CacheAddPage(obj, pos);
        }
        return pos;
    }

    void DumpPage(Ptr pos) {
        recycle_bin.push_back(pos);
    }

};

#endif //TICKETSYSTEM_FILESYSTEM_H
