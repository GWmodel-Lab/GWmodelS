#ifndef GWMFLOWDATADIALOG_H
#define GWMFLOWDATADIALOG_H

#include <QDialog>
#include <QStringList>
#include <QSet>

class QLineEdit;
class QComboBox;
class QListWidget;
class QLabel;
class QDialogButtonBox;
class QToolButton;
class QgsProjectionSelectionWidget;
class QgsVectorLayer;

namespace Ui {
class GwmFlowDataDialog;
}

class GwmFlowDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmFlowDataDialog(QWidget* parent = nullptr);
    ~GwmFlowDataDialog() override;

    QgsVectorLayer* takeResultLayer();

private slots:
    void onBrowseClicked();
    void onFilePathChanged(const QString& path);
    void onEncodingChanged(int index);
    void onDelimiterChanged(int index);
    void onFieldSelectionChanged();
    void onDialogAccepted();

private:
    void connectSignals();
    void clearFieldControls();
    void updateHeaders();
    void populateFieldCombos();
    void updateStatus(const QString& message);
    void updateAcceptButton();
    QChar currentDelimiter(const QString& sampleLine = QString()) const;
    QChar detectDelimiter(const QString& line) const;
    QString sanitizeFieldName(const QString& source,
                              QSet<QString>& existingNames) const;
    bool buildLayer();
    bool parseDouble(const QStringList& values, int index, double& target) const;
    int comboColumnIndex(QComboBox* combo) const;

private:
    Ui::GwmFlowDataDialog* ui = nullptr;
    QLineEdit* mFilePathEdit = nullptr;
    QToolButton* mBrowseButton = nullptr;
    QLineEdit* mLayerNameEdit = nullptr;
    QComboBox* mEncodingCombo = nullptr;
    QComboBox* mDelimiterCombo = nullptr;
    QgsProjectionSelectionWidget* mCrsSelector = nullptr;

    QComboBox* mFlowVolumeCombo = nullptr;
    QComboBox* mOriginXCombo = nullptr;
    QComboBox* mOriginYCombo = nullptr;
    QComboBox* mOriginZCombo = nullptr;
    QComboBox* mOriginMCombo = nullptr;
    QComboBox* mDestXCombo = nullptr;
    QComboBox* mDestYCombo = nullptr;
    QComboBox* mDestZCombo = nullptr;
    QComboBox* mDestMCombo = nullptr;
    QComboBox* mOriginValueCombo = nullptr;
    QComboBox* mDestValueCombo = nullptr;

    QLabel* mStatusLabel = nullptr;
    QDialogButtonBox* mButtonBox = nullptr;

    QStringList mHeaders;
    QgsVectorLayer* mResultLayer = nullptr;
};

#endif // GWMFLOWDATADIALOG_H


