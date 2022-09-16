#include "licenses.h"
#include "ui_licenses.h"

Licenses::Licenses(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Licenses)
{
    ui->setupUi(this);
}

Licenses::~Licenses()
{
    delete ui;
}
