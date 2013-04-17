#ifndef LIGHT_H
#define LIGHT_H

#include <QColor>
#include <QVector3D>

class Light
{
public:
    Light();
    Light(QColor const &iS, QColor const &iD, QColor const &iA, const QVector3D &pos);
    Light(Light *l);
    void setIS(const QColor &iS);
    void setID(const QColor &iD);
    void setIA(const QColor &iA);
    void setPos(const QVector3D &pos);
    QColor getIS() const;
    QColor getID() const;
    QColor getIA() const;
    QVector3D getPos() const;
//    Vertex *getPosPtr();
private:
    QColor iS;
    QColor iD;
    QColor iA;
    QVector3D pos;
};

#endif // LIGHT_H


