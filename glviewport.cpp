#include "glviewport.h"
#include "light.h"
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

GLViewport::GLViewport(QWidget *parent) :
    QGLWidget(parent),
    lastPolygonMaterial(NULL),
    backgroundColor(Qt::black),
    defaultMaterial(NULL),
    objectModel(NULL),
    objectRotation(),
    shiftX(0.0),
    shiftY(0.0),
    objectScale(1.0),
    fieldOfView(1.0),
    eyePositionPitch(0.0),
    eyePositionYaw(0.0),
    shiftMode(false),
    ground(Material()),
    wall(Material()),
    lightsList(0),
    shadowMapSize(512),
    firstLightPOV()
//    debugMode(0)
{
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    ground.setMapD(":/textures/ground.png");
    ground.setKS(Qt::black);
    wall.setMapD(":/textures/wall.png");
    wall.setKS(Qt::black);
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
        shiftX += 2.0 * static_cast<double>(deltas.x())/min(width(),height());
        shiftY -= 2.0 * static_cast<double>(deltas.y())/min(width(),height());
        if (shiftX>2.0) shiftX = 2.0;
        if (shiftY>2.0) shiftY = 2.0;
        if (shiftX<-2.0) shiftX = -2.0;
        if (shiftY<-2.0) shiftY = -2.0;
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

void GLViewport::updateCameraView()
{
    QMatrix4x4 eyeRotation;
    eyeRotation.rotate(eyePositionPitch, -1.0, 0.0);
    eyeRotation.rotate(eyePositionYaw, 0.0, 1.0);
    QVector3D eyePosition = eyeRotation.mapVector(QVector3D(0.0, 0.0, 4.5)) + QVector3D(0.0, 1.75*objectScale+2.0, -4.5);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(eyePosition.x(), eyePosition.y(), eyePosition.z(),
              0.0, 1.75*objectScale+2.0, -4.5,
              0.0, 1.0, 0.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, cameraView);
    glPopMatrix();
}

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
        updateLightMatrices();
        updateCameraView();
    }
    updateGL();
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

//    glGenFramebuffers(1, &depthFramebuffer);
    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0,
        GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);

//    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

//    glDrawBuffer(GL_NONE); // No color buffer is drawn to.

//    // Always check that our framebuffer is ok
//    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//    return false;
    updateCameraView();
    updateLightMatrices();
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

//     glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
//    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);

void GLViewport::renderObject()
{

    if (objectModel != NULL) {

        glPushMatrix();

        glTranslated(shiftX, shiftY, 0.0);
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

void GLViewport::paintGL()
{

    // First render to get the shadow map
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(lightProjection);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(lightView);

    glViewport(0, 0, shadowMapSize, shadowMapSize);

    glCullFace(GL_FRONT);
    glShadeModel(GL_FLAT);
    glColorMask(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);

    renderEnvironment();
    renderObject();

    //Read the depth buffer into the shadow map texture
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapSize, shadowMapSize);

    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);
    glColorMask(1, 1, 1, 1);


    // Set GL to draw in the screen again
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width(), height());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(cameraProjection);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(cameraView);

    // Second render to the screen with simple ambient light
    // Setup a dim light for the shadow pass
    glEnable(GL_LIGHTING);
    static GLenum names[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
    for (unsigned i=0; i<8; i++)
        glDisable(names[i]);
    glEnable(GL_LIGHT0);
    static GLfloat black[3] = {0.0f, 0.0f, 0.0f};
    static GLfloat gray80[3] = {0.4f, 0.4f, 0.4f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, gray80);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, black);
    glLightfv(GL_LIGHT0, GL_EMISSION, black);
    glLightfv(GL_LIGHT0, GL_SPECULAR, black);

    renderEnvironment();
    renderObject();
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);

    // Third render to the screen with lights and the mask
    //Texture coordinates projection matrix.
    static QMatrix4x4 eye2texture = QMatrix4x4(
                0.5f, 0.0f, 0.0f, 0.5f,
                0.0f, 0.5f, 0.0f, 0.5f,
                0.0f, 0.0f, 0.5f, 0.5f,
                0.0f, 0.0f, 0.0f, 1.0f);
    QMatrix4x4 textureMatrix = eye2texture * toQMatrix4x4(lightProjection) * toQMatrix4x4(lightView);

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
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_ALPHA);

    glAlphaFunc(GL_GEQUAL, 0.90f);
    glEnable(GL_ALPHA_TEST);

    glActiveTexture(GL_TEXTURE0);

    glCallList(lightsList);
    renderEnvironment();
    renderObject();

    glDisable(GL_TEXTURE_2D);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    glDisable(GL_ALPHA_TEST);
}

