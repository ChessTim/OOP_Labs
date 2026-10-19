#include <iostream>
#include "geometry.hpp"
#include <memory>
#include <string>
#include "editor.hpp"

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


void testCircle() {
    Circle c(Point2D(0, 0), 2);
    CHECK(c.contains(Point2D(0, 0)));
    CHECK(c.contains(Point2D(2, 0)));      // граница
    CHECK(!c.contains(Point2D(2, 1)));
    CHECK(c.boundingBox() == Rect2D(Point2D(-2, -2), Point2D(2, 2)));
    c.move(Vector2D(1, 1));
    CHECK(c.contains(Point2D(3, 1)));
    CHECK(c.boundingBox() == Rect2D(Point2D(-1, -1), Point2D(3, 3)));
    CHECK(throws([] { Circle(Point2D(0, 0), 0); }));
    CHECK(throws([] { Circle(Point2D(0, 0), -5); }));
}

void testRectangle() {
    Rectangle r(Rect2D(Point2D(0, 0), Point2D(4, 2)));
    CHECK(r.contains(Point2D(4, 2)));
    CHECK(!r.contains(Point2D(5, 1)));
    r.move(Vector2D(10, 0));
    CHECK(r.contains(Point2D(12, 1)));
    CHECK(!r.contains(Point2D(2, 1)));
    CHECK(r.boundingBox() == Rect2D(Point2D(10, 0), Point2D(14, 2)));
}

void testSegment() {
    Segment s(Point2D(0, 0), Point2D(10, 0), 0.1);
    CHECK(s.contains(Point2D(5, 0)));
    CHECK(s.contains(Point2D(5, 0.05)));   // в пределах погрешности
    CHECK(!s.contains(Point2D(5, 0.5)));
    CHECK(s.contains(Point2D(10, 0)));     // конец отрезка
    CHECK(!s.contains(Point2D(11, 0)));    // за концом
    CHECK(s.boundingBox() == Rect2D(Point2D(0, 0), Point2D(10, 0)));
    Segment diag(Point2D(3, 4), Point2D(1, 2));
    CHECK(diag.boundingBox() == Rect2D(Point2D(1, 2), Point2D(3, 4)));
    Segment dot(Point2D(1, 1), Point2D(1, 1), 0.1); // вырожденный отрезок
    CHECK(dot.contains(Point2D(1, 1)));
    CHECK(!dot.contains(Point2D(2, 1)));
    s.move(Vector2D(0, 3));
    CHECK(s.contains(Point2D(5, 3)));
    CHECK(throws([] { Segment(Point2D(0, 0), Point2D(1, 1), -1); }));
}

void testGroup() {
    auto g = std::make_unique<Group>(std::make_unique<Circle>(Point2D(0, 0), 1));
    g->add(std::make_unique<Rectangle>(Rect2D(Point2D(5, 5), Point2D(7, 7))));
    CHECK(g->boundingBox() == Rect2D(Point2D(-1, -1), Point2D(7, 7)));
    CHECK(g->contains(Point2D(6, 6)));
    CHECK(!g->contains(Point2D(3, 3)));
    g->move(Vector2D(1, 1));
    CHECK(g->boundingBox() == Rect2D(Point2D(0, 0), Point2D(8, 8)));
    CHECK(g->contains(Point2D(1, 1)));
    CHECK(g->toString().find("Circle") != std::string::npos);
    CHECK(g->toString().find("Rectangle") != std::string::npos);

    // вложенная группа
    auto inner = std::make_unique<Group>(std::make_unique<Circle>(Point2D(20, 20), 1));
    g->add(std::move(inner));
    CHECK(g->boundingBox() == Rect2D(Point2D(0, 0), Point2D(21, 21)));
    CHECK(g->contains(Point2D(20, 20)));

    CHECK(throws([] { Group(nullptr); }));
    CHECK(throws([&] { g->add(nullptr); }));
}

