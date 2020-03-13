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
//#include <qgssymbolslistwidget.h>

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
class SymbolLayerItem;
QT_END_NAMESPACE

class DataDefinedRestorer: public QObject
{
    Q_OBJECT
  public:
    DataDefinedRestorer( QgsSymbol *symbol, const QgsSymbolLayer *symbolLayer );

  public slots:
    void restore();

  private:
    QgsMarkerSymbol *mMarker = nullptr;
    const QgsMarkerSymbolLayer *mMarkerSymbolLayer = nullptr;
    double mSize;
    double mAngle;
    QPointF mMarkerOffset;
    QgsProperty mDDSize;
    QgsProperty mDDAngle;

    QgsLineSymbol *mLine = nullptr;
    const QgsLineSymbolLayer *mLineSymbolLayer = nullptr;
    double mWidth;
    double mLineOffset;
    QgsProperty mDDWidth;

    void save();
};


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
     * @brief singleSymbol 窗口
     */
    QWidget* singleSymbolWidget;
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
     * @brief 符号修改图层
     */
    QgsVectorLayer* mLayer;

    QTreeView* layersTree;

    QComboBox* colorCombobox;

    QStandardItemModel *model = nullptr;

    QgsStyle *mStyle = nullptr;
    QgsSymbol *mSymbol = nullptr;
    std::unique_ptr<DataDefinedRestorer> mDataDefineRestorer;
    QMenu *mAdvancedMenu = nullptr;

    QStackedWidget* stackedWidget;

protected:
//    std::unique_ptr< QgsCategorizedSymbolRenderer > mRenderer;

//    std::unique_ptr< QgsSymbol > mCategorizedSymbol;

//    QgsCategorizedSymbolRendererModel *mModel = nullptr;

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

    void getLayerSymbol();

    void applyLayerSymbol();
//    void setWidget( QWidget *widget );
//    void layerChanged();

    void loadSymbol( QgsSymbol *symbol, SymbolLayerItem *parent = nullptr );


public slots:
    void toggleWidget(int index);


};

#endif // GWMSYMBOLWINDOW_H
