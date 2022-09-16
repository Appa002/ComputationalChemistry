#include "lewisview.h"
#include <QDebug>
#include <iostream>
#include <QMouseEvent>
#include <QPushButton>
#include <QMessageBox>
#include <algorithm>
#include <QComboBox>
#include <common/data_elements.h>
#include <ui/floatinghintwidget.h>


LewisView::LewisView(QWidget *parent) : QWidget(parent)
{
    this->window()->findChild<QFrame*>("frame_torsion")->hide();
    this->window()->findChild<QFrame*>("frame_bond")->hide();

    // Fetch all Widgets that will be changed throughout execution...
    sideSymbol = this->window()->findChild<QLabel*>("side_symbol");
    sideElementName = this->window()->findChild<QComboBox*>("side_elementName");
    sideType = this->window()->findChild<QComboBox*>("side_type");
    sideX = this->window()->findChild<QLineEdit*>("side_x");
    sideY = this->window()->findChild<QLineEdit*>("side_y");
    sideZ = this->window()->findChild<QLineEdit*>("side_z");
    sideMass = this->window()->findChild<QLineEdit*>("side_mass");
    sideCharge = this->window()->findChild<QLineEdit*>("side_charge");
    sideBondRadius = this->window()->findChild<QLineEdit*>("side_bondRadius");
    sideNonebondDistance = this->window()->findChild<QLineEdit*>("side_nonebondDistance");
    sideChargeParam = this->window()->findChild<QLineEdit*>("side_chargeParam");
    sideValenzAngle = this->window()->findChild<QLineEdit*>("side_valenzAngle");

    sideBondOrder = this->window()->findChild<QLineEdit*>("side_bondOrder");

    sideAddChildBtn = this->window()->findChild<QPushButton*>("btn_addChild");
    sideDeleteBtn = this->window()->findChild<QPushButton*>("btn_delete");

    sideNTorsional = this->window()->findChild<QLineEdit*>("side_nTorsional");
    sideVTorsional = this->window()->findChild<QLineEdit*>("side_vTorsional");
    sideAngleTorsional = this->window()->findChild<QLineEdit*>("side_angleTorsional");

    fhw = new FloatingHintWidget(this, this);
    fhw->changeTip(Tips::MOLECULE_BUILDER_GENERAL);

    connect(sideAddChildBtn, &QPushButton::clicked, this, &LewisView::onAddChildPressed);
    connect(sideDeleteBtn, &QPushButton::clicked, this, &LewisView::onDeletePressed);

}

LewisView::~LewisView()
{
    delete molecule;
}

