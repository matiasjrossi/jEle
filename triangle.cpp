#include "triangle.h"


Triangle::Triangle(Vertex *a, Vertex *b, Vertex *c) :
    _a(a),
    _b(b),
    _c(c),
    _normal(Vertex(Vertex::crossProduct(*_a, *_b, *_c).getNormalized())),
    _vnA(Vertex(0, 0, 0)),
    _vnB(Vertex(0, 0, 0)),
    _vnC(Vertex(0, 0, 0)),
    vnsSet(false)
{
    a->addTriangleBackReference(this);
    b->addTriangleBackReference(this);
    c->addTriangleBackReference(this);
}

Vertex* Triangle::a()
{
    return _a;
}

Vertex* Triangle::b()
{
    return _b;
}

Vertex* Triangle::c()
{
    return _c;
}

Vertex Triangle::normal() const
{
    return _normal;
}

Vertex Triangle::orthoCenter() const
{
    return (*_a + *_b + *_c)/3;
}

void Triangle::setVNs(Vertex a, Vertex b, Vertex c)
{
    vnsSet = true;
    _vnA = a;
    _vnB = b;
    _vnC = c;
}

Vertex Triangle::vnA()
{
    return _vnA;
}

Vertex Triangle::vnB()
{
    return _vnB;
}

Vertex Triangle::vnC()
{
    return _vnC;
}

bool Triangle::isVNSSet()
{
    return vnsSet;
}
