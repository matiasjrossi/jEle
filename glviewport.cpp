#include "glviewport.h"
#include <QDebug>
#include <QWheelEvent>
#include "objectmodel.h"
#include "vertex.h"
#include "polygon.h"
#include <QImageReader>
#include <QMessageBox>
#if defined(__APPLE__) || defined(MACOSX)
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

#define min(A,B) (A>B ? B : A)
#define max(A,B) (A<B ? B : A)

#define FUCK 200

#define UP_DIRECTION QVector3D(0.0, 1.0, 0.0)

GLViewport::GLViewport(QWidget *parent) :
    QGLWidget(parent),
    lastPolygonMaterial(NULL),
    backgroundColor(Qt::black),
    defaultMaterial(NULL),
    objectModel(NULL),
    objectRotation(),
    objectShiftX(0.0),
    objectShiftY(0.0),
    objectScale(1.0),
    fieldOfView(1.0),
    eyePositionPitch(0.0),
    eyePositionYaw(0.0),
    shiftMode(false),
    ground(Material()),
    wall(Material()),
    shadowMapSize(512),
    dimLight(Qt::black, Qt::black, QColor(30, 30, 30), Vertex(0.0, 0.0, 0.0))
//    debugMode(0)
{
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);

    ground.setMapD(":/textures/ground.png");
    ground.setKS(Qt::black);
    wall.setMapD(":/textures/wall.png");
    wall.setKS(Qt::black);

    initLightProjection();

    qglClearColor(backgroundColor);
}

QColor GLViewport::getBackgroundColor() const
{
    return backgroundColor;
}

void  GLViewport::setBackgroundColor(QColor &color)
{
    backgroundColor = color;
    qglClearColor(backgroundColor);
    updateGL();
}

Material *GLViewport::getDefaultMaterial() const
{
    return defaultMaterial;
}

void GLViewport::setDefaultMaterial(Material *m)
{
    defaultMaterial = m;
    updateGL();
}

ObjectModel *GLViewport::getObjectModel() const
{
    return objectModel;
}

void GLViewport::setObjectModel(ObjectModel *om)
{
    QList<GLuint> texture_names = textures.values();
    while (!texture_names.empty())
        deleteTexture(texture_names.takeFirst());
    objectModel = om;
    updateGL();
}

void GLViewport::setLights(std::vector<Light*> & lights)
{
    this->lights = lights;
    setLayers(lights.size()*2+2);
    updateGL();
}

void GLViewport::increaseRotation(double x, double y)
{
    objectRotation.rotate(x, 1.0, 0.0);
    objectRotation.rotate(y, 0.0, 1.0);
    updateGL();
}

void GLViewport::mousePressEvent(QMouseEvent *e)
{
    if (e->type() == QMouseEvent::MouseButtonDblClick && e->button() == Qt::LeftButton) {
        shiftMode = true;
        setCursor(Qt::SizeAllCursor);
    } else if (e->type() == QMouseEvent::MouseButtonPress && e->button() == Qt::LeftButton) {
        shiftMode = false;
        setCursor(Qt::ClosedHandCursor);
    }
//    if (e->button() == Qt::RightButton) setFocus();
    lastMousePos = e->pos();
}

void GLViewport::mouseReleaseEvent(QMouseEvent *e)
{
    if (!shiftMode)
        if (e->button() == Qt::LeftButton)
            setCursor(Qt::OpenHandCursor);
}

void GLViewport::mouseMoveEvent(QMouseEvent *e)
{
    QPoint deltas = e->pos()-lastMousePos;
    if (shiftMode) {
        objectShiftX += 2.0 * static_cast<double>(deltas.x())/min(width(),height());
        objectShiftY -= 2.0 * static_cast<double>(deltas.y())/min(width(),height());
        if (objectShiftX>2.0) objectShiftX = 2.0;
        if (objectShiftY>2.0) objectShiftY = 2.0;
        if (objectShiftX<-2.0) objectShiftX = -2.0;
        if (objectShiftY<-2.0) objectShiftY = -2.0;
        updateGL();
    } else {
        if(e->buttons() & Qt::LeftButton)
        {
            if (e->modifiers() & Qt::ControlModifier) {
                objectRotation.rotate(360.0 * static_cast<double>(deltas.x())/width(), objectRotation.mapVector(QVector3D(0.0, 1.0, 0.0)));
                objectRotation.rotate(360.0 * static_cast<double>(deltas.y())/height(), objectRotation.mapVector(QVector3D(1.0, 0.0, 0.0)));
            } else {
                eyePositionPitch += 360.0 * static_cast<double>(deltas.y())/height();
                eyePositionYaw -= 360.0 * static_cast<double>(deltas.x())/width();
                if (eyePositionPitch > 100.0) eyePositionPitch = 100.0;
                if (eyePositionPitch < -10.0) eyePositionPitch = -10.0;
                if (eyePositionYaw > 120.0) eyePositionYaw = 120.0;
                if (eyePositionYaw < -120.0) eyePositionYaw = -120.0;
                updateCameraView();
            }
            updateGL();
        }
    }
    lastMousePos = e->pos();
}

