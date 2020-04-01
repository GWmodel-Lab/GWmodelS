#ifndef GWMDEVATTRTABLE_H
#define GWMDEVATTRTABLE_H

#include "prefix.h"
#include <QDialog>
#include "ui_qgsattributetabledialog.h"

#include <qgsvectorlayer.h>

class GwmDevAttrTable: public QDialog, public Ui::QgsAttributeTableDialog
{
    Q_OBJECT
public:
    //GwmDevAttrTable(QgsVectorLayer* theVecLayer, QgsMapCanvas* map, QWidget *parent = 0);
    GwmDevAttrTable(QgsVectorLayer* theVecLayer, QgsMapCanvas* myMapCanvas, QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window);
    ~GwmDevAttrTable();

    //! starts/stops editing mode of a layer
    bool toggleEditing2( QgsMapLayer *layer, bool allowCancel = true );

    /**
     * Save edits of a layer
     * \param leaveEditable leave the layer in editing mode when done
     * \param triggerRepaint send layer signal to repaint canvas when done
     */
    void saveEdits( QgsMapLayer *layer, bool leaveEditable = true, bool triggerRepaint = true );
private:
    QgsVectorLayer *currentLayer;
    QgsMapCanvas *mymapCanvas;
    QToolButton *mActionFeatureActions = nullptr;
    void updateMultiEditButtonState();
    //bool toggleEditing2(QgsMapLayer *layer, bool allowCancel);
private slots:
    /**
     * Toggles editing mode
     */
    void mActionToggleEditing_toggled( bool );
    /**
     * Select all
     */
    void mActionSelectAll_triggered();
    /**
     * Inverts selection
     */
    void mActionInvertSelection_triggered();
    /**
     * Clears selection
     */
    void mActionRemoveSelection_triggered();
    /**
     * Moves selected lines to the top
     */
    void mActionSelectedToTop_toggled( bool );
    /**
     * Pans to selected features
     */
    void mActionPanMapToSelectedRows_triggered();
    /**
     * Zooms to selected features
     */
    void mActionZoomMapToSelectedRows_triggered();
    /**
     * Opens field calculator dialog
     */
    void mActionOpenFieldCalculator_triggered();
    void layerActionTriggered();

    void updateFieldFromExpression();
    void updateFieldFromExpressionSelected();
    void runFieldCalculation( QgsVectorLayer *layer, const QString &fieldName, const QString &expression, const QgsFeatureIds &filteredIds = QgsFeatureIds() );
    void updateButtonStatus( const QString &fieldName, bool isValid );

    void updateTitle();

    /**
     * Opens dialog to add new attribute
     */
    void mActionAddAttribute_triggered();

    /**
     * Opens dialog to remove attribute
     */
    void mActionRemoveAttribute_triggered();
public slots:

  /**
   * Toggles editing mode
   */
  void editingToggled();

  /**
   * add feature
   */
  //void mActionAddFeature_triggered();
  /**
   * deletes the selected features
   */
//  void mActionDeleteSelected_triggered();
  /**
   * Saves edits
   */
  void mActionSaveEdits_triggered();
};

#endif // GWMDEVATTRTABLE_H
