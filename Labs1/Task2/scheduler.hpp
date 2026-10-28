#pragma once
#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "heap.hpp"
#include "task.hpp"

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