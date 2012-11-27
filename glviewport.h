#ifndef GLVIEWPORT_H
#define GLVIEWPORT_H

#include <QtOpenGL/QGLWidget>
#include <vector>

class Light;
class Material;
class ObjectModel;

class GLViewport : public QGLWidget
{
public:
    GLViewport(QWidget *parent);
    void resetLights(std::vector<Light*> &lights);
    QColor getBackgroundColor() const;
    void setBackgroundColor(QColor&);
    Material *getMaterial() const;
    void setMaterial(Material*);
    ObjectModel *getObjectModel() const;
    void setObjectModel(ObjectModel*);
    bool getWireframeVisibility() const;
    void setWireframeVisibility(bool);

    void changeRotation(double x, double y);
    double getRotationX() const;
    double getRotationY() const;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

private:
    void loadMaterial();

    QColor backgroundColor;
    Material *material;
    ObjectModel *objectModel;

    double rotX;
    double rotY;
    bool wireframeVisibility;

};

#endif // GLVIEWPORT_H
