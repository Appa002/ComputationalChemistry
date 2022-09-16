#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "moleculedialog.h"
#include <ui/moleculedetails.h>
#include <QScrollArea>
#include <unordered_map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    std::unordered_map<std::string, std::vector<float>> colourMapToStd( std::vector<std::pair<QColor, std::string>> const & it);
    std::vector<std::pair<QColor, std::string>> createColourMap(Atom* atom);
    QColor randomColour();
    QColor hsvToRgb(float H, float S, float V);

    std::unordered_map<Atom*, std::string> moleculeNameMap;

    Ui::MainWindow *ui;
    MoleculeDialog* md;

    QWidget* infoFrame;


private slots:
    void moleculeButtonSlot();
    void onMoleculeChanged(Atom* molecule);
};
#endif // MAINWINDOW_H
