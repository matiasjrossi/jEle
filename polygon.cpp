#include "polygon.h"

#include <QDebug>
#include "material.h"


void Polygon::initialize()
{
    for (int i=0; i<vertices.size(); i++)
        vertices.at(i)->addPolygonBackReference(this);
    if (normals.size() != 0)
        if (vertices.size() != normals.size()) {
            qDebug() << "[Polygon::Polygon()] vertices.size() != normals.size() !!!";
            normals.clear();
        }
    if (textureVertices.size() != 0)
        if (vertices.size() != textureVertices.size()) {
            qDebug() << "[Polygon::Polygon()] vertices.size() != textureVertices.size() !!!";
            textureVertices.clear();
        }
}

Polygon::Polygon(QList<Vertex*> vertices, Material *material) :
    vertices(vertices),
    textureVertices(),
    normals(),
    material(material)
{
    initialize();
}

Polygon::Polygon(QList<Vertex *> vertices, QList<Vertex *> textureVertices, QList<Vertex *> normals, Material *material) :
    vertices(vertices),
    textureVertices(textureVertices),
    normals(normals),
    material(material)
{
    initialize();
}

Polygon *Polygon::mapClone(std::map<Vertex *, Vertex *> &map, Material *material)
{
    QList<Vertex*> newVertices, newTextureVertices, newNormals;
    for (int i=0; i<vertices.size(); i++)
        newVertices.append(map[vertices.at(i)]);
    for (int i=0; i<textureVertices.size(); i++)
        newTextureVertices.append(map[textureVertices.at(i)]);
    for (int i=0; i<normals.size(); i++)
        newNormals.append(map[normals.at(i)]);
    return new Polygon(vertices, textureVertices, normals, material);
}

Vertex Polygon::getVertex(int i) const
{
    return *(vertices.at(i));
}

Vertex Polygon::getTextureVertex(int i) const
{
    return *(textureVertices.at(i));
}

Vertex Polygon::getNormal(int i) const
{
    if (normals.size() != 0)
        return *(normals.at(i));
    else
        return vertices.at(i)->getSyntheticNormal();
}

Material *Polygon::getMaterial() const
{
    return material;
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

bool Polygon::hasTextureMapping() const
{
    return (textureVertices.size() != 0);
}
