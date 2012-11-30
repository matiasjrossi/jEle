#ifndef LIGHT_H
#define LIGHT_H

#include <QColor>
#include "vertex.h"

class Light
{
public:
    Light();
    Light(QColor const &iS, QColor const &iD, QColor const &iA, const Vertex &pos);
    Light(Light *l);
    ~Light();
    void setIS(const QColor &iS);
    void setID(const QColor &iD);
    void setIA(const QColor &iA);
    void setPos(const Vertex &pos);
    QColor getIS() const;
    QColor getID() const;
    QColor getIA() const;
    Vertex getPos() const;
    Vertex *getPosPtr();
private:
    QColor iS;
    QColor iD;
    QColor iA;
    Vertex *pos;
};

#endif // LIGHT_H


