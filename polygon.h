#ifndef POLYGON_H
#define POLYGON_H

#include "vertex.h"
#include <map>
#include <QList>

namespace jEle {
    class Material;

    class Polygon
    {
    public:
        Polygon(QList<Vertex*> vertices, Material *material = NULL);
        Polygon(QList<Vertex*> vertices, QList<Vertex*> textureVertices, QList<Vertex*> normals, Material *material = NULL);
        Polygon *mapClone(std::map<Vertex*, Vertex*> &, Material *material = NULL);

        Vertex getVertex(int i) const;
        Vertex getTextureVertex(int i) const;
        Vertex getNormal(int i) const;
        Material *getMaterial() const;

        Vertex triangleNormal() const;
        bool isTriangle() const;
        bool hasTextureMapping() const;

        int size() const;

    private:
        QList<Vertex*> vertices;
        QList<Vertex*> textureVertices;
        QList<Vertex*> normals;
        void initialize();
        Material *material;
    };

}

#endif // POLYGON_H
