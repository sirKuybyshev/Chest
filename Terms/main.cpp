//
// Created by timofey on 4/6/21.
//
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "test_runner.h"

using namespace std;
using namespace std::placeholders;

#define ADD_TERM(vault, term) (vault).AddTerm((term), &(term)->func_)

struct BasicTerm {
    virtual ~BasicTerm() = default;
};

template<typename Type, typename... Args>
struct Term : BasicTerm {
    explicit Term(function<Type(Args...)> func) : func_(std::move(func)) {}
    template<typename... Holders>
    void SetHolders(Holders... holders) {
        binded_ = bind(func_, forward<Holders>(holders)...);
    }
    template<typename... Vars>
    Type operator()(Vars... vars) {
        if (binded_) {
            return binded_(forward<Vars>(vars)...);
        } else {
            return func_(forward<Vars>(vars)...);
        }
    }
    function<Type(Args...)> func_;
    function<Type(Args...)> binded_ = nullptr;
};


class Vault {
    using Bucket = unordered_multimap<void *, shared_ptr<BasicTerm>>;

    int bucketCount_ = 5;
    vector<Bucket> vault_;
    vector<mutex> mutexes_;

public:
    Vault() : vault_(bucketCount_), mutexes_(bucketCount_){};
    explicit Vault(int bucketCount) : bucketCount_(bucketCount), vault_(bucketCount_), mutexes_(bucketCount_){};
    void AddTerm(shared_ptr<BasicTerm> term, void *func) {
        size_t bucket = hash<void *>{}(func) % bucketCount_;
        lock_guard<mutex> lg(mutexes_[bucket]);
        vault_[bucket].emplace(func, move(term));
    };
    template<typename Type, typename... Args>
    shared_ptr<Term<Type, Args...>> ExtractTerm(const shared_ptr<Term<Type, Args...>> &term) {
        size_t bucket = hash<void *>{}(&term->func_) % bucketCount_;
        shared_ptr<Term<Type, Args...>> term2;
        {
            lock_guard<mutex> lg(mutexes_[bucket]);
            auto item = vault_[bucket].find(&term->func_);
            if (item == vault_[bucket].end()) {

                throw out_of_range("No such term");
            }
            auto node = vault_[bucket].extract(item);
            term2 = reinterpret_pointer_cast<Term<Type, Args...>>(node.mapped());
        }
        return term2;
    };
};

void PromisesAdding(Vault &vault, vector<shared_ptr<Term<int, int>>> &terms, int N, int Z) {
    using term = Term<int, int>;
    for (int i = N; i < Z; i++) {
        function<int(int)> lambda = [i](int x) {
            cout << x << ' ' << i << '\n';
            return x + i;
        };
        terms[i] = make_shared<term>(lambda);
        ADD_TERM(vault, terms[i]);
    }
}
void PromisesExtracting(Vault &vault, vector<shared_ptr<Term<int, int>>> &terms, int N, int Z) {
    using term = Term<int, int>;
    for (int i = N; i < Z; i++) {
        (*vault.ExtractTerm(terms[i]))(i);
    }
}


void TestConcurrency() {
    using term = Term<int, int>;
    using termPtr = shared_ptr<term>;
    vector<termPtr> terms(10'000);
    Vault vault;
    {//Оборачиваем futures для ожидания окончания выполнения via destructor
        vector<future<void>> futures;
        for (int i = 0; i < terms.size(); i += terms.size() / 4) {
            int N = i;
            int Z = i + terms.size() / 4;
            futures.push_back(async([&vault, &terms, N, Z]() { PromisesAdding(ref(vault), ref(terms), N, Z); }));
        }
    }
    {
        vector<future<void>> futures;
        for (int i = 0; i < terms.size(); i += terms.size() / 4) {
            int N = i;
            int Z = i + terms.size() / 4;
            futures.push_back(async([&vault, &terms, N, Z]() { PromisesExtracting(ref(vault), ref(terms), N, Z); }));
        }
    }
}

void TestSanity() {
    function<int(int)> lambda = [](int x) {
        cout << x << '\n';
        return x;
    };
    auto term = make_shared<Term<int, int>>(lambda);
    auto term1 = make_shared<Term<int, int, int>>([](int x, int y) {
        cout << x << ' ' << y << '\n';
        return x + y;
    });
    term1->SetHolders(1, _2);
    Vault vault;
    ADD_TERM(vault, term);
    ADD_TERM(vault, term1);
    auto term2 = vault.ExtractTerm(term1);
    cout << (*term2)(10, 10) << '\n';
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    RUN_TEST(tr, TestConcurrency);
    return EXIT_SUCCESS;
}