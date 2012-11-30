#include "glviewport.h"
#include "light.h"
#include "material.h"
#include <QDebug>
#include <QWheelEvent>
#include "objectmodel.h"
#include "vertex.h"
#include "triangle.h"

#define min(A,B) (A>B ? B : A)
#define max(A,B) (A<B ? B : A)

GLViewport::GLViewport(QWidget *parent) :
    QGLWidget(parent),
    backgroundColor(QColor(0,25,40)),
    material(NULL),
    objectModel(NULL),
    rotX(0.0),
    rotY(0.0),
    shiftX(0.0),
    shiftY(0.0),
    zoom(1.0),
    wireframeVisibility(false),
    shiftMode(false)
{
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
}

QColor GLViewport::getBackgroundColor() const
{
    return backgroundColor;
}

void  GLViewport::setBackgroundColor(QColor &color)
{
    backgroundColor = color;
    updateGL();
}

Material *GLViewport::getMaterial() const
{
    return material;
}

void GLViewport::setMaterial(Material *m)
{
    material = m;
    updateGL();
}

ObjectModel *GLViewport::getObjectModel() const
{
    return objectModel;
}

void GLViewport::setObjectModel(ObjectModel *om)
{
    objectModel = om;
    updateGL();
}

bool GLViewport::getWireframeVisibility() const
{
    return wireframeVisibility;
}

void GLViewport::setWireframeVisibility(bool v)
{
    wireframeVisibility = v;
    updateGL();
}

void GLViewport::increaseRotation(double x, double y)
{
    rotX += x;
    rotY += y;
    updateGL();
}

double GLViewport::getRotationX() const
{
    return rotX;
}

double GLViewport::getRotationY() const
{
    return rotY;
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
        updateGL();
    } else {
        if(e->buttons() & Qt::LeftButton)
        {
            rotX += 360.0 * static_cast<double>(deltas.x())/width();
            rotY += 360.0 * static_cast<double>(deltas.y())/height();
            updateGL();
        }
    }
    lastMousePos = e->pos();
}

void GLViewport::wheelEvent(QWheelEvent *e)
{
    zoom += double(e->delta())/120.0/20.0;
    if (zoom > 2.0) zoom = 2.0;
    if (zoom < 0.1) zoom = 0.1;
    updateGL();
}

void GLViewport::initializeGL()
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
}

void GLViewport::paintGL()
{
    qglClearColor(backgroundColor);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (objectModel != NULL) {

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        // And shift the object
        glTranslated(shiftX, shiftY, 0.0);

        // Then apply zoom
        glScaled(zoom, zoom, zoom);

        // Rotate first
        glRotated(rotX, 1.0, 0.0, 0.0);
        glRotated(rotY, 0.0, 1.0, 0.0);


        loadMaterial();

        glBegin(GL_TRIANGLES);

        for (int i = 0; i<objectModel->getTriangles().size(); i++) {
            GLfloat vertex[4];
            objectModel->getTriangles().at(i)->normal().getArray(vertex);
            glNormal3fv(vertex);
            objectModel->getTriangles().at(i)->a()->getArray(vertex);
            glVertex4fv(vertex);
            objectModel->getTriangles().at(i)->b()->getArray(vertex);
            glVertex4fv(vertex);
            objectModel->getTriangles().at(i)->c()->getArray(vertex);
            glVertex4fv(vertex);
        }

        glEnd();
    }
}

void GLViewport::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double hor = max(static_cast<double>(w)/static_cast<double>(h),1.0);
    double ver = max(static_cast<double>(h)/static_cast<double>(w),1.0);
    glOrtho(-hor, hor, -ver, ver, -2.0, 2.0);
}

void GLViewport::resetLights(std::vector<Light*> & lights)
{
    GLenum names[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
    qreal color[4];
    GLfloat colorF[4];
    GLfloat position[4];
    for (unsigned i=0; i<8; i++)
        glDisable(names[i]);
    if (!lights.empty()) {
        for (unsigned i=0; i<lights.size(); i++) {
            glEnable(names[i]);
            // Ambient
            lights.at(i)->getIA().getRgbF(&color[0], &color[1], &color[2], &color[3]);
            for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
            glLightfv(names[i], GL_AMBIENT, colorF);
            // Diffuse
            lights.at(i)->getID().getRgbF(&color[0], &color[1], &color[2], &color[3]);
            for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
            glLightfv(names[i], GL_DIFFUSE, colorF);
            // Specular
            lights.at(i)->getIS().getRgbF(&color[0], &color[1], &color[2], &color[3]);
            for (unsigned j=0; j<4; j++) colorF[j] = float(color[j]);
            glLightfv(names[i], GL_SPECULAR, colorF);
            // Position
            lights.at(i)->getPos().getArray(position);
            glLightfv(names[i], GL_POSITION, position);
        }
    }
    updateGL();
}

void GLViewport::loadMaterial() {
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