//void GLViewport::keyPressEvent(QKeyEvent *e)
//{
//    if (e->key() == Qt::Key_A)
//        debugMode = 0;
//    else if (e->key() == Qt::Key_S)
//        debugMode = 1;
//    else if (e->key() == Qt::Key_D)
//        debugMode = 2;
//    else if (e->key() == Qt::Key_F)
//        debugMode = 3;
//    updateGL();
//    qDebug() << debugMode;
//}

void GLViewport::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        fieldOfView += double(e->delta())/120.0/20.0;
        if (fieldOfView < 0.3) fieldOfView = 0.3;
        if (fieldOfView > 3.0) fieldOfView = 3.0;
        updateCameraProjection();
    } else {
        objectScale += double(e->delta())/120.0/20.0;
        if (objectScale > 2.0) objectScale = 2.0;
        if (objectScale < 0.1) objectScale = 0.1;
        updateCameraView();
    }
    updateGL();
}

void GLViewport::renderEnvironment()
{

    // wall
    loadMaterial(&wall);
    loadTexture(&wall);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, 1.0);

        glTexCoord2f(0.0, 6.0);
        glVertex3f(-10.0, 0.0, -8.0);

        glTexCoord2f(10.0, 6.0);
        glVertex3f(10.0, 0.0, -8.0);

        glTexCoord2f(10.0, 0.0);
        glVertex3f(10.0, 12.0, -8.0);

        glTexCoord2f(0.0, 0.0);
        glVertex3f(-10.0, 12.0, -8.0);
    glEnd();


    // ground
    loadMaterial(&ground);
    loadTexture(&ground);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin(GL_QUADS);
        glNormal3f(0.0, 1.0, 0.0);

        glTexCoord2f(0.0, 10.0);
        glVertex3f(-10.0, 0.0, 11.0);

        glTexCoord2f(10.0, 10.0);
        glVertex3f(10.0, 0.0, 11.0);

        glTexCoord2f(10.0, 0.0);
        glVertex3f(10.0, 0.0, -8.0);

        glTexCoord2f(0.0, 0.0);
        glVertex3f(-10.0, 0.0, -8.0);
    glEnd();

}

void GLViewport::renderObject()
{

    if (objectModel != NULL) {

        glPushMatrix();

        glTranslated(objectShiftX, objectShiftY, 0.0);
        glTranslated(0.0, 0.0, -4.5); // Push the object inside the frustum (past zNear)
        glTranslated(0.0, 1.75*objectScale+2.0, 0.0); // Push the object over the ground (0.0)
        glScaled(objectScale, objectScale, objectScale);
        glMultMatrixd(objectRotation.inverted().constData());

        loadDefaultMaterial();

        GLfloat vertex[4];
        for (int i = 0; i<objectModel->getPolygons().size(); i++) {
            Polygon *polygon = objectModel->getPolygons().at(i);

            loadPolygonMaterial(polygon->getMaterial());

            // glBegin()
            switch(polygon->size()) {
                case 0: case 1: case 2:
                    qDebug() << "[GLViewport::paintGL()] : polygon size is " << polygon->size() << ". Ignoring.";
                    continue;
                case 3:
                    glBegin(GL_TRIANGLES);
                    break;
                case 4:
                    glBegin(GL_QUADS);
                    break;
                default:
                    glBegin(GL_POLYGON);
                    break;
            }

            // glVertex*(), etc.
            for (int j = 0; j<polygon->size(); j++) {
                if (polygon->hasTextureMapping()) {
                    polygon->getTextureVertex(j).getArray(vertex);
                    glTexCoord2fv(vertex);
                }
                polygon->getNormal(j).getArray(vertex);
                glNormal3fv(vertex);
                polygon->getVertex(j).getArray(vertex);
                glVertex4fv(vertex);
            }

            // glEnd()
            glEnd();
        }

        glPopMatrix();
    }
}

void GLViewport::renderLayer(GLuint id)
{
    glBindTexture(GL_TEXTURE_2D, id);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
}

