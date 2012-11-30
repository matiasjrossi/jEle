#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H
#include <QList>

class Triangle;
class Vertex;

class ObjectModel
{
public:
    ObjectModel();
    ObjectModel(ObjectModel *o);
    ~ObjectModel();
    void addVertex(double x, double y, double z);
    void addTriangle(Triangle*);
    void addTriangle(int a, int b, int c);
    QList<Triangle*> &getTriangles();
    QList<Vertex*> &getVertexes();
    void normalize();
private:
    QList<Triangle*> triangles;
    QList<Vertex*> vertexes;
    double minX, maxX, minY, maxY, minZ, maxZ;
};

#endif // OBJECTMODEL_H
