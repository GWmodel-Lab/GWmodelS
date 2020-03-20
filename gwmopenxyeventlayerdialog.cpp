#include "gwmopenxyeventlayerdialog.h"
#include "ui_gwmopenxyeventlayerdialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <qgsvectordataprovider.h>
#include <qgsproviderregistry.h>
#include <DelimitedText/qgsdelimitedtextprovider.h>

const int MAX_SAMPLE_LENGTH = 200;

GwmOpenXYEventLayerDialog::GwmOpenXYEventLayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmOpenXYEventLayerDialog),
    mFile(qgis::make_unique<QgsDelimitedTextFile>())
{
    ui->setupUi(this);
    mCRSSelector = new QgsProjectionSelectionWidget(this);
    ui->geometryCRSSettingLayout->addWidget(mCRSSelector);

    // 字符编码选择框
    ui->encodingCombo->clear();
    ui->encodingCombo->addItems(QgsVectorDataProvider::availableEncodings());
    ui->encodingCombo->setCurrentIndex(ui->encodingCombo->findText(QStringLiteral("UTF-8")));

    // 文件格式配置区
    connect(ui->formatCSVRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled);
    connect(ui->formatRegRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatRegRadioToggled);
    connect(ui->formatCustomRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled);

    updateFieldsAndEnable();

    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::enableAccept);
    connect(ui->fileNameOpenFileDialogBtn, &QToolButton::clicked, this, &GwmOpenXYEventLayerDialog::onFileNameOpenFileDialogBtnClicked);
    connect(ui->encodingCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->formatCSVRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatRegRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->formatCustomDelimiterCommaCheck, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterSpaceCheck, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterTabCheck, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterSemicolonCheck, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterColonCheck, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->formatCustomDelimiterOthersCheck, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterOthersCheck, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCustomDelimiterOthersCheckToggled);
    connect(ui->formatCustomDelimiterOthersEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomEscapeEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomQuoteEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->recordNumberDiscardLinesSpin, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordFirstLineAsFieldNames, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordDiscardEmptyFields, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordTrimFields, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordDecimalSepComma, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );

    connect(ui->geometryDMSCoord, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(mCRSSelector, &QgsProjectionSelectionWidget::crsChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );

    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFileName);
    // 设置对话框属性
    this->setAttribute(Qt::WA_DeleteOnClose);
    connect(this, &GwmOpenXYEventLayerDialog::enableButtons, ui->dialogBtn->buttons()[0], &QPushButton::setEnabled);
}

GwmOpenXYEventLayerDialog::~GwmOpenXYEventLayerDialog()
{
    delete ui;
}

void GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled(bool checked)
{
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(0);
    }
}

void GwmOpenXYEventLayerDialog::onFormatRegRadioToggled(bool checked)
{
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(1);
    }
}

void GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled(bool checked)
{
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(2);
    }
}

void GwmOpenXYEventLayerDialog::updateFieldsAndEnable()
{
    this->updateFieldLists();
    this->enableAccept();
}

