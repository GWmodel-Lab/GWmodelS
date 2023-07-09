#ifndef GWMDELIMITEDTEXTFILE_H
#define GWMDELIMITEDTEXTFILE_H

#include <QObject>
#include <QStringList>
#include <QRegExp>
#include <QUrl>

class QgsFeature;
class QgsField;
class QFile;
class QFileSystemWatcher;
class QTextStream;

class GwmDelimitedTextFile : public QObject
{
    Q_OBJECT
public:

    enum Status
    {
      RecordOk,
      InvalidDefinition,
      RecordEmpty,
      RecordInvalid,
      RecordEOF
    };

    enum DelimiterType
    {
      DelimTypeWhitespace,
      DelimTypeCSV,
      DelimTypeRegexp
    };

    explicit GwmDelimitedTextFile(const QString &url = QString());

    ~GwmDelimitedTextFile() override;
    /**
     * Set the filename
     * \param filename  the name of the file
     */
    void setFileName( const QString &filename );

    /**
     * Returns the filename
     * \returns filename  the name of the file
     */
    QString fileName()
    {
      return mFileName;
    }

    /**
     * Set the file encoding (defuault is UTF-8)
     *  \param encoding the encoding to use for the fileName()
     */
    void setEncoding( const QString &encoding );

    /**
     * Returns the file encoding
     *  \returns encoding The file encoding
     */
    QString encoding() { return mEncoding; }

    /**
     * Decode the parser settings from a url as a string
     *  \param url  The url from which the delimiter and delimiterType items are read
     */
    bool setFromUrl( const QString &url );

    /**
     * Decode the parser settings from a url
     *  \param url  The url from which the delimiter and delimiterType items are read
     */
    bool setFromUrl( const QUrl &url );

    /**
     * Encode the parser settings into a QUrl
     *  \returns url  The url into which the delimiter and delimiterType items are set
     */
    QUrl url();

    /**
     * Set the parser for parsing CSV files
     */
    void setTypeWhitespace();

    /**
     * Set the parser for parsing based on a reqular expression delimiter
     *  \param regexp A string defining the regular expression
     */
    void setTypeRegexp( const QString &regexp );

    /**
     * Set the parser to use a character type delimiter.
     *  \param delim  The field delimiter character set
     *  \param quote  The quote character, used to define quoted fields
     *  \param escape The escape character used to escape quote or delim
     *                characters.
     */
    void setTypeCSV( const QString &delim = QString( "," ), const QString &quote = QString( "\"" ), const QString &escape = QString( "\"" ) );

    /**
     * Set the number of header lines to skip
     * \param skiplines The maximum lines to skip
     */
    void setSkipLines( int skiplines );

    /**
     * Returns the number of header lines to skip
     * \returns skiplines The maximum lines to skip
     */
    int skipLines()
    {
      return mSkipLines;
    }

    /**
     * Set reading field names from the first record
     * \param useheaders Field names will be read if true
     */
    void setUseHeader( bool useheader = true );

    /**
     * Returns the option for reading field names from the first record
     * \returns useheaders Field names will be read if true
     */
    bool useHeader()
    {
      return mUseHeader;
    }

    /**
     * Set the option for dicarding empty fields
     * \param useheaders Empty fields will be discarded if true
     */
    void setDiscardEmptyFields( bool discardEmptyFields = true );

    /**
     * Returns the option for discarding empty fields
     * \returns useheaders Empty fields will be discarded if true
     */
    bool discardEmptyFields()
    {
      return mDiscardEmptyFields;
    }

    /**
     * Set the option for trimming whitespace from fields
     * \param trimFields Fields will be trimmed if true
     */
    void setTrimFields( bool trimFields = true );

    /**
     * Returns the option for trimming empty fields
     * \returns useheaders Empty fields will be trimmed if true
     */
    bool trimFields()
    {
      return mTrimFields;
    }

    /**
     * Set the maximum number of fields that will be read from a record
     *  By default the maximum number is unlimited (0)
     *  \param maxFields  The maximum number of fields that will be read
     */
    void setMaxFields( int maxFields );

    /**
     * Returns the maximum number of fields that will be read
     *  \returns maxFields The maximum number of fields that will be read
     */
    int maxFields() { return mMaxFields; }

    /**
     * Set the field names
     *  Field names are set from QStringList.  Names may be modified
     *  to ensure that they are unique, not empty, and do not conflict
     *  with default field name (field_##)
     *  \param names  A list of proposed field names
     */
    void setFieldNames( const QStringList &names );

    /**
     * Returns the field names read from the header, or default names
     *  field_## if none defined.  Will open and read the head of the file
     *  if required, then reset.  Note that if header record record has
     *  not been read then the field names are empty until records have
     *  been read.  The list may be expanded as the file is read and records
     *  with more fields are loaded.
     *  \returns names  A list of field names in the file
     */
    QStringList &fieldNames();