void GLViewport::renderOverlays()
{
    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport(0,height()-FUCK,FUCK,FUCK);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);

    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 1.0, 0.0);
    glEnd();

    glViewport(0,height()-2*FUCK,FUCK,FUCK);
    glBindTexture(GL_TEXTURE_2D, fuckTex);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 1.0, 0.0);
    glEnd();

    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();

    for (unsigned i=0; i<layers.size(); ++i)
    {
        glPushAttrib(GL_VIEWPORT_BIT);

        glViewport(width()-FUCK,height()-FUCK-FUCK*i,FUCK,FUCK);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);

        glBindTexture(GL_TEXTURE_2D, layers.at(i));
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(0.0, 0.0, 0.0);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(1.0, 0.0, 0.0);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(1.0, 1.0, 0.0);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(0.0, 1.0, 0.0);
        glEnd();

        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glPopAttrib();
    }
}

void GLViewport::initializeGL()
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);

    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &fuckTex);
    glBindTexture(GL_TEXTURE_2D, fuckTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shadowMapSize, shadowMapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &shadowMapFB);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFB);
//    glDrawBuffer(GL_NONE);
//    glReadBuffer(GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fuckTex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << "Framebuffer status != GL_FRAMEBUFFER_COMPLETE";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &targetFB);

    setLayers(0);
    setLayers(lights.size()*2+2);

    updateCameraView();
}

