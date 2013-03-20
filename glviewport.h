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
    Material *getDefaultMaterial() const;
    void setDefaultMaterial(Material*);
    ObjectModel *getObjectModel() const;
    void setObjectModel(ObjectModel*);
    bool getWireframeVisibility() const;
    void setWireframeVisibility(bool);

    void increaseRotation(double x, double y);
    double getRotationX() const;
    double getRotationY() const;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:
    void loadPolygonMaterial(Material *material);
    void loadTextureContext(Material *material);
    void loadDefaultMaterial();
    void loadMaterial(Material *material);

    Material *lastPolygonMaterial;

    QColor backgroundColor;
    Material *defaultMaterial;
    ObjectModel *objectModel;

    double rotX;
    double rotY;
    double shiftX;
    double shiftY;
    double zoom;
    bool wireframeVisibility;

    QPoint lastMousePos;
    bool shiftMode;

};

#endif // GLVIEWPORT_H
