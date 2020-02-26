#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>

//namespace Ui {
//class MainLayout;
//}

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QHBoxLayout;
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    QWidget* toolBar;
    QWidget* map;

    void createToolBar();
    void createMap();
};

#endif // MAINLAYOUT_H
