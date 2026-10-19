#include <iostream>
#include "geometry.hpp"

using namespace geo;

static int g_failed = 0;
#define CHECK(cond) \
    do { if (!(cond)) { ++g_failed; std::cout << "FAIL: " #cond " (строка " << __LINE__ << ")\n"; } } while (0)

template <class F>
bool throws(F f) {
    try { f(); } catch (const std::exception&) { return true; }
    return false;
}

void testVector() {
    Vector2D a(1, 2), b(3, 4);
    CHECK(a + b == Vector2D(4, 6));
    CHECK(b - a == Vector2D(2, 2));
    CHECK(a * 2 == Vector2D(2, 4));
    CHECK(b / 2 == Vector2D(1.5, 2));
    CHECK(a != b);
    CHECK(nearlyEqual(Vector2D(3, 4).length(), 5.0));
    CHECK(throws([&] { return a / 0; }));
    CHECK(a == Vector2D(1, 2));
}

void testPoint() {
    Point2D p(1, 1);
    Vector2D v(2, 3);
    CHECK(p + v == Point2D(3, 4));
    CHECK(Point2D(5, 5) - Point2D(2, 3) == Vector2D(3, 2));
    CHECK(p == Point2D(1, 1));
    CHECK(Point2D(0.1 + 0.2, 0) == Point2D(0.3, 0));
}

void testRect() {
    Rect2D r(Point2D(0, 0), Point2D(4, 4));
    CHECK(r.contains(Point2D(2, 2)));
    CHECK(r.contains(Point2D(4, 4)));
    CHECK(!r.contains(Point2D(5, 2)));
    CHECK(nearlyEqual(r.width(), 4));

    CHECK(throws([] { return Rect2D(Point2D(5, 5), Point2D(0, 0)); }));

    auto i = r & Rect2D(Point2D(2, 2), Point2D(6, 6));
    CHECK(i.has_value());
    CHECK(*i == Rect2D(Point2D(2, 2), Point2D(4, 4)));

    CHECK(!(r & Rect2D(Point2D(10, 10), Point2D(11, 11))).has_value());

    CHECK((r & Rect2D(Point2D(4, 0), Point2D(8, 4))).has_value());

    CHECK(r.united(Rect2D(Point2D(3, 3), Point2D(9, 9))) == Rect2D(Point2D(0, 0), Point2D(9, 9)));
    CHECK(r.moved(Vector2D(1, 1)) == Rect2D(Point2D(1, 1), Point2D(5, 5)));
    CHECK(r == Rect2D(Point2D(0, 0), Point2D(4, 4)));
}

int main() {
    testVector();
    testPoint();
    testRect();
    if (g_failed == 0) std::cout << "Все тесты пройдены\n";
    else std::cout << "Провалено проверок: " << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}