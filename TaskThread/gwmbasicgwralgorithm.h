#ifndef GWMBASICGWRALGORITHM_H
#define GWMBASICGWRALGORITHM_H

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"


class GwmBasicGWRAlgorithm : public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable, public IIndependentVariableSelectable
{
    Q_OBJECT

    enum BandwidthSelectionCriterionType
    {
        AIC,
        CV
    };

    typedef double (GwmBasicGWRAlgorithm::*BandwidthSizeCriterionFunction)(GwmBandwidthWeight*);

public:
    GwmBasicGWRAlgorithm();

    bool autoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool value);

    bool autoselectIndepVars() const;
    void setIsAutoselectIndepVars(bool value);

    double indepVarSelectionThreshold() const;
    void setIndepVarSelectionThreshold(double indepVarSelectionThreshold);

    QgsVectorLayer* regressionLayer() const;
    void setRegressionLayer(QgsVectorLayer* layer);


public:     // GwmTaskThread interface
    QString name() const override { return tr("Basic GWR"); };


protected:  // GwmSpatialAlgorithm interface
    bool isValid() override;


protected:  // QThread interface
    void run() override;


public:     // IBandwidthSizeSelectable interface
    inline double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSizeCriterion)(bandwidthWeight);
    }

public:     // IIndependentVariableSelectable interface
    inline double criterion(QList<GwmVariable> indepVars) override;


protected:  // IRegressionAnalysis interface
    mat regression(const mat& x, const vec& y) override;

protected:
    mat regression(const mat& x, const vec& y, mat& betasSE, vec& shat, vec& qDiag, mat& S);


protected:
    GwmDiagnostic calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);
    void createResultLayer(QList<QPair<QString, const mat> > data);
    double bandwidthSizeCriterionAIC(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionCV(GwmBandwidthWeight* bandwidthWeight);

    inline bool isStoreS()
    {
        return hasHatMatrix && (mDataPoints.n_rows < 8192);
    }


protected:
    GwmFTestResult mF1TestResult;
    GwmFTestResult mF2TestResult;
    QList<GwmFTestResult> mF3TestResult;
    GwmFTestResult mF4TestResult;


private:
    bool hasHatMatrix = true;
    bool hasFTest = false;

    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    bool isAutoselectBandwidth;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::AIC;
    BandwidthSizeCriterionFunction mBandwidthSizeCriterion;

    GwmIndependentVariableSelector mIndepVarSelector;
    bool isAutoselectIndepVars;
    double mIndepVarSelectionThreshold = 3.0;
};

#endif // GWMBASICGWRALGORITHM_H
