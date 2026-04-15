#include "gwmpropertyswimtab.h"
#include "ui_gwmpropertyswimtab.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QDebug>
#include <armadillo>
#include <QTextStream>
#include <QFileInfo>
#include <cmath>
#include <QVariant>
#include <algorithm>

#include "SpatialWeight/gwmbandwidthweight.h"
#include "SpatialWeight/gwmcrsdistance.h"
#include "SpatialWeight/gwmminkwoskidistance.h"
#include "SpatialWeight/gwmdmatdistance.h"
#include "gwmplot.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

using namespace arma;

GwmPropertySWIMTab::GwmPropertySWIMTab(QWidget *parent, GwmSWIMTaskThread* taskThread) :
    QWidget(parent),
    ui(new Ui::GwmPropertySWIMTab),
    mTaskThread(taskThread)
{
    ui->setupUi(this);
    if (ui->verticalLayout_Bandwidth)
    {
        mBandwidthPlot = new GwmPlot(ui->groupBox_BandwidthView);
        ui->verticalLayout_Bandwidth->addWidget(mBandwidthPlot);
        mBandwidthPlot->hide();
    }
    // Do not call updateUI() here - it will be called explicitly from outside
    // This follows the pattern used by other property tabs (GWR, GTWR, etc.)
}

GwmPropertySWIMTab::~GwmPropertySWIMTab()
{
    delete ui;
}

void GwmPropertySWIMTab::setTaskThread(GwmSWIMTaskThread* taskThread)
{
    mTaskThread = taskThread;
    if (mTaskThread && ui)
    {
        updateUI();
    }
}

void GwmPropertySWIMTab::updateUI()
{
    if (!mTaskThread || !ui)
    {
        qDebug() << "[GwmPropertySWIMTab::updateUI] Task thread or UI is null.";
        return;
    }

    mHasCachedOls = false;
    bool ok = false;
    OlsResult computed = computeGlobalOls(ok);
    if (ok)
    {
        mCachedOlsResult = computed;
        mHasCachedOls = true;
    }

    displayGlobalDiagnostics();
    populateGlobalCoefficients();
    displaySwimCalibrationInfo();
    populateSwimCoefficients();
    displaySwimDiagnostics();
    updateBandwidthSelectionView();
}

void GwmPropertySWIMTab::displayGlobalDiagnostics()
{
    if (!ui) return;

    auto setFromValue = [&](QLabel* label, double value, int precision)
    {
        setLabelText(label, formatNumber(value, precision));
    };

    if (mHasCachedOls && mCachedOlsResult.valid)
    {
        setFromValue(ui->lblGlobalAIC, mCachedOlsResult.aic, 4);
        setFromValue(ui->lblGlobalAICc, mCachedOlsResult.aicc, 4);
        setFromValue(ui->lblGlobalRSS, mCachedOlsResult.rss, 4);
        setFromValue(ui->lblGlobalRSquared, mCachedOlsResult.rSquared, 4);
        setFromValue(ui->lblGlobalAdjRSquared, mCachedOlsResult.adjRSquared, 4);
    }
    else
    {
        setLabelText(ui->lblGlobalAIC, tr("-"));
        setLabelText(ui->lblGlobalAICc, tr("-"));
        setLabelText(ui->lblGlobalRSS, tr("-"));
        setLabelText(ui->lblGlobalRSquared, tr("-"));
        setLabelText(ui->lblGlobalAdjRSquared, tr("-"));
    }
}

