#include "glviewport.h"
#include "light.h"
#include "material.h"
#include <QDebug>

GLViewport::GLViewport(QWidget *parent) :
    QGLWidget(parent),
    backgroundColor(QColor(0,25,40)),
    material(NULL),
    objectModel(NULL),
    rotX(0.0),
    rotY(0.0),
    wireframeVisibility(false)
{
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

void GLViewport::changeRotation(double x, double y)
{
    rotX = x;
    rotY = y;
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

        // Rotate
        glRotated(rotX, 1.0, 0.0, 0.0);
        glRotated(rotY, 0.0, 1.0, 0.0);

        loadMaterial();

        glBegin(GL_TRIANGLES);

        for (unsigned i = 0; i<objectModel->getTriangles().size(); i++) {
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
    glMatrixMode(GL_MODELVIEW);
}

void GLViewport::resetLights(std::vector<Light*> & lights)
{
    GLenum names[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
    qreal color[4];
    GLfloat colorF[4];
    GLfloat position[4];
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
