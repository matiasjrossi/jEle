#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QFileDialog>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>
#include <QShowEvent>
#include <QToolBar>

#include "objectmodel.h"
#include "surreader.h"
#include "objreader.h"
#include "material.h"
#include "light.h"
#include "glviewport.h"

#define FPS 50

namespace jEle {

    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        vp(new GLViewport(this)),
        timer(new QTimer(this)), // used to tick the auto-rotate animation
        background(new QPushButton(this)),
        objectMaterial(new Material(QColor(135,160,180), QColor(255,255,255), QColor(170, 200, 225), QColor(0,0,0), 20.0f)),
        isAnimated(false)
    {
        ui->setupUi(this);

    #ifdef Q_WS_MAC
        QString macMod = QString::fromUtf8("\u2318") + QString(" +");
        ui->labelCtrl1->setText(macMod);
        ui->labelCtrl2->setText(macMod);
    #endif

        setCentralWidget(vp);

        ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
        ui->actionAnimation->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

        QToolBar *toolBar = new QToolBar();
        toolBar->addAction(ui->actionOpen);
        toolBar->addAction(ui->actionAnimation);

        ui->dockWidgetContents->layout()->addWidget(toolBar);

        // Background button setup
        background->setText("Background");
        QWidget *stretch = new QWidget(toolBar);
        stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        toolBar->addWidget(stretch);
        toolBar->addWidget(background);
        connect(background, SIGNAL(released()), this, SLOT(changeBackgroundColor()));

        //Right pane setup
        ui->dockWidget->setTitleBarWidget(new QWidget(this));

        //Object tab
        ui->objectAmbientButton->setPalette(QPalette(objectMaterial->getKA()));
        ui->objectAmbientButton->setText(objectMaterial->getKA().name());
        ui->objectDiffuseButton->setPalette(QPalette(objectMaterial->getKD()));
        ui->objectDiffuseButton->setText(objectMaterial->getKD().name());
        ui->objectSpecularButton->setPalette(QPalette(objectMaterial->getKS()));
        ui->objectSpecularButton->setText(objectMaterial->getKS().name());
        ui->objectEmissionButton->setPalette(QPalette(objectMaterial->getKE()));
        ui->objectEmissionButton->setText(objectMaterial->getKE().name());
        ui->objectShininessSpinbox->setValue(objectMaterial->getQ());
        connect(ui->objectAmbientButton, SIGNAL(released()), this, SLOT(changeObjectAmbient()));
        connect(ui->objectDiffuseButton, SIGNAL(released()), this, SLOT(changeObjectDiffuse()));
        connect(ui->objectSpecularButton, SIGNAL(released()), this, SLOT(changeObjectSpecular()));
        connect(ui->objectEmissionButton, SIGNAL(released()), this, SLOT(changeObjectEmission()));
        connect(ui->objectShininessSpinbox, SIGNAL(valueChanged(double)), this, SLOT(changeObjectShininess(double)));

        //Lights tab
        connect(ui->lightsAddButton, SIGNAL(released()), this, SLOT(addLight()));
        connect(ui->lightsDeleteButton, SIGNAL(released()), this, SLOT(deleteSelectedLight()));
        connect(ui->lightsListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateLightButtons()));
        connect(ui->lightsDiffuseButton, SIGNAL(released()), this, SLOT(changeLightDiffuse()));
        connect(ui->lightsSpecularButton, SIGNAL(released()), this, SLOT(changeLightSpecular()));
        connect(ui->lightsAmbientButton, SIGNAL(released()), this, SLOT(changeLightAmbient()));
        connect(ui->lightsXSpinbox, SIGNAL(valueChanged(double)), this, SLOT(changeLightPosition()));
        connect(ui->lightsYSpinbox, SIGNAL(valueChanged(double)), this, SLOT(changeLightPosition()));
        connect(ui->lightsZSpinbox, SIGNAL(valueChanged(double)), this, SLOT(changeLightPosition()));
        addLight();
        lightsContext.at(0)->setID(QColor("#e6e0c0"));
        lightsContext.at(0)->setPos(QVector3D(0.5, 1.0, 1.0));
        updateLightButtons();
        vp->setLights(lightsContext);
        updateLightButtons();

        vp->setDefaultMaterial(objectMaterial);

        timer->setInterval(1000/FPS);
        connect(timer, SIGNAL(timeout()), this, SLOT(autoRotate()));

