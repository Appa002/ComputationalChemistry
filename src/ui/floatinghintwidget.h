#ifndef FLOATINGHINTWIDGET_H
#define FLOATINGHINTWIDGET_H

#include <QWidget>
#include <interfaces/iemitresize.h>

enum class Tips{
    RENDERWIDGET,
    MOLECULE_BUILDER_GENERAL,
    MOLECULE_BUILDER_BOND
};

namespace Ui {
class FloatingHintWidget;
}

class FloatingHintWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingHintWidget(QWidget *parent = nullptr, IEmitResize* target = nullptr);
    ~FloatingHintWidget();

    void changeTip(Tips tip);

private:
    Ui::FloatingHintWidget *ui;

protected:
    void resizeEvent(QResizeEvent* event);

public slots:
    void updatePosition(int w, int h);
};

#endif // FLOATINGHINTWIDGET_H
