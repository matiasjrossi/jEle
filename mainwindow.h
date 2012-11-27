#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "transformation.h"

class GLViewport;
//class ObjectModel;
class QTimer;
class QPushButton;
class Material;
class Light;
class Vertex;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    GLViewport *vp;
    QTimer *timer;
    QPushButton *background;
    Material *objectMaterial;
    std::vector<Light*> lightsContext;
    QList<Transformation*> *transformations;
    QString vertex2String(Vertex v);
    bool isAnimated;

private slots:
    void on_actionOpen_triggered();
    void on_actionAnimation_toggled(bool);
    void on_actionToggle_wireframe_toggled(bool);
    void rotate(double x, double y);
    void autoRotate();
    void changeBackgroundColor();
    void addLight();
    void deleteSelectedLight();
    void updateLightButtons();
    void changeLightDiffuse();
    void changeLightSpecular();
    void changeLightAmbient();
    void changeLightPosition();
    void changeObjectAmbient();
    void changeObjectDiffuse();
    void changeObjectSpecular();
    void changeObjectEmission();
    void changeObjectShininess(double);
    void addTransformation();
    void deleteSelectedTransformation();
    void updateTransformationUI();
    void readTransformationUI();
    void zoom(int);
    void shift(double, double);
};

#endif // MAINWINDOW_H
