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

class GwmToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit GwmToolbar(QWidget *parent = nullptr);
    ~GwmToolbar();

private:
    QPushButton* openLayerBtn;
    QPushButton* openByXYBtn;
    QPushButton* saveLayerBtn;
    QPushButton* exportLayerBtn;
    QPushButton* selectBtn;
    QPushButton* moveBtn;
    QPushButton* editBtn;
    QPushButton* fullScreenBtn;
    QPushButton* showPositionBtn;
    QPushButton* gwmodelGWRBtn;
    QPushButton* gwmodelGWSSBtn;
    QPushButton* gwmodelGWPCABtn;

    QLabel* openBtnInfo;
    QLabel* openByXYBtnInfo;
    QLabel* saveBtnInfo;
    QLabel* exportBtnInfo;
    QLabel* editBtnInfo;
    QLabel* moveBtnInfo;
    QLabel* fullScreenBtnInfo;
    QLabel* showPositionBtnInfo;

signals:
    void openFileImportShapefileSignal();
    void openFileImportJsonSignal();
    void openFileImportCsvSignal();
    void openByXYBtnSingnal();
    void selectBtnSignal();
    void editBtnSignal();
    void moveBtnSignal();
    void fullScreenBtnSignal();
    void showPositionBtnSignal();
    void gwmodelGWRBtnSignal();
    void gwmodelGWSSBtnSignal();
    void gwmodelGWPCABtnSignal();

private:
    void createButtons();
    void openFileImportShapefile();
    void openFileImportJson();
    void openByXYBtnSlot();
    void openFileImportCsv();
    void editBtnSlot();
    void moveBtnSlot();
    void fullScreenBtnSlot();
    void showPositionBtnSlot();
    void gwmodelGWRBtnSlot();
    void gwmodelGWSSBtnSlot();
    void gwmodelGWPCABtnSlot();

    bool eventFilter(QObject *watched, QEvent *event);
};

#endif // GWMODELTOOLBAR_H
