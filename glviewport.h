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
//    void keyPressEvent(QKeyEvent *);

private:
    void setLight(Light*);
    void loadPolygonMaterial(Material *material);
    void loadTexture(Material *material);
    void loadDefaultMaterial();
    void loadMaterial(Material *material);
    void setLayers(int);

    void renderEnvironment();
    void renderObject();
    void renderLayer(GLuint);
    void renderOverlays();

    void updateCameraView();
    void updateCameraProjection();
    void initLightProjection();
    QMatrix4x4 getLightView(Light *);
    void getLightView(Light *, GLdouble *);

    QVector3D getObjectCenter();
//    QMatrix4x4 toQMatrix4x4(GLdouble a[]);
    void getDoubleArray(QVector4D, GLdouble a[]);

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
//    QVector3D eyePosition;

    QPoint lastMousePos;
    bool shiftMode;

    QMap<QString, GLuint> textures;

    Material ground, wall;

//    GLuint lightsList;

    QMatrix4x4 cameraView, cameraProjection, lightProjection;
    GLdouble cameraView2[16], cameraProjection2[16], lightProjection2[16];

    //Shadow mapping
    GLuint shadowMapFB;
    GLuint shadowMapTexture, fuckTex;
    QVector<GLuint> layers;
    GLuint targetFB;

    const GLuint shadowMapSize;

//    QVector3D firstLightPOV;

//    short debugMode;

    std::vector<Light*> lights;
    Light dimLight;

};

#endif // GLVIEWPORT_H
