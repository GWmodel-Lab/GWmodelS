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
    QPushButton* zoomFull;
    QPushButton* zoomToLayer;
    QPushButton* zoomToSelection;
    QPushButton* gwmodelGWRBtn;
    QPushButton* gwmodelGWSSBtn;
    QPushButton* gwmodelGWPCABtn;

signals:
    void openFileImportShapefileSignal();
    void openFileImportJsonSignal();
    void openFileImportCsvSignal();
    void openByXYBtnSingnal();
    void selectBtnSignal();
    void editBtnSignal();
    void moveBtnSignal();
    void zoomFullBtnSignal();
    void zoomToLayerBtnSignal();
    void zoomToSelectionBtnSignal();
    void gwmodelGWRBtnSignal();
    void gwmodelGWSSBtnSignal();
    void gwmodelGWPCABtnSignal();

private:
    void setupUi();
    void createButtons();
};

#endif // GWMODELTOOLBAR_H
