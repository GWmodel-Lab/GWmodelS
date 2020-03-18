#ifndef GWMSYMBOLWINDOW_H
#define GWMSYMBOLWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <qgscategorizedsymbolrenderer.h>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTreeView>
//#include <qgslayerpropertieswidget.h>
#include <qgsstyleitemslistwidget.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbolselectordialog.h>
#include <qgsgraduatedsymbolrendererwidget.h>
#include "qgssinglesymbolrendererwidget.h"
#include <qgscategorizedsymbolrendererwidget.h>
#include <qgsrulebasedrendererwidget.h>
#include <qgsnullsymbolrendererwidget.h>

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
    explicit GwmSymbolWindow(QgsVectorLayer* vectorLayer,QWidget *parent = nullptr);

signals:
    /**
     * @brief 地图画布刷新信号
     */
    void canvasRefreshSingal();

public:
    /**
     * @brief 确定按钮
     */
    QPushButton* okBtn;
    /**
     * @brief 取消按钮
     */
    QPushButton* cancelBtn;
    /**
     * @brief 应用按钮
     */
    QPushButton* applyBtn;
    /**
     * @brief 帮助按钮
     */
    QPushButton* helpBtn;

    /**
     * @brief 符号类型选择下拉框
     */
    QComboBox* symbolTypeSelect;

    /**
     * @brief 分页窗口
     */
    QStackedWidget* stack;
    /**
     * @brief 目前激活的符号窗口
     */
    QgsRendererWidget *mActiveWidget = nullptr;
    QWidget* pageNoWidget = new QWidget(this);
    /**
     * @brief singleSymbol 窗口
     */
//    QgsSymbolSelectorWidget* singleSymbolWidget;
    QgsSingleSymbolRendererWidget* singleSymbolWidget;
    /**
     * @brief categorized 窗口
     */
    QWidget* categorizedWidget;
    /**
     * @brief graduated 窗口
     */
    QWidget* graduatedWidget;

    /**
     * @brief 底部按钮容器
     */
    QWidget* btnContainer;

    /**
     * @brief 目标图层
     */
    QgsVectorLayer* mLayer;
    /**
     * @brief 图层样式
     */
    QgsStyle *mStyle = QgsStyle::defaultStyle();
    /**
     * @brief 图层符号
     */
    QgsSymbol *mSymbol = nullptr;
    /**
      * @brief 未修改的图层符号
      */
    QgsSymbol* mLastSymbol = nullptr;
     std::unique_ptr< QgsSymbol > symbol;
    /**
     * @brief mSingleRenderer
     */
    QgsSingleSymbolRenderer* mSingleRenderer;

private:
    /**
     * @brief 创建底部按钮
     */
    void createButtons();
    /**
     * @brief 创建分页窗口
     */
    void createStackWidget();
    /**
     * @brief 创建窗口布局
     */
    void createLayout();
    /**
     * @brief 创建符号类型选择下拉框
     */
    void createComboBox();
    /**
     * @brief 创建singleSymbol 窗口
     */
    void createsingleSymbolWidget();
    /**
     * @brief 创建categorized 窗口
     */
    void createcategorizedWidget();
    /**
     * @brief 创建graduated 窗口
     */
    void creategraduatedWidget();
    /**
     * @brief 获取图层的符号样式属性
     */
    void getLayerSymbol();
    /**
     * @brief 修改后符号应用到图层上
     */
    void applyLayerSymbol();
    /**
     * @brief OK按钮响应函数
     */
    void onOkBtnClicked();
    /**
     * @brief closeEvent 关闭窗口前改回图层符号
     * @param ev 窗口关闭信号
     */
//    virtual void closeEvent(QCloseEvent* ev)override;

public slots:
    /**
     * @brief 切换StackWidget窗口
     * @param index 窗口索引号
     */
    void rendererChanged(int index);


};

#endif // GWMSYMBOLWINDOW_H
