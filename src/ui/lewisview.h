#ifndef LEWISVIEW_H
#define LEWISVIEW_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <common/atom.h>
#include <vector>
#include <functional>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <common/data_elements.h>
#include <ui/floatinghintwidget.h>
#include <interfaces/iemitresize.h>

struct Shape{
    Shape() = default;
    Shape(QPointF p1, QPointF p2, QPointF p3, QPointF p4);

    QPointF p1;
    QPointF p2;
    QPointF p3;
    QPointF p4;

    bool contains (QPoint point) const;
    float areaOfTriangle(QPointF ap1, QPointF ap2, QPointF ap3) const;
    bool nearlyEqual (float a, float b, float margin) const;
    void draw(QPainter* painter) const;
};

struct Clickable{
    Clickable() = default;
    Clickable(Shape r, int p, Atom* a, Atom* o, std::function<void (QMouseEvent* event, Atom* atom, Atom* other)> h)
        : priority(p), shape(r), atom(a), other(o), handler(h) {}

    int priority = 0;
    Shape shape;
    Atom* atom;
    Atom* other;
    std::function<void (QMouseEvent* event, Atom* atom, Atom* other)> handler;
};

class LewisView : public QWidget, public IEmitResize
{
    Q_OBJECT
    Q_INTERFACES(IEmitResize)

public:
    explicit LewisView(QWidget *parent = nullptr);
    virtual ~LewisView() override;

    void paint(QPainter *painter, QPaintEvent *event);
    void display(Atom* m);
    Atom* getMolecule();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent* event) override;

private:
    bool isElementSelected(Atom* atom);
    bool isFullChain();
    bool isFullChain(Atom* atom, Atom* other);

    DATA_Elements globalData;
    QPoint lastMousePos;

    float camX = 0;
    float camY = 0;

    Atom* currentlySelected = nullptr;
    Atom* selectedBondOne = nullptr;
    Atom* selectedBondTwo = nullptr;
    int torsionChildIdx = 0;
    int forceSimpleBondDisplay = 0;

    std::vector<Clickable> clickables;

    double zoom = 1.0l;

    Atom* molecule;

    void paintArrow(double posX, double posY, QPainter *painter, QPaintEvent*);
    QRectF paintTxt(double posX, double posY, Atom* atom, QPainter *painter, QPaintEvent*);

    void onElementClicked(QMouseEvent* event, Atom* atom, Atom*);
    void onBondClicked(QMouseEvent* event, Atom* atom, Atom* other);
    float sideInputToFloat(QString const & text);

    void updateSideUi(Atom* atom);

    void updateAtomToNewElementOrType(Atom* atom,  QString element, QString type, QString charge);

    std::string toupper(std::string str);
    std::string tolower(std::string str);

    FloatingHintWidget* fhw;

    QPushButton* sideDeleteBtn;
    QPushButton* sideAddChildBtn;
    QLabel* sideSymbol;
    QComboBox* sideElementName;
    QComboBox* sideType;
    QLineEdit* sideX;
    QLineEdit* sideY;
    QLineEdit* sideZ;
    QLineEdit* sideMass;
    QLineEdit* sideCharge;
    QLineEdit* sideBondRadius;
    QLineEdit* sideNonebondDistance;
    QLineEdit* sideChargeParam;
    QLineEdit* sideValenzAngle;

    QLineEdit* sideNTorsional;
    QLineEdit* sideVTorsional;
    QLineEdit* sideAngleTorsional;

    QLineEdit* sideBondOrder;



signals:
    void resized(int, int) override;

private slots:
    void onDeletePressed();
    void onAddChildPressed();

};

#endif // LEWISVIEW_H
