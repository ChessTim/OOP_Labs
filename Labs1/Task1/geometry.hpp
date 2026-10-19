#pragma once
#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>

namespace geo {

constexpr double EPS = 1e-9;

inline bool nearlyEqual(double a, double b) {
    return std::fabs(a - b) <= EPS;
}

//Вектор: смещение (dx, dy)
class Vector2D {
public:
    Vector2D(double dx, double dy) : _dx(dx), _dy(dy) {}

    double dx() const { return _dx; }
    double dy() const { return _dy; }
    double length() const { return std::hypot(_dx, _dy); }

    Vector2D operator+(const Vector2D& o) const { return {_dx + o._dx, _dy + o._dy}; }
    Vector2D operator-(const Vector2D& o) const { return {_dx - o._dx, _dy - o._dy}; }
    Vector2D operator*(double k) const { return {_dx * k, _dy * k}; }
    Vector2D operator/(double k) const {
        if (nearlyEqual(k, 0.0)) {
            throw std::invalid_argument("Vector2D: деление на ноль");
        }
        return {_dx / k, _dy / k};
    }

    bool operator==(const Vector2D& o) const {
        return nearlyEqual(_dx, o._dx) && nearlyEqual(_dy, o._dy);
    }
    bool operator!=(const Vector2D& o) const { return !(*this == o); }

private:
    double _dx;
    double _dy;
};

//Точка
class Point2D {
public:
    Point2D(double x, double y) : _x(x), _y(y) {}

    double x() const { return _x; }
    double y() const { return _y; }

    // Точка + вектор = новая точка
    Point2D operator+(const Vector2D& v) const { return {_x + v.dx(), _y + v.dy()}; }
    // Точка - точка = вектор между ними
    Vector2D operator-(const Point2D& o) const { return {_x - o._x, _y - o._y}; }

    bool operator==(const Point2D& o) const {
        return nearlyEqual(_x, o._x) && nearlyEqual(_y, o._y);
    }
    bool operator!=(const Point2D& o) const { return !(*this == o); }

private:
    double _x;
    double _y;
};

//Прямоугольная область (границы включены)
class Rect2D {
public:
    // min — левый нижний угол, max — правый верхний
    Rect2D(const Point2D& min, const Point2D& max) : _min(min), _max(max) {
        if (min.x() > max.x() || min.y() > max.y()) {
            throw std::invalid_argument("Rect2D: min должен быть не больше max");
        }
    }

    double left() const { return _min.x(); }
    double right() const { return _max.x(); }
    double bottom() const { return _min.y(); }
    double top() const { return _max.y(); }
    double width() const { return right() - left(); }
    double height() const { return top() - bottom(); }

    bool contains(const Point2D& p) const {
        return p.x() >= left() - EPS && p.x() <= right() + EPS &&
               p.y() >= bottom() - EPS && p.y() <= top() + EPS;
    }

    std::optional<Rect2D> operator&(const Rect2D& o) const {
        double l = std::max(left(), o.left());
        double b = std::max(bottom(), o.bottom());
        double r = std::min(right(), o.right());
        double t = std::min(top(), o.top());
        if (l > r || b > t) {
            return std::nullopt;
        }
        return Rect2D(Point2D(l, b), Point2D(r, t));
    }

    Rect2D united(const Rect2D& o) const {
        return Rect2D(
            Point2D(std::min(left(), o.left()), std::min(bottom(), o.bottom())),
            Point2D(std::max(right(), o.right()), std::max(top(), o.top())));
    }

    Rect2D moved(const Vector2D& v) const { return Rect2D(_min + v, _max + v); }

    bool operator==(const Rect2D& o) const { return _min == o._min && _max == o._max; }
    bool operator!=(const Rect2D& o) const { return !(*this == o); }

private:
    Point2D _min;
    Point2D _max;
};

}