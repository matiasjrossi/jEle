#include "objectmodel.h"

#include <map>
#include <QString>
#include <QDebug>

#include "polygon.h"
#include "vertex.h"
#include "material.h"


double min(double a, double b)
{
    return (a<b?a:b);
}

double max(double a, double b)
{
    return (a>b?a:b);
}

double max(double a, double b, double c)
{
    return max(max(a, b), c);
}

/*
 *
 */
namespace jEle {
    ObjectModel::ObjectModel()
    {
    }

    ObjectModel::ObjectModel(ObjectModel *o) :
        materials_by_name(o->materials_by_name)
    {
        std::map<Vertex*,Vertex*> vMap;
        for (int i=0; i<o->vertices.size(); i++)
        {
            Vertex *newVertex = new Vertex(o->vertices.at(i));
            vertices.push_back(newVertex);
            vMap[o->vertices.at(i)] = newVertex;
        }
        for (int i=0; i<o->normals.size(); i++)
        {
            Vertex *newNormal = new Vertex(o->normals.at(i));
            normals.push_back(newNormal);
            vMap[o->normals.at(i)] = newNormal;
        }
        for (int i=0; i<o->materials.size(); i++)
        {
            Material *newMaterial = new Material(*o->materials.at(i));
            materials.push_back(newMaterial);
        }
        for (int i=0; i<o->textureVertices.size(); i++)
        {
            Vertex *newTextureVertex = new Vertex(o->textureVertices.at(i));
            textureVertices.push_back(newTextureVertex);
            vMap[o->textureVertices.at(i)] = newTextureVertex;
        }

        for (int i=0; i<o->polygons.size(); i++)
        {
            polygons.push_back(
                        o->polygons.at(i)->mapClone(
                            vMap,
                            materials.at(
                                o->materials.indexOf(
                                    o->polygons.at(i)->getMaterial())
                                )
                            )
                        );
        }


    }

    ObjectModel::~ObjectModel()
    {
        while (!polygons.empty()) {
            delete polygons.takeFirst();
        }

        while (!vertices.empty()) {
            delete vertices.takeFirst();
        }

        while (!normals.empty()) {
            delete normals.takeFirst();
        }

        while (!textureVertices.empty()) {
            delete textureVertices.takeFirst();
        }

        while (!materials.empty()) {
            delete materials.takeFirst();
        }
    }


    void ObjectModel::addVertex(double x, double y, double z)
    {
        if (vertices.empty())
        {
            minX = maxX = x;
            minY = maxY = y;
            minZ = maxZ = z;
        }
        else
        {
            minX = min(minX,x);
            minY = min(minY,y);
            minZ = min(minZ,z);
            maxX = max(maxX,x);
            maxY = max(maxY,y);
            maxZ = max(maxZ,z);
        }
        vertices.push_back(new Vertex(x, y, z));
    }

    void ObjectModel::addVertex(double x, double y, double z, double w)
    {
        addVertex(x/w, y/w, z/w);
    }

    void ObjectModel::addNormal(double x, double y, double z)
    {
        normals.push_back(new Vertex(x, y, z));
    }

    void ObjectModel::addTextureVertex(double x, double y, double z)
    {
        textureVertices.push_back(new Vertex(x, y, z));
    }

    void ObjectModel::addMaterial(QString name, Material *material)
    {
        materials_by_name[name] = materials.size();
        materials.append(material);
    }

    Polygon *ObjectModel::addPolygon(QList<int> vertex_indexes)
    {
        return addPolygon("", vertex_indexes, QList<int>(), QList<int>());
    }

    Polygon *ObjectModel::addPolygon(QString material, QList<int> vertex_indexes, QList<int> texture_indexes, QList<int> normal_indexes)
    {
        QList<Vertex*> _normals;
        for (int i=0; i<normal_indexes.size(); i++) {
            int ni = normal_indexes.at(i);
            if (ni<normals.size()) {
                if (ni<0)
                    _normals.push_back(normals.at(0));
                else
                    _normals.push_back(normals.at(ni));
            }
            else
                qDebug() << "[ObjectModel::addPolygon()] normal_indexes contains an index that is not valid (" << ni << "|" << normals.size() << ")";
        }

        QList<Vertex*> _vertices;
        for (int i=0; i<vertex_indexes.size(); i++) {
            int vi = vertex_indexes.at(i);
            if (vi<vertices.size()) {
                if (vi<0)
                    _vertices.push_back(vertices.at(0));
                else
                    _vertices.push_back(vertices.at(vi));
            }
            else
                qDebug() << "[ObjectModel::addPolygon()] vertex_indexes contains an index that is not valid (" << vi << "|" << vertices.size() << ")";
        }

        QList<Vertex*> _textureVertices;
        for (int i=0; i<texture_indexes.size(); i++) {
            int ti = texture_indexes.at(i);
            if (ti<textureVertices.size()) {
                if (ti<0)
                    _textureVertices.push_back(textureVertices.at(0));
                else
                    _textureVertices.push_back(textureVertices.at(ti));
            } else
                qDebug() << "[ObjectModel::addPolygon()] texture_indexes contains an index that is not valid (" << ti << "|" << textureVertices.size() << ")";
        }

        Material *_material = NULL;
        if (materials_by_name.contains(material))
            _material = materials.at(materials_by_name.value(material));
        Polygon *toAdd = new Polygon(_vertices, _textureVertices, _normals, _material);
        polygons.push_back(toAdd);
        return toAdd;
    }

    QList<Polygon*> &ObjectModel::getPolygons()
    {
        return polygons;
    }

    QList<Vertex*> &ObjectModel::getVertices()
    {
        return vertices;
    }

    void ObjectModel::normalize()
    {
        double scaleX = maxX-minX, scaleY = maxY-minY, scaleZ = maxZ-minZ;
        double scale = max(scaleX, scaleY, scaleZ);
        double halfScale = scale / 2;
        double xOffset = minX + scaleX/2, yOffset = minY + scaleY/2, zOffset = minZ + scaleZ/2;

        for (int i=0; i<vertices.size(); i++)
        {
            vertices.at(i)->setX((vertices.at(i)->x() - xOffset)/halfScale);
            vertices.at(i)->setY((vertices.at(i)->y() - yOffset)/halfScale);
            vertices.at(i)->setZ((vertices.at(i)->z() - zOffset)/halfScale);
        }
    }
}
