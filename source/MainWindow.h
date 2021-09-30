/* Copyright (C) 2005-2018, UNIGINE. All rights reserved.
 *
 * This file is a part of the UNIGINE 2.7.2.1 SDK.
 *
 * Your use and / or redistribution of this software in source and / or
 * binary form, with or without modification, is subject to: (i) your
 * ongoing acceptance of and compliance with the terms and conditions of
 * the UNIGINE License Agreement; and (ii) your inclusion of this notice
 * in any version of this software that you use or redistribute.
 * A copy of the UNIGINE License Agreement is available by contacting
 * UNIGINE. at http://unigine.com/
 */

#pragma once

#include <map>

#include <QMainWindow>
#include <QTreeView>

// Unigine
#include "MaintenanceEditorLogic.h"
#include "MaintenanceSystemLogic.h"
#include "MaintenanceWorldLogic.h"

#include <UnigineEngine.h>

// Common
#include "MaintenanceGL.h"
#include "ResizeControlWidget.h"
#include "TreeModel.h"
#include "OfficeImport.h"

namespace Ui
{
class MainWindow;

static constexpr const int COLUMN_DATA_INDEX = 0;
static constexpr const int COLUMN_ID_INDEX = 1;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    explicit MainWindow(bool isEditable, bool isImport, QString modelName, QString modelPath, QString sofficePath, QWidget *parent = 0);
	~MainWindow();

private:
    Ui::MainWindow *_ui;

    Maintenance* _maintenance = nullptr;

    ResizeControlWidget* _resizeWidget = nullptr;

    QTreeView* _treeView = nullptr;
    TreeModel* _modelsModel = nullptr;
    TreeModel* _partsModel = nullptr;

    // UnigineLogic
    MaintenanceSystemLogic _system_logic;
    MaintenanceWorldLogic  _world_logic;
    MaintenanceEditorLogic _editor_logic;

    // ~~~ Custom ~~~
    QStringList _models;

    std::map<int, int> _visibleUpdateMap; // <id, state(tri-)>

    // maintenance settings
    bool _isEditable;  // if true - can edit model parts and model sections
    bool _isImport;  // if true - can import model if _modelName and _modelPath are empty,
                     // and import office files if _sofficePath is not empty and valid
    QString _modelName;  // name of model
    QString _modelPath;  // path to .node file of model
    QString _sofficePath;  // path to soffice binary

    OfficeImport* _officeImporter = nullptr;

    // ~~~ Utilities ~~~
    void _updateTreeView(const QStringList& header, const QVariantList& items);

    //! Update states of TreeView checkboxes up to tree
    void _updateUpTreeViewCheckState(const QModelIndex& index);

    //! Update states of TreeView checkboxes down to tree
    void _updateDownTreeViewCheckState(const QModelIndex& index, int state);

    //! Change visible of part by checkbox
    void _updateCheckbox(const QModelIndex &index);

    //! Change name of part on 3D model
    void _updateName(const QModelIndex &index);

private slots:
    //! Get parts of model
    void _onInitComplete();

    //! Return to models list
    void _onBackButtonPush();

    //! View part in 3D window
    void _onPartSelect();

    //! Import 3D model file
    void _onFbxStpSelect();

    //! Import document file
    void _onDocumentSelect();

    //! Highlight row in treeView,
    //! while select object in 3D
    void _onPartSelected(int partId);

    //! TreeView item update handler
    void _onTreeViewItemChange(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    //! Save part names changes
    void _onPartNamesSave();

    //! Get parts of imported model
    void _onImportComplete(const QString& modelName, const QVariantList& parts);

    //! Show/hide initial names tree
    void _onChangeInitialTreeVisibility();
};
