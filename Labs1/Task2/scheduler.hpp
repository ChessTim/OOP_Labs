#pragma once
#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "heap.hpp"

class Task {
public:
    Task(int id, std::string title, int priority, int deadline)
        : _id(id), _title(std::move(title)), _priority(priority), _deadline(deadline) {
        if (id <= 0) {
            throw std::invalid_argument("Task: id должен быть положительным");
        }
        if (_title.empty()) {
            throw std::invalid_argument("Task: название не может быть пустым");
        }
        if (deadline < 0) {
            throw std::invalid_argument("Task: срок не может быть отрицательным");
        }
    }

    int id() const { return _id; }
    const std::string& title() const { return _title; }
    int priority() const { return _priority; }
    int deadline() const { return _deadline; }

private:
    int _id;
    std::string _title;
    int _priority;
    int _deadline;
};

struct ByPriority {
    bool operator()(const Task& a, const Task& b) const {
        if (a.priority() != b.priority()) {
            return a.priority() > b.priority();
        }
        return a.id() < b.id();
    }
};

struct ByDeadline {
    bool operator()(const Task& a, const Task& b) const {
        if (a.deadline() != b.deadline()) {
            return a.deadline() < b.deadline();
        }
        return a.id() < b.id();
    }
};

template <class Rule>
class Scheduler {
public:
    explicit Scheduler(Rule rule = Rule()) : _queue(std::move(rule)) {}

    void add(const Task& task) {
        if (_ids.count(task.id()) > 0) {
            throw std::invalid_argument("Scheduler: задача с таким id уже есть");
        }
        _ids.insert(task.id());
        _queue.push(task);
    }

    const Task& next() const { return _queue.top(); }

    Task execute() {
        Task task = _queue.pop();
        _ids.erase(task.id());
        return task;
    }

    std::size_t pendingCount() const { return _queue.size(); }

    bool empty() const { return _queue.empty(); }

    std::vector<Task> runAll() {
        std::vector<Task> done;
        while (!empty()) {
            done.push_back(execute());
        }
        return done;
    }

private:
    BinaryHeap<Task, Rule> _queue;
    std::set<int> _ids;
};