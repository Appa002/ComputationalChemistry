#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <simulation/scene.h>
#include <QWidget>
#include <QOpenGLWidget>
#include <QThread>
#include <mutex>
#include <QWheelEvent>
#include <interfaces/iemitresize.h>


class RenderWidget : public QOpenGLWidget, public IEmitResize
{
    Q_OBJECT
    Q_INTERFACES(IEmitResize)

public:
    explicit RenderWidget(QWidget *parent = nullptr);
    ~RenderWidget() override;

    void loadNewMolecule(Atom* molecule);
    void dropMolecule();

protected:

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent*) override;
    void focusInEvent(QFocusEvent*) override;


private:
    std::thread m_renderThread;
    Scene* m_scene;
    std::mutex* m_renderThreadMutex;
    bool m_closeRenderThread = false;
    bool m_loaded= false;

    QPoint m_lastMousePos;

signals:
    void resized(int w, int h) override;

public slots:
    void onSimulate();
    void onStep();
};

#endif // RENDERWIDGET_H
