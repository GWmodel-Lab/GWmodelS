#ifndef GWMODELTOOLBAR_H
#define GWMODELTOOLBAR_H

#include <QtWidgets>

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

class GWmodelToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit GWmodelToolbar(QWidget *parent = nullptr);
    ~GWmodelToolbar();

private:
    QPushButton* openLayerBtn;
    QPushButton* saveLayerBtn;
    QPushButton* exportLayerBtn;
    QPushButton* editBtn;
    QPushButton* moveBtn;
    QPushButton* fullScreenBtn;
    QPushButton* showPositionBtn;
    QPushButton* gwmodelGWRBtn;
    QPushButton* gwmodelGWSSBtn;
    QPushButton* gwmodelGWPCABtn;

private:
    void createButtons();
};

#endif // GWMODELTOOLBAR_H
