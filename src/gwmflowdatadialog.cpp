#include "gwmflowdatadialog.h"
#include "ui_gwmflowdatadialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextCodec>
#include <QRegularExpression>
#include <QPushButton>
#include <memory>

#include <qgsprojectionselectionwidget.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsfeature.h>
#include <qgsfields.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgspointxy.h>
#include <qgsproject.h>
#include <qgslinesymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsarrowsymbollayer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <QColor>

namespace
{
    QString trimmedHeader(const QString& header)
    {
        QString result = header.trimmed();
        return result;
    }
}

GwmFlowDataDialog::GwmFlowDataDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::GwmFlowDataDialog)
{
    ui->setupUi(this);

    // Map internal pointers to ui widgets for reuse in existing logic
    mFilePathEdit   = ui->fileNameEdit;
    mBrowseButton   = ui->fileNameOpenFileDialogBtn;
    mLayerNameEdit  = ui->layerNameEdit;
    mEncodingCombo  = ui->encodingCombo;
    mDelimiterCombo = ui->delimiterCombo;
    mCrsSelector    = ui->crsSelector;
    if (mCrsSelector)
    {
        QgsCoordinateReferenceSystem currentCrs = QgsProject::instance()->crs();
        if (currentCrs.isValid())
        {
            mCrsSelector->setCrs(currentCrs);
        }
    }

    mFlowVolumeCombo = ui->flowVolumeCombo;
    mOriginXCombo    = ui->originXCombo;
    mOriginYCombo    = ui->originYCombo;
    mOriginZCombo    = ui->originZCombo;
    mOriginMCombo    = ui->originMCombo;
    mDestXCombo      = ui->destXCombo;
    mDestYCombo      = ui->destYCombo;
    mDestZCombo      = ui->destZCombo;
    mDestMCombo      = ui->destMCombo;

    // No explicit origin/destination attribute combos; all extra columns are kept automatically.
    mOriginValueCombo = nullptr;
    mDestValueCombo   = nullptr;
    mStatusLabel = ui->statusLabel;
    mButtonBox   = ui->buttonBox;

    // Populate encoding options
    if (mEncodingCombo)
    {
        mEncodingCombo->clear();
        mEncodingCombo->addItems(QgsVectorDataProvider::availableEncodings());
        int utfIndex = mEncodingCombo->findText(QStringLiteral("UTF-8"), Qt::MatchFixedString);
        if (utfIndex >= 0)
            mEncodingCombo->setCurrentIndex(utfIndex);
    }

    // Populate delimiter options
    if (mDelimiterCombo)
    {
        mDelimiterCombo->clear();
        mDelimiterCombo->addItem(tr("Auto detect"), QString());
        mDelimiterCombo->addItem(tr("Comma (,)"), QStringLiteral(","));
        mDelimiterCombo->addItem(tr("Tab (\\t)"), QStringLiteral("\t"));
        mDelimiterCombo->addItem(tr("Semicolon (;)"), QStringLiteral(";"));
        mDelimiterCombo->addItem(tr("Pipe (|)"), QStringLiteral("|"));
        mDelimiterCombo->setCurrentIndex(0);
    }

    connectSignals();
    updateStatus(tr("Please select a CSV file containing flow data."));
    updateAcceptButton();
}

GwmFlowDataDialog::~GwmFlowDataDialog()
{
    delete mResultLayer;
    delete ui;
}

QgsVectorLayer* GwmFlowDataDialog::takeResultLayer()
{
    QgsVectorLayer* layer = mResultLayer;
    mResultLayer = nullptr;
    return layer;
}