void GwmPropertySWIMTab::populateGlobalCoefficients()
{
    if (!ui || !ui->tableGlobalCoefficients) return;

    const QStringList headers = { tr("Name"), tr("Estimate"), tr("Std. Error"), tr("t-value") };
    setupTableHeaders(ui->tableGlobalCoefficients, headers);

    if (!mHasCachedOls || !mCachedOlsResult.valid || mCachedOlsResult.names.isEmpty())
    {
        populateEmptyTableMessage(ui->tableGlobalCoefficients, tr("No OLS summary is available."));
        adjustTableToContents(ui->tableGlobalCoefficients);
        return;
    }

    const int rows = mCachedOlsResult.names.size();
    ui->tableGlobalCoefficients->setRowCount(rows);
    for (int i = 0; i < rows; ++i)
    {
        ui->tableGlobalCoefficients->setItem(i, 0, new QTableWidgetItem(mCachedOlsResult.names.value(i)));
        ui->tableGlobalCoefficients->setItem(i, 1, new QTableWidgetItem(formatNumber(mCachedOlsResult.estimates.value(i))));
        ui->tableGlobalCoefficients->setItem(i, 2, new QTableWidgetItem(formatNumber(mCachedOlsResult.stdErrors.value(i))));
        ui->tableGlobalCoefficients->setItem(i, 3, new QTableWidgetItem(formatNumber(mCachedOlsResult.tValues.value(i))));
    }
    ui->tableGlobalCoefficients->resizeColumnsToContents();
    adjustTableToContents(ui->tableGlobalCoefficients);
}

void GwmPropertySWIMTab::displaySwimCalibrationInfo()
{
    if (!ui || !mTaskThread) return;

    setLabelText(ui->lblCalibrationFocusType, focusTypeDescription());
    setLabelText(ui->lblCalibrationKernel, kernelDescription());

    QString adaptiveText = tr("Adaptive: -");
    const GwmSpatialWeight weight = mTaskThread->spatialWeight();
    if (auto* bw = weight.weight<GwmBandwidthWeight>())
    {
        if (bw->adaptive())
        {
            const int neighbours = static_cast<int>(std::round(std::max(0.0, bw->bandwidth())));
            adaptiveText = tr("Adaptive: %1 (number of nearest neighbours)").arg(neighbours);
        }
        else
        {
            adaptiveText = tr("Adaptive: Fixed bandwidth (%1)").arg(QString::number(bw->bandwidth(), 'f', 4));
        }
    }
    setLabelText(ui->lblCalibrationAdaptive, adaptiveText);
    setLabelText(ui->lblCalibrationRegressionPoints, tr("The same location as observations are used."));
    setLabelText(ui->lblCalibrationDistanceMetric, distanceDescription());
}

void GwmPropertySWIMTab::populateSwimCoefficients()
{
    if (!ui || !ui->tableSwimCoefficients) return;

    const QStringList headers = { tr("Name"), tr("Min"), tr("1st Qu"), tr("Median"), tr("3rd Qu"), tr("Max") };
    setupTableHeaders(ui->tableSwimCoefficients, headers);

    if (!mTaskThread)
    {
        populateEmptyTableMessage(ui->tableSwimCoefficients, tr("No SWIM coefficient summary is available."));
        adjustTableToContents(ui->tableSwimCoefficients);
        return;
    }

    // Get coefficients matrix from result list
    mat coefficients;
    bool found = false;
    const auto resultList = mTaskThread->resultList();
    for (const auto& pair : resultList)
    {
        if (pair.first == QStringLiteral("Coefficients"))
        {
            coefficients = pair.second;
            found = true;
            break;
        }
    }

    if (!found || coefficients.n_rows == 0 || coefficients.n_cols == 0)
    {
        populateEmptyTableMessage(ui->tableSwimCoefficients, tr("No SWIM coefficient summary is available."));
        adjustTableToContents(ui->tableSwimCoefficients);
        return;
    }

    // Get coefficient names
    QStringList names;
    names << tr("Intercept");
    const QStringList mappingNames = mTaskThread->fieldMapping().independentVarNames;
    const int numIndepVars = coefficients.n_cols - 1; // First column is intercept
    for (int idx = 0; idx < numIndepVars; ++idx)
    {
        if (idx < mappingNames.size())
            names << mappingNames[idx];
        else
            names << tr("Var %1").arg(idx + 1);
    }

    // Calculate statistics for each coefficient
    const int numCoeffs = static_cast<int>(coefficients.n_cols);
    if (numCoeffs != names.size())
    {
        populateEmptyTableMessage(ui->tableSwimCoefficients, tr("Coefficient count mismatch."));
        adjustTableToContents(ui->tableSwimCoefficients);
        return;
    }

    ui->tableSwimCoefficients->setRowCount(numCoeffs);
    for (int col = 0; col < numCoeffs; ++col)
    {
        // Extract column vector
        vec coeffCol = coefficients.col(col);

        // Remove NaN and Inf values
        QVector<double> validValues;
        for (uword i = 0; i < coeffCol.n_elem; ++i)
        {
            const double val = coeffCol(i);
            if (std::isfinite(val))
            {
                validValues.append(val);
            }
        }

        if (validValues.isEmpty())
        {
            // All values are invalid
            ui->tableSwimCoefficients->setItem(col, 0, new QTableWidgetItem(names[col]));
            for (int statCol = 1; statCol < headers.size(); ++statCol)
            {
                ui->tableSwimCoefficients->setItem(col, statCol, new QTableWidgetItem(tr("-")));
            }
            continue;
        }

        // Sort for quantile calculation
        std::sort(validValues.begin(), validValues.end());

        // Calculate statistics
        const int n = validValues.size();
        const double min = validValues.first();
        const double max = validValues.last();
        const double median = calculateQuantile(validValues, 0.5);
        const double q1 = calculateQuantile(validValues, 0.25);
        const double q3 = calculateQuantile(validValues, 0.75);

        // Populate row
        ui->tableSwimCoefficients->setItem(col, 0, new QTableWidgetItem(names[col]));
        ui->tableSwimCoefficients->setItem(col, 1, new QTableWidgetItem(formatNumber(min, 6)));
        ui->tableSwimCoefficients->setItem(col, 2, new QTableWidgetItem(formatNumber(q1, 6)));
        ui->tableSwimCoefficients->setItem(col, 3, new QTableWidgetItem(formatNumber(median, 6)));
        ui->tableSwimCoefficients->setItem(col, 4, new QTableWidgetItem(formatNumber(q3, 6)));
        ui->tableSwimCoefficients->setItem(col, 5, new QTableWidgetItem(formatNumber(max, 6)));
    }

    ui->tableSwimCoefficients->resizeColumnsToContents();
    adjustTableToContents(ui->tableSwimCoefficients);
}

