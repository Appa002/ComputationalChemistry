#include "moleculedialog.h"
#include <QKeyEvent>
#include <QDialogButtonBox>
#include <QMenuBar>
#include <QFileDialog>
#include <iostream>
#include <common/fileloading.h>
#include <QMessageBox>

MoleculeDialog::MoleculeDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    QMenuBar* menuBar = new QMenuBar(this); // menu bars can not be added in the qt designer for some reason :(
    QMenu* fileMenu = new QMenu("File", menuBar);

    QAction* loadAction = fileMenu->addAction("Load Molecule...");
    QAction* saveAction = fileMenu->addAction("Save Molecule...");

    menuBar->addMenu(fileMenu);

    QVBoxLayout* layout = static_cast<QVBoxLayout*>(this->layout());
    layout->setMargin(0);
    layout->insertWidget(0, menuBar);

    connect(loadAction, &QAction::triggered, this, &MoleculeDialog::handleLoadMolecule);
    connect(saveAction, &QAction::triggered, this, &MoleculeDialog::handleSaveMolecule);

    connect(ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::pressed, this,
            [&]()->void{
                if(ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->underMouse()){
                    this->accept();
                    emit moleculeChanged(ui.lewisView->getMolecule());
                }
     });

}

void MoleculeDialog::display(Atom *atom)
{
    ui.lewisView->display(atom);
    this->show();
}

void MoleculeDialog::showEvent(QShowEvent*)
{
    // Avoid accepting the dialog when presseing <enter>
    disconnect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    prevLoaded = nullptr; // do not delete the molecule, that memory is handled by either the MainWindow or MoleculeDialog::handleLoadMolecule



}

void MoleculeDialog::keyPressEvent(QKeyEvent *)
{

}

void MoleculeDialog::handleLoadMolecule()
{
    if(prevLoaded)
        delete prevLoaded;

    QString qFilePath = QFileDialog::getOpenFileName(this, "Open Molecule", "./", "Molecular Dynamics XML File (*.mdx);;Plain XML File (*.xml);;Other(*.*)");
    std::string filePath = qFilePath.toStdString();

    if(filePath.empty())
        return;

    Atom* atom = FileLoading::loadMolecule(filePath);
    if(!atom){
        QMessageBox msgBox;
        msgBox.setText("The contents of the file are ill-formed and could not be loaded.");
        msgBox.exec();
        return;
    }

    prevLoaded = atom;
    ui.lewisView->display(atom);

}

void MoleculeDialog::handleSaveMolecule()
{
    QString qFilePath = QFileDialog::getSaveFileName(this, "Save Molecule", "./", "Molecular Dynamics XML File (*.mdx);;Plain XML File (*.xml)");

    std::string filePath = qFilePath.toStdString();
    std::string fileName = QFileInfo(qFilePath).baseName().toStdString();

    if(fileName.empty())
        return;

    std::string out = FileLoading::storeMolecule(filePath, fileName, ui.lewisView->getMolecule());

    if(!out.empty()){
        QMessageBox msgBox;
        msgBox.setText(("There was a problem storing your file: " + out).c_str());
        msgBox.exec();
    }
}
