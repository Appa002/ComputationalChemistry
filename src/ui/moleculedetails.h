#ifndef MOLECULEDETAILS_H
#define MOLECULEDETAILS_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <common/atom.h>

namespace Ui {
class MoleculeDetails;
}

class MoleculeDetails : public QFrame
{
    Q_OBJECT

public:
    explicit MoleculeDetails(QWidget *parent = nullptr);
    MoleculeDetails(QWidget *parent, std::string moleculeName, std::vector<std::pair<QColor, std::string>> colourPairs);
    ~MoleculeDetails();

    int atomIdx = -1;

private:


    bool expanded = false;
    Ui::MoleculeDetails *ui;

    QPushButton* moreLessBtn;
    QLabel* moleculeName;
    QFrame* coloursFrame;
    QFormLayout* coloursFrameLayout;

signals:
    void onDisplay(int);
    void onEdit(int);
    void onDelete(int);

};

#endif // MOLECULEDETAILS_H