void GLViewport::paintGL()
{

    for (unsigned i=0; i<lights.size(); i++)
    {

        // Render to get the shadow map
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFB);
        glViewport(0, 0, shadowMapSize, shadowMapSize);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(lightProjection.constData());

        QMatrix4x4 lightView = getLightView(lights.at(i));
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(lightView.constData());

        glCullFace(GL_FRONT);
        glShadeModel(GL_FLAT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        renderEnvironment();
        renderObject();


        glCullFace(GL_BACK);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Prepare GL to draw to the target again
        glBindFramebuffer(GL_FRAMEBUFFER, targetFB);
        glViewport(0, 0, width()-2*FUCK, height());

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(cameraProjection.constData());

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(cameraView.constData());

        // Render to the target with lighting and the mask
        // Texture coordinates projection matrix.
        static QMatrix4x4 eye2texture = QMatrix4x4(
                    0.5f, 0.0f, 0.0f, 0.5f,
                    0.0f, 0.5f, 0.0f, 0.5f,
                    0.0f, 0.0f, 0.5f, 0.5f,
                    0.0f, 0.0f, 0.0f, 1.0f);
        QMatrix4x4 textureMatrix = eye2texture * lightProjection * lightView;

        //Set up texture coordinate generation.
        glActiveTexture(GL_TEXTURE1);
        GLdouble a[4];
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        getDoubleArray(textureMatrix.row(0), a);
        glTexGendv(GL_S, GL_EYE_PLANE, a);
        glEnable(GL_TEXTURE_GEN_S);

        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        getDoubleArray(textureMatrix.row(1), a);
        glTexGendv(GL_T, GL_EYE_PLANE, a);
        glEnable(GL_TEXTURE_GEN_T);

        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        getDoubleArray(textureMatrix.row(2), a);
        glTexGendv(GL_R, GL_EYE_PLANE, a);
        glEnable(GL_TEXTURE_GEN_R);

        glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        getDoubleArray(textureMatrix.row(3), a);
        glTexGendv(GL_Q, GL_EYE_PLANE, a);
        glEnable(GL_TEXTURE_GEN_Q);

        glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
        glEnable(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

        glAlphaFunc(GL_GEQUAL, 0.99f);
        glEnable(GL_ALPHA_TEST);

        glActiveTexture(GL_TEXTURE0);

        setLight(lights.at(i));

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layers.at(i), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderEnvironment();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layers.at(lights.size()+i), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderObject();


        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
        glDisable(GL_TEXTURE_GEN_Q);

        glDisable(GL_ALPHA_TEST);
    }

    // Shadowed areas base
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(cameraProjection.constData());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(cameraView.constData());

    setLight(&dimLight);

    // ... for the environment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layers.at(layers.size()-2), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderEnvironment();

    // ... for the object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layers.at(layers.size()-1), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderObject();

    // Now compose the layers to the window manager framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(FUCK, 0, width()-2*FUCK, height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float factor = 1.0f / lights.size();

    renderLayer(layers.at(layers.size()-2));

//    glBlendColor(factor, factor, factor, factor);
//    glBlendFunc(GL_CONSTANT_COLOR, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor4f(1.0, 1.0, 1.0, factor);
    for (unsigned i=0; i < lights.size(); i++)
        renderLayer(layers.at(i));

    glDisable(GL_BLEND);
    glBlendColor(0.0, 0.0, 0.0, 0.0);
    glBlendFunc(GL_ONE, GL_ZERO);

//    renderLayer(layers.at(layers.size()-1));

//    glBlendColor(factor, factor, factor, factor);
//    glBlendFunc(GL_CONSTANT_COLOR, GL_ONE);
//    glEnable(GL_BLEND);
//    for (unsigned i=lights.size(); i < lights.size()*2; i++)
//        renderLayer(layers.at(i));

//    glDisable(GL_BLEND);
//    glBlendColor(0.0, 0.0, 0.0, 0.0);
//    glBlendFunc(GL_ONE, GL_ZERO);

    renderOverlays();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    ///////////

    /////////
}

void GLViewport::resizeGL(int w, int h)
{
    updateCameraProjection();
    QGLWidget::resizeGL(w, h);
    unsigned layersCount = layers.size();
    setLayers(0);
    setLayers(layersCount);
}

//QMatrix4x4 GLViewport::toQMatrix4x4(GLdouble a[])
//{
//    QMatrix4x4 m;
//    for (int i=0; i<4; ++i)
//        m.setColumn(i, QVector4D(a[i*4], a[1+i*4], a[2+i*4], a[3+i*4]));
//    return m;
//}

void GLViewport::getDoubleArray(QVector4D v, GLdouble a[])
{
    a[0] = v.x();
    a[1] = v.y();
    a[2] = v.z();
    a[3] = v.w();
}

void GLViewport::updateCameraProjection()
{
    double hor = max(static_cast<double>(width()-200)/static_cast<double>(height()),1.0);
    double ver = max(static_cast<double>(height())/static_cast<double>(width()-200),1.0);

    cameraProjection.setToIdentity();
    cameraProjection.frustum((-hor)*fieldOfView, hor*fieldOfView, (-ver)*fieldOfView, ver*fieldOfView, 1.0, 25.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glFrustum((-hor)*fieldOfView, hor*fieldOfView, (-ver)*fieldOfView, ver*fieldOfView, 1.0, 25.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, cameraProjection2);
    glPopMatrix();
}

void GLViewport::updateCameraView()
{
    QMatrix4x4 eyeRotation;
    eyeRotation.rotate(eyePositionPitch, -1.0, 0.0);
    eyeRotation.rotate(eyePositionYaw, 0.0, 1.0);
    QVector3D eyePosition = eyeRotation.mapVector(QVector3D(0.0, 0.0, 4.5)) + getObjectCenter();

    cameraView.setToIdentity();
    cameraView.lookAt(eyePosition,
                      getObjectCenter(),
                      UP_DIRECTION);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(eyePosition.x(), eyePosition.y(), eyePosition.z(),
              getObjectCenter().x(), getObjectCenter().y(), getObjectCenter().z(),
              UP_DIRECTION.x(), UP_DIRECTION.y(), UP_DIRECTION.z());
    glGetDoublev(GL_MODELVIEW_MATRIX, cameraView2);
    glPopMatrix();
}

QMatrix4x4 GLViewport::getLightView(Light *light)
{
    QMatrix4x4 lightView;
    lightView.lookAt(light->getPos().toQVector().toVector3D(),
                  getObjectCenter(),
                  UP_DIRECTION);
    return lightView;
}

void GLViewport::getLightView(Light *light, GLdouble *matrix)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    gluLookAt(light->getPos().x(), light->getPos().y(), light->getPos().z(),
              getObjectCenter().x(), getObjectCenter().y(), getObjectCenter().z(),
              UP_DIRECTION.x(), UP_DIRECTION.y(), UP_DIRECTION.z());
    glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();
}

void GLViewport::initLightProjection()
{
    lightProjection.setToIdentity();
    lightProjection.frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glFrustum(-0.5, 0.5, -0.5, 0.5, 1.0, 20.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, lightProjection2);
    glPopMatrix();
}

QVector3D GLViewport::getObjectCenter()
{
    return QVector3D(0.0, 1.75*objectScale+2.0, -4.5);
}


//    GLenum names[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
//    if (lightsList != 0) {
//        glDeleteLists(lightsList, 1);
//        lightsList = 0;
//    }

void GLViewport::setLight(Light *light)
{

    glPushMatrix();

    glTranslatef(0.0, 4.0, -4.5);
    glScalef(4.0, 4.0, 3.4);

    qreal color[4];
    GLfloat colorF[4];
    GLfloat position[4];

    glEnable(GL_LIGHT0);
    // Ambient
    light->getIA().getRgbF(&color[0], &color[1], &color[2], &color[3]);
    for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
    glLightfv(GL_LIGHT0, GL_AMBIENT, colorF);
    // Specular
    light->getIS().getRgbF(&color[0], &color[1], &color[2], &color[3]);
    for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
    glLightfv(GL_LIGHT0, GL_SPECULAR, colorF);
    // Diffuse
    light->getID().getRgbF(&color[0], &color[1], &color[2], &color[3]);
    for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, colorF);
    // Position
    light->getPos().getArray(position);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glPopMatrix();

//    QMatrix4x4 scale;
//        scale.scale(4.0, 4.0, 3.4);
//        firstLightPOV = scale.mapVector(lights.at(0)->getPos().toQVector().toVector3D()) + QVector3D(0.0, 4.0, -4.5);
//        updateLightMatrices();

//        lightsList = glGenLists(1);
//        glNewList(lightsList, GL_COMPILE);

//            glMatrixMode(GL_MODELVIEW);

//            glDisable(GL_LIGHTING); // Disable lighting to render the spheres
//            glEnable(GL_COLOR_MATERIAL);

//            for (unsigned i=0; i<lights.size(); i++) {

//                // Sphere
//                glPushMatrix();
//                glTranslatef(position[0], position[1], position[2]);
//                glColor4fv(colorF);
//                GLUquadric *q = gluNewQuadric();
//                gluSphere(q, 0.02, 50, 10);
//                gluDeleteQuadric(q);
//                glPopMatrix();

//            }

//            glEnable(GL_LIGHTING);
//            glDisable(GL_COLOR_MATERIAL);

//            glPopMatrix();

//        glEndList();
//    }
}

