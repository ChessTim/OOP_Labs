#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "heap.hpp"
#include "scheduler.hpp"

static int g_failed = 0;
#define CHECK(cond) \
    do { if (!(cond)) { ++g_failed; std::cout << "FAIL: " #cond " (строка " << __LINE__ << ")\n"; } } while (0)

template <class F>
bool throws(F f) {
    try { f(); } catch (const std::exception&) { return true; }
    return false;
}

struct ByLength {
    bool operator()(const std::string& a, const std::string& b) const {
        return a.size() < b.size();
    }
};

std::vector<int> idsOf(const std::vector<Task>& tasks) {
    std::vector<int> result;
    for (const Task& t : tasks) {
        result.push_back(t.id());
    }
    return result;
}

template <class Rule>
void fillTasks(Scheduler<Rule>& s) {
    s.add(Task(1, "Написать отчёт", 2, 20));
    s.add(Task(2, "Починить баг", 5, 30));
    s.add(Task(3, "Созвон с заказчиком", 1, 10));
    s.add(Task(4, "Код-ревью", 5, 15));
    s.add(Task(5, "Обновить документацию", 3, 25));
}

void testHeapEmpty() {
    BinaryHeap<int> h;
    CHECK(h.empty());
    CHECK(h.size() == 0);
    CHECK(h.isValid());
    CHECK(throws([&] { h.top(); }));
    CHECK(throws([&] { h.pop(); }));
}

void testHeapSingle() {
    BinaryHeap<int> h;
    h.push(42);
    CHECK(!h.empty());
    CHECK(h.size() == 1);
    CHECK(h.top() == 42);
    CHECK(h.pop() == 42);
    CHECK(h.empty());
    CHECK(throws([&] { h.pop(); }));
}

void testHeapMinOrder() {
    BinaryHeap<int> h;
    for (int v : {5, 3, 8, 1, 9, 2, 7}) {
        h.push(v);
        CHECK(h.isValid());
    }
    CHECK(h.top() == 1);
    CHECK(h.size() == 7);
    std::vector<int> out;
    while (!h.empty()) {
        out.push_back(h.pop());
        CHECK(h.isValid());
    }
    const std::vector<int> expected{1, 2, 3, 5, 7, 8, 9};
    CHECK(out == expected);
}

void testHeapMaxOrder() {
    BinaryHeap<int, std::greater<int>> h;
    for (int v : {5, 3, 8, 1, 9, 2, 7}) {
        h.push(v);
    }
    std::vector<int> out;
    while (!h.empty()) {
        out.push_back(h.pop());
    }
    const std::vector<int> expected{9, 8, 7, 5, 3, 2, 1};
    CHECK(out == expected);
}

void testHeapDuplicates() {
    BinaryHeap<int> h;
    for (int v : {2, 2, 2, 1, 1}) {
        h.push(v);
    }
    std::vector<int> out;
    while (!h.empty()) {
        out.push_back(h.pop());
    }
    const std::vector<int> expected{1, 1, 2, 2, 2};
    CHECK(out == expected);
}

void testHeapOtherTypeAndRule() {
    BinaryHeap<std::string, ByLength> h;
    h.push("ccc");
    h.push("a");
    h.push("dddd");
    h.push("bb");
    CHECK(h.top() == "a");
    CHECK(h.pop() == "a");
    CHECK(h.pop() == "bb");
    CHECK(h.pop() == "ccc");
    CHECK(h.pop() == "dddd");
    CHECK(h.empty());
}

void testHeapMixedOperations() {
    BinaryHeap<int> h;
    h.push(10);
    h.push(4);
    CHECK(h.pop() == 4);
    h.push(1);
    h.push(7);
    CHECK(h.isValid());
    CHECK(h.pop() == 1);
    CHECK(h.pop() == 7);
    CHECK(h.pop() == 10);
    CHECK(h.empty());
}

void testHeapMany() {
    BinaryHeap<int> h;
    bool allValid = true;
    for (int i = 0; i < 100; ++i) {
        h.push((i * 37) % 100);
        if (!h.isValid()) {
            allValid = false;
        }
    }
    CHECK(allValid);
    CHECK(h.size() == 100);
    bool sorted = true;
    for (int expected = 0; expected < 100; ++expected) {
        if (h.pop() != expected) {
            sorted = false;
        }
        if (!h.isValid()) {
            allValid = false;
        }
    }
    CHECK(sorted);
    CHECK(allValid);
    CHECK(h.empty());
}

void testTask() {
    Task t(1, "Тест", 3, 10);
    CHECK(t.id() == 1);
    CHECK(t.title() == "Тест");
    CHECK(t.priority() == 3);
    CHECK(t.deadline() == 10);
    CHECK(throws([] { Task(0, "x", 1, 1); }));
    CHECK(throws([] { Task(-1, "x", 1, 1); }));
    CHECK(throws([] { Task(1, "", 1, 1); }));
    CHECK(throws([] { Task(1, "x", 1, -5); }));
}

void testSchedulerEmpty() {
    Scheduler<ByPriority> s;
    CHECK(s.empty());
    CHECK(s.pendingCount() == 0);
    CHECK(throws([&] { s.next(); }));
    CHECK(throws([&] { s.execute(); }));
    CHECK(s.runAll().empty());
}

void testSchedulerByPriority() {
    Scheduler<ByPriority> s;
    fillTasks(s);
    CHECK(s.pendingCount() == 5);
    const std::vector<int> expected{2, 4, 5, 1, 3};
    CHECK(idsOf(s.runAll()) == expected);
    CHECK(s.empty());
}

void testSchedulerByDeadline() {
    Scheduler<ByDeadline> s;
    fillTasks(s);
    const std::vector<int> expected{3, 4, 1, 5, 2};
    CHECK(idsOf(s.runAll()) == expected);
}

void testSchedulerNextDoesNotRemove() {
    Scheduler<ByPriority> s;
    fillTasks(s);
    CHECK(s.next().id() == 2);
    CHECK(s.next().id() == 2);
    CHECK(s.pendingCount() == 5);
    Task done = s.execute();
    CHECK(done.id() == 2);
    CHECK(s.pendingCount() == 4);
    CHECK(s.next().id() == 4);
}

void testSchedulerDuplicateId() {
    Scheduler<ByPriority> s;
    s.add(Task(1, "Первая", 1, 1));
    CHECK(throws([&] { s.add(Task(1, "Другая", 9, 9)); }));
    CHECK(s.pendingCount() == 1);
    s.execute();
    s.add(Task(1, "Снова первая", 1, 1));
    CHECK(s.pendingCount() == 1);
}

void testSchedulerTies() {
    Scheduler<ByPriority> s;
    s.add(Task(7, "Г", 3, 5));
    s.add(Task(3, "В", 3, 5));
    s.add(Task(5, "Б", 3, 5));
    const std::vector<int> expected{3, 5, 7};
    CHECK(idsOf(s.runAll()) == expected);
}

std::string describe(const Task& t) {
    return "#" + std::to_string(t.id()) + " " + t.title() +
           " (приоритет " + std::to_string(t.priority()) +
           ", срок " + std::to_string(t.deadline()) + ")";
}

template <class Rule>
void runDemo(const std::string& ruleName, Scheduler<Rule>& s) {
    fillTasks(s);
    std::cout << "Правило: " << ruleName << "\n";
    std::cout << "Ожидает задач: " << s.pendingCount() << "\n";
    std::cout << "Следующая (без удаления): " << describe(s.next()) << "\n";
    int step = 1;
    while (!s.empty()) {
        std::cout << step << ") " << describe(s.execute()) << "\n";
        ++step;
    }
    std::cout << "Осталось задач: " << s.pendingCount() << "\n\n";
}

void demoEmpty() {
    Scheduler<ByPriority> s;
    std::cout << "Пустой планировщик, задач: " << s.pendingCount() << "\n";
    try {
        s.next();
    } catch (const std::out_of_range& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
}

int main() {
    testHeapEmpty();
    testHeapSingle();
    testHeapMinOrder();
    testHeapMaxOrder();
    testHeapDuplicates();
    testHeapOtherTypeAndRule();
    testHeapMixedOperations();
    testHeapMany();
    testTask();
    testSchedulerEmpty();
    testSchedulerByPriority();
    testSchedulerByDeadline();
    testSchedulerNextDoesNotRemove();
    testSchedulerDuplicateId();
    testSchedulerTies();

    if (g_failed == 0) std::cout << "Все тесты пройдены\n\n";
    else std::cout << "Провалено проверок: " << g_failed << "\n\n";

    Scheduler<ByPriority> byPriority;
    runDemo("сначала больший приоритет", byPriority);

    Scheduler<ByDeadline> byDeadline;
    runDemo("сначала ближайший срок", byDeadline);

    demoEmpty();
    return g_failed == 0 ? 0 : 1;
}