    /**
     * Returns the index of a names field
     *  \param name    The name of the field to find.  This will also accept an
     *                 integer string ("1" = first field).
     *  \returns index  The zero based index of the field name, or -1 if the field
     *                 name does not exist or cannot be inferred
     */
    int fieldIndex( const QString &name );

    /**
     * Reads the next record from the stream splits into string fields.
     *  \param fields  The string list to populate with the fields
     *  \returns status The result of trying to parse a record.  RecordOk
     *                 if read successfully, RecordEOF if reached the end of the
     *                 file, RecordEmpty if no data on the next line, and
     *                 RecordInvalid if the record is ill-formatted.
     */
    Status nextRecord( QStringList &fields );

    /**
     * Returns the line number of the start of the last record read
     *  \returns linenumber  The line number of the start of the record
     */
    int recordId()
    {
      return mRecordLineNumber;
    }

    /**
     * Set the index of the next record to return.
     *  \param  nextRecordId The id to set the next record to
     *  \returns valid  True if the next record can be located
     */
    bool setNextRecordId( long nextRecordId );

    /**
     * Number record number of records visited. After scanning the file
     *  serves as a record count.
     *  \returns maxRecordNumber The maximum record number
     */
    long recordCount() { return mMaxRecordNumber; }

    /**
     * Reset the file to reread from the beginning
     */
    Status reset();

    /**
     * Returns a string defining the type of the delimiter as a string
     *  \returns type The delimiter type as a string
     */
    QString type();

    /**
     * Check that provider is valid (filename and definition valid)
     *
     * \returns valid True if the provider is valid
     */
    bool isValid();

    /**
     * Encode characters - used to convert delimiter/quote/escape characters to
     *  encoded form (e.g., replace tab with \t)
     *  \param string  The unencoded string
     *  \returns encstring  The encoded string
     */
    static QString encodeChars( QString string );

    /**
     * Encode characters - used to encoded character strings to
     *  decoded form (e.g., replace \t with tab)
     *  \param string  The unencoded string
     *  \returns decstring  The decoded string
     */
    static QString decodeChars( QString string );

    /**
     * Set to use or not use a QFileWatcher to notify of changes to the file
     * \param useWatcher True to use a watcher, false otherwise
     */

    void setUseWatcher( bool useWatcher );

  signals:

    /**
     * Signal sent when the file is updated by another process
     */
    void fileUpdated();

  public slots:

    /**
     * Slot used by watcher to notify of file updates
     */
    void updateFile();

  private:

    /**
     * Open the file
     *
     * \returns valid  True if the file is successfully opened
     */
    bool open();

    /**
     * Close the text file
     */
    void close();

    /**
     * Reset the status if the definition is changing (e.g., clear
     *  existing field names, etc...
     */
    void resetDefinition();

    //! Parse reqular expression delimited fields
    Status parseRegexp( QString &buffer, QStringList &fields );
    //! Parse quote delimited fields, where quote and escape are different
    Status parseQuoted( QString &buffer, QStringList &fields );

    /**
     * Returns the next line from the data file.  If skipBlank is true then
     * blank lines will be skipped - this is for compatibility with previous
     * delimited text parser implementation.
     */
    Status nextLine( QString &buffer, bool skipBlank = false );

    /**
     * Set the next line to read from the file.
     */
    bool setNextLineNumber( long nextLineNumber );

    /**
     * Utility routine to add a field to a record, accounting for trimming
     *  and discarding, and maximum field count
     */
    void appendField( QStringList &record, QString field, bool quoted = false );

    // Pointer to the currently selected parser
    Status( GwmDelimitedTextFile::*mParser )( QString &buffer, QStringList &fields );

    QString mFileName;
    QString mEncoding;
    QFile *mFile = nullptr;
    QTextStream *mStream = nullptr;
    bool mUseWatcher = false;
    QFileSystemWatcher *mWatcher = nullptr;

    // Parameters common to parsers
    bool mDefinitionValid = false;
    DelimiterType mType;
    bool mUseHeader = true;
    bool mDiscardEmptyFields = false;
    bool mTrimFields = false;
    int mSkipLines = 0;
    int mMaxFields = 0;
    int mMaxNameLength = 200;

    // Parameters used by parsers
    QRegExp mDelimRegexp;
    bool mAnchoredRegexp = false;
    QString mDelimChars;
    QString mQuoteChar;
    QString mEscapeChar;

    // Information extracted from file
    QStringList mFieldNames;
    long mLineNumber = -1;
    long mRecordLineNumber = -1;
    long mRecordNumber = -1;
    QStringList mCurrentRecord;
    bool mHoldCurrentRecord = false;
    // Maximum number of record (ie maximum record number visited)
    long mMaxRecordNumber = -1;
    int mMaxFieldCount = 0;

    QString mDefaultFieldName;
    QRegExp mDefaultFieldRegexp;
};

#endif // GWMDELIMITEDTEXTFILE_H
