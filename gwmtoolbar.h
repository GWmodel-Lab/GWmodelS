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

namespace Ui {
class GwmToolbar;
}

class GwmToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit GwmToolbar(QWidget *parent = nullptr);
    ~GwmToolbar();

private:
    Ui::GwmToolbar* ui;

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
    void gwmScalableGWRBtnSignal();
    void saveLayerBtnSignal();
    void exportLayerBtnSignal();

    void gwmodelRobustGWRBtnSignal();

public:
    void setBtnEnabled(bool flag);
};

#endif // GWMODELTOOLBAR_H