void GwmOpenXYEventLayerDialog::accept()
{

    // The following conditions should not be hit! OK will not be enabled...
    if ( ui->layerNameEdit->text().isEmpty() )
    {
      QMessageBox::warning( this, tr( "No layer name" ), tr( "Please enter a layer name before adding the layer to the map" ) );
      ui->layerNameEdit->setFocus();
      return;
    }
    if ( ui->formatCustomRadio->isChecked() )
    {
      if ( selectedChars().isEmpty() )
      {
        QMessageBox::warning( this, tr( "No delimiters set" ), tr( "Use one or more characters as the delimiter, or choose a different delimiter type" ) );
        ui->formatCustomDelimiterOthersEdit->setFocus();
        return;
      }
    }
    if ( ui->formatRegRadio->isChecked() )
    {
      QRegExp re( ui->formatExpressionEdit->text() );
      if ( ! re.isValid() )
      {
        QMessageBox::warning( this, tr( "Invalid regular expression" ), tr( "Please enter a valid regular expression as the delimiter, or choose a different delimiter type" ) );
        ui->formatExpressionEdit->setFocus();
        return;
      }
    }
    if ( ! mFile->isValid() )
    {
      QMessageBox::warning( this, tr( "Invalid delimited text file" ), tr( "Please enter a valid file and delimiter" ) );
      return;
    }

    //Build the delimited text URI from the user provided information

    QUrl url = mFile->url();
    QUrlQuery query( url );

    query.addQueryItem( QStringLiteral( "detectTypes" ), ui->recordDetectFieldTypes->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );

    if ( ui->recordDecimalSepComma->isChecked() )
    {
      query.addQueryItem( QStringLiteral( "decimalPoint" ), QStringLiteral( "," ) );
    }
    if ( ui->geometryDMSCoord->isChecked() )
    {
      query.addQueryItem( QStringLiteral( "xyDms" ), QStringLiteral( "yes" ) );
    }

    bool haveGeom = true;
//    if (geomTypeXY->isChecked())
    if (true)
    {
      QString field;
      if ( !ui->geometryXField->currentText().isEmpty() && !ui->geometryYField->currentText().isEmpty() )
      {
        field = ui->geometryXField->currentText();
        query.addQueryItem( QStringLiteral( "xField" ), field );
        field = ui->geometryYField->currentText();
        query.addQueryItem( QStringLiteral( "yField" ), field );
      }
      if ( !ui->geometryZField->currentText().isEmpty() )
      {
        field = ui->geometryZField->currentText();
        query.addQueryItem( QStringLiteral( "zField" ), field );
      }
      if ( !ui->geometryMField->currentText().isEmpty() )
      {
        field = ui->geometryMField->currentText();
        query.addQueryItem( QStringLiteral( "mField" ), field );
      }
    }
//    else if ( geomTypeWKT->isChecked() )
//    {
//      if ( ! cmbWktField->currentText().isEmpty() )
//      {
//        QString field = cmbWktField->currentText();
//        query.addQueryItem( QStringLiteral( "wktField" ), field );
//      }
//      if ( cmbGeometryType->currentIndex() > 0 )
//      {
//        query.addQueryItem( QStringLiteral( "geomType" ), cmbGeometryType->currentText() );
//      }
//    }
//    else
//    {
//      haveGeom = false;
//      query.addQueryItem( QStringLiteral( "geomType" ), QStringLiteral( "none" ) );
//    }
    if ( haveGeom )
    {
      QgsCoordinateReferenceSystem crs = mCRSSelector->crs();
      if ( crs.isValid() )
      {
        query.addQueryItem( QStringLiteral( "crs" ), crs.authid() );
      }

    }

//    if ( ! geomTypeNone->isChecked() )
    if ( true )
    {
      query.addQueryItem( QStringLiteral( "spatialIndex" ), ui->layerUseSpatialIndexCombo->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );
    }

    query.addQueryItem( QStringLiteral( "subsetIndex" ), ui->layerUseSubsetIndexCombo->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );
    query.addQueryItem( QStringLiteral( "watchFile" ), ui->layerWatchFileCombo->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );

    url.setQuery( query );
    // store the settings
//    saveSettings();
//    saveSettingsForFile( mFileWidget->filePath() );


    // add the layer to the map
    emit addVectorLayerSignal( QString::fromLatin1( url.toEncoded() ), ui->layerNameEdit->text(), "delimitedtext" );

    // clear the file and layer name show something has happened, ready for another file

    ui->fileNameEdit->setText(QString());
    ui->layerNameEdit->setText( QString() );

    QDialog::accept();
}

