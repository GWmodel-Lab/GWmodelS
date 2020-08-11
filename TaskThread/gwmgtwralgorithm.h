#ifndef GWMGTWRALGORITHM_H
#define GWMGTWRALGORITHM_H

#include <QObject>

#include "TaskThread/gwmspatialtemporalmonoscale.h"
#include "TaskThread/iregressionanalysis.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

class GwmGTWRAlgorithm : public GwmSpatialTemporalMonoscaleAlgorithm, public IRegressionAnalysis, public IBandwidthSizeSelectable
{
    Q_OBJECT

public:
    enum BandwidthSelectionCriterionType
    {
        AIC,
        CV
    };

public:
    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    static vec Fitted(const mat& x, const mat& betas)
    {
        return sum(betas % x, 1);
    }

    static double RSS(const mat& x, const mat& y, const mat& betas)
    {
        vec r = y - Fitted(x, betas);
        return sum(r % r);
    }

    static double AICc(const mat& x, const mat& y, const mat& betas, const vec& shat)
    {
        double ss = RSS(x, y, betas), n = x.n_rows;
        return n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    }

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

public:
    GwmGTWRAlgorithm();

    bool hasHatMatrix() const;
    void setHasHatMatrix(bool hasHatMatrix);

    GwmVariable timeVar() const;
    void setTimeVar(const GwmVariable &timeVar);

    QgsVectorLayer *regressionLayer() const;
    void setRegressionLayer(QgsVectorLayer *regressionLayer);

    bool isAutoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool isAutoselectBandwidth);

    bool hasPredict() const;
    void setHasPredict(bool hasPredict);

    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    mat betas() const;

    BandwidthCriterionList bandwidthSelectorCriterions() const;

    // QThread interface
protected:
    void run() override;

    // GwmSpatialAlgorithm interface
public:
    bool isValid() override;

    // IRegressionAnalysis interface
public:
    GwmVariable dependentVariable() const override;
    void setDependentVariable(const GwmVariable &variable) override;
    QList<GwmVariable> independentVariables() const override;
    void setIndependentVariables(const QList<GwmVariable> &variables) override;
    GwmDiagnostic diagnostic() const override;
    arma::mat regression(const arma::mat &x, const arma::vec &y) override;

    // IBandwidthSizeSelectable interface
public:
    double criterion(GwmBandwidthWeight *weight) override;

public:
    bool hasRegressionLayer();

protected:
    virtual void initPoints();
    virtual void initXY(mat& x, mat& y, const GwmVariable& depVar, const QList<GwmVariable>& indepVars);

private:
    mat regressionSerial(const mat& x, const vec& y);
    mat regressionHatmatrixSerial(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag);

    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* bandwidthWeight);

    bool isStoreS()
    {
        return mHasHatMatrix && (mDataPoints.n_rows < 8192);
    }

    void createResultLayer(CreateResultLayerData data, QString name = QStringLiteral("_GWR"));

protected:
    QgsVectorLayer* mRegressionLayer = nullptr;
    mat mDataPoints;
    mat mRegressionPoints;
    vec mDataTimeStamp;
    vec mRegTimeStamp;

    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmVariable mTimeVar;

    GwmDiagnostic mDiagnostic;

    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    bool mIsAutoselectBandwidth = false;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = CV;

    mat mX;
    vec mY;
    mat mBetas;
    mat mBetasSE;
    vec mSHat;
    vec mQDiag;

    bool mHasRegressionLayerXY = false;
    bool mHasPredict = false;
    vec mRegressionLayerY;
    mat mRegressionLayerX;

    bool mHasHatMatrix = true;
};

inline GwmVariable GwmGTWRAlgorithm::dependentVariable() const
{
    return mDepVar;
}

inline void GwmGTWRAlgorithm::setDependentVariable(const GwmVariable &variable)
{
    mDepVar = variable;
}

inline QList<GwmVariable> GwmGTWRAlgorithm::independentVariables() const
{
    return mIndepVars;
}

inline void GwmGTWRAlgorithm::setIndependentVariables(const QList<GwmVariable> &variables)
{
    mIndepVars = variables;
}

inline GwmDiagnostic GwmGTWRAlgorithm::diagnostic() const
{
    return mDiagnostic;
}

inline bool GwmGTWRAlgorithm::hasRegressionLayer()
{
    return mRegressionLayer != nullptr;
}

inline GwmGTWRAlgorithm::BandwidthSelectionCriterionType GwmGTWRAlgorithm::bandwidthSelectionCriterionType() const
{
    return mBandwidthSelectionCriterionType;
}

inline void GwmGTWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
}

inline bool GwmGTWRAlgorithm::hasPredict() const
{
    return mHasPredict;
}

inline void GwmGTWRAlgorithm::setHasPredict(bool hasPredict)
{
    mHasPredict = hasPredict;
}

inline bool GwmGTWRAlgorithm::isAutoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

inline void GwmGTWRAlgorithm::setIsAutoselectBandwidth(bool isAutoselectBandwidth)
{
    mIsAutoselectBandwidth = isAutoselectBandwidth;
}

inline QgsVectorLayer *GwmGTWRAlgorithm::regressionLayer() const
{
    return mRegressionLayer;
}

inline void GwmGTWRAlgorithm::setRegressionLayer(QgsVectorLayer *regressionLayer)
{
    mRegressionLayer = regressionLayer;
}

inline GwmVariable GwmGTWRAlgorithm::timeVar() const
{
    return mTimeVar;
}

inline void GwmGTWRAlgorithm::setTimeVar(const GwmVariable &timeVar)
{
    mTimeVar = timeVar;
}

inline bool GwmGTWRAlgorithm::hasHatMatrix() const
{
    return mHasHatMatrix;
}

inline void GwmGTWRAlgorithm::setHasHatMatrix(bool hasHatMatrix)
{
    mHasHatMatrix = hasHatMatrix;
}

inline double GwmGTWRAlgorithm::criterion(GwmBandwidthWeight *weight)
{
    return bandwidthSizeCriterionCVSerial(weight);
}

inline mat GwmGTWRAlgorithm::betas() const
{
    return mBetas;
}

inline BandwidthCriterionList GwmGTWRAlgorithm::bandwidthSelectorCriterions() const
{
    return mBandwidthSizeSelector.bandwidthCriterion();
}



#endif // GWMGTWRALGORITHM_H
