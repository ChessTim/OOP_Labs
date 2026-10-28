#pragma once
#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "geometry.hpp"

namespace geo {

inline std::string fmt(double v) {
    std::ostringstream os;
    os << v;
    return os.str();
}

inline std::string fmt(const Point2D& p) {
    return "(" + fmt(p.x()) + ", " + fmt(p.y()) + ")";
}

class Shape {
public:
    virtual ~Shape() = default;

    virtual Rect2D boundingBox() const = 0;
    virtual void move(const Vector2D& v) = 0;
    virtual bool contains(const Point2D& p) const = 0;
    virtual std::string toString() const = 0;
    virtual char symbol() const = 0;

    virtual const Shape* find(const Point2D& p) const {
        return contains(p) ? this : nullptr;
    }

    virtual bool holds(const Shape* s) const { return s == this; }
};

class Circle : public Shape {
public:
    Circle(const Point2D& center, double radius) : _center(center), _radius(radius) {
        if (!std::isfinite(radius) || radius <= 0) {
            throw std::invalid_argument("Circle: радиус должен быть конечным и больше 0");
        }
    }

    Rect2D boundingBox() const override {
        return Rect2D(_center + Vector2D(-_radius, -_radius),
                      _center + Vector2D(_radius, _radius));
    }

    void move(const Vector2D& v) override { _center = _center + v; }

    bool contains(const Point2D& p) const override {
        return (p - _center).length() <= _radius + EPS;
    }

    std::string toString() const override {
        return "Circle(center=" + fmt(_center) + ", r=" + fmt(_radius) + ")";
    }

    char symbol() const override { return 'O'; }

private:
    Point2D _center;
    double _radius;
};

class Rectangle : public Shape {
public:
    explicit Rectangle(const Rect2D& area) : _area(area) {}

    Rect2D boundingBox() const override { return _area; }

    void move(const Vector2D& v) override { _area = _area.moved(v); }

    bool contains(const Point2D& p) const override { return _area.contains(p); }

    std::string toString() const override {
        return "Rectangle(" + fmt(Point2D(_area.left(), _area.bottom())) + " - " +
               fmt(Point2D(_area.right(), _area.top())) + ")";
    }

    char symbol() const override { return 'R'; }

private:
    Rect2D _area;
};

class Segment : public Shape {
public:
    Segment(const Point2D& a, const Point2D& b, double tolerance = 0.5)
        : _a(a), _b(b), _tolerance(tolerance) {
        if (!std::isfinite(tolerance) || tolerance < 0) {
            throw std::invalid_argument("Segment: погрешность не может быть отрицательной");
        }
    }

    Rect2D boundingBox() const override {
        return Rect2D(Point2D(std::min(_a.x(), _b.x()), std::min(_a.y(), _b.y())),
                      Point2D(std::max(_a.x(), _b.x()), std::max(_a.y(), _b.y())));
    }

    void move(const Vector2D& v) override {
        _a = _a + v;
        _b = _b + v;
    }

    bool contains(const Point2D& p) const override {
        return distanceTo(p) <= _tolerance + EPS;
    }

    std::string toString() const override {
        return "Segment(" + fmt(_a) + " - " + fmt(_b) + ")";
    }

    char symbol() const override { return '*'; }

private:
    double distanceTo(const Point2D& p) const {
        Vector2D ab = _b - _a;
        Vector2D ap = p - _a;
        double len2 = ab.dx() * ab.dx() + ab.dy() * ab.dy();
        double t = 0.0;
        if (len2 > EPS) {
            t = (ap.dx() * ab.dx() + ap.dy() * ab.dy()) / len2;
        }
        t = std::clamp(t, 0.0, 1.0);
        Point2D closest = _a + ab * t;
        return (p - closest).length();
    }

    Point2D _a;
    Point2D _b;
    double _tolerance;
};

class Group : public Shape {
public:
    explicit Group(std::unique_ptr<Shape> first) {
        add(std::move(first));
    }

    void add(std::unique_ptr<Shape> shape) {
        if (!shape) {
            throw std::invalid_argument("Group: нельзя добавить пустой указатель");
        }
        if (shape->holds(this)) {
            throw std::invalid_argument("Group: группа не может содержать саму себя");
        }
        _children.push_back(std::move(shape));
    }

    Rect2D boundingBox() const override {
        Rect2D result = _children.front()->boundingBox();
        for (size_t i = 1; i < _children.size(); ++i) {
            result = result.united(_children[i]->boundingBox());
        }
        return result;
    }

    void move(const Vector2D& v) override {
        for (auto& child : _children) {
            child->move(v);
        }
    }

    bool contains(const Point2D& p) const override {
        for (const auto& child : _children) {
            if (child->contains(p)) {
                return true;
            }
        }
        return false;
    }

    std::string toString() const override {
        std::string result = "Group{";
        for (size_t i = 0; i < _children.size(); ++i) {
            if (i > 0) result += ", ";
            result += _children[i]->toString();
        }
        return result + "}";
    }

    char symbol() const override { return '#'; }

    bool holds(const Shape* s) const override {
        if (s == this) return true;
        for (const auto& child : _children) {
            if (child->holds(s)) return true;
        }
        return false;
    }

    const Shape* find(const Point2D& p) const override {
        for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
            if (const Shape* found = (*it)->find(p)) {
                return found;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<Shape>> _children;
};

}