void GwmPropertySWIMTab::displaySwimDiagnostics()
{
    if (!ui || !mTaskThread) return;

    const GwmSWIMDiagnostics diag = mTaskThread->diagnostics();
    setLabelText(ui->lblSwimDataPoints, diag.dataPoints > 0 ? QString::number(diag.dataPoints) : tr("-"));
    setLabelText(ui->lblSwimEffectiveParams, formatNumber(diag.effectiveParameters));
    setLabelText(ui->lblSwimEffectiveDof, formatNumber(diag.effectiveDof));
    setLabelText(ui->lblSwimAIC, formatNumber(diag.aic));
    setLabelText(ui->lblSwimAICc, formatNumber(diag.aicc));
    // For Poisson SWIM, RSS is less meaningful; expose deviance instead,
    // but keep the label for backward compatibility.
    setLabelText(ui->lblSwimRSS, formatNumber(diag.deviance));
    // Interpret rSquared field as McFadden-style pseudo R², adjRSquared as adjusted McFadden pseudo R²
    setLabelText(ui->lblSwimRSquared, formatNumber(diag.rSquared));
    setLabelText(ui->lblSwimAdjRSquared, formatNumber(diag.adjRSquared));
}

void GwmPropertySWIMTab::updateBandwidthSelectionView()
{
    if (!ui) return;

    BandwidthCriterionList trace;
    if (mTaskThread)
    {
        trace = mTaskThread->bandwidthTrace();
    }
    
    if (trace.empty())
    {
        if (mBandwidthPlot)
            mBandwidthPlot->hide();
        if (ui->lblBandwidthPlotPlaceholder)
        {
            QString message = tr("Bandwidth Selection View\nNo bandwidth search results available.");
            ui->lblBandwidthPlotPlaceholder->setText(message);
            ui->lblBandwidthPlotPlaceholder->show();
        }
        return;
    }

    if (!mBandwidthPlot)
        return;

    QVariant data = QVariant::fromValue(trace);
    GwmBandwidthSizeSelector::PlotBandwidthResult(data, mBandwidthPlot);
    mBandwidthPlot->show();
    if (ui->lblBandwidthPlotPlaceholder)
        ui->lblBandwidthPlotPlaceholder->hide();
}

