#include "triangle.h"


Triangle::Triangle(Vertex *a, Vertex *b, Vertex *c) :
    _a(a),
    _b(b),
    _c(c),
    _normal(Vertex(Vertex::crossProduct(*_a, *_b, *_c).getNormalized()))
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