        on_actionOpen_triggered(); // Load dialog at create
        ui->actionAnimation->toggle();
    }

    MainWindow::~MainWindow()
    {
        delete ui;
    }

    void MainWindow::changeEvent(QEvent *e)
    {
        QMainWindow::changeEvent(e);
        switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
        }
    }

    void MainWindow::on_actionOpen_triggered()
    {
        bool wasAnimated = isAnimated;
        if (wasAnimated)
            ui->actionAnimation->toggle();
        QString filepath = QFileDialog::getOpenFileName(this, QString("Select the surface to load"), NULL, "All supported formats (*.sur *.SUR *.obj *.OBJ); Surfaces (*.sur *.SUR); Wavefront OBJ (*.obj *.OBJ");
        if (filepath != NULL) {
            ObjectModel *_new = NULL;
            bool mod_material;
            if (filepath.endsWith(".sur", Qt::CaseInsensitive)) {
                _new = SURReader::openSUR(filepath);
                mod_material = true;
            } else if (filepath.endsWith(".obj", Qt::CaseInsensitive)) {
                _new = OBJReader::openOBJ(filepath);
                mod_material = false;
            }

            if (_new != NULL) {
                ObjectModel *_old = vp->getObjectModel();
                vp->setObjectModel(_new);
                if (_old != NULL)
                    delete _old;
                ui->objectTab->setEnabled(mod_material);
            }
        }
        if (wasAnimated)
            ui->actionAnimation->toggle();
        else
            vp->updateGL();
    }

    void MainWindow::on_actionAnimation_toggled(bool s)
    {
        if (s)
        {
            timer->start();
        } else {
            timer->stop();
        }
        isAnimated = s;
    }

    void MainWindow::autoRotate()
    {
        vp->increaseRotation(360.0/(FPS*4), 360.0/(FPS*40));
    }

    void MainWindow::addLight()
    {
        Light *l = new Light();
        lightsContext.push_back(l);
        ui->lightsListWidget->setCurrentItem(new QListWidgetItem(vector2String(l->getPos()), ui->lightsListWidget));
        vp->setLights(lightsContext);
    }

    void MainWindow::deleteSelectedLight()
    {
        unsigned pos = ui->lightsListWidget->currentRow();
        delete (ui->lightsListWidget->takeItem(pos)); // This fires an event that calls updateLightsButtons().
        lightsContext.erase(lightsContext.begin() + pos); //Don't remove the light before that event is processed!
        ui->lightsAddButton->setEnabled(ui->lightsListWidget->count() < 8);
        vp->setLights(lightsContext);
    }

    QString MainWindow::vector2String(QVector3D v)
    {
        return QString("(%1, %2, %3)").arg(v.x()).arg(v.y()).arg(v.z());
    }


    void MainWindow::updateLightButtons()
    {
        ui->lightsDiffuseButton->blockSignals(true);
        ui->lightsSpecularButton->blockSignals(true);
        ui->lightsAmbientButton->blockSignals(true);
        ui->lightsXSpinbox->blockSignals(true);
        ui->lightsYSpinbox->blockSignals(true);
        ui->lightsZSpinbox->blockSignals(true);
        int pos = ui->lightsListWidget->currentRow();
        if (pos == -1) {
            ui->lightsDiffuseButton->setPalette(QPalette(Qt::gray));
            ui->lightsDiffuseButton->setText("");
            ui->lightsSpecularButton->setPalette(QPalette(Qt::gray));
            ui->lightsSpecularButton->setText("");
            ui->lightsAmbientButton->setPalette(QPalette(Qt::gray));
            ui->lightsAmbientButton->setText("");
            //Vector
            ui->lightsXSpinbox->setValue(0.0);
            ui->lightsYSpinbox->setValue(0.0);
            ui->lightsZSpinbox->setValue(0.0);
        } else {
            Light l = lightsContext.at(pos);
            //Intensities
            ui->lightsDiffuseButton->setPalette(QPalette(l.getID()));
            ui->lightsDiffuseButton->setText(l.getID().name());
            ui->lightsSpecularButton->setPalette(QPalette(l.getIS()));
            ui->lightsSpecularButton->setText(l.getIS().name());
            ui->lightsAmbientButton->setPalette(QPalette(l.getIA()));
            ui->lightsAmbientButton->setText(l.getIA().name());
            //Vector
            ui->lightsXSpinbox->setValue(l.getPos().x());
            ui->lightsYSpinbox->setValue(l.getPos().y());
            ui->lightsZSpinbox->setValue(l.getPos().z());
        }
        bool valid = (pos != -1);
        ui->lightsDiffuseButton->setEnabled(valid);
        ui->lightsSpecularButton->setEnabled(valid);
        ui->lightsAmbientButton->setEnabled(valid);
        ui->lightsXSpinbox->setEnabled(valid);
        ui->lightsYSpinbox->setEnabled(valid);
        ui->lightsZSpinbox->setEnabled(valid);
        ui->lightsDeleteButton->setEnabled(valid);
        ui->lightsDiffuseButton->blockSignals(false);
        ui->lightsSpecularButton->blockSignals(false);
        ui->lightsAmbientButton->blockSignals(false);
        ui->lightsXSpinbox->blockSignals(false);
        ui->lightsYSpinbox->blockSignals(false);
        ui->lightsZSpinbox->blockSignals(false);
    }

    /*
     * Handler for color change
     */
    void MainWindow::changeBackgroundColor()
    {
        QColor color;
        if((color = QColorDialog::getColor(vp->getBackgroundColor(), this)).isValid())
        {
            vp->setBackgroundColor(color);
        }
    }

    void MainWindow::changeObjectAmbient()
    {
        QColor color;
        if((color = QColorDialog::getColor(objectMaterial->getKA(), this)).isValid())
        {
            objectMaterial->setKA(color);
            ui->objectAmbientButton->setPalette(QPalette(color));
            ui->objectAmbientButton->setText(color.name());
            vp->updateGL();
        }
    }

    void MainWindow::changeObjectDiffuse()
    {
        QColor color;
        if((color = QColorDialog::getColor(objectMaterial->getKD(), this)).isValid())
        {
            objectMaterial->setKD(color);
            ui->objectDiffuseButton->setPalette(QPalette(color));
            ui->objectDiffuseButton->setText(color.name());
            vp->updateGL();
        }
    }

    void MainWindow::changeObjectSpecular()
    {
        QColor color;
        if((color = QColorDialog::getColor(objectMaterial->getKS(), this)).isValid())
        {
            objectMaterial->setKS(color);
            ui->objectSpecularButton->setPalette(QPalette(color));
            ui->objectSpecularButton->setText(color.name());
            vp->updateGL();
        }
    }

    void MainWindow::changeObjectEmission()
    {
        QColor color;
        if((color = QColorDialog::getColor(objectMaterial->getKE(), this)).isValid())
        {
            objectMaterial->setKE(color);
            ui->objectEmissionButton->setPalette(QPalette(color));
            ui->objectEmissionButton->setText(color.name());
            vp->updateGL();
        }
    }

    void MainWindow::changeObjectShininess(double d)
    {
        objectMaterial->setQ(d);
        vp->updateGL();
    }

    void MainWindow::changeLightAmbient()
    {
        int pos = ui->lightsListWidget->currentRow();
        if (pos != -1) {
            Light *l = lightsContext.at(pos);
            QColor color;
            if((color = QColorDialog::getColor(l->getIA(), this)).isValid())
            {
                l->setIA(color);
                ui->lightsAmbientButton->setPalette(QPalette(color));
                ui->lightsAmbientButton->setText(color.name());
                vp->setLights(lightsContext);
            }
        }
    }

    void MainWindow::changeLightDiffuse()
    {
        int pos = ui->lightsListWidget->currentRow();
        if (pos != -1) {
            Light *l = lightsContext.at(pos);
            QColor color;
            if((color = QColorDialog::getColor(l->getID(), this)).isValid())
            {
                l->setID(color);
                ui->lightsDiffuseButton->setPalette(QPalette(color));
                ui->lightsDiffuseButton->setText(color.name());
                vp->setLights(lightsContext);
            }
        }
    }

    void MainWindow::changeLightSpecular()
    {
        int pos = ui->lightsListWidget->currentRow();
        if (pos != -1) {
            Light *l = lightsContext.at(pos);
            QColor color;
            if((color = QColorDialog::getColor(l->getIS(), this)).isValid())
            {
                l->setIS(color);
                ui->lightsSpecularButton->setPalette(QPalette(color));
                ui->lightsSpecularButton->setText(color.name());
                vp->setLights(lightsContext);
            }
        }
    }

    void MainWindow::changeLightPosition()
    {
        int pos = ui->lightsListWidget->currentRow();
        if (pos != -1) {
            Light *l = lightsContext.at(pos);
            l->setPos(QVector3D(ui->lightsXSpinbox->value(), ui->lightsYSpinbox->value(), ui->lightsZSpinbox->value()));
            ui->lightsListWidget->currentItem()->setText(vector2String(l->getPos()));
            vp->setLights(lightsContext);
        }
    }
}
