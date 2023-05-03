
#ifndef TICKETSYSTEM_LINKEDHASHMAP_H
#define TICKETSYSTEM_LINKEDHASHMAP_H

template <typename valueType, int m>
class LinkedHashMap {
private:
    struct node {
        int index;
        valueType value;
        node *nxt;

        node() : index(0), value(), nxt(nullptr) {}
        node(const int &ind, const valueType &val, node *next) : index(ind), value(val), nxt(next) {}
    };
    node *hash_map[m + 5] = {nullptr};

public:
    LinkedHashMap() {
        for (int i = 0; i < m; ++i) {
            hash_map[i] = new node();
        }
    }

    ~LinkedHashMap() {
        for (int i = 0; i < m; ++i) {
            node *tmp = hash_map[i];
            while (tmp) {
                node *p = tmp->nxt;
                delete tmp;
                tmp = p;
            }
        }
    }

    std::pair<valueType, bool> FindValueAt(int index) {
        node *tmp = hash_map[index % m];
        while (tmp->nxt) {
            tmp = tmp->nxt;
            if (tmp->index == index) return {tmp->value, true};
        }
        return {valueType(), false};
    }

    // 如果index已经存在，修改值，否则添加一个结点
    void SetValueAt(int index, const valueType &value) {
        node *tmp = hash_map[index % m];
        while (tmp->nxt) {
            tmp = tmp->nxt;
            if (tmp->index == index) {
                tmp->value = value;
                return;
            }
        }
        tmp->nxt = new node(index, value, nullptr);
    }

    // 如果index不存在，什么都不做
    void EraseValueAt(int index) {
        node *tmp = hash_map[index % m];
        while (tmp->nxt) {
            node *p = tmp->nxt;
            if (p->index == index) {
                tmp->nxt = p->nxt;
                delete p;
                return;
            }
            tmp = tmp->nxt;
        }
    }

};

#endif //TICKETSYSTEM_LINKEDHASHMAP_H
