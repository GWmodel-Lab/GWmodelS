#ifndef GWMSYMBOLWINDOW_H
#define GWMSYMBOLWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <qgscategorizedsymbolrenderer.h>
#include <QStackedWidget>

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
class QVBoxLayout;
QT_END_NAMESPACE

class GwmSymbolWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GwmSymbolWindow(QWidget *parent = nullptr);

signals:


public:
    QPushButton* okBtn;
    QPushButton* cancelBtn;
    QPushButton* applyBtn;
    QPushButton* helpBtn;

    QComboBox* symbolTypeSelect;

    QStackedWidget* stack;

    QWidget* singleSymbolWidget;
    QWidget* categorizedWidget;
    QWidget* graduatedWidget;
    QWidget* btnContainer;


protected:
//    std::unique_ptr< QgsCategorizedSymbolRenderer > mRenderer;

//    std::unique_ptr< QgsSymbol > mCategorizedSymbol;

//    QgsCategorizedSymbolRendererModel *mModel = nullptr;

private:
    void createButtons();
    void createStackWidget();
    void createLayout();
    void createComboBox();
    void createsingleSymbolWidget();
    void createcategorizedWidget();
    void creategraduatedWidget();


public slots:
    void toggleWidget(int index);


};

#endif // GWMSYMBOLWINDOW_H
