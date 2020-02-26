#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmodeltoolbar.h>

//namespace Ui {
//class MainLayout;
//}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    GWmodelToolbar* toolBar;
    QWidget* map;

    void createToolBar();
    void createMap();
};

#endif // MAINLAYOUT_H
