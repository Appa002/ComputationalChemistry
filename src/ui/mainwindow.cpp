#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QInputDialog>
#include <common/fileloading.h>
#include <ui/licenses.h>
#include <set>
#include <QRandomGenerator>
#include <ui/floatinghintwidget.h>
#include <QLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    md = new MoleculeDialog();

    QSizePolicy sp_retain = ui->verticalSpaceHolder->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->verticalSpaceHolder->setSizePolicy(sp_retain);
    ui->verticalSpaceHolder->hide();

    auto fhw = new FloatingHintWidget(ui->renderWidget, ui->renderWidget);
    fhw->changeTip(Tips::RENDERWIDGET);
   // fhw->setGeometry(QRect(0, 0, 0, 0));

    auto runButton = ui->centralwidget->findChild<QPushButton*>("runButton");
    auto stepButton = ui->centralwidget->findChild<QPushButton*>("stepButton");
    auto moleculeButton = ui->centralwidget->findChild<QPushButton*>("moleculeButton");
    auto renderWidget = ui->centralwidget->findChild<RenderWidget*>("renderWidget");



    connect(runButton, &QPushButton::pressed, renderWidget, &RenderWidget::onSimulate);
    connect(stepButton, &QPushButton::pressed, renderWidget, &RenderWidget::onStep);
    connect(moleculeButton, &QPushButton::pressed, this, &MainWindow::moleculeButtonSlot);

    connect(ui->action_license, &QAction::triggered, this, [this](){ auto l = Licenses(this); l.exec();});


    connect(md, &MoleculeDialog::moleculeChanged, this, &MainWindow::onMoleculeChanged);

    infoFrame = ui->centralwidget->findChild<QWidget*>("info_frame");
}

MainWindow::~MainWindow()
{
    delete ui;
}

std::unordered_map<std::string, std::vector<float> > MainWindow::colourMapToStd(const std::vector<std::pair<QColor, std::string> > &it)
{
    std::unordered_map<std::string, std::vector<float> > out;
    for(auto const & pair : it){
        out[pair.second] = std::vector<float>({pair.first.red() / 255.0f, pair.first.green() / 255.0f, pair.first.blue() / 255.0f});
    }

    return out;
}

std::vector<std::pair<QColor, std::string> > MainWindow::createColourMap(Atom *atom)
{
    std::set<std::string> set;
    std::vector<std::pair<QColor, std::string>> out;

    std::function<void(Atom*)> dfs = [&](Atom* atom) -> void{
        if(set.find(atom->name) == set.end()){
            set.emplace(atom->name);
            out.emplace_back(randomColour(), atom->name);
        }

        for(auto const & child : atom->children)
            dfs(child);

    };

    dfs(atom);
    return out;

}

QColor MainWindow::randomColour()
{
    // To generate a random colour we use the HSV colour space, see: https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/

    static float h = 0.0f; // not really random as we start h with 0, but that is not significant
    h += 0.618033988749895f; // conjugate of golden ration
    h = std::fmod(h, 1);
    return hsvToRgb(360 * h, 1.0f, 0.89f);
}

QColor MainWindow::hsvToRgb(float H, float S, float V)
{
    // See 'HSV to RGB alternative' https://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB

    std::function<float(float, float, float)> min3 = [](float a, float b, float c) -> float{ // convenience
        return std::min(std::min(a,b), c);
    };

    std::function<float(int)> f = [&](int n) -> float {
        float k = static_cast<int>(n + H/60.0f)%6;
        return (V - V*S*std::max(0.0f, min3(k, 4 - k, 1))) * 255.0f;
    };

    return QColor(f(5), f(3), f(1));
}


void MainWindow::moleculeButtonSlot()
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

    md->disconnect(SIGNAL(MoleculeDialog::reject));
    connect(md, &MoleculeDialog::reject, this, [=](){delete a;}); // new capture for new atom pointer

    md->display(a);
}

void MainWindow::onMoleculeChanged(Atom *molecule)
{
    md->disconnect(SIGNAL(MoleculeDialog::reject));


    auto renderWidget = ui->centralwidget->findChild<RenderWidget*>("renderWidget");


    if(moleculeNameMap.find(molecule) == moleculeNameMap.end()){
        bool ok;
        QString qName;
        if(molecule->display_moleculeName.empty())
            qName = QInputDialog::getText(this, "Molecule Name", "Please enter the molecule's new name", QLineEdit::Normal, QString(), &ok);
        else{
            ok = true;
            qName = molecule->display_moleculeName.c_str();
        }

        if (ok && !qName.isEmpty())
               moleculeNameMap[molecule] = qName.toStdString();
        else
            moleculeNameMap[molecule] = "Unnamed";

        QVBoxLayout* layout = static_cast<QVBoxLayout*>(infoFrame->layout());

        auto colourMap = createColourMap(molecule);
        molecule->display_colourMap = colourMapToStd(colourMap);

        MoleculeDetails* molecularDetails = new MoleculeDetails(infoFrame,  moleculeNameMap[molecule], colourMap);

        layout->insertWidget(0, molecularDetails);
        /*if(layout->count() != 3){
            auto line = new QFrame(infoFrame);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            layout->insertWidget(1, line);
        }*/

        connect(molecularDetails, &MoleculeDetails::onEdit, this, [=](){md->display(molecule);});
        connect(molecularDetails, &MoleculeDetails::onDisplay, this, [=](){renderWidget->loadNewMolecule(molecule);});
        connect(molecularDetails, &MoleculeDetails::onDelete, this, [=](){
            moleculeNameMap.erase(molecule);
            delete molecule;
            molecularDetails->hide();
            molecularDetails->deleteLater();
            renderWidget->dropMolecule();

        });

        renderWidget->loadNewMolecule(molecule);

    } else {
        renderWidget->loadNewMolecule(molecule);
    }


}




