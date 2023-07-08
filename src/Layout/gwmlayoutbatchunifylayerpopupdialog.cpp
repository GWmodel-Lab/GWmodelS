#include "gwmlayoutbatchunifylayerpopupdialog.h"
#include "ui_gwmlayoutbatchunifylayerpopupdialog.h"

#include <qgsapplication.h>
#include <qgsrendererregistry.h>

GwmLayoutBatchUnifyLayerPopupDialog::GwmLayoutBatchUnifyLayerPopupDialog(QgsVectorLayer *layer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmLayoutBatchUnifyLayerPopupDialog)
{
    ui->setupUi(this);

    QgsRendererRegistry *reg = QgsApplication::rendererRegistry();
    const QStringList constRenderers = {
        "singleSymbol",
        "graduatedSymbol"
    };
    for (const QString& name: constRenderers)
    {
        QgsRendererAbstractMetadata* m = reg->rendererMetadata(name);
        ui->cmbSymbolType->addItem(m->icon(), m->visibleName(), name);
    }
    ui->cmbSymbolType->setCurrentIndex(-1);
    connect(ui->cmbSymbolType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](int index)
    {
        if (index > -1)
        {
            mSelectedRenderer = constRenderers[index];
            ui->stkSymbolConfiguration->setCurrentIndex(index);
        }
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [&]()
    {
        accept();
    });
}

GwmLayoutBatchUnifyLayerPopupDialog::~GwmLayoutBatchUnifyLayerPopupDialog()
{
    delete ui;
}


void GwmLayoutBatchUnifyLayerPopupDialog::accept()
{
    QDialog::accept();
}

QString GwmLayoutBatchUnifyLayerPopupDialog::selectedRenderer() const
{
    return mSelectedRenderer;
}