void LewisView::paint(QPainter *painter, QPaintEvent *event)
{
    const float spacing = 60.0f;
    float pi = 3.14159265359f;
    clickables.clear();

    std::function<void(Atom*, float, float, float)> dfs = [&](Atom* atom, float x, float y, float prevAngle) -> void{
        if(isElementSelected(atom))
            painter->setPen(QColor(0xff, 0x10, 0x53));

        QRectF rect = paintTxt(x, y, atom, painter, event);
        Shape shape(rect.topLeft(), rect.bottomLeft(), rect.bottomRight(), rect.topRight());
        clickables.emplace_back(shape, 10, atom, nullptr, std::bind(&LewisView::onElementClicked, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        //shape.draw(painter);

        painter->setPen(QColor(0x00, 0x00, 0x00));

        float dangle = pi / (atom->children.size());
        int i = 1;

        for(auto child : atom->children){
            float spacingMod = 20 * child->countSubTree(); // This increase complexity by a lot, consider if it is worth it....

            spacingMod = std::min(spacingMod, 100.0f);

            float newX = x + std::cos(atom->display_inversion * (-pi + dangle * i))* (spacing + spacingMod) * static_cast<float>(zoom);
            float newY = y + std::sin(atom->display_inversion * (-pi + dangle * i)) * (spacing + spacingMod) * static_cast<float>(zoom);

            if(isElementSelected(atom) && isElementSelected(child))
                 painter->setPen(QColor(0xff, 0x10, 0x53));

            painter->drawLine(x - camX, y - camY, newX - camX, newY - camY);

            painter->setPen(QColor(0x00, 0x00, 0x00));

            Shape lineShape(QPointF(x - 10*zoom - camX, y - 10*zoom - camY),
                            QPointF(x + 10*zoom - camX, y + 10*zoom - camY),
                            QPointF(newX + 10*zoom - camX, newY + 10*zoom - camY),
                            QPointF(newX - 10*zoom - camX, newY - 10*zoom - camY));

            if(isFullChain(atom, child))
                paintArrow((x + newX)/2.0f, ( y + newY) / 2.0f, painter, event);

            //lineShape.draw(painter);
            clickables.emplace_back(lineShape, 1, atom, child, std::bind(&LewisView::onBondClicked, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            dfs(child, newX, newY, prevAngle + dangle * i);
            ++i;
        }

    };

    dfs(molecule, width() / 2, height() / 2, -pi);

}

void LewisView::display(Atom *m)
{
    fhw->changeTip(Tips::MOLECULE_BUILDER_GENERAL);
    this->molecule = m;
    onElementClicked(nullptr, m, nullptr);
    update();
}

Atom *LewisView::getMolecule()
{
    return this->molecule;
}

void LewisView::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    this->paint(&painter, event);
    painter.end();
}

void LewisView::mousePressEvent(QMouseEvent *event)
{
    Clickable target;
    int highestPriority = -1;

    if(event->buttons() & Qt::LeftButton){
        for(auto const& clickable : clickables){
            if(clickable.shape.contains(event->pos())){
                if(clickable.priority > highestPriority && clickable.priority != -1){
                    target = clickable;
                    highestPriority = clickable.priority;
                }
            }
        }
        if(highestPriority != -1)
            target.handler(event, target.atom, target.other);

    }



    lastMousePos = event->pos();

    event->accept();
    update();
}

void LewisView::mouseMoveEvent(QMouseEvent *event)
{
   const float f = 0.7f;

   if(event->buttons() & Qt::RightButton){
       QPoint delta = event->pos() - lastMousePos;
       lastMousePos = event->pos();

       camX -= delta.x() * f;
       camY -= delta.y() * f;
   }

   event->accept();
   update();
}

void LewisView::wheelEvent(QWheelEvent *event)
{
    zoom += event->angleDelta().y() * -0.002f;
    zoom = std::clamp<double>(zoom, 0.28l, 2.2l);

    this->update();
    event->accept();

}

void LewisView::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size().width(), event->size().height());
    event->accept();

}

void LewisView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    fhw->updatePosition(0, 0);
}

bool LewisView::isElementSelected(Atom *atom)
{
    if(atom == currentlySelected)
        return true;


    if(atom == selectedBondOne || atom == selectedBondTwo)
        return true;

    if(!isFullChain() || forceSimpleBondDisplay == 1)
        return false;

    if(selectedBondOne != nullptr && atom == selectedBondOne->parent)
        return true;

    if(selectedBondTwo != nullptr &&
            selectedBondTwo->children.size() > torsionChildIdx && atom == selectedBondTwo->children.at(torsionChildIdx))
        return true;
    return false;
}

bool LewisView::isFullChain()
{
    return selectedBondOne != nullptr
            && selectedBondTwo != nullptr
            && selectedBondOne->parent != nullptr
            && !selectedBondTwo->children.empty();
}

bool LewisView::isFullChain(Atom *atom, Atom* other)
{
    return atom != nullptr
            && other != nullptr
            && atom->parent != nullptr
            && !other->children.empty();
}

void LewisView::paintArrow(double posX, double posY, QPainter *painter, QPaintEvent *)
{
    double s = 20 * zoom;
    //painter->drawRect(QRectF(posX - s / 2, posY-s/2, s, s));
    QRectF boundingRect;
    painter->drawText(QRectF(0, 0, 0, 0), Qt::AlignHCenter, QString("↻"), &boundingRect);

    double recX = posX - boundingRect.width() / 2.0;
    double recY = posY - boundingRect.height() / 2.0;

    recX -= camX;
    recY -= camY;

    QRectF rect(recX, recY, boundingRect.width(), boundingRect.height());
    painter->drawText(rect, Qt::AlignHCenter, QString("↻"), &boundingRect);
}

