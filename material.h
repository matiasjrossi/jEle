#ifndef MATERIAL_H
#define MATERIAL_H

#include <QColor>

namespace jEle {

    class Material
    {
    public:
        Material();
        Material(const QColor &kA, const QColor &kS, const QColor &kD, const QColor &kE, const float &q);
        void setKA(const QColor &kA);
        void setKS(const QColor &kS);
        void setKD(const QColor &kD);
        void setKE(const QColor &kE);
        void setMapD(const QString &mapD);
        void setQ(const float &q);
        QColor getKA() const;
        QColor getKS() const;
        QColor getKD() const;
        QColor getKE() const;
        QString getMapD() const;
        float getQ() const;
    private:
        QColor kA;
        QColor kS;
        QColor kD;
        QColor kE;
        QString mapD;
        float q;
    };

}

#endif // MATERIAL_H
