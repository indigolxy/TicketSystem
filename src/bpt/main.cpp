#include "bpt.h"

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

    BPlusTree<MyPair, int, 2, 2> b("file");
    int n;
    std::cin >> n;
    std::string instruction;
    std::string key;
    int value;
    while (n--) {
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
            b.print();
        }
    }

    return 0;
}