QMatrix4x4 GLViewport::toQMatrix4x4(GLdouble a[])
{
    QMatrix4x4 m;
    for (int i=0; i<4; ++i)
        m.setColumn(i, QVector4D(a[i*4], a[1+i*4], a[2+i*4], a[3+i*4]));
    return m;
}

void GLViewport::getDoubleArray(QVector4D v, GLdouble a[])
{
    a[0] = v.x();
    a[1] = v.y();
    a[2] = v.z();
    a[3] = v.w();
}

void GLViewport::resizeGL(int w, int h)
{
    updateCameraProjection();
}

void GLViewport::updateCameraProjection()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    double hor = max(static_cast<double>(width())/static_cast<double>(height()),1.0);
    double ver = max(static_cast<double>(height())/static_cast<double>(width()),1.0);
    glFrustum((-hor)*fieldOfView, hor*fieldOfView, (-ver)*fieldOfView, ver*fieldOfView, 1.0, 15.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, cameraProjection);
    glPopMatrix();
}

void GLViewport::updateLightMatrices()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    gluLookAt(firstLightPOV.x(), firstLightPOV.y(), firstLightPOV.z(),
              0.0, 1.75*objectScale+2.0, -4.5,
              0.0, 1.0, 0.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, lightView);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glFrustum(-0.5, 0.5, -0.5, 0.5, 1.0, 20.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, lightProjection);
    glPopMatrix();
}

void GLViewport::resetLights(std::vector<Light*> & lights)
{
    GLenum names[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
    if (lightsList != 0) {
        glDeleteLists(lightsList, 1);
        lightsList = 0;
    }
    qreal color[4];
    GLfloat colorF[4];
    GLfloat position[4];
    for (unsigned i=0; i<8; i++)
        glDisable(names[i]);
    if (!lights.empty()) {

        QMatrix4x4 scale;
        scale.scale(4.0, 4.0, 3.4);
        firstLightPOV = scale.mapVector(lights.at(0)->getPos().toQVector().toVector3D()) + QVector3D(0.0, 4.0, -4.5);
        updateLightMatrices();

        lightsList = glGenLists(1);
        glNewList(lightsList, GL_COMPILE);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();

            glTranslatef(0.0, 4.0, -4.5);
            glScalef(4.0, 4.0, 3.4);

            glDisable(GL_LIGHTING); // Disable lighting to render the spheres
            glEnable(GL_COLOR_MATERIAL);

            for (unsigned i=0; i<lights.size(); i++) {
                glEnable(names[i]);
                // Ambient
                lights.at(i)->getIA().getRgbF(&color[0], &color[1], &color[2], &color[3]);
                for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
                glLightfv(names[i], GL_AMBIENT, colorF);
                // Specular
                lights.at(i)->getIS().getRgbF(&color[0], &color[1], &color[2], &color[3]);
                for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
                glLightfv(names[i], GL_SPECULAR, colorF);
                // Diffuse
                lights.at(i)->getID().getRgbF(&color[0], &color[1], &color[2], &color[3]);
                for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
                glLightfv(names[i], GL_DIFFUSE, colorF);
                // Position
                lights.at(i)->getPos().getArray(position);
                glLightfv(names[i], GL_POSITION, position);

                // Sphere
                glPushMatrix();
                glTranslatef(position[0], position[1], position[2]);
                glColor4fv(colorF);
                GLUquadric *q = gluNewQuadric();
                gluSphere(q, 0.02, 50, 10);
                gluDeleteQuadric(q);
                glPopMatrix();

            }

            glEnable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);

            glPopMatrix();

        glEndList();
    }
    updateGL();
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
