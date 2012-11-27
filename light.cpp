#include "light.h"

#define RANDCOORD ((rand()%40-20)/10.0)

Light::Light() :
    iS(Qt::white),
    iD(Qt::white),
    iA(Qt::white),
    pos(new Vertex(RANDCOORD, RANDCOORD, RANDCOORD))
{
}

Light::Light(const QColor &iS, const QColor &iD, const QColor &iA, const Vertex &pos) :
    iS(iS),
    iD(iD),
    iA(iA),
    pos(new Vertex(pos))
{
}

Light::Light(Light *l) :
    iS(l->iS),
    iD(l->iD),
    iA(l->iA),
    pos(new Vertex(l->pos))
{
}

Light::~Light()
{
    delete pos;
}

void Light::setID(const QColor &iD)
{
    Light::iD = iD;
}

void Light::setIS(const QColor &iS)
{
    Light::iS = iS;
}

void Light::setIA(const QColor &iA)
{
    Light::iA = iA;
}

void Light::setPos(const Vertex &pos)
{
    Vertex *old = Light::pos;
    Light::pos = new Vertex(pos);
    delete old;
}

QColor Light::getID() const
{
    return iD;
}

QColor Light::getIS() const
{
    return iS;
}

QColor Light::getIA() const
{
    return iA;
}

Vertex Light::getPos() const
{
    return *pos;
}

Vertex *Light::getPosPtr()
{
    return pos;
}
