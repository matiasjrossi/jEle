#include "vertex.h"

#include "triangle.h"
#include <cmath>
#include <QDebug>

Vertex::Vertex(double x, double y, double z) :
    _x(x),
    _y(y),
    _z(z),
    referencedBy()
{
}


Vertex::Vertex(Vertex *o) :
    _x(o->_x),
    _y(o->_y),
    _z(o->_z),
    referencedBy()
{
}

double Vertex::x() const
{
    return _x;
}

double Vertex::y() const
{
    return _y;
}

double Vertex::z() const
{
    return _z;
}

void Vertex::setX(double x)
{
    Vertex::_x = x;
}

void Vertex::setY(double y)
{
    Vertex::_y = y;
}

void Vertex::setZ(double z)
{
    Vertex::_z = z;
}

void Vertex::addTriangleBackReference(Triangle *t)
{
    referencedBy.push_back(t);
}

QVector4D Vertex::toQVector() const
{
    return QVector4D(_x, _y, _z, 1);
}

void Vertex::setQVector(QVector4D v)
{
    _x = v.x();
    _y = v.y();
    _z = v.z();
}

Vertex Vertex::operator + (const Vertex o) const
{
    return Vertex(_x + o._x, _y + o._y, _z + o._z);
}

Vertex Vertex::operator - (const Vertex o) const
{
    return Vertex(_x - o._x, _y - o._y, _z - o._z);
}

Vertex Vertex::operator / (const double r) const
{
    return Vertex(_x/r, _y/r, _z/r);
}

double Vertex::operator * (const Vertex o) const
{
    return _x*o._x + _y*o._y + _z*o._z;
}

Vertex Vertex::operator * (const double f) const
{
    return Vertex(_x * f, _y * f, _z * f);
}

Vertex Vertex::crossProduct(const Vertex &a, const Vertex &b, const Vertex &c)
{
    double
            ax = b.x() - a.x(),
            ay = b.y() - a.y(),
            az = b.z() - a.z(),
            bx = c.x() - a.x(),
            by = c.y() - a.y(),
            bz = c.z() - a.z();
    return Vertex( bz*ay - az*by,
                   bx*az - ax*bz,
                   ax*by - bx*ay );
}

Vertex Vertex::getNormalized() const
{
    double norm = sqrt(pow(_x,2)+pow(_y,2)+pow(_z,2));
    return Vertex(_x/norm, _y/norm, _z/norm);
}

Vertex Vertex::getDistance(Vertex o)
{
    return o - this;
}

void Vertex::getArray(float *array) const
{
    array[0] = _x;
    array[1] = _y;
    array[2] = _z;
    array[3] = 1.0f;
}

Vertex Vertex::getNormal() const
{
    Vertex n(0,0,0);
    if (referencedBy.size() > 0) {
        for (int i = 0; i < referencedBy.size(); i++)
            n = n + referencedBy.at(i)->normal();
        n = n / referencedBy.size();
    }
    return n;
}

