#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H
#include <QList>
#include <QMap>

namespace jEle {

    class Polygon;
    class Vertex;
    class Material;

    class ObjectModel
    {
    public:
        ObjectModel();
        ObjectModel(ObjectModel *o);
        ~ObjectModel();
        void addVertex(double x, double y, double z);
        void addVertex(double x, double y, double z, double w);
        void addNormal(double x, double y, double z);
        void addTextureVertex(double x, double y, double z = 0.0);
        void addMaterial(QString name, Material *material);
        Polygon *addPolygon(QList<int> vertex_indexes);
        Polygon *addPolygon(QString material, QList<int> vertex_indexes, QList<int> texture_indexes, QList<int> normal_indexes);
        QList<Polygon*> &getPolygons();
        QList<Vertex*> &getVertices();
        void normalize();
    private:
        QList<Polygon*> polygons;
        QList<Vertex*> vertices;
        QList<Vertex*> normals;
        QList<Vertex*> textureVertices;
        QList<Material*> materials;
        QMap<QString, int> materials_by_name;
        double minX, maxX, minY, maxY, minZ, maxZ;
    };

}

#endif // OBJECTMODEL_H
