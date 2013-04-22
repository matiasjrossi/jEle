#include "light.h"

#define RANDCOORD ((rand()%40-20)/20.0)

namespace jEle {

    Light::Light() :
        iS(Qt::white),
        iD(QColor()),
        iA(QColor(10, 10, 10)),
        pos(QVector3D(RANDCOORD, RANDCOORD, RANDCOORD))
    {
        iD.setHsv(rand()%100, 127+rand()%127, 255);
    }

    Light::Light(const QColor &iS, const QColor &iD, const QColor &iA, const QVector3D &pos) :
        iS(iS),
        iD(iD),
        iA(iA),
        pos(pos)
    {
    }

    Light::Light(Light *l) :
        iS(l->iS),
        iD(l->iD),
        iA(l->iA),
        pos(l->pos)
    {
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

    void Light::setPos(const QVector3D &pos)
    {
        Light::pos = pos;
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

    QVector3D Light::getPos() const
    {
        return pos;
    }
}
