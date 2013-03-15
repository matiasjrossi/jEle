#include "objreader.h"
#include <QDebug>
#include <QStringList>
#include "objectmodel.h"
#include "polygon.h"

ObjectModel *OBJReader::openOBJ(QString filename)
{
    Parser parser(filename);
    ObjectModel *om = new ObjectModel();
    for (int i=0; i<parser.getV().size(); i++)
    {
        Parser::V v = parser.getV().at(i);
        om->addVertex(v.x, v.y, v.z, v.w);
    }
    for (int i=0; i<parser.getVN().size(); i++)
    {
        Parser::VN vn = parser.getVN().at(i);
        om->addNormal(vn.x, vn.y, vn.z);
    }
    for (int i=0; i<parser.getF().size(); i++)
    {
        Parser::F f = parser.getF().at(i);
        om->addPolygon(f.v, f.vn);
    }
    om->normalize();
    return om;
}

OBJReader::Parser::Parser(QString filename)
{
    pushFile(filename);

    while ( !files.empty() ) {
        while ( !files.top()->atEnd() ) {
            QString line = files.top()->readLine().trimmed();
            lineno.top()++;
            if (line.length() == 0)
                continue; // Ignore the line, it is empty
            if (line.startsWith('#'))
                continue; // Ignore the line, it is a comment

            QStringList items = line.split(" ", QString::SkipEmptyParts);
            if (items[0] == "call") { // This line calls another OBJ
                //TODO: handle args
                pushFile(items[1]);
            } else
            if (items[0] == "v") { // Item is a vertex
                if (items.size() < 4) {
                    qDebug() << "[OBJReader::Parser::Parser] ERROR: Ignoring malformed v line:" << line;
                } else {
                    if (items.size() > 5) {
                        qDebug() << "[OBJReader::Parser::Parser] WARNING: Malformed v line:" << line << "- Discarding extra values.";
                    }

                    V _v;
                    _v.x = items[1].toDouble();
                    _v.y = items[2].toDouble();
                    _v.z = items[3].toDouble();
                    if (items.size() > 4)
                        _v.w = items[4].toDouble();
                    else
                        _v.w = 1.0;
                    v.push_back(_v);
                }
            } else
            if (items[0] == "vt") { // Texture vertex
            } else
            if (items[0] == "vn") { // Normal vertex
                if (items.size() < 4) {
                    qDebug() << "[OBJReader::Parser::Parser] ERROR: Ignoring malformed vn line:" << line;
                } else {
                    if (items.size() > 4) {
                        qDebug() << "[OBJReader::Parser::Parser] WARNING: Malformed vn line:" << line << "- Discarding extra values.";
                    }

                    VN _vn;
                    _vn.x = items[1].toDouble();
                    _vn.y = items[2].toDouble();
                    _vn.z = items[3].toDouble();
                    vn.push_back(_vn);
                }
            } else
            if (items[0] == "vp") { // Parameter space vertices - NOT supported

            } else
            if (items[0] == "f") { // This items is a polygon face
                if (items.size() < 4) {
                    qDebug() << "[OBJReader::Parser::Parser] ERROR: Ignoring malformed f line:" << line;
                } else {
//                    if (items.size() > 4) {
//                        qDebug() << "[OBJReader::Parser::Parser] WARNING: Malformed f line:" << line << "- Discarding extra values.";
//                    }

                    F _f;
                    switch (items[1].split('/').size()) {
                        case 1:
                            for (int i = 1; i<items.size(); i++) {
                                _f.v.push_back(items[i].split('/')[0].toInt() - 1);
                            }
                            f.push_back(_f);
                            break;
                        case 2:
                            for (int i = 1; i<items.size(); i++) {
                                _f.v.push_back(items[i].split('/')[0].toInt() - 1);
                                _f.vt.push_back(items[i].split('/')[1].toInt() - 1);
                            }
                            f.push_back(_f);
                        case 3:
                            for (int i = 1; i<items.size(); i++) {
                                _f.v.push_back(items[i].split('/')[0].toInt() - 1);
                                _f.vt.push_back(items[i].split('/')[1].toInt() - 1);
                                _f.vn.push_back(items[i].split('/')[2].toInt() - 1);
                            }
                            f.push_back(_f);
                            break;
                        default:
                            qDebug() << "[OBJReader::Parser::Parser] WARNING: Wrong count of slashes in f line:" << line << ". Ignoring.";
                    }

                }
            } else
            if (items[0] == "mtllib") { // Material library

            } else
            if (items[0] == "usemtl") { // Material use definition

            } else
            if (items[0] == "s") { // S

            } else
            if (items[0] == "g") { // Group definition

            } else
                qDebug() << "Unsuported item identifier in line " << lineno.top() << ": " << items[0];
        }
        popFile();
    }
}

QList<OBJReader::Parser::V> &OBJReader::Parser::getV() {
    return this->v;
}

QList<OBJReader::Parser::VT> &OBJReader::Parser::getVT() {
    return this->vt;
}

QList<OBJReader::Parser::VN> &OBJReader::Parser::getVN() {
    return this->vn;
}

QList<OBJReader::Parser::F> &OBJReader::Parser::getF() {
    return this->f;
}

void OBJReader::Parser::pushFile(QString &filename) {
    QFile *file = new QFile(filename);
    if (file->open(QFile::ReadOnly)) {
        files.push(file);
        filenames.push(filename);
        lineno.push(0);
    } else {
        qDebug() << "Failed to open the file '" << filename << "'";
    }
}

void OBJReader::Parser::popFile() {
    files.top()->close();
    delete files.pop();
    filenames.pop();
    lineno.pop();
}
