#ifndef MOLECULEDIALOG_H
#define MOLECULEDIALOG_H

#include <QWidget>
#include <QDialog>
#include "ui_newMolecule.h"


class MoleculeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MoleculeDialog(QWidget *parent = nullptr);
    void display(Atom* atom);

private:
    Atom* prevLoaded = nullptr;
    Ui::NewMoleculeDialog ui;
    void showEvent(QShowEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private slots:
    void handleLoadMolecule();
    void handleSaveMolecule();

signals:
    void moleculeChanged(Atom*);

};

#endif // MOLECULEDIALOG_H
