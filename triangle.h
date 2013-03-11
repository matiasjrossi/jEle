#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vertex.h"

class Triangle
{
public:
    Triangle(Vertex*, Vertex*, Vertex*);

    Vertex *a();
    Vertex *b();
    Vertex *c();

    void setVNs(Vertex a, Vertex b, Vertex c);

    Vertex normal() const;

    Vertex orthoCenter() const;

    Vertex vnA();
    Vertex vnB();
    Vertex vnC();

    bool isVNSSet();

private:
    Vertex *_a, *_b, *_c, _normal;
    Vertex _vnA, _vnB, _vnC;
    bool vnsSet;
};


#endif // TRIANGLE_H
