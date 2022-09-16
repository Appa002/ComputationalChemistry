#include "moleculedetails.h"
#include "ui_moleculedetails.h"
#include <sstream>
#include <iostream>

MoleculeDetails::MoleculeDetails(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MoleculeDetails)
{
    ui->setupUi(this);

    moreLessBtn = this->findChild<QPushButton*>("more_less_btn");
    moleculeName = this->findChild<QLabel*>("molecule_name");
    coloursFrame = this->findChild<QFrame*>("colours_frame");
}

MoleculeDetails::MoleculeDetails(QWidget *parent, std::string moleculeNameStr, std::vector<std::pair<QColor, std::string>> colourPairs)
    :
        QFrame(parent),
        ui(new Ui::MoleculeDetails)
{
    ui->setupUi(this);

    moreLessBtn = this->findChild<QPushButton*>("more_less_btn");
    moleculeName = this->findChild<QLabel*>("molecule_name");
    coloursFrame = this->findChild<QFrame*>("colours_frame");
    coloursFrame->hide();

    coloursFrameLayout = static_cast<QFormLayout*>(coloursFrame->layout());

    auto editBtn = this->findChild<QPushButton*>("edit_btn");
    auto deleteBtn = this->findChild<QPushButton*>("delete_btn");
    auto displayBtn = this->findChild<QPushButton*>("display_btn");

    connect(editBtn, &QPushButton::pressed, this, [this](){emit onEdit(this->atomIdx);});
    connect(deleteBtn, &QPushButton::pressed, this, [this](){emit onDelete(this->atomIdx);});
    connect(displayBtn, &QPushButton::pressed, this, [this](){emit onDisplay(this->atomIdx);});

    connect(moreLessBtn, &QPushButton::pressed, this, [=](){
        expanded = !expanded;
        if(expanded){
            moreLessBtn->setText("Show Less");
            coloursFrame->show();
        }else{
            moreLessBtn->setText("Show More");
            coloursFrame->hide();
        }
    });


    moleculeName->setText(QString(moleculeNameStr.c_str()));

    // ■

    for(auto const & pair : colourPairs){
        std::stringstream html;
        html << R"(<span style="font-size:14pt; color:)";
        html << pair.first.name().toStdString();
        html << R"(;">■</span>)";

        coloursFrameLayout->addRow(new QLabel(html.str().c_str(), coloursFrame), new QLabel(pair.second.c_str(), coloursFrame));
    }




}

MoleculeDetails::~MoleculeDetails()
{
    delete ui;
}
