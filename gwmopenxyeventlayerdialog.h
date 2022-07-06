#ifndef GWMOPENXYEVENTLAYERDIALOG_H
#define GWMOPENXYEVENTLAYERDIALOG_H

#include "prefix.h"
#include <QDialog>
#include <qgsprojectionselectionwidget.h>

#include "DelimitedText/qgsdelimitedtextfile.h"

namespace Ui {
class GwmOpenXYEventLayerDialog;
}

class GwmOpenXYEventLayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmOpenXYEventLayerDialog(QWidget *parent = nullptr);
    ~GwmOpenXYEventLayerDialog();

signals:
    void enableButtons(bool enable);
    void addVectorLayerSignal( const QString &uri, const QString &layerName, const QString &providerKey = QString() );

private slots:
    void onFormatCSVRadioToggled(bool checked);

    void onFormatRegRadioToggled(bool checked);

    void onFormatCustomRadioToggled(bool checked);

    void onFileNameOpenFileDialogBtnClicked();

    void accept() override;

    void onFormatCustomDelimiterOthersCheckToggled(bool checked);

private:
    Ui::GwmOpenXYEventLayerDialog *ui;
    std::unique_ptr<QgsDelimitedTextFile> mFile;
    QgsProjectionSelectionWidget* mCRSSelector;

    int mExampleRowCount = 20;
    int mBadRowCount = 0;

    void updateFieldsAndEnable();
    void updateFieldLists();
    void enableAccept();
    void updateFileName(const QString &text);
    bool loadDelimitedFileDefinition();
    bool validate();

    QString selectedChars();
    void setSelectedChars(const QString &delimiters);
    bool trySetXYField( QStringList &fields, QList<bool> &isValidNumber, const QString &xname, const QString &yname );
};

#endif // GWMOPENXYEVENTLAYERDIALOG_H
