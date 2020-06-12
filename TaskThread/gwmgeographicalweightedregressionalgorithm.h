#ifndef GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H
#define GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H

#include <QObject>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/iregressionanalysis.h"
#include "TaskThread/gwmgwrtaskthread.h"

using namespace arma;

class GwmGeographicalWeightedRegressionAlgorithm : public GwmSpatialMonoscaleAlgorithm, public IRegressionAnalysis
{
    Q_OBJECT

public:
    inline static vec Fitted(const mat& x, const mat& betas)
    {
        return sum(betas % x, 1);
    }

public:     // 构造和属性
    GwmGeographicalWeightedRegressionAlgorithm();

    mat betas() const;
    void setBetas(const mat &betas);


protected:  // QThread interface
    void run();


public:     // GwmTaskThread interface
    QString name() const override { return tr("GWR"); };


protected:  // GwmSpatialAlgorithm interface
    bool isValid();


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
    void init(mat& x, mat& y);
    GwmDiagnostic calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);
    void createResultLayer(QList<QPair<QString, const mat&> > data);

protected:
    QgsVectorLayer* mRegressionLayer;
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

    mat mBetas;

};

#endif // GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H