QRectF LewisView::paintTxt(double posX, double posY, Atom* atom, QPainter *painter, QPaintEvent *)
{
    QRectF boundingRect;
    double s = 20 * zoom;
    painter->setFont(QFont("Sans Serif", static_cast<int>(s)));
    painter->drawText(QRectF(0, 0, 0, 0), Qt::AlignHCenter, QString(atom->symbol.c_str()), &boundingRect); // this sometimes(after a wheel event) throws an exception but it gets dealt with internaly. Not to sure what's going on...

    float recX = posX - boundingRect.width() / 2;
    float recY = posY - boundingRect.height() / 2;

    recX -= camX;
    recY -= camY;

    QRectF rect(recX, recY, boundingRect.width(), boundingRect.height());

    painter->drawText(rect, Qt::AlignHCenter, QString(atom->symbol.c_str()), &boundingRect);

    // Drawing smaller charge symbol
    if(atom->display_chargeParam != "+0" && atom->display_chargeParam != "0"){
        painter->setFont(QFont("Sans Serif", static_cast<int>(s/2)));

        if(atom->display_chargeParam[0] != '+' && atom->display_chargeParam[0] != '-')
            atom->display_chargeParam = "+" + atom->display_chargeParam;

        painter->drawText(rect.x() + rect.width() - 5, rect.y() + 5, QString(atom->display_chargeParam.c_str()));
    }

    return rect;
}

void LewisView::onElementClicked(QMouseEvent *event, Atom *atom, Atom*)
{
    fhw->changeTip(Tips::MOLECULE_BUILDER_GENERAL);
    fhw->updatePosition(0, 0);

    const float pi = 3.14159265359f;

    //std::cout << atom->symbol << std::endl;
    if(this->currentlySelected == atom){
        // The user clicked an atom twice...
        atom->display_inversion *= -1.0f;
    }


    this->currentlySelected = atom;
    this->selectedBondOne = nullptr;
    this->selectedBondTwo = nullptr;

    // Display the ui for elements and hide for bond

    sideSymbol->setText(atom->symbol.c_str());
    this->window()->findChild<QFrame*>("frame_element")->show();
    this->window()->findChild<QFrame*>("frame_torsion")->hide();
    this->window()->findChild<QFrame*>("frame_bond")->hide();
    sideDeleteBtn->show();
    sideAddChildBtn->show();


    sideX->disconnect();
    sideY->disconnect();
    sideZ->disconnect();
    sideMass->disconnect();
    sideCharge->disconnect();
    sideBondRadius->disconnect();
    sideNonebondDistance->disconnect();

    sideElementName->disconnect();
    sideType->disconnect();
    sideChargeParam->disconnect();
    sideValenzAngle->disconnect();

    sideAngleTorsional->disconnect();
    sideNTorsional->disconnect();
    sideVTorsional->disconnect();

    sideBondOrder->disconnect();

    updateSideUi(atom);

    connect(sideX, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->x = sideInputToFloat(text);});
    connect(sideY, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->y = sideInputToFloat(text);});
    connect(sideZ, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->z = sideInputToFloat(text);});
    connect(sideMass, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->mass = sideInputToFloat(text);});
    connect(sideCharge, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->partialCharge = sideInputToFloat(text);});
    connect(sideBondRadius, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->bondRadius = sideInputToFloat(text);});
    connect(sideNonebondDistance, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->noneBondDistance = sideInputToFloat(text);});
    connect(sideValenzAngle, &QLineEdit::textEdited, this, [=](const QString& text) -> void {atom->valenzAngle = sideInputToFloat(text) * pi/180.0f;});

    connect(sideElementName, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
        [=](const QString &text){
            updateAtomToNewElementOrType(atom, text, QString(atom->display_type.c_str()), QString(atom->display_chargeParam.c_str()));
        });

    connect(sideType, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
        [=](const QString &text){
            updateAtomToNewElementOrType(atom, QString(atom->name.c_str()), text, QString(atom->display_chargeParam.c_str()));
        });

    connect(sideChargeParam, &QLineEdit::editingFinished, this, [=]() -> void {
        std::string text = sideChargeParam->text().toStdString();
        int i = 0;
        for(char c : text){
            if (! ((c >= 0x30 && c <= 0x39) || ( i== 0 && (c != '+' || c != '-')) ))
                return;
            ++i;
        }
        updateAtomToNewElementOrType(atom, QString(atom->name.c_str()), QString(atom->display_type.c_str()), QString(text.c_str()));
    });

}

