#ifndef GWMLAYOUTBATCHUNIFYLAYERPOPUPDIALOG_H
#define GWMLAYOUTBATCHUNIFYLAYERPOPUPDIALOG_H

#include <QDialog>

namespace Ui {
class GwmLayoutBatchUnifyLayerPopupDialog;
}

class QgsVectorLayer;

class GwmLayoutBatchUnifyLayerPopupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmLayoutBatchUnifyLayerPopupDialog(QgsVectorLayer* layer, QWidget *parent = nullptr);
    ~GwmLayoutBatchUnifyLayerPopupDialog();

    QString selectedRenderer() const;

public slots:
    void accept() override;

private:
    Ui::GwmLayoutBatchUnifyLayerPopupDialog *ui;

    QString mSelectedRenderer = "singleSymbol";
};

#endif // GWMLAYOUTBATCHUNIFYLAYERPOPUPDIALOG_H
