#ifndef GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H
#define GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H

#include <QObject>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/iregressionanalysis.h"
#include "TaskThread/gwmgwrtaskthread.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"

using namespace arma;

class GwmGeographicalWeightedRegressionAlgorithm : public GwmSpatialMonoscaleAlgorithm, public IRegressionAnalysis, public IBandwidthSizeSelectable, public IIndependentVariableSelectable
{
    Q_OBJECT

    enum BandwidthSelectionCriterionType
    {
        AIC,
        CV
    };

    typedef double (GwmGeographicalWeightedRegressionAlgorithm::*BandwidthSizeCriterionFunction)(GwmBandwidthWeight*);

public:
    inline static vec Fitted(const mat& x, const mat& betas)
    {
        return sum(betas % x, 1);
    }

    inline static double RSS(const mat& x, const mat& y, const mat& betas)
    {
        vec r = y - Fitted(x, betas);
        return sum(r % r);
    }

    inline static double AICc(const mat& x, const mat& y, const mat& betas, const vec& shat)
    {
        double ss = RSS(x, y, betas), n = x.n_rows;
        return n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    }

public:     // 构造和属性
    GwmGeographicalWeightedRegressionAlgorithm();

    mat betas() const;
    void setBetas(const mat &betas);

    bool autoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool value);

    bool autoselectIndepVars() const;
    void setIsAutoselectIndepVars(bool value);

    double indepVarSelectionThreshold() const;
    void setIndepVarSelectionThreshold(double indepVarSelectionThreshold);


protected:  // QThread interface
    void run() override;


public:     // GwmTaskThread interface
    QString name() const override { return tr("GWR"); };


protected:  // GwmSpatialAlgorithm interface
    bool isValid() override;


public:     // IBandwidthSizeSelectable interface
    inline double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSizeCriterion)(bandwidthWeight);
    }

public:     // IIndependentVariableSelectable interface
    inline double criterion(QList<GwmVariable> indepVars) override;


public:     // IRegressionAnalysis interface
    inline GwmVariable dependentVariable() const override
    {
        return mDepVar;
    }

    inline void setDependentVariable(const GwmVariable &variable) override
    {
        mDepVar = variable;
    }

    inline void setDependentVariable(const GwmVariable &&variable) override
    {
        mDepVar = variable;
    }

    inline QList<GwmVariable> independentVariables() const override
    {
        return mIndepVars;
    }

    inline void setIndependentVariables(const QList<GwmVariable> &variables) override
    {
        mIndepVars = variables;
    }

    inline void setIndependentVariables(const QList<GwmVariable> &&variables) override
    {
        mIndepVars = variables;
    }

    inline GwmDiagnostic diagnostic() const override
    {
        return mDiagnostic;
    }

    mat regression(const mat& x, const vec& y) override;

protected:
    mat regression(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);

    inline bool hasRegressionLayer()
    {
        return mRegressionLayer != nullptr;
    }

    inline bool isStoreS()
    {
        return hasHatMatrix && (mDataPoints.n_rows < 8192);
    }

private:
    void initPoints();
    void initXY(mat& x, mat& y, const GwmVariable& depVar, const QList<GwmVariable>& indepVars);
    GwmDiagnostic calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);
    void createResultLayer(QList<QPair<QString, const mat> > data);
    double bandwidthSizeCriterionAIC(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionCV(GwmBandwidthWeight* bandwidthWeight);

protected:
    QgsVectorLayer* mRegressionLayer = nullptr;
    mat mDataPoints;
    mat mRegressionPoints;

    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;

    GwmDiagnostic mDiagnostic;
    GwmFTestResult mF1TestResult;
    GwmFTestResult mF2TestResult;
    QList<GwmFTestResult> mF3TestResult;
    GwmFTestResult mF4TestResult;

    bool hasHatMatrix = true;
    bool hasFTest = false;

    mat mX;
    vec mY;
    mat mBetas;

private:
    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    bool isAutoselectBandwidth;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::AIC;
    BandwidthSizeCriterionFunction mBandwidthSizeCriterion;

    GwmIndependentVariableSelector mIndepVarSelector;
    bool isAutoselectIndepVars;
    double mIndepVarSelectionThreshold = 3.0;

};

#endif // GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H
