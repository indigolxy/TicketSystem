
#ifndef BOOKSTORE_2022_1774_H
#define BOOKSTORE_2022_1774_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <set>
#include <vector>

constexpr int MaxBits = 66;
constexpr int MAXSIZE = 2000; // !!1-base!!
constexpr int MINSIZE = 1000; // !!1-base!!

#pragma pack(push,1)
class _pair {
public:
    char index[MaxBits] = {'\0'};
    int value;
    _pair(const char * _index,int value) : value(value){
        strcpy(index,_index);
    }
    _pair() : value(0) {}

    bool operator==(const _pair &obj);

    bool operator<(const _pair &obj);

    bool operator>(const _pair &obj) { return !(*this < obj || *this == obj); }

    bool operator<=(const _pair &obj) { return *this < obj || *this == obj; }

    bool operator>=(const _pair &obj) { return !(*this < obj); }
};

class node {
    friend class BlockList;
    _pair st;
    _pair ed;
    int size; // !!1-base!!
    long prev;
    long next;
    _pair data[MAXSIZE];

public:
    node() : prev(-1),next(-1),size(1) {};
    node(_pair st,_pair ed,int size,long prev,long next) : st(st),ed(ed),size(size),prev(prev),next(next) {}
};

#pragma pack(pop)

class BlockList {
//    friend int main();
    std::fstream file; // "other.txt" "file.txt"
    bool flag_start = false;
    long tail = 0;

public:

    /*void printAll() {
        long p = 0;
        while (p != -1) {
            node nd = ReadNode(p);
            p = nd.next;
            std::cout << "[" << nd.st.index << "," << nd.st.value << "] ";
            std::cout << "{" << nd.ed.index << "," << nd.ed.value << "} ";

            for (int i = 0; i < nd.size; ++i) {
                std::cout << "(" << nd.data[i].index << "," << nd.data[i].value << ") ";
            }
            std::cout << std::endl;
        }
    }*/

    BlockList();

    ~BlockList();

    /*
     * 从file的pos位置读入一个完整node
     * 函数执行结束后读指针走到data的最后
     */

    node ReadNode(const long &pos);

    /*
     * 向file的pos位置中写入一个完整的node
     * 写完后写指针移动至data[MAXSIZE - 1]的最后(empty_pair也要写)
     * 若在文件最后写入，pos == -1
     * 否则pos为要写入的块的位置
     */

    void WriteNode(const node &obj,const long &pos);

    /*
     * 找到target应当属于的block（无论target是否存在）
     * 若target属于一个block的[st,ed]，返回该block的起始位置
     * 若target位于两个block的ed和st之间，返回后一个block的起始位置
     * 若target>最大的ed，返回-1
     */

    long FindTheBlock(const _pair &target);

    /*
     * 找到第一个index_pair应当出现的block（无论index是否存在）
     * 若index属于一个block的[st,ed]，返回该block的起始位置
     * 若index位于两个block的ed和st之间，返回后一个block的起始位置
     * 若index>最大的ed，返回-1
     */

    long FindTheBlockIndex(const char *index);

    /*
     * 从p块中删除data[num]
     * 会修改st，ed(if necessary)（文件和node_p都修改）
     * 会修改node_p和文件中的size
     * 会将num后的元素前移一位（文件和node_p都会修改）
     */

    void RemovePair(node &node_p,const long &p,const int &num);

    /*
     * 在一个block内插入target（插入为data[num]）
     * num <= node_p.size(在尾部添加时num == node_p.size)
     * 文件和node_p均修改：st,ed,size,data
     */

    void InsertPair(node &node_p,const long &p,const _pair &target,const int &num);

    /*
     * 插入：[st,ed]插入
     * 前一个的ed与后一个的st之间：插入后面
     * size==maxsize - 1时插入后裂开，size<maxsize - 1时直接插
     * 如果增加新节点，data数组必须开满，不足的用empty_pair填充
     * 增加一个新节点：直接在文件末尾增加————节点在文件中不按顺序存储！
     * 新增节点需考虑更新tail
     * 在文件开头创建第一个块（节点）
     * head 恒等于 0 (裂块向后裂，并块并到前一个)，保持前一个块不动
     * 若已存在与target一模一样的元素，则不插入
    */