void GwmFlowDataDialog::connectSignals()
{
    connect(mBrowseButton, &QToolButton::clicked, this, &GwmFlowDataDialog::onBrowseClicked);
    connect(mFilePathEdit, &QLineEdit::textChanged, this, &GwmFlowDataDialog::onFilePathChanged);
    connect(mLayerNameEdit, &QLineEdit::textChanged, this, &GwmFlowDataDialog::onFieldSelectionChanged);
    connect(mEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GwmFlowDataDialog::onEncodingChanged);
    connect(mDelimiterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GwmFlowDataDialog::onDelimiterChanged);

    auto connectCombo = [this](QComboBox* combo)
    {
        if (!combo) return;
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &GwmFlowDataDialog::onFieldSelectionChanged);
    };

    connectCombo(mFlowVolumeCombo);
    connectCombo(mOriginXCombo);
    connectCombo(mOriginYCombo);
    connectCombo(mDestXCombo);
    connectCombo(mDestYCombo);
    // mOriginValueCombo/mDestValueCombo are not used in the simplified UI

    connect(mButtonBox, &QDialogButtonBox::accepted, this, &GwmFlowDataDialog::onDialogAccepted);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &GwmFlowDataDialog::reject);
}

void GwmFlowDataDialog::onBrowseClicked()
{
    QString filter = tr("CSV files (*.csv *.txt);;All files (*.*)");
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select flow data file"), QString(), filter);
    if (!filePath.isEmpty())
    {
        mFilePathEdit->setText(filePath);
    }
}

void GwmFlowDataDialog::onFilePathChanged(const QString& path)
{
    QFileInfo info(path);
    if (info.exists() && mLayerNameEdit->text().isEmpty())
    {
        mLayerNameEdit->setText(info.completeBaseName());
    }
    updateHeaders();
    updateAcceptButton();
}

void GwmFlowDataDialog::onEncodingChanged(int)
{
    updateHeaders();
}

void GwmFlowDataDialog::onDelimiterChanged(int)
{
    updateHeaders();
}

void GwmFlowDataDialog::onFieldSelectionChanged()
{
    updateAcceptButton();
}

void GwmFlowDataDialog::onDialogAccepted()
{
    if (!buildLayer())
        return;
    accept();
}

void GwmFlowDataDialog::clearFieldControls()
{
    auto resetCombo = [](QComboBox* combo)
    {
        combo->blockSignals(true);
        combo->clear();
        combo->addItem(QObject::tr("-- Select --"), -1);
        combo->setCurrentIndex(0);
        combo->setEnabled(false);
        combo->blockSignals(false);
    };

    resetCombo(mFlowVolumeCombo);
    resetCombo(mOriginXCombo);
    resetCombo(mOriginYCombo);
    resetCombo(mOriginZCombo);
    resetCombo(mOriginMCombo);
    resetCombo(mDestXCombo);
    resetCombo(mDestYCombo);
    resetCombo(mDestZCombo);
    resetCombo(mDestMCombo);
    if (mOriginValueCombo) resetCombo(mOriginValueCombo);
    if (mDestValueCombo) resetCombo(mDestValueCombo);
}

void GwmFlowDataDialog::updateHeaders()
{
    clearFieldControls();
    mHeaders.clear();

    const QString filePath = mFilePathEdit->text().trimmed();
    if (filePath.isEmpty())
    {
        updateStatus(tr("Please select a CSV file first."));
        return;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        updateStatus(tr("Failed to open file: %1").arg(filePath));
        return;
    }
    QTextStream stream(&file);
    if (mEncodingCombo && mEncodingCombo->currentIndex() >= 0)
    {
        QTextCodec* codec = QTextCodec::codecForName(mEncodingCombo->currentText().toUtf8());
        if (codec)
            stream.setCodec(codec);
    }
    QString headerLine = stream.readLine();
    if (headerLine.isNull())
    {
        updateStatus(tr("No valid header row found in file."));
        return;
    }
    QChar delimiter = currentDelimiter(headerLine);
    QStringList headers = headerLine.split(delimiter, Qt::KeepEmptyParts);
    if (headers.isEmpty())
    {
        updateStatus(tr("Failed to parse CSV header."));
        return;
    }
    for (QString& header : headers)
    {
        header = trimmedHeader(header);
    }
    mHeaders = headers;
    populateFieldCombos();
    updateStatus(tr("Detected %1 fields, please complete the mapping.").arg(headers.size()));
}

