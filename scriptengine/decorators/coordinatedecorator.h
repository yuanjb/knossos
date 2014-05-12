#ifndef COORDINATEDECORATOR_H
#define COORDINATEDECORATOR_H

#include <QObject>

class Coordinate;
class CoordinateDecorator : public QObject
{
    Q_OBJECT
public:
    explicit CoordinateDecorator(QObject *parent = 0);


signals:

public slots:

    Coordinate *new_Coordinate();
    Coordinate *new_Coordinate(int x, int y, int z);

    int x(Coordinate *self);
    int y(Coordinate *self);
    int z(Coordinate *self);

    void setx(Coordinate *self, int x);
    void sety(Coordinate *self, int y);
    void setz(Coordinate *self, int z);

    QString static_Coordinate_help();


};

#endif // COORDINATEDECORATOR_H