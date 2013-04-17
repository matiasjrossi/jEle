#ifndef GLVIEWPORT_H
#define GLVIEWPORT_H

#include <QtOpenGL/QGLWidget>
#include <vector>
#include <QMatrix4x4>
#include <QVector3D>

#include "light.h"
#include "material.h"
class ObjectModel;

class GLViewport : public QGLWidget
{
public:
    GLViewport(QWidget *parent);

    QColor getBackgroundColor() const;
    void setBackgroundColor(QColor&);
    Material *getDefaultMaterial() const;
    void setDefaultMaterial(Material*);
    ObjectModel *getObjectModel() const;
    void setObjectModel(ObjectModel*);
    void setLights(std::vector<Light*> &lights);

    void increaseRotation(double x, double y);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:
    void setLight(Light);
    void loadPolygonMaterial(Material *material);
    void loadTexture(Material *material);
    void loadDefaultMaterial();
    void loadMaterial(Material *material);

    void renderEnvironment();
    void renderObject();

    void updateCameraView();
    void updateCameraProjection();
    void initLightProjection();
    QMatrix4x4 getLightView(Light);
    void getLightView(Light *, GLdouble *);

    QVector3D getObjectCenter();
    void getDoubleArray(QVector4D, GLdouble a[]);
    Light withEyeCoords(Light *);

    Material *lastPolygonMaterial;

    QColor backgroundColor;
    Material *defaultMaterial;
    ObjectModel *objectModel;

    QMatrix4x4 objectRotation;
    double objectShiftX;
    double objectShiftY;
    double objectScale;
    double fieldOfView;
    double eyePositionPitch, eyePositionYaw;

    QPoint lastMousePos;
    bool shiftMode;

    QMap<QString, GLuint> textures;

    Material ground, wall;

    QMatrix4x4 cameraView, cameraProjection, lightProjection;

    //Shadow mapping
    GLuint shadowMapFB;
    GLuint shadowMapTexture;

    const GLuint shadowMapSize;

    std::vector<Light*> lights;
    Light dimLight;

};

#endif // GLVIEWPORT_H