    void insert(char *index,const int &value);

    /*
     * 把node_q里的第take_num个元素借到node_p的第insert_num个元素位置
     * 同时修改node和文件
     */

    void BorrowElement(node &node_q,const long &q,node &node_p,const long &p,int take_num,int insert_num);

    /*
     * 并块：把p并到q的后面（q为p的前驱）
     * 更新tail (if necessary)
     * 同时修改node和文件
     */

    void Merge(node &node_p,const long &p,node &node_q,const long &q);

    /*
     * 删除以 [index] 为索引，值为 [value] 的条目。请注意，删除的条目可能不存在
     * 若删完后（--size后）size<MINSIZE，需要并块（==MINSIZE不并）
     *      具体逻辑：if (p == tail) : 向前驱借元素或并到前驱
     *              if (p == 0) 向后驱借元素或把后驱并过来 (保持head不变)
     *              else 向后驱借->向前驱借->并到前驱
     * head 恒等于 0 (裂块向后裂，并块并到前一个)，保持前一个块不变
     */

    void remove(char *index,const int &value);

    /*
     * 找出所有索引为 [index] 的条目，并以 [value] 升序顺序输出，每个 [value] 间用一个空格隔开。
     * 若索引值为 [index] 的条目不存在，则输出 [null]。
     * 在整个指令输出结束后输出 \n
     */

    std::vector<int> find(char *index);

};

bool _pair::operator==(const _pair &obj) {
    if (strcmp(index,obj.index) == 0 && value == obj.value) return true;
    else return false;
}

bool _pair::operator<(const _pair &obj) {
    if (strcmp(index,obj.index) < 0) return true;
    else if (strcmp(index,obj.index) > 0) return false;
    else {
        if (value < obj.value) return true;
        else return false;
    }
}

BlockList::BlockList() {
    file.open("other.txt",std::ifstream::in | std::ifstream::binary);
    if (file.good()) { // 文件存在 需要读入
        file.seekg(std::ios::beg);
        file.read(reinterpret_cast<char *> (&flag_start),sizeof(bool));
        file.read(reinterpret_cast<char *> (&tail),sizeof(long));
    }
    file.close();
    file.clear();
    file.open("file.txt",std::fstream::app | std::fstream::binary);
    file.close();
    file.clear();
    file.open("file.txt",std::fstream::in | std::fstream::out | std::fstream::binary);
}

BlockList::~BlockList() {
    file.close();
    file.clear();
    file.open("other.txt",std::ofstream::out | std::ofstream::binary);
    file.write(reinterpret_cast<const char *> (&flag_start),sizeof(bool));
    file.write(reinterpret_cast<const char *> (&tail),sizeof(long));
    file.close();
    file.clear();
}

node BlockList::ReadNode(const long &pos) { // 读完读指针走到data有效数据的最后
    file.seekg(pos);
    node tmp;
    file.read(reinterpret_cast<char *>(&tmp), sizeof(node));
    return tmp;
}

void BlockList::WriteNode(const node &obj,const long &pos) {
    if (pos == -1) file.seekp(0,std::ios::end);
    else file.seekp(pos);
    file.write(reinterpret_cast<char const*>(&obj), sizeof(node));
}

long BlockList::FindTheBlock(const _pair &target) {
    long p = 0;
    while (true) {
        file.seekg(p + sizeof(_pair));
        _pair p_ed;
        file.read(reinterpret_cast<char *> (&p_ed),sizeof(_pair));
        if (p_ed >= target) {
            return p;
        }
        else {
            if (p == tail) return -1;
            // p = node_p.next;
            file.seekg(p + sizeof(_pair) * 2 + sizeof(int) + sizeof(long));
            file.read(reinterpret_cast<char *> (&p),sizeof(long));
        }
    }
}

long BlockList::FindTheBlockIndex(const char *index) {
    long p = 0;
    while (true) {
        file.seekg(p + sizeof(_pair));
        char p_ed_index[MaxBits] = {0};
        file.read(reinterpret_cast<char *> (&p_ed_index),MaxBits);
        if (strcmp(index,p_ed_index) <= 0) return p;
        if (p == tail) return -1;
        // p = node_p.next;
        file.seekg(p + sizeof(_pair) * 2 + sizeof(int) + sizeof(long));
        file.read(reinterpret_cast<char *> (&p),sizeof(long));
    }
}