void GwmFlowDataDialog::populateFieldCombos()
{
    if (mHeaders.isEmpty())
        return;

    auto setupCombo = [&](QComboBox* combo, const QString& keyword)
    {
        if (!combo) return;
        combo->blockSignals(true);
        combo->setEnabled(true);
        combo->clear();
        combo->addItem(tr("-- Select --"), -1);
        for (int i = 0; i < mHeaders.size(); ++i)
        {
            combo->addItem(mHeaders.at(i), i);
        }
        int defaultIndex = -1;
        if (!keyword.isEmpty())
        {
            for (int i = 0; i < mHeaders.size(); ++i)
            {
                if (mHeaders.at(i).compare(keyword, Qt::CaseInsensitive) == 0)
                {
                    defaultIndex = i;
                    break;
                }
            }
        }
        if (defaultIndex >= 0)
        {
            int comboIndex = combo->findData(defaultIndex);
            if (comboIndex >= 0)
                combo->setCurrentIndex(comboIndex);
        }
        else
        {
            combo->setCurrentIndex(0);
        }
        combo->blockSignals(false);
    };

    setupCombo(mFlowVolumeCombo, QStringLiteral("flow_volume"));
    setupCombo(mOriginXCombo, QStringLiteral("origin_x"));
    setupCombo(mOriginYCombo, QStringLiteral("origin_y"));
    setupCombo(mOriginZCombo, QStringLiteral("origin_z"));
    setupCombo(mOriginMCombo, QStringLiteral("origin_m"));
    setupCombo(mDestXCombo, QStringLiteral("dest_x"));
    setupCombo(mDestYCombo, QStringLiteral("dest_y"));
    setupCombo(mDestZCombo, QStringLiteral("dest_z"));
    setupCombo(mDestMCombo, QStringLiteral("dest_m"));
    setupCombo(mOriginValueCombo, QStringLiteral("origin_value"));
    setupCombo(mDestValueCombo, QStringLiteral("dest_value"));

}

void GwmFlowDataDialog::updateStatus(const QString& message)
{
    if (mStatusLabel)
        mStatusLabel->setText(message);
}

void GwmFlowDataDialog::updateAcceptButton()
{
    bool enabled = true;
    if (mFilePathEdit->text().trimmed().isEmpty())
        enabled = false;
    if (mLayerNameEdit->text().trimmed().isEmpty())
        enabled = false;
    if (!mCrsSelector->crs().isValid())
        enabled = false;
    // Require only origin/destination coordinates, flow volume is optional
    if (comboColumnIndex(mOriginXCombo) < 0 ||
        comboColumnIndex(mOriginYCombo) < 0 ||
        comboColumnIndex(mDestXCombo) < 0 ||
        comboColumnIndex(mDestYCombo) < 0)
    {
        enabled = false;
    }
    if (mHeaders.isEmpty())
        enabled = false;

    if (mButtonBox)
    {
        if (QPushButton* okButton = mButtonBox->button(QDialogButtonBox::Ok))
            okButton->setEnabled(enabled);
    }
}

QChar GwmFlowDataDialog::currentDelimiter(const QString& sampleLine) const
{
    QVariant data = mDelimiterCombo->currentData();
    QString text = data.toString();
    if (!text.isEmpty())
        return text.at(0);
    if (!sampleLine.isEmpty())
        return detectDelimiter(sampleLine);
    return ',';
}

QChar GwmFlowDataDialog::detectDelimiter(const QString& line) const
{
    struct Candidate
    {
        QChar ch;
        int count;
    };
    QList<Candidate> candidates = {
        { '\t', line.count('\t') },
        { ',', line.count(',') },
        { ';', line.count(';') },
        { '|', line.count('|') }
    };
    Candidate best = candidates.first();
    for (const Candidate& c : candidates)
    {
        if (c.count > best.count)
            best = c;
    }
    return best.count > 0 ? best.ch : ',';
}

QString GwmFlowDataDialog::sanitizeFieldName(const QString& source, QSet<QString>& existingNames) const
{
    QString name = source;
    if (name.isEmpty())
        name = QStringLiteral("var");
    name.replace(QRegularExpression("[^A-Za-z0-9_]"), QStringLiteral("_"));
    if (name.isEmpty())
        name = QStringLiteral("var");
    if (name.at(0).isDigit())
        name.prepend(QStringLiteral("_"));

    QString base = name;
    int counter = 1;
    QString lower = name.toLower();
    while (existingNames.contains(lower))
    {
        name = QStringLiteral("%1_%2").arg(base).arg(counter++);
        lower = name.toLower();
    }
    existingNames.insert(lower);
    return name;
}