void testPolymorphism() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(Point2D(0, 0), 1));
    shapes.push_back(std::make_unique<Rectangle>(Rect2D(Point2D(0, 0), Point2D(2, 2))));
    shapes.push_back(std::make_unique<Segment>(Point2D(0, 0), Point2D(2, 0)));
    for (auto& s : shapes) {
        s->move(Vector2D(10, 10));        // один вызов, разное поведение
        CHECK(s->boundingBox().left() >= 9 - EPS);
    }
}

void testEditor() {
    Editor ed;
    Shape* c = ed.add(std::make_unique<Circle>(Point2D(5, 5), 3));
    Shape* r = ed.add(std::make_unique<Rectangle>(Rect2D(Point2D(4, 4), Point2D(10, 10))));
    Shape* s = ed.add(std::make_unique<Segment>(Point2D(20, 20), Point2D(30, 20)));
    CHECK(ed.size() == 3);

    CHECK(ed.topAt(Point2D(5, 5)) == r);
    CHECK(ed.topAt(Point2D(2, 5)) == c);
    CHECK(ed.topAt(Point2D(25, 20)) == s);
    CHECK(ed.topAt(Point2D(100, 100)) == nullptr);

    CHECK(ed.inRegion(Rect2D(Point2D(0, 0), Point2D(3, 3))).size() == 1);
    CHECK(ed.inRegion(Rect2D(Point2D(0, 0), Point2D(50, 50))).size() == 3);
    CHECK(ed.inRegion(Rect2D(Point2D(100, 100), Point2D(110, 110))).empty());

    CHECK(ed.moveInRegion(Rect2D(Point2D(0, 0), Point2D(3, 3)), Vector2D(1, 0)) == 1);
    CHECK(c->boundingBox() == Rect2D(Point2D(3, 2), Point2D(9, 8)));
    CHECK(r->boundingBox() == Rect2D(Point2D(4, 4), Point2D(10, 10)));

    CHECK(ed.move(r, Vector2D(0, 1)));
    CHECK(r->boundingBox() == Rect2D(Point2D(4, 5), Point2D(10, 11)));

    Circle foreign(Point2D(0, 0), 1);
    CHECK(!ed.remove(&foreign));
    CHECK(!ed.move(&foreign, Vector2D(1, 1)));

    CHECK(ed.remove(r));
    CHECK(ed.size() == 2);

    CHECK(throws([&] { ed.add(nullptr); }));

    Editor ed2;
    auto g = std::make_unique<Group>(std::make_unique<Circle>(Point2D(0, 0), 1));
    g->add(std::make_unique<Rectangle>(Rect2D(Point2D(5, 5), Point2D(6, 6))));
    Shape* group = ed2.add(std::move(g));
    CHECK(ed2.topAt(Point2D(0, 0)) == group);
    CHECK(ed2.topAt(Point2D(5, 5)) == group);
}

void testRender() {
    Editor ed;
    ed.add(std::make_unique<Rectangle>(Rect2D(Point2D(0, 0), Point2D(2, 1))));
    Rect2D area(Point2D(0, 0), Point2D(3, 1));
    CHECK(render(ed, area) == "RRR \nRRR \n");

    ed.add(std::make_unique<Circle>(Point2D(1, 1), 0.5)); // лежит выше
    CHECK(render(ed, area) == "ROR \nRRR \n");

    Editor empty;
    CHECK(render(empty, area) == "    \n    \n");
}

void demo() {
    Editor ed;
    ed.add(std::make_unique<Circle>(Point2D(10, 6), 5));
    ed.add(std::make_unique<Rectangle>(Rect2D(Point2D(14, 3), Point2D(24, 8))));
    ed.add(std::make_unique<Segment>(Point2D(0, 0), Point2D(28, 11)));
    std::cout << render(ed, Rect2D(Point2D(0, 0), Point2D(30, 12))) << "\n";
}


int main() {
    testVector();
    testPoint();
    testRect();
    testCircle();
    testRectangle();
    testSegment();
    testGroup();
    testPolymorphism();
    testEditor();
    testRender();
    if (g_failed == 0) std::cout << "Все тесты пройдены\n";
    else std::cout << "Провалено проверок: " << g_failed << "\n";

    demo();
    return g_failed == 0 ? 0 : 1;
}