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
    OBJReader(QString &filename);
    void parseMTL(QString filename);
    void pushFile(QString &filename);
    void popFile();
    QStack<QString> filenames;
    QStack<QFile*> files;
    QStack<unsigned> lineno;
    ObjectModel *om;
};

#endif // OBJREADER_H
