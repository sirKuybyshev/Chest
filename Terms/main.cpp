//
// Created by timofey on 4/6/21.
//
#include <iostream>
#include <future>
#include <vector>

#include "test_runner.h"
#include "vault.h"

void PromisesAdding(Vault &vault, vector<shared_ptr<Term<int, int>>> &terms, uint beginSlice, uint endSlice) {
    for (uint i = beginSlice; i < endSlice; i++) {
        function<int(int)> lambda = [i](int x) {
          cout << x << ' ' << i << '\n';
          return x + i;
        };
        terms[i] = make_shared<Term<int, int>>(lambda);
        ADD_TERM(vault, terms[i]);
    }
}
void PromisesExtracting(Vault &vault, vector<shared_ptr<Term<int, int>>> &terms, uint beginSlice, uint endSlice) {
    for (uint i = beginSlice; i < endSlice; i++) {
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
        for (uint i = 0; i < terms.size(); i += terms.size() / 4) {
            uint beginSlise = i;
            uint endSlice = i + terms.size() / 4;
            futures.push_back(async([&vault, &terms, beginSlise, endSlice]()
                                    {PromisesAdding(ref(vault), ref(terms), beginSlise, endSlice);}));
        }
    }
    {
        vector<future<void>> futures;
        for (uint i = 0; i < terms.size(); i += terms.size() / 4) {
            uint beginSlise = i;
            uint endSlice = i + terms.size() / 4;
            futures.push_back(async([&vault, &terms, beginSlise, endSlice]()
                                    {PromisesExtracting(ref(vault), ref(terms), beginSlise, endSlice);}));
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
    term->SetHolders(1);
    Vault vault;
    ADD_TERM(vault, term);
    ADD_TERM(vault, term1);
    auto term2 = vault.ExtractTerm(term1);
    auto term3 = vault.ExtractTerm(term);
    cout << (*term2)(10, 10) << '\n';
    cout << (*term3)(15) << '\n';
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestConcurrency);
    RUN_TEST(tr, TestSanity);
    return EXIT_SUCCESS;
}