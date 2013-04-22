#ifndef SUROPENER_H
#define SUROPENER_H
#include <QString>
#include <QList>
#include <QVector>

namespace jEle {
    class ObjectModel;

    class SURReader
    {
    public:
        static ObjectModel *openSUR(QString fileName);
    private:
        class Parser
        {
        public:
            Parser(QString fileName);
            typedef struct {int first, last;} Group;
            typedef struct {int a, b, c;} Incidence;
            typedef struct {double x, y, z;} Coordinate;
            QList<Group> &getGroups();
            QList<Incidence> &getIncidences();
            QList<Coordinate> &getCoordinates();
        private:
            typedef enum {INITIAL = 0, GROUPS, INCIDENCE, COORDINATES, VOLUME} State;
            Parser::State state;
            QList<Group> groups;
            int groupCount;
            QList<Incidence> incidences;
            QList<Coordinate> coordinates;
            int coordinateCount;

            unsigned lineno;
            QString fileName;

            void parseHeader(QString);
            bool isHeader(QString);
            void dispatch(QString);
            void parseGroup(QString);
            void parseIncidence(QString);
            void parseCoordinate(QString);
            void ignoreLine(QString);
        };
    };

}

#endif // SUROPENER_H