void GwmOpenXYEventLayerDialog::updateFieldLists()
{
    qDebug() << "[GwmOpenXYEventLayerDialog::updateFieldLists]";
    disconnect(ui->geometryXField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &GwmOpenXYEventLayerDialog::enableAccept );
    disconnect(ui->geometryYField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &GwmOpenXYEventLayerDialog::enableAccept );

    QString columnX = ui->geometryXField->currentText();
    QString columnY = ui->geometryYField->currentText();
    QString columnZ = ui->geometryZField->currentText();
    QString columnM = ui->geometryMField->currentText();

    ui->geometryXField->clear();
    ui->geometryYField->clear();
    ui->geometryZField->clear();
    ui->geometryMField->clear();

    if (!loadDelimitedFileDefinition())
        return;

    QList<bool> isValidCoordinate;
    QList<bool> isEmpty;
    int counter = 0;
    int nColumn = 0;
    mBadRowCount = 0;
    QStringList values;

    while (counter < mExampleRowCount)
    {
        QgsDelimitedTextFile::Status status = mFile->nextRecord(values);
        if (status == QgsDelimitedTextFile::RecordEOF) break;
        if (status == QgsDelimitedTextFile::RecordOk)
        {
            counter++;
            int nv = values.size();
            while (nv > 0 && values[nv-1].isEmpty()) {
                nv--;
            }
            if (isEmpty.size() < nv)
            {
                while (isEmpty.size() < nv)
                {
                    isEmpty.append(true);
                    isValidCoordinate.append(false);
                }
                nColumn = nv;
            }
            bool xyDms = ui->geometryDMSCoord->isChecked();
            for (int i = 0; i < nColumn; ++i)
            {
                QString value = i < nv ? values[i] : QString();
                if ( value.length() > MAX_SAMPLE_LENGTH )
                    value = value.mid( 0, MAX_SAMPLE_LENGTH ) + QChar( 0x2026 );
                if (!value.isEmpty())
                {
                    if (isEmpty[i])
                    {
                        isEmpty[i] = false;
                        isValidCoordinate[i] = true;
                    }
                    if (isValidCoordinate[i])
                    {
                        bool ok = true;
                        if (ui->recordDecimalSepComma->isChecked() )
                        {
                            value.replace( ',', '.' );
                        }
                        if ( xyDms )
                        {
                            ok = QgsDelimitedTextProvider::sCrdDmsRegexp.indexIn( value ) == 0;
                        }
                        else
                        {
                            value.toDouble( &ok );
                        }
                        isValidCoordinate[i] = ok;
                    }
                }
            }
        }
        else
        {
            mBadRowCount++;
        }
    }

    QStringList fieldList = mFile->fieldNames();
    if (isEmpty.size() < fieldList.size())
    {
        while ( isEmpty.size() < fieldList.size() )
        {
            isEmpty.append( true );
            isValidCoordinate.append( false );
        }
    }

    int fieldNo = 0;
    for (int i = 0; i < fieldList.size(); ++i)
    {
        QString field = fieldList[i];
        if (!field.isEmpty())
        {
            ui->geometryXField->addItem(field);
            ui->geometryYField->addItem(field);
            ui->geometryZField->addItem(field);
            ui->geometryMField->addItem(field);
            fieldNo++;
        }
    }

    ui->geometryXField->setCurrentIndex(ui->geometryXField->findText(columnX));
    ui->geometryYField->setCurrentIndex(ui->geometryYField->findText(columnY));
    ui->geometryZField->setCurrentIndex(ui->geometryZField->findText(columnZ));
    ui->geometryMField->setCurrentIndex(ui->geometryMField->findText(columnM));

    trySetXYField(fieldList, isValidCoordinate, QStringLiteral( "longitude" ), QStringLiteral( "latitude" ) );
    trySetXYField(fieldList, isValidCoordinate, QStringLiteral( "lon" ), QStringLiteral( "lat" ) );
    trySetXYField(fieldList, isValidCoordinate, QStringLiteral( "east" ), QStringLiteral( "north" ) );
    trySetXYField(fieldList, isValidCoordinate, QStringLiteral( "x" ), QStringLiteral( "y" ) );
    trySetXYField(fieldList, isValidCoordinate, QStringLiteral( "e" ), QStringLiteral( "n" ) );

    bool haveFields = fieldNo > 0;
    if (haveFields)
    {
        connect(ui->geometryXField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &GwmOpenXYEventLayerDialog::enableAccept );
        connect(ui->geometryYField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &GwmOpenXYEventLayerDialog::enableAccept );
    }

}

void GwmOpenXYEventLayerDialog::enableAccept()
{
    emit enableButtons(validate());
}

void GwmOpenXYEventLayerDialog::updateFileName(const QString &text)
{
    QString fileName = text;
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists())
    {
        ui->layerNameEdit->setText(fileInfo.completeBaseName());
        updateFieldsAndEnable();
    }
}

