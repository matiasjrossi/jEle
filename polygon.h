#ifndef POLYGON_H
#define POLYGON_H

#include "vertex.h"
#include <map>
#include <QList>

class Polygon
{
public:
    Polygon(QList<Vertex*> vertices);
    Polygon(QList<Vertex*> vertices, QList<Vertex*> normals);
    Polygon *mapClone(std::map<Vertex*, Vertex*> &);

    Vertex getVertex(int i) const;
    Vertex getNormal(int i) const;
    Vertex triangleNormal() const;
    bool isTriangle() const;

    int size() const;

private:
    QList<Vertex*> vertices;
    QList<Vertex*> normals;
    void initialize();
};


#endif // POLYGON_H