void LewisView::onBondClicked(QMouseEvent *, Atom *atom, Atom* other)
{ 
    fhw->changeTip(Tips::MOLECULE_BUILDER_BOND);

    const float pi = 3.14159265359f;

    bool isTorsion;
    if(forceSimpleBondDisplay == 1)
        isTorsion = false;
    else
        isTorsion = isFullChain(atom, other);

   /* if(!isFullChain(atom, other)){
        selectedBondOne = nullptr;
        selectedBondTwo = nullptr;
        return;
    }*/

    if(selectedBondOne == atom && selectedBondTwo == other){
        torsionChildIdx++;
        if((isTorsion && torsionChildIdx >= selectedBondTwo->children.size()) || forceSimpleBondDisplay == 1){
            ++forceSimpleBondDisplay;
            torsionChildIdx = 0;
            if(forceSimpleBondDisplay == 1)
                isTorsion = false;
            else if (forceSimpleBondDisplay == 2){
                forceSimpleBondDisplay = 0;
                isTorsion = isFullChain(atom, other);
            }
        }

    } else{
        forceSimpleBondDisplay = 0;
        isTorsion = isFullChain(atom, other);
        torsionChildIdx = 0;
    }

    selectedBondOne = atom;
    selectedBondTwo = other;
    currentlySelected = nullptr;

    // Disconnect everything...
    sideX->disconnect();
    sideY->disconnect();
    sideZ->disconnect();
    sideMass->disconnect();
    sideCharge->disconnect();
    sideBondRadius->disconnect();
    sideNonebondDistance->disconnect();

    sideElementName->disconnect();
    sideType->disconnect();
    sideChargeParam->disconnect();
    sideValenzAngle->disconnect();

    sideAngleTorsional->disconnect();
    sideNTorsional->disconnect();
    sideVTorsional->disconnect();

    sideBondOrder->disconnect();


    //Hide ui for elements...
    this->window()->findChild<QFrame*>("frame_element")->hide();
    sideDeleteBtn->hide();
    sideAddChildBtn->hide();
    if(isTorsion)
        this->window()->findChild<QFrame*>("frame_bond")->hide();
    else
        this->window()->findChild<QFrame*>("frame_torsion")->hide();

    // Setup bond user interface

    if(isTorsion){
        sideSymbol->setText((atom->parent->symbol + "-" + atom->symbol + "-" + other->symbol + "-" + other->children[torsionChildIdx]->symbol ).c_str());
        this->window()->findChild<QFrame*>("frame_torsion")->show();
    } else {
        sideSymbol->setText((atom->symbol + "-" + other->symbol).c_str());
        this->window()->findChild<QFrame*>("frame_bond")->show();
    }

    updateSideUi(other);

    if(isTorsion){
        Atom* storage = other->children[torsionChildIdx]; // for convenience

        connect(sideAngleTorsional, &QLineEdit::textEdited, this, [=](const QString& text) -> void {storage->torsionalAngle = sideInputToFloat(text) * pi/180.0f;});
        connect(sideNTorsional, &QLineEdit::textEdited, this, [=](const QString& text) -> void {storage->torsionalN = sideInputToFloat(text);});
        connect(sideVTorsional, &QLineEdit::textEdited, this, [=](const QString& text) -> void {storage->torsionalV = sideInputToFloat(text);});
    } else {
        size_t idx = 0;
        while(atom->children[idx] != other)
            ++idx;

        sideBondOrder->setText(std::to_string(atom->bondOrders[idx]).c_str());

        connect(sideBondOrder, &QLineEdit::textEdited, this, [=](const QString& text) -> void {
            atom->bondOrders[idx] = sideInputToFloat(text);
        });

    }

}

float LewisView::sideInputToFloat(const QString &text)
{
    std::wstring t = text.toStdWString();
    try {
        return std::stof(t);
    } catch (std::invalid_argument const &) {
        return 0.0f;
    }
}

