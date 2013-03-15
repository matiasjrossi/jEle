#include "surreader.h"
#include "objectmodel.h"
#include <QFile>
#include <QDebug>
#include <QStringList>

ObjectModel *SURReader::openSUR(QString fileName)
{
    Parser parser(fileName);
    ObjectModel *om = new ObjectModel();
    for (int i=0; i<parser.getCoordinates().size(); i++)
    {
        Parser::Coordinate c = parser.getCoordinates().at(i);
        om->addVertex(c.x, c.y, c.z);
    }
    for (int i=0; i<parser.getIncidences().size(); i++)
    {
        Parser::Incidence in= parser.getIncidences().at(i);
        QList<int> vertices;
        vertices.append(in.a-1);
        vertices.append(in.b-1);
        vertices.append(in.c-1);
        om->addPolygon(vertices);
    }
    om->normalize();
    return om;
}

SURReader::Parser::Parser(QString fileName) :
        groupCount(-1),
        coordinateCount(-1),
        lineno(0),
        fileName(fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!file.atEnd()) {
            lineno++;
            QString line = file.readLine().trimmed();
            if (line.size() != 0) {
                if (isHeader(line))
                    parseHeader(line);
                else
                    dispatch(line);
            }
        }
        file.close();
    }
}

QList<SURReader::Parser::Group> &SURReader::Parser::getGroups()
{
    return this->groups;
}

QList<SURReader::Parser::Incidence> &SURReader::Parser::getIncidences()
{
    return this->incidences;
}

QList<SURReader::Parser::Coordinate> &SURReader::Parser::getCoordinates()
{
    return this->coordinates;
}

bool SURReader::Parser::isHeader(QString line)
{
    return (line.at(0) == '*');
}

void SURReader::Parser::parseHeader(QString line)
{
    if (line.startsWith("*ELEMENT GROUPS", Qt::CaseInsensitive))
        state = GROUPS;
    else if (line.startsWith("*COORDINATES", Qt::CaseInsensitive))
        state = COORDINATES;
    else if (line.startsWith("*INCIDENCE", Qt::CaseInsensitive))
        state = INCIDENCE;
    else if (line.startsWith("*FRONT_VOLUME", Qt::CaseInsensitive))
        state = VOLUME;
    else
        ignoreLine(line);
}

void SURReader::Parser::ignoreLine(QString line)
{
    qDebug() << "[SUROpener::Parser] Ignoring line "<< fileName << ":" << lineno << " | " << line;
}

void SURReader::Parser::dispatch(QString line)
{
    switch (state)
    {
    case GROUPS:
        parseGroup(line);
        break;
    case INCIDENCE:
        parseIncidence(line);
        break;
    case COORDINATES:
        parseCoordinate(line);
        break;
    case VOLUME:
        ignoreLine(line);
        break;
    default:
        ignoreLine(line);
    }
}

void SURReader::Parser::parseGroup(QString line)
{
    QStringList items = line.split(' ', QString::SkipEmptyParts);
    /*
     * The first line of the "ELEMENT GROUPS" block indicates how many
     * groups are contained within the section
     */
    if (groupCount == -1)
    {
        if (items.size() < 1) {
            qDebug() << "[SUROpener::Parser::parseGroup] ERROR: Ignoring malformed group count line:" << line;
        } else {
            if (items.size() > 1) {
                qDebug() << "[SUROpener::Parser::parseGroup] WARNING: Malformed group count line:" << line << "- Discarding extra values.";
            }
            groupCount = items.at(0).toInt();
        }
    }
    else
    {
        if (items.size() < 3) {
            qDebug() << "[SUROpener::Parser::parseGroup] ERROR: Ignoring malformed group line:" << line;
        } else {
            if (items.size() > 3) {
                qDebug() << "[SUROpener::Parser::parseGroup] WARNING: Malformed group line:" << line << "- Discarding extra values.";
            }
            Group g;
            g.first = items.at(0).toInt();
            g.last = items.at(1).toInt();
            groups.push_back(g);
        }
    }
}

void SURReader::Parser::parseIncidence(QString line)
{
    QStringList items = line.split(' ', QString::SkipEmptyParts);
    if (items.size() < 3) {
        qDebug() << "[SUROpener::Parser::parseIncidence] ERROR: Ignoring malformed incidence line:" << line;
    } else {
        if (items.size() > 3) {
            qDebug() << "[SUROpener::Parser::parseIncidence] WARNING: Malformed incidence line:" << line << "- Discarding extra values.";
        }
        Incidence i;
        i.a = items.at(0).toInt();
        i.b = items.at(1).toInt();
        i.c = items.at(2).toInt();
        incidences.push_back(i);
    }
}

void SURReader::Parser::parseCoordinate(QString line)
{
    QStringList items = line.split(' ', QString::SkipEmptyParts);
    /*
     * The first line of the "COORDINATES" block indicates how many
     * coordinates are contained within the section
     */
    if (coordinateCount == -1)
    {
        if (items.size() < 1) {
            qDebug() << QString("[SUROpener::Parser::parseCoordinate] ERROR: Ignoring malformed coordinate count line: ") << line;
        } else {
            if (items.size() > 1) {
                qDebug() << QString("[SUROpener::Parser::parseCoordinate] WARNING: Malformed coordinate count line: ") << line << QString(" - Discarding extra values.");
            }
            coordinateCount = items.at(0).toInt();
        }
    }
    else
    {
        if (items.size() < 4) {
            qDebug() << QString("[SUROpener::Parser::parseCoordinate] ERROR: Ignoring malformed coordinate line: ") << line;
        } else {
            if (items.size() > 4) {
                qDebug() << QString("[SUROpener::Parser::parseCoordinate] WARNING: Malformed coordinate line: ") << line << QString(" - Discarding extra values.");
            }
            Coordinate c;
            c.x = items.at(1).toDouble();
            c.y = items.at(2).toDouble();
            c.z = items.at(3).toDouble();
            coordinates.push_back(c);
        }
    }
}
