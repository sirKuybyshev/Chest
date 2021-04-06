#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "test_runner.h"

using namespace std;

class Item {
protected:
    string item_;

public:
    Item() = default;
    explicit Item(string item) : item_(std::move(item)){};
    Item(const Item &item) {
        item_ = item.item_;
    }
    Item(Item &&item) noexcept {
        item_ = move(item.item_);
    }
    Item &operator=(const Item& item) {
        Item item1(item);
        swap(*this, item1);
        return *this;
    }
    Item &operator=(Item &&item) noexcept {
        swap(item, *this);
        return *this;
    }
    friend ostream &operator<<(ostream &out, const Item &item);
    friend bool operator==(const Item &lhs, const Item &rhs);
    friend void swap (Item &lhs, Item &rhs);
};
ostream &operator<<(ostream &out, const Item &item) {
    out << item.item_;
    return out;
}
bool operator==(const Item &lhs, const Item &rhs) {
    return rhs.item_ == lhs.item_;
}
void swap(Item &lhs, Item &rhs) {
    swap(lhs.item_, rhs.item_);
}

class Chest : public Item {
    // Владение указателями не забирается в сундук. За удаление и валидность отвественнен пользователь
    unordered_set<Item *> items_;
    unordered_set<void (*)(Chest *, Item *)> handlers_;

public:
    Chest() = default;
    Chest(const string& name) : Item(name){}
    Chest(const Chest &chest) {
        items_ = chest.items_;
        item_ = chest.item_;
        handlers_ = chest.handlers_;
    }
    Chest(Chest &&chest) noexcept {
        items_ = move(chest.items_);
        item_ = move(chest.item_);
        handlers_ = move(chest.handlers_);
    }
    Chest &operator=(Chest chest) {
        swap(chest, *this);
        return *this;
    }
    Chest &operator=(Chest &&chest) noexcept {
        swap(chest, *this);
        return *this;
    }
    friend void swap (Chest &lhs, Chest &rhs);
    void Notify(Item *item) {
        for (const auto &func : handlers_) {
            func(this, item);
        }
    }
    bool AddItem(Item *item) {
        if (items_.count(item)) {
            return false;
        } else {
            items_.emplace(item);
            Notify(item);
            return true;
        }
    }
    bool ExtractItem(Item *item) {
        if (items_.count(item)) {
            items_.erase(item);
            Notify(item);
            return true;
        } else {
            return false;
        }
    }
    bool AddHandler(void (*func)(Chest *, Item *)) {
        if (handlers_.count(func)) {
            return false;
        } else {
            handlers_.insert(func);
            return true;
        }
    }
    bool ExtractHandler(void (*func)(Chest *, Item *)) {
        if (handlers_.count(func)) {
            handlers_.erase(func);
            return true;
        } else {
            return false;
        }
    }
    string GetName() {
        return item_;
    }
};
void swap(Chest &lhs, Chest &rhs) {
    swap(lhs.items_, rhs.items_);
    swap(lhs.item_, rhs.item_);
    swap(lhs.handlers_, rhs.handlers_);
}

void TestSanity() {
    auto item1 = make_unique<Item>("Hello");
    auto item2 = make_shared<Item>("Hello, there");
    Chest chest("s");
    chest.AddItem(item1.get());
    chest.AddHandler([](Chest *chest1, Item *item1) {
        cout << "Handled first: " << *item1 << ' ' <<  chest1->GetName() << '\n';
    });
    chest.AddItem(item2.get());
    auto item3 = item2;
    ASSERT(!chest.AddItem(item1.get()))
    auto lambda = [](Chest *chest1, Item *item1) {
      cout << "Handled second: " <<*item1 << ' ' <<  chest1->GetName() << '\n';
    };
    chest.AddHandler(lambda);
    Chest bigChest("b");
    bigChest.AddHandler(lambda);
    bigChest.AddItem(&chest);
    chest.AddItem(item3.get());
    *item3 = Item("There");
    chest.ExtractItem(item3.get());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
}