void LewisView::updateSideUi(Atom *atom)
{
    const float pi = 3.14159265359f;

    sideSymbol->setText(atom->symbol.c_str());
    sideElementName->setCurrentIndex(sideElementName->findText(atom->name.c_str()));
    sideType->setCurrentIndex(sideType->findText(atom->display_type.c_str()));
    sideX->setText(std::to_string(atom->x).c_str());
    sideY->setText(std::to_string(atom->y).c_str());
    sideZ->setText(std::to_string(atom->z).c_str());
    sideMass->setText(std::to_string(atom->mass).c_str());
    sideCharge->setText(std::to_string(atom->partialCharge).c_str());
    sideBondRadius->setText(std::to_string(atom->bondRadius).c_str());
    sideNonebondDistance->setText(std::to_string(atom->noneBondDistance).c_str());
    sideValenzAngle->setText(std::to_string(atom->valenzAngle * 180.0f/pi).c_str());

    if(atom->children.size() > torsionChildIdx){
        Atom* storage = atom->children[torsionChildIdx]; // for convenience
        sideAngleTorsional->setText(std::to_string(storage->torsionalAngle * 180.0f/pi).c_str());
        sideNTorsional->setText(std::to_string(storage->torsionalN).c_str());
        sideVTorsional->setText(std::to_string(storage->torsionalV).c_str());
    }



    if(atom->display_chargeParam[0] != '+' && atom->display_chargeParam[0] != '-')
        atom->display_chargeParam = "+" + atom->display_chargeParam;

    sideChargeParam->setText(atom->display_chargeParam.c_str());
}

void LewisView::updateAtomToNewElementOrType(Atom *atom, QString qelement, QString qtype, QString qcharge)
{
    const float pi = 3.14159265359f;
    std::string element = toupper(qelement.toStdString());
    std::string type = toupper(qtype.toStdString());
    std::string charge = qcharge.toStdString();

    if(charge[0] != '+' && charge[0] != '-')
        charge = "+" + charge;

    DATA_Element dataElement;

    try {
       dataElement = globalData.data.at(element).at(type).at(charge);
    } catch (std::out_of_range&) {
        try {
            // if we can't find one, try again but whith generic charge; if that one also fails then alert the user
            dataElement = globalData.data.at(element).at(type).begin()->second;
            charge = globalData.data.at(element).at(type).begin()->first; // set charge to whatever we have queried...
        } catch (std::out_of_range&) {
            QMessageBox msgBox;
            msgBox.setText("Can not find element with your selected type. If you belive your combination should exist, "
                           "you may still setup the parameters as you please - the type will just be displayed incorrectly.");
            msgBox.exec();
            dataElement = globalData.data.at(element).begin()->second.begin()->second; // fetch the element with generic type and charge
            charge = globalData.data.at(element).begin()->second.begin()->first; // set charge to whatever we have queried...

        }
    }

    atom->symbol = dataElement.symbol;
    atom->name = qelement.toStdString();
    atom->display_chargeParam = charge;


    atom->display_type = dataElement.formatedType;

    atom->mass = dataElement.mass;
    atom->bondRadius = dataElement.bondRadius;
    atom->valenzAngle = dataElement.valenzAngle;
    atom->noneBondDistance = dataElement.noneBondDistance;
    atom->partialCharge = dataElement.partialCharge;
    atom->valenzAngle = dataElement.valenzAngle * pi/180.0f;

    updateSideUi(atom);
    update();

}

std::string LewisView::toupper(std::string str)
{
    for(char& c : str)
        c = static_cast<char>(std::toupper(c));
    return str;
}

std::string LewisView::tolower(std::string str)
{
    for(char& c : str)
        c = static_cast<char>(std::tolower(c));
    return str;
}

