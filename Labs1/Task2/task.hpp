#pragma once
#include <string>
#include <stdexcept>
#include <utility>

// Предметная область: одна задача. Знает только про себя, ничего про кучу и планировщик.
// Инварианты: id > 0, название не пустое, срок >= 0.
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
