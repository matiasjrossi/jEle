#ifndef OBJREADER_H
#define OBJREADER_H

#include "vertex.h"
#include <QString>
#include <QVector>
#include <QStack>
#include <QFile>
class ObjectModel;

class OBJReader
{
public:
    static ObjectModel *openOBJ(QString filename);
private:
    class Parser
    {
    public:
        Parser(QString filename);
        typedef struct {double x, y, z, w;} V;
        typedef struct {double x, y, z;} VT;
        typedef struct {double x, y, z;} VN;
        typedef struct {QList<int> v, vt, vn;} F;
        QList<V> &getV();
        QList<VT> &getVT();
        QList<VN> &getVN();
        QList<F> &getF();
    private:
        void pushFile(QString &filename);
        void popFile();
        QStack<QString> filenames;
        QStack<QFile*> files;
        QStack<unsigned> lineno;
        QList<V> v;
        QList<VT> vt;
        QList<VN> vn;
        QList<F> f;
    };
};

#endif // OBJREADER_H
