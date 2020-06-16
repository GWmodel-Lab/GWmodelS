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

public:     // 构造和属性
    GwmGeographicalWeightedRegressionAlgorithm();

    mat betas() const;
    void setBetas(const mat &betas);


public:     // GwmTaskThread interface
    QString name() const override { return tr("GWR"); };


public:  // GwmSpatialAlgorithm interface
    bool isValid() override;


public:     // IRegressionAnalysis interface
    inline GwmVariable dependentVariable() const override
    {
        return mDepVar;
    }

    inline void setDependentVariable(const GwmVariable &variable) override
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

    inline GwmDiagnostic diagnostic() const override
    {
        return mDiagnostic;
    }

public:

    inline bool hasRegressionLayer()
    {
        return mRegressionLayer != nullptr;
    }

protected:
    virtual void initPoints();
    virtual void initXY(mat& x, mat& y, const GwmVariable& depVar, const QList<GwmVariable>& indepVars);

protected:
    QgsVectorLayer* mRegressionLayer = nullptr;
    mat mDataPoints;
    mat mRegressionPoints;

    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;

    mat mX;
    vec mY;
    mat mBetas;

    GwmDiagnostic mDiagnostic;
};

inline mat GwmGeographicalWeightedRegressionAlgorithm::betas() const
{
    return mBetas;
}

inline void GwmGeographicalWeightedRegressionAlgorithm::setBetas(const mat &betas)
{
    mBetas = betas;
}

#endif // GWMGEOGRAPHICALWEIGHTEDREGRESSIONALGORITHM_H