void BlockList::RemovePair(node &node_p,const long &p,const int &num) {
    if (num == 0 && num == node_p.size - 1) {
        _pair empty_pair;
        node_p.st = empty_pair;
        node_p.ed = empty_pair;
        if (p == 0) flag_start = false;
    }
    if (num == 0) {
        node_p.st = node_p.data[1];
    }
    else if (num == node_p.size - 1) {
        node_p.ed = node_p.data[node_p.size - 2];
    }

    --node_p.size;
    for (int i = num;i < node_p.size;++i) {
        node_p.data[i] = node_p.data[i + 1];
    }
    _pair empty_pair;
    node_p.data[node_p.size] = empty_pair;

    WriteNode(node_p,p);
}

void BlockList::InsertPair(node &node_p,const long &p,const _pair &target,const int &num) {
    if (num == 0) {
        node_p.st = target;
    }
    else if (num == node_p.size) {
        node_p.ed = target;
    }
    for (int i = node_p.size;i > num;--i) {
        node_p.data[i] = node_p.data[i - 1];
    }
    node_p.data[num] = target;
    ++node_p.size;

    WriteNode(node_p,p);
}

void BlockList::insert(char *index,const int &value) {
    const _pair target(index,value);
    if (!flag_start) {
        // 第一次insert:the very first _pair data
        // 在文件开头创建第一个块（节点）
        node tmp(target,target,1,-1,-1);
        tmp.data[0] = target;
        WriteNode(tmp,0);

        flag_start = true;
        tail = 0;
        return;
    }

    long p = FindTheBlock(target);
    if (p == -1) p = tail; // 遍历到最后都>每一个ed：插入最后一个节点或者新增一个节点 相当于插入最后一个节点（或许再裂块
    node node_p = ReadNode(p);

    // 先直接插
    int i = 0;
    while (node_p.data[i] < target && i < node_p.size) {
        ++i;
    }
    if (node_p.data[i] ==  target) return; // 若重复，不插入
    InsertPair(node_p,p,target,i);

    // 再裂开:向后增加 (考虑更新tail）
    if (node_p.size == MAXSIZE) {
        // 增加一个新节点：直接在文件末尾增加————节点在文件中不按顺序存储！
        node tmp(node_p.data[MINSIZE],node_p.data[MAXSIZE - 1],MINSIZE,p,node_p.next);
        for (int j = MINSIZE;j < MAXSIZE;++j) {
            tmp.data[j - MINSIZE] = node_p.data[j];
        }
        file.seekp(0, std::ios::end);
        const long q = file.tellp();
        WriteNode(tmp,q);

        long p_next_original = node_p.next;

        // 修改p中数据
        node_p.ed = node_p.data[MINSIZE - 1];
        node_p.size = MINSIZE;
        node_p.next = q;
        _pair empty_pair;
        for (int j = MINSIZE;j < MAXSIZE;++j) {
            node_p.data[j] = empty_pair;
        }
        WriteNode(node_p,p);

        if (p == tail) tail = q;
        else {
            // p->next->prev = q;
            file.seekp(p_next_original + 2 * sizeof(_pair) + sizeof(int));
            file.write(reinterpret_cast<const char *> (&q), sizeof(long));
        }
    }
}

void BlockList::BorrowElement(node &node_q,const long &q,node &node_p,const long &p,int take_num,int insert_num) {
    _pair borrow = node_q.data[take_num];
    RemovePair(node_q,q,take_num);
    InsertPair(node_p,p,borrow,insert_num);
}

void BlockList::Merge(node &node_p,const long &p,node &node_q,const long &q) {
    // 先修改前驱数据
    node_q.ed = node_p.ed;
    node_q.next = node_p.next;
    for (int i = 0;i < node_p.size;++i) {
        node_q.data[node_q.size + i] = node_p.data[i];
    }
    node_q.size = node_q.size + node_p.size;
    WriteNode(node_q,q);

    // 再修改node_p.next.prev = q
    if (p == tail) tail = q;
    else {
        file.seekp(node_p.next + 2 * sizeof(_pair) + sizeof(int));
        file.write(reinterpret_cast<const char *> (&q),sizeof(long));
    }
}

