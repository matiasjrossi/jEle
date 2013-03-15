#include "polygon.h"

#include <QDebug>


void Polygon::initialize()
{
    for (int i=0; i<vertices.size(); i++)
        vertices.at(i)->addPolygonBackReference(this);
    if (normals.size() != 0)
        if (vertices.size() != normals.size())
            qDebug() << "[Polygon::Polygon()] vertices.size() != normals.size() !!!";
}

Polygon::Polygon(QList<Vertex*> vertices) :
    vertices(vertices),
    normals()
{
    initialize();
}

Polygon::Polygon(QList<Vertex *> vertices, QList<Vertex *> normals) :
    vertices(vertices),
    normals(normals)
{
    initialize();
}

Polygon *Polygon::mapClone(std::map<Vertex *, Vertex *> &map)
{
    QList<Vertex*> newVertices, newNormals;
    for (int i=0; i<vertices.size(); i++)
        newVertices.append(map[vertices.at(i)]);
    for (int i=0; i<normals.size(); i++)
        newNormals.append(map[normals.at(i)]);
    return new Polygon(vertices, normals);
}

Vertex Polygon::getVertex(int i) const
{
    return *(vertices.at(i));
}

Vertex Polygon::getNormal(int i) const
{
    if (normals.size() != 0)
        return *(normals.at(i));
    else
        return vertices.at(i)->getSyntheticNormal();
}

int Polygon::size() const
{
    return vertices.size();
}

Vertex Polygon::triangleNormal() const
{
    if (vertices.size() != 3)
        qDebug() << "[Polygon::triangleNormal()] This ain't no triangle!";
    return Vertex::crossProduct(*(vertices.at(0)), *(vertices.at(1)), *(vertices.at(2))).getNormalized();
}

bool Polygon::isTriangle() const
{
    return (vertices.size() == 3);
}
