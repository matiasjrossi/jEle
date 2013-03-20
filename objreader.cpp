#include "objreader.h"
#include <QDebug>
#include <QStringList>
#include <QFileInfo>
#include <QMessageBox>
#include "objectmodel.h"
#include "polygon.h"
#include "material.h"

ObjectModel *OBJReader::openOBJ(QString filename)
{
    OBJReader reader(filename);
    if (reader.om != NULL) reader.om->normalize();
    return reader.om;
//    OBJParser parser(filename);
//    ObjectModel *om = new ObjectModel();
//    for (int i=0; i<parser.getV().size(); i++)
//    {
//        OBJParser::V v = parser.getV().at(i);
//        om->addVertex(v.x, v.y, v.z, v.w);
//    }
//    for (int i=0; i<parser.getVN().size(); i++)
//    {
//        OBJParser::VN vn = parser.getVN().at(i);
//        om->addNormal(vn.x, vn.y, vn.z);
//    }
//    for (int i=0; i<parser.getF().size(); i++)
//    {
//        OBJParser::F f = parser.getF().at(i);
//        om->addPolygon(f.v, f.vn);
//    }
//    om->normalize();
//    return om;
}

OBJReader::OBJReader(QString &filename) :
    om(NULL)
{
    pushFile(filename);
    while ( !files.empty() ) {
        if (om == NULL) om = new ObjectModel();
        QString usemtl;
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

                    if (items.size() > 4)
                        om->addVertex(
                                    items[1].toDouble(),
                                    items[2].toDouble(),
                                    items[3].toDouble(),
                                    items[4].toDouble());
                    else
                        om->addVertex(
                                    items[1].toDouble(),
                                    items[2].toDouble(),
                                    items[3].toDouble());
                }
            } else
            if (items[0] == "vt") { // Texture vertex
                if (items.size() < 3) {
                    qDebug() << "[OBJReader::Parser::Parser] ERROR: Ignoring malformed vt line:" << line;
                }
                else {
                    if (items.size() > 4) {
                        qDebug() << "[OBJReader::Parser::Parser] WARNING: Malformed vt line:" << line << "- Discarding extra values.";
                    }

                    if (items.size() > 3)
                        om->addTextureVertex(
                                    items[1].toDouble(),
                                    items[2].toDouble(),
                                    items[3].toDouble());
                    else
                        om->addTextureVertex(
                                    items[1].toDouble(),
                                    items[2].toDouble());

                }
            } else
            if (items[0] == "vn") { // Normal vertex
                if (items.size() < 4) {
                    qDebug() << "[OBJReader::Parser::Parser] ERROR: Ignoring malformed vn line:" << line;
                } else {
                    if (items.size() > 4) {
                        qDebug() << "[OBJReader::Parser::Parser] WARNING: Malformed vn line:" << line << "- Discarding extra values.";
                    }

                    om->addNormal(
                                items[1].toDouble(),
                                items[2].toDouble(),
                                items[3].toDouble());
                }
            } else
            if (items[0] == "vp") { // Parameter space vertices - NOT supported

            } else
            if (items[0] == "f") { // This items is a polygon face
                if (items.size() < 4) {
                    qDebug() << "[OBJReader::Parser::Parser] ERROR: Ignoring malformed f line:" << line;
                } else {
                    int sub_value_count = items[1].split('/').size();
                    if (sub_value_count > 3)
                        qDebug() << "[OBJReader::Parser::Parser] WARNING: Wrong count of slashes in f line:" << line << ". Discarding extra values.";
                    else {
                        QList<int> v, vt, vn;
                        switch (sub_value_count) {
                        case 1:
                            for (int i = 1; i<items.size(); i++) {
                                v.push_back(items[i].split('/')[0].toInt() - 1);
                            }
                            break;
                        case 2:
                            for (int i = 1; i<items.size(); i++) {
                                v.push_back(items[i].split('/')[0].toInt() - 1);
                                vt.push_back(items[i].split('/')[1].toInt() - 1);
                            }
                            break;
                        case 3:
                            for (int i = 1; i<items.size(); i++) {
                                v.push_back(items[i].split('/')[0].toInt() - 1);
                                vt.push_back(items[i].split('/')[1].toInt() - 1);
                                vn.push_back(items[i].split('/')[2].toInt() - 1);
                            }
                            break;
                        }
                        om->addPolygon(usemtl, v, vt, vn);
                    }

                }
            } else
            if (items[0] == "mtllib") { // Material library
                QFileInfo fi(*files.top());
                parseMTL(fi.absolutePath() + "/" + items[1]);

            } else
            if (items[0] == "usemtl") { // Material use definition
                usemtl = items[1];
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

void OBJReader::pushFile(QString &filename) {
    QFile *file = new QFile(filename);
    if (file->open(QFile::ReadOnly)) {
        files.push(file);
        filenames.push(filename);
        lineno.push(0);
    } else {
        QMessageBox::critical(
                NULL,
                "Error",
                QString("Error opening object file: %1\n\n%2 (%3)").
                    arg(filename).
                    arg(file->errorString()).
                    arg(file->error()));
    }
}

void OBJReader::popFile() {
    files.top()->close();
    delete files.pop();
    filenames.pop();
    lineno.pop();
}

void OBJReader::parseMTL(QString filename)
{
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        unsigned lineno = 0;
        Material *newmtl = NULL;
        while ( !file.atEnd() ) {
            QString line = file.readLine().trimmed();
            lineno++;
            if (line.length() == 0)
                continue; // Ignore the line, it is empty
            if (line.startsWith('#'))
                continue; // Ignore the line, it is a comment

            QStringList items = line.split(" ", QString::SkipEmptyParts);
            if (items[0] == "newmtl") {
                newmtl = new Material();
                om->addMaterial(items[1], newmtl);
            } else
            if (items[0] == "d" || items[0] == "Tr") { // Dissolved/Transparency - NOT implemented
            } else
            if (items[0] == "illum") { // Illumination model - NOT implemented
            } else
            if (items[0] == "Ka") {
                newmtl->setKA(QColor(
                        int(items[1].toDouble() * 255),
                        int(items[2].toDouble() * 255),
                        int(items[3].toDouble() * 255)));
            } else
            if (items[0] == "Kd") {
                newmtl->setKD(QColor(
                        int(items[1].toDouble() * 255),
                        int(items[2].toDouble() * 255),
                        int(items[3].toDouble() * 255)));
            } else
            if (items[0] == "Ks") {
                newmtl->setKS(QColor(
                        int(items[1].toDouble() * 255),
                        int(items[2].toDouble() * 255),
                        int(items[3].toDouble() * 255)));
            } else
            if (items[0] == "Ke") {
                newmtl->setKE(QColor(
                        int(items[1].toDouble() * 255),
                        int(items[2].toDouble() * 255),
                        int(items[3].toDouble() * 255)));
            } else
            if (items[0] == "Tf") { // Transmission filter - NOT implemented
            } else
            if (items[0] == "Ni") { // Optical density - NOT implemented
            } else
            if (items[0] == "Ns") {
                newmtl->setQ(items[1].toDouble());
            } else
            if (items[0] == "map_d" || items[0] == "map_Kd") { // Map dissolve
                QFileInfo fi(file);
                newmtl->setMapD(fi.absolutePath() + "/" + items.last());
            } else
            if (items[0] == "bump") { // Bump mapping
            } else
                qDebug() << "Unsuported item identifier in line " << lineno << ": " << items[0];
        }
    } else
        QMessageBox::warning(
                NULL,
                "Warning",
                QString("Error opening material file: %1\n\n%2 (%3)").
                    arg(filename).
                    arg(file.errorString()).
                    arg(file.error()));

    file.close();
}
