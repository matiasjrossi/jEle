#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H
#include <QList>

class Polygon;
class Vertex;

class ObjectModel
{
public:
    ObjectModel();
    ObjectModel(ObjectModel *o);
    ~ObjectModel();
    void addVertex(double x, double y, double z);
    void addVertex(double x, double y, double z, double w);
    void addNormal(double x, double y, double z);
    Polygon *addPolygon(QList<int> vertex_indexes);
    Polygon *addPolygon(QList<int> vertex_indexes, QList<int> normal_indexes);
    QList<Polygon*> &getPolygons();
    QList<Vertex*> &getVertices();
    void normalize();
private:
    QList<Polygon*> polygons;
    QList<Vertex*> vertices;
    QList<Vertex*> normals;
    double minX, maxX, minY, maxY, minZ, maxZ;
};

#endif // OBJECTMODEL_H