void GwmPropertySWIMTab::setLabelText(QLabel* label, const QString& text)
{
    if (!label) return;
    label->setText(text.isEmpty() ? tr("-") : text);
}

void GwmPropertySWIMTab::setupTableHeaders(QTableWidget* table, const QStringList& headers)
{
    if (!table) return;
    table->clear();
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    if (auto* header = table->horizontalHeader())
    {
        header->setStretchLastSection(false);
        header->setSectionResizeMode(QHeaderView::ResizeToContents);
        header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    if (auto* vHeader = table->verticalHeader())
    {
        vHeader->setVisible(false);
    }
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    table->setWordWrap(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void GwmPropertySWIMTab::populateEmptyTableMessage(QTableWidget* table, const QString& message)
{
    if (!table) return;
    if (table->columnCount() == 0)
    {
        table->setColumnCount(1);
        table->setHorizontalHeaderLabels({QString()});
    }

    table->setRowCount(1);
    auto* item = new QTableWidgetItem(message);
    item->setFlags(Qt::ItemIsEnabled);
    table->setItem(0, 0, item);
    table->setSpan(0, 0, 1, table->columnCount());
}

QString GwmPropertySWIMTab::kernelDescription() const
{
    if (!mTaskThread) return tr("-");
    const GwmSpatialWeight weight = mTaskThread->spatialWeight();
    if (auto* bw = weight.weight<GwmBandwidthWeight>())
    {
        return GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(bw->kernel());
    }
    return tr("-");
}

QString GwmPropertySWIMTab::distanceDescription() const
{
    if (!mTaskThread) return tr("-");

    const GwmSpatialWeight spatialWeight = mTaskThread->spatialWeight();
    const GwmDistance* constDist = spatialWeight.distance();
    if (!constDist)
        return tr("-");

    GwmDistance* dist = const_cast<GwmDistance*>(constDist);
    switch (dist->type())
    {
    case GwmDistance::DistanceType::CRSDistance:
        if (const auto* crs = spatialWeight.distance<GwmCRSDistance>())
        {
            return crs->geographic()
            ? tr("Geographic CRS distance metric is used.")
            : tr("Planar CRS distance metric is used.");
        }
        break;
    case GwmDistance::DistanceType::MinkwoskiDistance:
        return tr("Minkowski distance metric is used.");
    case GwmDistance::DistanceType::DMatDistance:
        return tr("Distance matrix metric is used.");
    default:
        break;
    }
    return tr("Custom distance metric is used.");
}

QString GwmPropertySWIMTab::focusTypeDescription() const
{
    if (!mTaskThread)
        return tr("-");

    switch (mTaskThread->swimMode())
    {
    case SWIMMode::OriginFocused:
        return tr("Origin-focused (origin locations act as regression centers).");
    case SWIMMode::DestinationFocused:
        return tr("Destination-focused (destinations act as regression centers).");
    case SWIMMode::FlowFocusedEuclidean:
        return tr("Flow-focused (Euclidean midpoint is used).");
    case SWIMMode::FlowFocusedSOP:
        return tr("Flow-focused (sum of potentials midpoint is used).");
    default:
        break;
    }
    return tr("-");
}

void GwmPropertySWIMTab::adjustTableToContents(QTableWidget* table) const
{
    if (!table) return;
    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    constexpr int extraPadding = 16;
    for (int c = 0; c < table->columnCount(); ++c)
    {
        int currentWidth = table->columnWidth(c);
        table->setColumnWidth(c, currentWidth + extraPadding);
    }
}

GwmPropertySWIMTab::OlsResult GwmPropertySWIMTab::computeGlobalOls(bool& ok) const
{
    ok = false;
    OlsResult result;

    if (!mTaskThread)
        return result;

    const QList<GwmFlowData> flows = mTaskThread->flowData();
    if (flows.isEmpty())
        return result;

    const int indepCount = flows.first().independent_values.size();
    const int n = flows.size();
    const int k = indepCount + 1;
    if (n <= k)
        return result;

    mat X(n, k, fill::ones);
    vec y(n, fill::zeros);

    QStringList names;
    names << tr("Intercept");

    const QStringList mappingNames = mTaskThread->fieldMapping().independentVarNames;
    for (int idx = 0; idx < indepCount; ++idx)
    {
        if (idx < mappingNames.size())
            names << mappingNames[idx];
        else
            names << tr("Var %1").arg(idx + 1);
    }

    for (int i = 0; i < n; ++i)
    {
        const auto& flow = flows[i];
        y(i) = flow.flow_volume;
        for (int j = 0; j < indepCount; ++j)
        {
            double value = j < flow.independent_values.size() ? flow.independent_values[j] : 0.0;
            X(i, j + 1) = value;
        }
    }

    const mat pinvX = pinv(X);
    vec beta = pinvX * y;
    vec fitted = X * beta;
    vec residuals = y - fitted;
    double rss = dot(residuals, residuals);
    vec centeredY = y - mean(y);
    double tss = dot(centeredY, centeredY);
    double rSquared = (tss > 0.0) ? 1.0 - rss / tss : std::numeric_limits<double>::quiet_NaN();

    const int dof = n - k;
    if (dof <= 0)
        return result;

    double sigma2 = rss / static_cast<double>(dof);
    mat XtXInv = pinv(X.t() * X);
    vec stdErrors = sqrt(diagvec(XtXInv) * sigma2);
    vec tValues(beta.n_elem, fill::zeros);
    for (uword i = 0; i < beta.n_elem; ++i)
    {
        double se = stdErrors(i);
        tValues(i) = se > 0.0 ? beta(i) / se : std::numeric_limits<double>::quiet_NaN();
    }

    const double pi = 3.14159265358979323846;
    double sigma2Total = rss / static_cast<double>(n);
    double aic = n * std::log(sigma2Total) + n * (1.0 + std::log(2.0 * pi)) + 2.0 * k;
    double aicc = std::numeric_limits<double>::quiet_NaN();
    if (n - k - 1 != 0)
    {
        aicc = aic + (2.0 * k * (k + 1.0)) / (n - k - 1.0);
    }
    double adjRSquared = 1.0 - (1.0 - rSquared) * (n - 1.0) / (n - k);

    result.valid = true;
    result.names = names;
    result.estimates.reserve(beta.n_elem);
    result.stdErrors.reserve(stdErrors.n_elem);
    result.tValues.reserve(tValues.n_elem);

    for (uword i = 0; i < beta.n_elem; ++i)
    {
        result.estimates.append(beta(i));
        result.stdErrors.append(stdErrors(i));
        result.tValues.append(tValues(i));
    }

    result.rss = rss;
    result.aic = aic;
    result.aicc = aicc;
    result.rSquared = rSquared;
    result.adjRSquared = adjRSquared;
    result.valid = true;
    ok = true;
    return result;
}

QString GwmPropertySWIMTab::formatNumber(double value, int precision) const
{
    if (!std::isfinite(value))
        return tr("-");
    return QString::number(value, 'f', precision);
}

double GwmPropertySWIMTab::calculateQuantile(const QVector<double>& sortedValues, double quantile) const
{
    if (sortedValues.isEmpty() || quantile < 0.0 || quantile > 1.0)
        return std::numeric_limits<double>::quiet_NaN();

    if (quantile == 0.0)
        return sortedValues.first();
    if (quantile == 1.0)
        return sortedValues.last();

    const int n = sortedValues.size();
    const double pos = quantile * (n - 1);
    const int lower = static_cast<int>(std::floor(pos));
    const int upper = static_cast<int>(std::ceil(pos));

    if (lower == upper)
        return sortedValues[lower];

    const double weight = pos - lower;
    return sortedValues[lower] * (1.0 - weight) + sortedValues[upper] * weight;
}
void GwmPropertySWIMTab::on_btnSaveRes_clicked()
{
    if (!mTaskThread)
        return;

    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Save SWIM Results"),
                                                    QString(),
                                                    tr("Text Files (*.txt);;All Files (*.*)"));
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive))
    {
        filePath.append(QStringLiteral(".txt"));
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);

    auto writeSeparator = [&out]()
    {
        out << "*****************************************************" << Qt::endl;
    };

    auto writeLine = [&out](const QString& label, const QString& value)
    {
        out << label << "\t" << value << Qt::endl;
    };

    // Prepare OLS information
    bool hasOls = false;
    OlsResult olsResult;
    if (mHasCachedOls && mCachedOlsResult.valid)
    {
        olsResult = mCachedOlsResult;
        hasOls = true;
    }
    else
    {
        bool ok = false;
        olsResult = computeGlobalOls(ok);
        hasOls = ok && olsResult.valid;
    }

    // Global regression diagnostics
    writeSeparator();
    out << "  Results of Global Regression" << Qt::endl;
    writeSeparator();
    out << "Diagnostic Information" << Qt::endl;
    out << "----------------------------------------------" << Qt::endl;

    if (hasOls)
    {
        writeLine("AIC:", formatNumber(olsResult.aic));
        writeLine("AICc:", formatNumber(olsResult.aicc));
        writeLine("Residual sum of squares:", formatNumber(olsResult.rss));
        writeLine("R-squared value:", formatNumber(olsResult.rSquared));
        writeLine("Adjusted R-squared value:", formatNumber(olsResult.adjRSquared));
    }
    else
    {
        writeLine("AIC:", "-");
        writeLine("AICc:", "-");
        writeLine("Residual sum of squares:", "-");
        writeLine("R-squared value:", "-");
        writeLine("Adjusted R-squared value:", "-");
    }
    out << Qt::endl;

    out << "Summary of OLS Coefficient Estimates" << Qt::endl;
    out << "----------------------------------------------" << Qt::endl;
    out << "Name\tEstimate\tStd. Error\tt-value" << Qt::endl;
    if (hasOls && !olsResult.names.isEmpty())
    {
        const int rows = olsResult.names.size();
        for (int i = 0; i < rows; ++i)
        {
            const QString name = olsResult.names.value(i);
            const QString estimate = formatNumber(olsResult.estimates.value(i, std::numeric_limits<double>::quiet_NaN()));
            const QString stdErr = formatNumber(olsResult.stdErrors.value(i, std::numeric_limits<double>::quiet_NaN()));
            const QString tValue = formatNumber(olsResult.tValues.value(i, std::numeric_limits<double>::quiet_NaN()));
            out << name << "\t" << estimate << "\t" << stdErr << "\t" << tValue << Qt::endl;
        }
    }
    else
    {
        out << tr("No OLS summary is available.") << Qt::endl;
    }
    out << Qt::endl;

    // SWIM section
    writeSeparator();
    out << "  Results of SWIM" << Qt::endl;
    writeSeparator();
    out << "Model Calibration Information" << Qt::endl;
    out << "----------------------------------------------" << Qt::endl;
    writeLine("Focus type:", focusTypeDescription());
    writeLine("Kernel function:", kernelDescription());

    QString adaptiveText = tr("-");
    const GwmSpatialWeight weight = mTaskThread->spatialWeight();
    if (auto* bw = weight.weight<GwmBandwidthWeight>())
    {
        if (bw->adaptive())
        {
            const int neighbours = static_cast<int>(std::round(std::max(0.0, bw->bandwidth())));
            adaptiveText = tr("Adaptive (%1 nearest neighbours)").arg(neighbours);
        }
        else
        {
            adaptiveText = tr("Fixed bandwidth (%1)").arg(QString::number(bw->bandwidth(), 'f', 4));
        }
    }
    writeLine("Adaptive:", adaptiveText);
    writeLine("Regression points:", tr("The same location as observations are used."));
    writeLine("Distance metric:", distanceDescription());
    out << Qt::endl;

    out << "Summary of SWIM Coefficient Estimates" << Qt::endl;
    out << "----------------------------------------------" << Qt::endl;
    out << "Name\tMin\t1st Qu.\tMedian\t3rd Qu.\tMax" << Qt::endl;

    auto coefficientsList = mTaskThread->resultList();
    arma::mat coefficients;
    for (const auto& pair : coefficientsList)
    {
        if (pair.first == QStringLiteral("Coefficients"))
        {
            coefficients = pair.second;
            break;
        }
    }

    if (coefficients.n_rows > 0 && coefficients.n_cols > 0)
    {
        QStringList names;
        names << tr("Intercept");
        const QStringList mappingNames = mTaskThread->fieldMapping().independentVarNames;
        const int numIndepVars = static_cast<int>(coefficients.n_cols) - 1;
        for (int idx = 0; idx < numIndepVars; ++idx)
        {
            if (idx < mappingNames.size())
                names << mappingNames[idx];
            else
                names << tr("Var %1").arg(idx + 1);
        }

        const int numCoeffs = static_cast<int>(coefficients.n_cols);
        for (int col = 0; col < numCoeffs; ++col)
        {
            QVector<double> values;
            values.reserve(coefficients.n_rows);
            for (uword r = 0; r < coefficients.n_rows; ++r)
            {
                const double val = coefficients(r, col);
                if (std::isfinite(val))
                    values.append(val);
            }
            if (values.isEmpty())
            {
                out << names.value(col, tr("Var %1").arg(col)) << "\t-\t-\t-\t-\t-" << Qt::endl;
                continue;
            }

            std::sort(values.begin(), values.end());
            const auto writeCoeffLine = [&](const QVector<double>& sortedVals)
            {
                const QString minStr = formatNumber(sortedVals.first());
                const QString maxStr = formatNumber(sortedVals.last());
                const QString q1Str = formatNumber(calculateQuantile(sortedVals, 0.25));
                const QString medianStr = formatNumber(calculateQuantile(sortedVals, 0.5));
                const QString q3Str = formatNumber(calculateQuantile(sortedVals, 0.75));
                out << names.value(col, tr("Var %1").arg(col)) << "\t"
                    << minStr << "\t"
                    << q1Str << "\t"
                    << medianStr << "\t"
                    << q3Str << "\t"
                    << maxStr << Qt::endl;
            };
            writeCoeffLine(values);
        }
    }
    else
    {
        out << tr("No SWIM coefficient summary is available.") << Qt::endl;
    }
    out << Qt::endl;

    out << "Diagnostic Information" << Qt::endl;
    out << "----------------------------------------------" << Qt::endl;
    const GwmSWIMDiagnostics diag = mTaskThread->diagnostics();
    writeLine("Number of data points:", diag.dataPoints > 0 ? QString::number(diag.dataPoints) : tr("-"));
    writeLine("Effective number of parameters:", formatNumber(diag.effectiveParameters));
    writeLine("Effective degrees of freedom:", formatNumber(diag.effectiveDof));
    writeLine("AIC (Poisson SWIM):", formatNumber(diag.aic));
    writeLine("AICc (Nakaya 2005 Poisson):", formatNumber(diag.aicc));
    writeLine("Deviance:", formatNumber(diag.deviance));
    writeLine("Pseudo R-square (McFadden):", formatNumber(diag.rSquared));
    writeLine("Adjusted pseudo R-square (McFadden):", formatNumber(diag.adjRSquared));
    out << Qt::endl;

    writeSeparator();
    out << "GWmodel Lab\t" << "http://gwmodel.whu.edu.cn/" << Qt::endl;
    out << "Contact us\t" << "binbinlu@whu.edu.cn" << Qt::endl;

    file.close();
    QMessageBox::information(this, tr("Success"), tr("SWIM results exported to: %1").arg(filePath));
}

void GwmPropertySWIMTab::on_btnExportWeightMatrix_clicked()
{
    if (!mTaskThread)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("导出权重矩阵"), "", tr("CSV文件 (*.csv)"));
    if (fileName.isEmpty())
        return;

    mat weightMatrix = mTaskThread->weightMatrix();
    if (weightMatrix.n_rows == 0 || weightMatrix.n_cols == 0)
    {
        QMessageBox::warning(this, tr("Error"), tr("Weight matrix is empty."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);

    for (uword i = 0; i < weightMatrix.n_rows; i++)
    {
        for (uword j = 0; j < weightMatrix.n_cols; j++)
        {
            if (j > 0) out << ",";
            out << weightMatrix(i, j);
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Weight matrix exported to: %1").arg(fileName));
}


