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

    Vertex normal() const;

    Vertex orthoCenter() const;

private:
    Vertex *_a, *_b, *_c, _normal;
};


#endif // TRIANGLE_H
