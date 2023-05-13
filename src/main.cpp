
#include "TicketSystem.h"

/*
bool comp(const MyPair<64> &a, const MyPair<64> &b) {
    return strcmp(a.k.data, b.k.data) < 0;
}
 */

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

    BPlusTree<MyPair<64>, int, 27, 27> b("bpt_file1", "bpt_file2", "bpt_file3");
    int n;
    std::cin >> n;
    std::string instruction;
    std::string key;
    int value;
    for (int i = 1; i <= n; ++i) {
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
//            b.print();
        }
//        std::cout << i << std::endl;
//        b.print();
    }
     */
    TicketSystem ticket_system("order.file", "train.file", "user.file");
    std::string cmd;
    std::cin >> cmd;
    ticket_system.AcceptMsg(cmd);
    return 0;
}
