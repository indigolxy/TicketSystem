#include "bpt.h"
#include <random>

int main() {
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(1, 1e5);
    e.seed(time(0));

    BPlusTree<int, int, 2, 2> a("file");

    for (int i = 0; i < 50000; ++i) {
        int tmp = u(e);
        a.insert(tmp, tmp);
    }

    a.print();
    return 0;
}