void GLViewport::loadPolygonMaterial(Material *material) {
    if (lastPolygonMaterial != material) {
        if (material == NULL)
            loadDefaultMaterial();
        else {
            loadMaterial(material);
            loadTexture(material);
            lastPolygonMaterial = material;
        }
    }
}

void GLViewport::loadTexture(Material *material)
{
    QString filename = material->getMapD();
    if (filename == "") {
        glDisable(GL_TEXTURE_2D);
    } else {
        if (textures.contains(filename)) {
            glBindTexture(GL_TEXTURE_2D, textures.value(filename));
            glEnable(GL_TEXTURE_2D);
        } else {
            QImage texture;
            if (filename.startsWith(":/")) {
                texture.load(filename);
            } else {
                QImageReader fileReader(filename);
                texture = fileReader.read();
                if (texture.isNull()) {
                    QMessageBox::warning(
                            NULL,
                            "Warning",
                            QString("Error opening texture file: %1\n\n%2 (%3)").
                                arg(filename).
                                arg(fileReader.errorString()).
                                arg(fileReader.error()));
                    material->setMapD("");
                    glDisable(GL_TEXTURE_2D);
                    return;
                }
            }
            textures[filename] = bindTexture(texture);
            glEnable(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
    }
}

void GLViewport::loadDefaultMaterial() {
    loadMaterial(defaultMaterial);
    glDisable(GL_TEXTURE_2D);
    lastPolygonMaterial = NULL;
}

void GLViewport::loadMaterial(Material *material) {
    if (material != NULL) {
        qreal color[4];
        GLfloat colorF[4];
        // Ambient
        material->getKA().getRgbF(&color[0], &color[1], &color[2], &color[3]);
        for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
        glMaterialfv(GL_FRONT, GL_AMBIENT, colorF);
        // Diffuse
        material->getKD().getRgbF(&color[0], &color[1], &color[2], &color[3]);
        for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, colorF);
        // Specular
        material->getKS().getRgbF(&color[0], &color[1], &color[2], &color[3]);
        for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
        glMaterialfv(GL_FRONT, GL_SPECULAR, colorF);
        // Emission
        material->getKE().getRgbF(&color[0], &color[1], &color[2], &color[3]);
        for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
        glMaterialfv(GL_FRONT, GL_EMISSION, colorF);
        glMaterialf(GL_FRONT, GL_SHININESS, material->getQ());
    }
}

void GLViewport::setLayers(int n)
{
//    qDebug() << "setting layers to " << n;
    while (layers.size() < n) {
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width()-2*FUCK, height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glBindTexture(GL_TEXTURE_2D, 0);
        layers.push_back(id);
    }
    while (layers.size() > n) {
        glDeleteTextures(1, &layers.last());
        layers.pop_back();
    }
}
