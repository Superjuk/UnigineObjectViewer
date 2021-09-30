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

#include "ViewCube.h"

#include <UnigineLogic.h>
#include <UnigineStreams.h>
#include <UniginePlayers.h>
#include <UnigineSplineGraph.h>
#include <UnigineApp.h>
#include <UnigineNode.h>
#include <UnigineImage.h>
#include <UnigineWidgets.h>
#include <UnigineGui.h>

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QStringList>

#include <vector>
#include <memory>
#include <map>


class MaintenanceWorldLogic
    : public QObject
    , public Unigine::WorldLogic
{
    Q_OBJECT

public:
    MaintenanceWorldLogic();
    virtual ~MaintenanceWorldLogic();
	
	virtual int init();
	
	virtual int update();
	virtual int render();
	virtual int flush();
	
	virtual int shutdown();
	virtual int destroy();
	
	virtual int save(const Unigine::StreamPtr &stream);
	virtual int restore(const Unigine::StreamPtr &stream);

    // custom

    //! Get model parts
    QVariantList getParts(const Unigine::String& modelPath);

    //! Get initial names of model parts
    QVariantList getInitParts();

    //! View part
    void viewPart(int partId);

    //! Import model into viewer
    void importModel(const Unigine::String& path);

    //! Set visible part status
    void setPartVisible(int partId, int state);

    //! Change part`s name
    bool setPartName(int partId, const Unigine::String& name);

    //! Save model nodeRef immediately
    void savePartNamesToNode();

private:
    Unigine::PlayerPersecutorPtr _camera;
    Unigine::NodePtr             _target;
    Unigine::NodePtr             _center;

    Unigine::SplineGraphPtr _targetSpline;

    Unigine::WidgetHBoxPtr     _hLayout;
    Unigine::WidgetCheckBoxPtr _transparentToggle;
    Unigine::WidgetLabelPtr    _partNameLabel;

    Unigine::WidgetHBoxPtr     _hLayoutInfoPanel;
    Unigine::WidgetLabelPtr    _partModelHint;
    Unigine::WidgetLabelPtr    _visibleHint;
    Unigine::WidgetCheckBoxPtr _visibleToggle;

    Unigine::String _modelDirPath;

    // select element
    int _mouseLeftButtonState; // 1 - pressed

    // move camera from object to object
    bool  _startTargetAnimation;  // on/off camera target moving
    float _targetAnimStatus;      // from 0 (start point) to 1 (end point)
    bool  _fitZoomAfterCamAnimStop;  // zooming to part after camera animation stop

    // List of models
    std::vector<Unigine::String> _models;

    // Globals
    Unigine::ObjectPtr _selectedObject;
    Unigine::NodePtr   _visibleModel;
    Unigine::NodePtr   _selectedGroup;

    // Collect frame count for click control
    int _frameCount;

    // Enable mouse control
    bool _mouseControlEnabled;

    ViewCube* _viewcube;

    // need to save nodeRef while it renames
    bool _isModelChanged;

    // while node renames, it first name saves in map
    std::map<int, Unigine::String> _defaultNames;

    // ~~~ Utilities ~~~
    bool _getUnigineNode(Unigine::NodePtr& node, const char* nodeName);

    bool _uiInit();

    bool _cameraInit();

    Unigine::WidgetButtonPtr _createButton(
        QSize             size       = QSize(),
        QPoint            pos        = QPoint(),
        Unigine::String   toolTip    = Unigine::String(),
        Unigine::String   pathToIcon = Unigine::String(),
        Unigine::ImagePtr image      = Unigine::ImagePtr(nullptr),
        QSize             iconSize   = QSize(),
        bool              toggleable = false,
        bool              arrange    = false);

    //! Camera zoom action
    void _zoomCamera();

    //! Select node by mouse left button click
    void _selectElement();

    //! Move camera_target from its current position
    //! to center of object by spline
    void _moveTarget();

    //! Set outline effect to object
    void _setOutline(Unigine::ObjectPtr& object);

    //! Set outline effect to object
    void _setOutline(Unigine::NodePtr& node);

    //! Get coarse center of set of points
    Unigine::Math::Vec3 _getCoarseCenter(const Unigine::NodePtr& group);

    //! Set outline effect for all children of node
    void _setGroupOutline(Unigine::NodePtr& parent);

    //! Fit zoom to selected object
    void _fitZoom(Unigine::ObjectPtr& object);

    //! Apply standart material depending on surface
    //! name of parts
    void _applyStdMat(Unigine::NodePtr& model);

    //! Retrive material name from surface name
    Unigine::String _getMaterialName(const Unigine::NodePtr& part, const Unigine::String& postfix = "");
    Unigine::String _getMaterialName(const Unigine::ObjectMeshStaticPtr& mesh, const Unigine::String& postfix = "");

    //! Retrive all parts of model with hierarchy
    QVariantList _getParts(Unigine::NodePtr& parent);

    //! Retrive all parts of model with hierarchy
    QVariantList _getInitParts(Unigine::NodePtr& parent);

    //! Create spline to target
    //! and start camera moving
    void _createSplineToTarget(const Unigine::Math::vec3& target);

    //! Import CAD files
    void _importCAD(const Unigine::String& path);

    //! Init mount points
    bool _mountPointsInit();

    //! Make directory recursively
    //! \return 'true' - directory successfully created
    bool _mkdir(const Unigine::String& path);

    //! Replace file
    //! \arg oldPath - file to be replaced (absolute path)
    //! \arg newPath - new path with file basename (absolute path)
    //! \return 'true' - file replaced successfully
    bool _replace(const Unigine::String& oldPath, const Unigine::String& newPath);

    //! Remove file
    //! \return 'true' - file removed successfully
    bool _remove(const Unigine::String& path);

    //! Create outline material
    void _makeOutlineMaterial(const Unigine::String& path);

    //! Init models directory path
    bool _initModelPathDir();

    // ~~~ Events ~~~
    //!
    void _onTransparentToggleChanged();

    //!
    void _onWidgetEnterLeave();

signals:
    void partSelected(int id);

    void initComplete();

    void importComplete(const QString& modelName, const QVariantList& parts);
};
