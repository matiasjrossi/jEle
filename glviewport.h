#ifndef GLVIEWPORT_H
#define GLVIEWPORT_H

#include <QtOpenGL/QGLWidget>
#include <vector>
#include <QMatrix4x4>

class Light;
#include "material.h"
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

    void increaseRotation(double x, double y);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
//    void keyPressEvent(QKeyEvent *);

private:
    void loadPolygonMaterial(Material *material);
    void loadTexture(Material *material);
    void loadDefaultMaterial();
    void loadMaterial(Material *material);
    void renderEnvironment();
    void renderObject();
    void updateCameraView();
    void updateCameraProjection();
    void updateLightMatrices();
    QMatrix4x4 toQMatrix4x4(GLdouble a[]);
    void getDoubleArray(QVector4D, GLdouble a[]);

    Material *lastPolygonMaterial;

    QColor backgroundColor;
    Material *defaultMaterial;
    ObjectModel *objectModel;

    QMatrix4x4 objectRotation;
    double shiftX;
    double shiftY;
    double objectScale;
    double fieldOfView;
    double eyePositionPitch, eyePositionYaw;
//    QVector3D eyePosition;

    QPoint lastMousePos;
    bool shiftMode;

    QMap<QString, GLuint> textures;

    Material ground, wall;

    GLuint lightsList;

    GLdouble cameraView[16], cameraProjection[16], lightView[16], lightProjection[16];

    //Shadow mapping
//    GLuint depthFramebuffer;
    GLuint shadowMapTexture;

    const GLuint shadowMapSize;

    QVector3D firstLightPOV;

//    short debugMode;

};

#endif // GLVIEWPORT_H
