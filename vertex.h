#ifndef VERTEX_H
#define VERTEX_H

#include <QList>
#include <QVector4D>

namespace jEle {
    class Polygon;

    class Vertex
    {
    public:
        Vertex(double x, double y, double z);
        Vertex(Vertex *o);

        double x() const;
        double y() const;
        double z() const;

        void setX(double x);
        void setY(double y);
        void setZ(double z);

        void addPolygonBackReference(Polygon*);

        QVector4D toQVector() const;
        void setQVector(QVector4D);

        Vertex operator+(const Vertex o) const;
        Vertex operator-(const Vertex o) const;
        Vertex operator/(const double r) const;

        double operator*(const Vertex o) const;
        Vertex operator*(const double f) const;

        static Vertex crossProduct(const Vertex &a, const Vertex &b, const Vertex &c);

        Vertex getNormalized() const;
        Vertex getDistance(Vertex o);

        void getArray(float *array) const;

        Vertex getSyntheticNormal() const;

    private:
        double _x, _y, _z;
        QList<Polygon*> referencedBy;
    };

}

#endif // VERTEX_H