void BlockList::remove(char *index,const int &value) {
    if (!flag_start) return;

    const _pair target(index,value);
    long p = FindTheBlock(target);
    if (p == -1) return;
    node node_p = ReadNode(p);
    if (node_p.st > target || node_p.ed < target) return; // target不存在
    int num_of_target = -1;
    for (int i = 0;i < node_p.size;++i) {
        if (node_p.data[i] == target) { // found it!
            num_of_target = i;
            break;
        }
    }
    if (num_of_target  == -1) return; // target不存在

    RemovePair(node_p,p,num_of_target); // node_p也完全修改

    if (node_p.size >= MINSIZE) return;
    if (p == tail && p == 0) return; // 只有一个块，不并

    // 开始借元素or并块
    if (p == 0) {
        long q_next = node_p.next;
        node node_q_next = ReadNode(q_next);
        if (node_q_next.size > MINSIZE) {
            BorrowElement(node_q_next, q_next, node_p, p, 0, node_p.size);
            return;
        }
        Merge(node_q_next,q_next,node_p,p);
        return;
    }
    if (p == tail) {
        long q_prev = node_p.prev;
        node node_q_prev = ReadNode(q_prev);
        if (node_q_prev.size > MINSIZE) {
            BorrowElement(node_q_prev,q_prev,node_p,p,node_q_prev.size - 1,0);
            return;
        }
        Merge(node_p,p,node_q_prev,q_prev);
        return;
    }
    long q_next = node_p.next;
    node node_q_next = ReadNode(q_next);
    if (node_q_next.size > MINSIZE) {
        BorrowElement(node_q_next,q_next,node_p,p,0,node_p.size);
        return;
    }
    long q_prev = node_p.prev;
    node node_q_prev = ReadNode(q_prev);
    if (node_q_prev.size > MINSIZE) {
        BorrowElement(node_q_prev,q_prev,node_p,p,node_q_prev.size - 1,0);
        return;
    }
    Merge(node_p,p,node_q_prev,q_prev);
    return;
}

std::vector<int> BlockList::find(char *index) {
    std::vector<int> ans;
    if (!flag_start) return ans;
    long p = FindTheBlockIndex(index);
    if (p == -1) return ans;

    node node_p = ReadNode(p);
    if (strcmp(index,node_p.st.index) < 0) return ans; // index不存在

    bool found_index = false;
    for (int i = 0;i < node_p.size;++i) {
        if (strcmp(node_p.data[i].index,index) == 0) { // found it!
            found_index = true;
            ans.push_back(node_p.data[i].value);
        }
        else if (found_index) {
            return ans; // 在这个block内找完了
        }
    }
    if (!found_index) return ans; // index不存在

    while (true) {
        if (p == tail) return ans;
        p = node_p.next;
        node_p = ReadNode(p);
        for (int i = 0;i < node_p.size;++i) {
            if (strcmp(node_p.data[i].index,index) == 0) {
                ans.push_back(node_p.data[i].value);
            }
            else {
                return ans; // 在这个block内找完了
            }
        }
    }
}

int main() {
    int n = 0;
    std::cin >> n;
    std::string command;
    BlockList the_list;
    while (n--) {
        std::cin >> command;
        char index[MaxBits] = {0};

        if (command == "insert") {
            int value;
            std::cin >> index >> value;
            the_list.insert(index,value);
        }
        else if (command == "delete") {
            int value;
            std::cin >> index >> value;
            the_list.remove(index,value);
        }
        else if (command == "find") {
            std::cin >> index;
            std::vector<int> ans = the_list.find(index);

            if (ans.empty()) std::cout << "null" << std::endl;
            else {
                for (auto i : ans) {
                    std::cout << i << ' ';
                }
                std::cout << '\n';
            }
        }
//        if (n == 479 || n == 480 || n == 481) std::cout << "STATUS: " << n << ' ' << bool(the_list.file) << std::endl;
//        the_list.printAll();
    }
    return 0;
}


#endif //BOOKSTORE_2022_1774_H