#include "bpt.h"
#include <random>

bool comp(const MyPair &a, const MyPair &b) {
    return a.k < b.k;
}

int main() {
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(1, 1e5);
    e.seed(time(0));

    BPlusTree<int, int, 2, 2> a("file");

    sjtu::vector<int> insert_key;

//    a.insert({1, 1}, 1);
//    a.insert({2, 1}, 1);
//    a.insert({1, 2}, 2);
//    a.insert({3, 1}, 1);
//    a.insert({3, 2}, 2);
//    a.insert({1, 3}, 3);
//    a.insert({2, 3}, 3);
//    a.insert({4, 3}, 3);
//    a.insert({1, 4}, 4);
//    a.insert({4, 2}, 2);

//    a.remove({2, 1});
//    a.remove({1, 4});
//    a.remove({4, 3});
//    a.remove({3, 1});
//    a.remove({1, 1});

//    a.print();

//    a.find({1, 0}, comp).print();
//    a.find({2, 0}, comp).print();
//    a.find({3, 0}, comp).print();
//    a.find({5, 0}, comp).print();
//    a.find({4, 0}, comp).print();
//    for (int i = 0; i < 100; ++i) {
//        int tmp = u(e);
//        insert_key.push_back(tmp);
//        for (int j = 0; j < 10; ++j) {
//            a.insert({tmp, tmp + j}, tmp + j);
//        }
//    }
//
//    for (int i = 0; i < 10; ++i) {
//        int tmp = insert_key[i + 14];
//        a.find({tmp, 0}, comp).print();
//    }

//    a.print();

    return 0;
}