bool GwmFlowDataDialog::buildLayer()
{
    const QString filePath = mFilePathEdit->text().trimmed();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Failed to open file: %1").arg(filePath));
        return false;
    }
    QTextStream stream(&file);
    if (mEncodingCombo && mEncodingCombo->currentIndex() >= 0)
    {
        QTextCodec* codec = QTextCodec::codecForName(mEncodingCombo->currentText().toUtf8());
        if (codec)
            stream.setCodec(codec);
    }
    QString headerLine = stream.readLine();
    if (headerLine.isNull())
    {
        QMessageBox::warning(this, tr("Import failed"), tr("File does not contain a header row."));
        return false;
    }
    QChar delimiter = currentDelimiter(headerLine);
    QStringList headers = headerLine.split(delimiter, Qt::KeepEmptyParts);
    if (headers.isEmpty())
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Failed to parse CSV columns."));
        return false;
    }

    QgsCoordinateReferenceSystem crs = mCrsSelector->crs();
    if (!crs.isValid())
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Please select a valid coordinate reference system."));
        return false;
    }

    QString uri = QStringLiteral("LineString?crs=%1").arg(crs.authid());
    std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>(uri, mLayerNameEdit->text().trimmed(), QStringLiteral("memory"));
    if (!layer->isValid())
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Failed to create memory layer."));
        return false;
    }

    QSet<QString> existingNames;
    auto registerField = [&](const QString& name)
    {
        existingNames.insert(name.toLower());
    };

    QgsFields fields;
    fields.append(QgsField(QStringLiteral("flow_id"), QVariant::Int));
    registerField(QStringLiteral("flow_id"));
    fields.append(QgsField(QStringLiteral("flow_volume"), QVariant::Double));
    registerField(QStringLiteral("flow_volume"));
    fields.append(QgsField(QStringLiteral("origin_x"), QVariant::Double));
    registerField(QStringLiteral("origin_x"));
    fields.append(QgsField(QStringLiteral("origin_y"), QVariant::Double));
    registerField(QStringLiteral("origin_y"));
    fields.append(QgsField(QStringLiteral("dest_x"), QVariant::Double));
    registerField(QStringLiteral("dest_x"));
    fields.append(QgsField(QStringLiteral("dest_y"), QVariant::Double));
    registerField(QStringLiteral("dest_y"));

    int originValueFieldIndex = -1;
    if (comboColumnIndex(mOriginValueCombo) >= 0)
    {
        originValueFieldIndex = fields.size();
        fields.append(QgsField(QStringLiteral("origin_value"), QVariant::Double));
        registerField(QStringLiteral("origin_value"));
    }

    int destValueFieldIndex = -1;
    if (comboColumnIndex(mDestValueCombo) >= 0)
    {
        destValueFieldIndex = fields.size();
        fields.append(QgsField(QStringLiteral("dest_value"), QVariant::Double));
        registerField(QStringLiteral("dest_value"));
    }

    // Generic attribute fields: all CSV columns which are not used as coords/flow/explicit values
    struct AttrField
    {
        int csvIndex = -1;
        int fieldIndex = -1;
    };
    QList<AttrField> attrFields;

    const int originValueCsv = comboColumnIndex(mOriginValueCombo);
    const int destValueCsv   = comboColumnIndex(mDestValueCombo);
    const int flowVolumeCsv  = comboColumnIndex(mFlowVolumeCombo);
    const int originXCsv     = comboColumnIndex(mOriginXCombo);
    const int originYCsv     = comboColumnIndex(mOriginYCombo);
    const int destXCsv       = comboColumnIndex(mDestXCombo);
    const int destYCsv       = comboColumnIndex(mDestYCombo);

    QSet<int> reservedCsvIndices;
    if (flowVolumeCsv >= 0)   reservedCsvIndices.insert(flowVolumeCsv);
    if (originXCsv >= 0)      reservedCsvIndices.insert(originXCsv);
    if (originYCsv >= 0)      reservedCsvIndices.insert(originYCsv);
    if (destXCsv >= 0)        reservedCsvIndices.insert(destXCsv);
    if (destYCsv >= 0)        reservedCsvIndices.insert(destYCsv);
    if (originValueCsv >= 0)  reservedCsvIndices.insert(originValueCsv);
    if (destValueCsv >= 0)    reservedCsvIndices.insert(destValueCsv);

    for (int col = 0; col < headers.size(); ++col)
    {
        if (reservedCsvIndices.contains(col))
            continue;
        QString sanitized = sanitizeFieldName(headers.at(col), existingNames);
        int idx = fields.size();
        // Store as string to preserve original attribute values for re-use
        fields.append(QgsField(sanitized, QVariant::String));
        AttrField af;
        af.csvIndex = col;
        af.fieldIndex = idx;
        attrFields.append(af);
    }

    if (!layer->startEditing())
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Failed to start editing mode."));
        return false;
    }
    QList<QgsField> fieldList;
    for (int i = 0; i < fields.count(); ++i)
    {
        fieldList.append(fields.at(i));
    }
    layer->dataProvider()->addAttributes(fieldList);
    layer->updateFields();

    int flowIdField = layer->fields().indexOf(QStringLiteral("flow_id"));
    int flowVolumeField = layer->fields().indexOf(QStringLiteral("flow_volume"));
    int originXField = layer->fields().indexOf(QStringLiteral("origin_x"));
    int originYField = layer->fields().indexOf(QStringLiteral("origin_y"));
    int destXField = layer->fields().indexOf(QStringLiteral("dest_x"));
    int destYField = layer->fields().indexOf(QStringLiteral("dest_y"));
    if (flowIdField < 0 || flowVolumeField < 0 || originXField < 0 || originYField < 0 || destXField < 0 || destYField < 0)
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Failed to initialize fields."));
        return false;
    }

    int featureCount = 0;
    int skipped = 0;
    double minFlow = std::numeric_limits<double>::max();
    double maxFlow = std::numeric_limits<double>::lowest();
    bool hasFlowStats = false;
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty())
            continue;
        QStringList values = line.split(delimiter, Qt::KeepEmptyParts);

        // Origin/destination coordinates are mandatory
        double originX = 0.0, originY = 0.0, destX = 0.0, destY = 0.0;
        if (!parseDouble(values, originXCsv, originX) ||
            !parseDouble(values, originYCsv, originY) ||
            !parseDouble(values, destXCsv, destX) ||
            !parseDouble(values, destYCsv, destY))
        {
            ++skipped;
            continue;
        }

        // Flow volume is optional
        double flowVolume = 0.0;
        if (flowVolumeCsv >= 0)
        {
            if (!parseDouble(values, flowVolumeCsv, flowVolume))
            {
                ++skipped;
                continue;
            }
            if (!hasFlowStats)
            {
                minFlow = maxFlow = flowVolume;
                hasFlowStats = true;
            }
            else
            {
                minFlow = std::min(minFlow, flowVolume);
                maxFlow = std::max(maxFlow, flowVolume);
            }
        }

        QgsFeature feature(layer->fields());
        feature.setAttribute(flowIdField, featureCount);
        feature.setAttribute(flowVolumeField, flowVolume);
        feature.setAttribute(originXField, originX);
        feature.setAttribute(originYField, originY);
        feature.setAttribute(destXField, destX);
        feature.setAttribute(destYField, destY);

        if (originValueFieldIndex >= 0 && originValueFieldIndex < layer->fields().count())
        {
            double originValue = 0.0;
            if (originValueCsv >= 0 && parseDouble(values, originValueCsv, originValue))
                feature.setAttribute(originValueFieldIndex, originValue);
        }
        if (destValueFieldIndex >= 0 && destValueFieldIndex < layer->fields().count())
        {
            double destValue = 0.0;
            if (destValueCsv >= 0 && parseDouble(values, destValueCsv, destValue))
                feature.setAttribute(destValueFieldIndex, destValue);
        }

        // Set generic attribute fields from remaining CSV columns
        for (const AttrField& attr : attrFields)
        {
            if (attr.fieldIndex < 0)
                continue;
            QString v;
            if (attr.csvIndex >= 0 && attr.csvIndex < values.size())
                v = values.at(attr.csvIndex).trimmed();
            feature.setAttribute(attr.fieldIndex, v);
        }

        QgsPolylineXY lineGeom = {
            QgsPointXY(originX, originY),
            QgsPointXY(destX, destY)
        };
        feature.setGeometry(QgsGeometry::fromPolylineXY(lineGeom));
        if (!layer->addFeature(feature))
        {
            ++skipped;
            continue;
        }
        ++featureCount;
    }

    layer->updateExtents();
    if (!layer->commitChanges())
    {
        QMessageBox::warning(this, tr("Import failed"), tr("Failed to commit layer edits."));
        return false;
    }

    // Apply arrow symbology so each flow is rendered with an arrow along its line.
    if (hasFlowStats && maxFlow > minFlow)
    {
        // Build graduated renderer based on flow_volume to vary arrow color (yellow -> red).
        const int classCount = 5;
        const double range = maxFlow - minFlow;
        const double step = range / classCount;

        QList<QgsRendererRange> ranges;
        for (int i = 0; i < classCount; ++i)
        {
            double lower = (i == 0) ? minFlow : (minFlow + i * step);
            double upper = (i == classCount - 1) ? maxFlow : (minFlow + (i + 1) * step);

            std::unique_ptr<QgsLineSymbol> sym = std::make_unique<QgsLineSymbol>();
            sym->setOpacity(0.3);

            // 计算 0~1 的插值比例，用于颜色插值：0=黄色(255,255,0), 1=红色(255,0,0)
            double t = (classCount == 1) ? 0.0 : static_cast<double>(i) / (classCount - 1);
            int r = 255;
            int g = static_cast<int>(255.0 * (1.0 - t));
            int b = 0;
            QColor color(r, g, b);
            sym->setColor(color);

            QgsArrowSymbolLayer* arrow = new QgsArrowSymbolLayer();
            // Basic head geometry; avoid newer API not available in this QGIS version.
            arrow->setHeadLength(3.0);
            arrow->setHeadThickness(1.5);
            arrow->setColor(color);
            sym->changeSymbolLayer(0, arrow);

            QString label = QString("%1 - %2").arg(lower).arg(upper);
            ranges.append(QgsRendererRange(lower, upper, sym.release(), label));
        }

        QgsGraduatedSymbolRenderer* gradRenderer =
                new QgsGraduatedSymbolRenderer(QStringLiteral("flow_volume"), ranges);
        gradRenderer->setMode(QgsGraduatedSymbolRenderer::EqualInterval);
        layer->setRenderer(gradRenderer);
    }
    else
    {
        // No flow volume selected or stats available: use single arrow symbol.
        std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
        lineSymbol->setOpacity(0.3);
        QgsArrowSymbolLayer* arrowLayer = new QgsArrowSymbolLayer();
        // Use basic head geometry; avoid newer API not available in this QGIS version.
        arrowLayer->setHeadLength(3.0);
        arrowLayer->setHeadThickness(1.5);
        lineSymbol->changeSymbolLayer(0, arrowLayer);
        QgsSingleSymbolRenderer* renderer = new QgsSingleSymbolRenderer(lineSymbol.release());
        layer->setRenderer(renderer);
    }

    if (featureCount == 0)
    {
        QMessageBox::warning(this, tr("Import failed"), tr("No valid flow records were imported."));
        return false;
    }

    if (skipped > 0)
    {
        QMessageBox::information(this,
                                 tr("Import completed"),
                                 tr("Successfully imported %1 records, skipped %2.").arg(featureCount).arg(skipped));
    }

    delete mResultLayer;
    mResultLayer = layer.release();
    return true;
}

bool GwmFlowDataDialog::parseDouble(const QStringList& values, int index, double& target) const
{
    if (index < 0 || index >= values.size())
        return false;
    bool ok = false;
    target = values.at(index).trimmed().toDouble(&ok);
    return ok;
}

int GwmFlowDataDialog::comboColumnIndex(QComboBox* combo) const
{
    if (!combo)
        return -1;
    bool ok = false;
    int idx = combo->currentData().toInt(&ok);
    return ok ? idx : -1;
}


