#include "floatinghintwidget.h"
#include "ui_floatinghintwidget.h"
#include <iostream>
#include <ui/renderwidget.h>
#include <interfaces/iemitresize.h>

FloatingHintWidget::FloatingHintWidget(QWidget *parent, IEmitResize* target) :
    QWidget(parent),
    ui(new Ui::FloatingHintWidget)
{
    ui->setupUi(this);
    if(parent && dynamic_cast <IEmitResize*>(target) && dynamic_cast <QObject*>(target) )
        // We need the parent to implement the interface IEmitResize.
        // The reason for this is that QWidget does not have any signals that get emitted when a widget gets resized, so the only way is to override
        // QWidget::resizeEvent (or QOpenGLWidget::resizeGl) and emit your own signal.

        connect(dynamic_cast<QObject*>(target), SIGNAL(resized(int, int)), this, SLOT(updatePosition(int ,int)));
    else
        throw std::runtime_error("Target must be castable to IEmitResize and QObject");

    this->changeTip(Tips::RENDERWIDGET);
    this->move(QPoint(parent->width() - this->width(), 0));
}

FloatingHintWidget::~FloatingHintWidget()
{
    delete ui;
}

void FloatingHintWidget::changeTip(Tips tip)
{
    switch(tip){
        case Tips::RENDERWIDGET:
            ui->frame_render->show();
            ui->frame_mbond->hide();
            ui->frame_mgeneral->hide();
            break;
        case Tips::MOLECULE_BUILDER_BOND:
            ui->frame_render->hide();
            ui->frame_mbond->show();
            ui->frame_mgeneral->hide();
            break;
        case Tips::MOLECULE_BUILDER_GENERAL:
            ui->frame_render->hide();
            ui->frame_mbond->hide();
            ui->frame_mgeneral->show();
            break;
    }

    this->move(QPoint(parentWidget()->width() - this->width(), 0));
}

void FloatingHintWidget::resizeEvent(QResizeEvent *event)
{
    this->move(QPoint(parentWidget()->width() - event->size().width(), 0));

}

void FloatingHintWidget::updatePosition(int w, int h)
{
    auto* parent = this->parentWidget();


    this->move(QPoint(parent->width() - this->width(), 0));

}