void LewisView::onDeletePressed()
{
    Atom* toDelete = currentlySelected;

    if(molecule == toDelete){
        // Extra care needs be taken if toDelete is the root of our molecule
        if(toDelete->children.size() == 0){
            // Don't delete if there are no other children that can become the root
            QMessageBox msgBox;
            msgBox.setText("You may not delete the last remaining atom in a molecule.");
            msgBox.exec();
            return;
        }


        currentlySelected = toDelete->children[0]; // This is the new root
        this->molecule = currentlySelected;
        currentlySelected->parent = nullptr; // root has no parent

        for(int i = 1; i < toDelete->children.size(); i++){
            // loop through remaining children and set their parent to be the new root...
            toDelete->children[i]->parent = currentlySelected;
            // ... and add them to the new root's children vector
            currentlySelected->children.push_back(toDelete->children[i]);
            // ... and adding bond order term
            currentlySelected->bondOrders.push_back(toDelete->bondOrders[i]);
        }

        delete toDelete;
        onElementClicked(nullptr, currentlySelected, nullptr);
        update();
        return;
    }


    // removing toDelete from the children of its parent
    auto itr = toDelete->parent->children.begin();
    int idxForBondOrder = 0;
    while(itr != toDelete->parent->children.end() && *itr != toDelete){
        ++itr;
        ++idxForBondOrder;
    }
    toDelete->parent->children.erase(itr);
    // .. removing bond order info from parent
    toDelete->parent->bondOrders.erase(toDelete->parent->bondOrders.begin() + idxForBondOrder);


    // moving all children of toDelete to the parent of toDelete
    idxForBondOrder = 0;
    for(auto const& child : toDelete->children){
        toDelete->parent->children.push_back(child);
        child->parent = toDelete->parent;

        toDelete->parent->bondOrders.push_back(toDelete->bondOrders[idxForBondOrder]); // also adding to bond order

        ++idxForBondOrder;
    }

    // selecting the parent
    currentlySelected = toDelete->parent;
    delete toDelete;
    onElementClicked(nullptr, currentlySelected, nullptr);
    //cause redraw
    update();

}

void LewisView::onAddChildPressed()
{
    Atom* a = new Atom;
    a->partialCharge = 2.3f;
    a->bondRadius = 0.658f;
    a->noneBondDistance = 3.5f;
    a->bondOrders = {};
    a->valenzAngle = 1.8240436f;


    a->torsionalN = 0;
    a->torsionalV = 0.0f;
    a->torsionalAngle = 0.0f;

    a->symbol = "O";
    a->name = "Oxygen";
    a->mass = 15.999f;

    currentlySelected->children.push_back(a);
    currentlySelected->bondOrders.push_back(1.0f);
    a->parent = currentlySelected;

    update();
}



//--------------------------------------------------------------------





Shape::Shape(QPointF arg1, QPointF arg2, QPointF arg3, QPointF arg4)
    : p1(arg1), p2(arg2), p3(arg3), p4(arg4)
{

}

bool Shape::contains(QPoint point) const
{
    // See: https://math.stackexchange.com/questions/190111/how-to-check-if-a-point-is-inside-a-rectangle

    // Compute area of the shape
    float AShape = areaOfTriangle(p1, p2, p4) + areaOfTriangle(p2, p3, p4);

    // Compute area of trangles to point (read A1P4 as "area of the triangle made by point 1, the given point, and point 4")
    float A1P4 = areaOfTriangle(p1, point, p4);
    float A4P3 = areaOfTriangle(p4, point, p3);
    float A3P2 = areaOfTriangle(p3, point, p2);
    float AP21 = areaOfTriangle(point, p2, p1);

    float sum = A1P4 + A4P3 + A3P2 + AP21;

    return nearlyEqual(AShape, sum, 0.001f);
}

float Shape::areaOfTriangle(QPointF a, QPointF b, QPointF c) const
{
    float r = a.x()*(b.y()-c.y()) + b.x()*(c.y()-a.y()) + c.x()*(a.y()-b.y());
    return std::abs(r/2.0f);
}

bool Shape::nearlyEqual(float a, float b, float margin) const
{
    float absA = std::abs(a);
    float absB = std::abs(b);
    float diff = std::abs(a - b);

    if (a == b)
        return true;
    else if (a == 0.0f || b == 0.0f || (absA + absB < std::numeric_limits<float>::min()))
        return diff < (std::numeric_limits<float>::min() * margin);
    else
        return diff / (absA + absB) < margin;
}

void Shape::draw(QPainter *painter) const
{
    painter->drawLine(p1, p2);
    painter->drawLine(p2, p3);
    painter->drawLine(p3, p4);
    painter->drawLine(p4, p1);
}











