#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "shapes.hpp"

namespace geo {

class Editor {
public:
    Shape* add(std::unique_ptr<Shape> shape) {
        if (!shape) {
            throw std::invalid_argument("Editor: нельзя добавить пустой указатель");
        }
        _shapes.push_back(std::move(shape));
        return _shapes.back().get();
    }

    bool remove(const Shape* shape) {
        for (auto it = _shapes.begin(); it != _shapes.end(); ++it) {
            if (it->get() == shape) {
                _shapes.erase(it);
                return true;
            }
        }
        return false;
    }

    bool move(const Shape* shape, const Vector2D& v) {
        for (auto& s : _shapes) {
            if (s.get() == shape) {
                s->move(v);
                return true;
            }
        }
        return false;
    }

    Shape* topAt(const Point2D& p) const {
        for (auto it = _shapes.rbegin(); it != _shapes.rend(); ++it) {
            if ((*it)->contains(p)) {
                return it->get();
            }
        }
        return nullptr;
    }

    const Shape* leafAt(const Point2D& p) const {
        for (auto it = _shapes.rbegin(); it != _shapes.rend(); ++it) {
            if (const Shape* found = (*it)->find(p)) {
                return found;
            }
        }
        return nullptr;
    }

    std::vector<Shape*> inRegion(const Rect2D& region) const {
        std::vector<Shape*> result;
        for (const auto& s : _shapes) {
            if ((s->boundingBox() & region).has_value()) {
                result.push_back(s.get());
            }
        }
        return result;
    }

    size_t moveInRegion(const Rect2D& region, const Vector2D& v) {
        std::vector<Shape*> found = inRegion(region);
        for (Shape* s : found) {
            s->move(v);
        }
        return found.size();
    }

    size_t size() const { return _shapes.size(); }

private:
    std::vector<std::unique_ptr<Shape>> _shapes;
};

inline std::string render(const Editor& editor, const Rect2D& area) {
    std::string out;
    for (int y = static_cast<int>(area.top()); y >= static_cast<int>(area.bottom()); --y) {
        for (int x = static_cast<int>(area.left()); x <= static_cast<int>(area.right()); ++x) {
            const Shape* s = editor.leafAt(Point2D(x, y));
            out += s ? s->symbol() : ' ';
        }
        out += '\n';
    }
    return out;
}

}