bool GwmOpenXYEventLayerDialog::loadDelimitedFileDefinition()
{
    mFile->setFileName(ui->fileNameEdit->text());
    mFile->setEncoding(ui->encodingCombo->currentText());
    if (ui->formatCustomRadio->isChecked())
    {
        mFile->setTypeCSV(selectedChars(), ui->formatCustomQuoteEdit->text(), ui->formatCustomEscapeEdit->text());
    }
    else if (ui->formatRegRadio->isChecked())
    {
        mFile->setTypeRegexp(ui->formatExpressionEdit->text());
;    }
    else
    {
        mFile->setTypeCSV();
    }
    mFile->setSkipLines(ui->recordNumberDiscardLinesSpin->value());
    mFile->setUseHeader(ui->recordFirstLineAsFieldNames->isChecked());
    mFile->setDiscardEmptyFields(ui->recordDiscardEmptyFields->isChecked());
    mFile->setTrimFields(ui->recordTrimFields->isChecked());
    return mFile->isValid();
}

void GwmOpenXYEventLayerDialog::onFileNameOpenFileDialogBtnClicked()
{
    qDebug() << "[GwmOpenXYEventLayerDialog::onFileNameOpenFileDialogBtnClicked]";
    QString filter = QString()
            .append(tr("Comma Separated Value (*.csv);;"))
            .append(tr("Tab Separated Value (*.tsv);;"))
            .append(tr("Plain Text (*.txt);;"))
            .append(tr("All (*.*);;"));
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open Delimited Text"), tr(""), filter);
    ui->fileNameEdit->setText(filepath);
}

QString GwmOpenXYEventLayerDialog::selectedChars()
{
    QString chars;
    if (ui->formatCustomDelimiterCommaCheck->isChecked() )
        chars.append( ',' );
    if (ui->formatCustomDelimiterSpaceCheck->isChecked() )
        chars.append( ' ' );
    if (ui->formatCustomDelimiterTabCheck->isChecked() )
        chars.append( '\t' );
    if (ui->formatCustomDelimiterSemicolonCheck->isChecked() )
        chars.append( ';' );
    if (ui->formatCustomDelimiterColonCheck->isChecked() )
        chars.append( ':' );
    chars = QgsDelimitedTextFile::encodeChars( chars );
    if (ui->formatCustomDelimiterOthersCheck->isChecked())
    {
        chars.append(ui->formatCustomDelimiterOthersEdit->text() );
    }
    return chars;
}

void GwmOpenXYEventLayerDialog::setSelectedChars(const QString &delimiters)
{
    QString chars = QgsDelimitedTextFile::decodeChars( delimiters );
    ui->formatCustomDelimiterCommaCheck->setChecked( chars.contains( ',' ) );
    ui->formatCustomDelimiterSpaceCheck->setChecked( chars.contains( ' ' ) );
    ui->formatCustomDelimiterTabCheck->setChecked( chars.contains( '\t' ) );
    ui->formatCustomDelimiterColonCheck->setChecked( chars.contains( ':' ) );
    ui->formatCustomDelimiterSemicolonCheck->setChecked( chars.contains( ';' ) );
    chars = chars.remove( QRegExp( "[ ,:;\t]" ) );
    if (chars.size() > 0)
    {
        chars = QgsDelimitedTextFile::encodeChars( chars );
        ui->formatCustomDelimiterOthersCheck->setChecked(true);
        ui->formatCustomDelimiterOthersEdit->setText( chars );
    }
    else
    {
        ui->formatCustomDelimiterOthersCheck->setChecked(false);
        ui->formatCustomDelimiterOthersEdit->clear();

    }
}

