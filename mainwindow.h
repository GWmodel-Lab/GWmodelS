#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mainwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionScalable_GWR_triggered();

private:
    void setupMenus();
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    Ui::MainWindow *ui;

    MainWidget* mainWidget;
};
#endif // MAINWINDOW_H
