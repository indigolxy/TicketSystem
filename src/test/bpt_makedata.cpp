#include <iostream>
#include <random>
#include "../utils/vector.hpp"

int main() {
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(1, 30000);
    e.seed(time(0));
    sjtu::vector<int> k;

    for (int i = 0; i < 20000; ++i) {
        int tmp = u(e);
        std::cout << tmp << '\n';
        k.push_back(tmp);
    }
    for (int i = 0; i < 19900; ++i) {
        std::uniform_int_distribution<int> v(0, 19999 - i);
        int tmp = v(e);
        std::cout << k[tmp] << '\n';
        k.erase(tmp);
    }
    for (int i = 0; i < 90; ++i) {
        std::cout << k[i] << '\n';
    }

    return 0;
}