bool GwmOpenXYEventLayerDialog::trySetXYField(QStringList &fields, QList<bool> &isValidNumber, const QString &xname, const QString &yname)
{
    // If fields already set, then nothing to do
    if ( ui->geometryXField->currentIndex() >= 0 && ui->geometryYField->currentIndex() >= 0 ) return true;

    // Try and find a valid field name matching the x field
    int indexX = -1;
    int indexY = -1;

    for ( int i = 0; i < fields.size(); i++ )
    {
        // Only interested in number fields containing the xname string
        // that are in the X combo box
        if ( ! isValidNumber[i] ) continue;
        if ( ! fields[i].contains( xname, Qt::CaseInsensitive ) ) continue;
        indexX = ui->geometryXField->findText( fields[i] );
        if ( indexX < 0 ) continue;

        // Now look for potential y fields, like xname with x replaced with y
        QString xfield( fields[i] );
        int from = 0;
        while ( true )
        {
            int pos = xfield.indexOf( xname, from, Qt::CaseInsensitive );
            if ( pos < 0 ) break;
            from = pos + 1;
            QString yfield = xfield.mid( 0, pos ) + yname + xfield.mid( pos + xname.size() );
            if ( ! fields.contains( yfield, Qt::CaseInsensitive ) ) continue;
            for ( int iy = 0; iy < fields.size(); iy++ )
            {
                if ( ! isValidNumber[iy] ) continue;
                if ( iy == i ) continue;
                if ( fields[iy].compare( yfield, Qt::CaseInsensitive ) == 0 )
                {
                    indexY = ui->geometryYField->findText( fields[iy] );
                    break;
                }
            }
            if ( indexY >= 0 ) break;
        }
        if ( indexY >= 0 ) break;
    }
    if ( indexY >= 0 )
    {
        ui->geometryXField->setCurrentIndex( indexX );
        ui->geometryYField->setCurrentIndex( indexY );
    }
    return indexY >= 0;
}

bool GwmOpenXYEventLayerDialog::validate()
{
    // Check that input data is valid - provide a status message if not..

    QString message;
    bool enabled = false;

    QString filePath = ui->fileNameEdit->text();
    if ( filePath.trimmed().isEmpty() )
    {
        message = tr( "Please select an input file" );
    }
    else if ( ! QFileInfo::exists( filePath ) )
    {
        message = tr( "File %1 does not exist" ).arg( filePath );
    }
    else if ( ui->layerNameEdit->text().isEmpty() )
    {
        message = tr( "Please enter a layer name" );
    }
    else if ( ui->formatCustomRadio->isChecked() && selectedChars().isEmpty() )
    {
        message = tr( "At least one delimiter character must be specified" );
    }

    if ( message.isEmpty() && ui->formatRegRadio->isChecked() )
    {
        QRegExp re( ui->formatExpressionEdit->text() );
        if ( ! re.isValid() )
        {
            message = tr( "Regular expression is not valid" );
        }
        else if ( re.pattern().startsWith( '^' ) && re.captureCount() == 0 )
        {
            message = tr( "^.. expression needs capture groups" );
        }
        ui->formatRegExpErrorLabel->setText( message );
    }
    if ( ! message.isEmpty() )
    {
        // continue...
    }
    // Hopefully won't hit this none-specific message, but just in case ...
    else if ( ! mFile->isValid() )
    {
        message = tr( "Definition of filename and delimiters is not valid" );
    }
    // Assume that the sample table will have been populated if data was found
//    else if ( tblSample->rowCount() == 0 )
//    {
//        message = tr( "No data found in file" );
//        if ( mBadRowCount > 0 )
//        {
//            message = message + " (" + tr( "%1 badly formatted records discarded" ).arg( mBadRowCount ) + ')';
//        }
//    }
//    else if ( geomTypeXY->isChecked() && ( cmbXField->currentText().isEmpty()  || cmbYField->currentText().isEmpty() ) )
//    {
//        message = tr( "X and Y field names must be selected" );
//    }
//    else if ( geomTypeXY->isChecked() && ( cmbXField->currentText() == cmbYField->currentText() ) )
//    {
//        message = tr( "X and Y field names cannot be the same" );
//    }
//    else if ( geomTypeWKT->isChecked() && cmbWktField->currentText().isEmpty() )
//    {
//        message = tr( "The WKT field name must be selected" );
//    }
    else if (!mCRSSelector->crs().isValid())
    {
        message = tr( "The CRS must be selected" );
    }
    else
    {
        enabled = true;
        if ( mBadRowCount > 0 )
        {
            message = tr( "%1 badly formatted records discarded from sample data" ).arg( mBadRowCount );
        }

    }
    ui->statusLabel->setText( message );
    return enabled;
}

void GwmOpenXYEventLayerDialog::onFormatCustomDelimiterOthersCheckToggled(bool checked)
{
    ui->formatCustomDelimiterOthersEdit->setEnabled(checked);
}
