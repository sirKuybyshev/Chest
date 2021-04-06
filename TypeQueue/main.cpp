//
// Created by timofey on 4/6/21.
//
#include "tQueue.h"
#include <iostream>
#include <utility>

#define ADD_TYPE(g, queue) AddType<g, typeof(queue)>()
#define POP_TYPE(queue) std::make_pair(GetHeadT<typeof(queue)>(), PopFrontT<typeof(queue)>())

struct s {};
struct n {
    uint32_t a;
};
struct b {
    uint64_t b;
};
struct g {
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
};
struct e {
    uint64_t b1;
    uint64_t b2;
    uint64_t b3;
    uint64_t b4;
};


int main() {
    using namespace TopSort;
    SortT<TypeList<s, e, b, s, n>> queue;
    std::cout << typeid(queue).name() << '\n';
    auto secondQueue = ADD_TYPE(g, queue);
    std::cout << typeid(secondQueue).name() << '\n';
    auto [biggestType, thirdQueue] = POP_TYPE(secondQueue);
    std::cout << typeid(biggestType).name() << '\n';
    std::cout << typeid(thirdQueue).name() << '\n';
    std::cout << typeid(ADD_TYPE(e, thirdQueue)).name() << '\n';
    return EXIT_SUCCESS;
}