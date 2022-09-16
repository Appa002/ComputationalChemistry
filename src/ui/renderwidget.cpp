#include "renderwidget.h"

#include <fstream>
#include <sstream>
#include <QDebug>
#include <QOpenGLContext>
#include <QFrame>
#include <QPushButton>

RenderWidget::RenderWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    m_scene = new Scene;
    m_renderThreadMutex = new std::mutex;

    auto frame = this->window()->findChild<QFrame*>("frame_styleRenderWidget");
    QColor defaultColour = palette().color(QPalette::Window);
    frame->setStyleSheet("border: 2px solid " + defaultColour.name());
}

RenderWidget::~RenderWidget()
{
    m_closeRenderThread = true;
    m_renderThread.join();

    delete m_renderThreadMutex;
    delete m_scene;
}

void RenderWidget::loadNewMolecule(Atom *molecule)
{
    m_scene->loadMolecule(molecule);
}

void RenderWidget::dropMolecule()
{
    m_scene->dropMolecule();
}

void RenderWidget::initializeGL()
{
    try {
        m_scene->initializeGL();
    } catch (OpenGLError& err) {
        qFatal(err.what());
    }


    m_renderThread = std::thread([this](){
        auto t0 = std::chrono::high_resolution_clock::now();
        auto t1 = std::chrono::high_resolution_clock::now();
        auto myContext = new QOpenGLContext();
        myContext->create();

        myContext->makeCurrent(this->context()->surface());

        while(!m_closeRenderThread){


            myContext->makeCurrent(this->context()->surface());

            try {
                t1 = std::chrono::high_resolution_clock::now();
                m_scene->update((t1 - t0).count() * 1e-9);
            } catch (OpenGLError& err) {
                qFatal(err.what());
            }


            myContext->makeCurrent(this->context()->surface());
            myContext->swapBuffers(this->context()->surface());
            this->update();

            t0 = t1;
        }
        delete myContext;
    });

    m_loaded = true;
}

void RenderWidget::resizeGL(int w, int h)
{
    emit resized(w, h);
    m_renderThreadMutex->lock();
    try {
        m_scene->resizeGL(w, h);
    } catch (OpenGLError& err) {
        qFatal(err.what());
    }
    m_renderThreadMutex->unlock();
}

void RenderWidget::paintGL()
{
   m_renderThreadMutex->lock();
    try {
        m_scene->paintGL();
    } catch (OpenGLError& err) {
        qFatal(err.what());
    }
   m_renderThreadMutex->unlock();
}

void RenderWidget::wheelEvent(QWheelEvent *event)
{
    m_renderThreadMutex->lock();
     try {
         m_scene->wheelEvent(event->angleDelta().y());
     } catch (OpenGLError& err) {
         qFatal(err.what());
     }
    m_renderThreadMutex->unlock();

    event->accept();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        QPoint const delta = m_lastMousePos - event->pos();
        m_lastMousePos = event->pos();

        m_renderThreadMutex->lock();
         try {
             m_scene->mouseLeftDrag(delta.x(), delta.y());
         } catch (OpenGLError& err) {
             qFatal(err.what());
         }
        m_renderThreadMutex->unlock();
    } else if (event->buttons() & Qt::RightButton){
        QPoint const delta = m_lastMousePos - event->pos();
        m_lastMousePos = event->pos();

        m_renderThreadMutex->lock();
         try {
             m_scene->mouseRightDrag(delta.x(), delta.y());
         } catch (OpenGLError& err) {
             qFatal(err.what());
         }
        m_renderThreadMutex->unlock();
    }

    event->accept();
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    this->setFocus(Qt::FocusReason::MouseFocusReason);


    m_lastMousePos = event->pos();
    event->accept();
}

void RenderWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_R)
        m_scene->resetCamera();
    event->accept();
}

void RenderWidget::focusOutEvent(QFocusEvent *)
{
    QColor defaultColour = palette().color(QPalette::Window);

    auto frame = this->window()->findChild<QFrame*>("frame_styleRenderWidget");
    if(!m_scene->m_temprun)
        frame->setStyleSheet("border: 2px solid " + defaultColour.name());
    else
        frame->setStyleSheet("border: 2px solid #ff7878;");
}

void RenderWidget::focusInEvent(QFocusEvent *)
{
    QColor defaultColour = palette().color(QPalette::Window);

    auto frame = this->window()->findChild<QFrame*>("frame_styleRenderWidget");
    if(!m_scene->m_temprun)
        frame->setStyleSheet("border: 2px solid #00a1ad;");
    else
        frame->setStyleSheet("border: 2px solid red;");
}


void RenderWidget::onSimulate()
{
    //if(m_loaded){
    //    m_sim.run();
    //}
    if(m_loaded){
        m_scene->m_temprun = !m_scene->m_temprun;
        auto btn = this->window()->findChild<QPushButton*>("runButton");

        if(m_scene->m_temprun){
            btn->setText("Stop");
        } else {
            btn->setText("Run");
        }

        this->setFocus(Qt::FocusReason::MouseFocusReason);
    }

}

void RenderWidget::onStep()
{
    if(m_loaded && !m_scene->m_temprun){
        m_scene->m_tempSingleStep = true;
        m_scene->m_temprun = true;
    }

}
