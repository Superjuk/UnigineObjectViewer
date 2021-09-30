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

#include "defines.h"
#include "MaintenanceWorldLogic.h"

#include <UnigineBounds.h>
#include <UnigineConsole.h>
#include <UnigineDir.h>
#include <UnigineEditor.h>
#include <UnigineEngine.h>
#include <UnigineFileSystem.h>
#include <UnigineImport.h>
#include <UnigineInterpreter.h>
#include <UnigineGame.h>
#include <UnigineGUID.h>
#include <UnigineMaterial.h>
#include <UnigineMaterials.h>
#include <UnigineSplineGraph.h>
#include <UnigineString.h>

#include <QVariantMap>

#include <string>
#include <algorithm>
#include <math.h>


using namespace Unigine;
using namespace Maintenance;
using namespace Math;
using namespace std;


MaintenanceWorldLogic::MaintenanceWorldLogic()
    : _modelDirPath("")
    , _mouseLeftButtonState(0)
    , _fitZoomAfterCamAnimStop(false)
    , _frameCount(0)
    , _mouseControlEnabled(true)
    , _isModelChanged(false)
{}

MaintenanceWorldLogic::~MaintenanceWorldLogic()
{
    //! \todo find objects to clear
    if(_viewcube)
        delete _viewcube;
}

int MaintenanceWorldLogic::init()
{
    _getUnigineNode(_target, TARGET);
    _getUnigineNode(_center, SCENE_CENTER);

    _cameraInit();

    _uiInit();

    _initModelPathDir();

    _mountPointsInit();

    /* Mouse cursor */
    App::get()->setMouseShow(1);

    /* Target and camera route */
    _targetSpline = SplineGraph::create();

    emit initComplete();

	return 1;
}

// start of the main loop
int MaintenanceWorldLogic::update()
{
    _zoomCamera();

    if(_viewcube && !_viewcube->isMouseOver() && _mouseControlEnabled)
        _selectElement();

    _moveTarget();

    ControlsApp::get()->setMouseEnabled(App::get()->getMouseButton() & App::BUTTON_LEFT);

    if(_viewcube)
        _viewcube->update(vec3(_target->getWorldPosition()));

	return 1;
}

int MaintenanceWorldLogic::render()
{
    _viewcube->render();

	return 1;
}

int MaintenanceWorldLogic::flush()
{
	return 1;
}

int MaintenanceWorldLogic::shutdown()
{
    if(_isModelChanged)
        savePartNamesToNode();

	return 1;
}

int MaintenanceWorldLogic::destroy()
{
	return 1;
}

int MaintenanceWorldLogic::save(const Unigine::StreamPtr &stream)
{
	UNIGINE_UNUSED(stream);
	return 1;
}

int MaintenanceWorldLogic::restore(const Unigine::StreamPtr &stream)
{
    UNIGINE_UNUSED(stream);
    return 1;
}

QVariantList MaintenanceWorldLogic::getParts(const String& modelPath)
{
    // models runtime load
    auto nodeRef = NodeReference::create(modelPath);
    nodeRef->release();
    auto node = nodeRef->getReference();
    if(!node.get())
    {
        Log::warning("NodeReference \"%s\" does not exist\n", modelPath.get());
        return {};
    }

    Editor::get()->addNode(node, 1);
    node->setParent(_center);
    node->setPosition(Vec3::ZERO);
    node->setEnabled(1);

    _visibleModel = node;
    _applyStdMat(node);

    _partNameLabel->setText(node->getName());

    return _getParts(node);
}

QVariantList MaintenanceWorldLogic::getInitParts()
{
    if(!_visibleModel.get())
        return QVariantList{};

    return _getInitParts(_visibleModel);
}

void MaintenanceWorldLogic::viewPart(int partId)
{
    auto node = Editor::get()->getNodeById(partId);

    String partName;
    if(!node.get())
    {
        Log::warning("Part with id = %s missed\n", String::itoa(partId).get());
        return;
    }
    partName = node->getName();

    if(node->getType() == Node::NODE_DUMMY)
    {
        _createSplineToTarget(vec3(_getCoarseCenter(node)));

        _setOutline(node);

        _selectedObject = ObjectPtr{};
        _selectedGroup = node;

        _partNameLabel->setText(partName);
        _fitZoomAfterCamAnimStop = true;
        return;
    }

    auto object = Object::cast(node);
    _createSplineToTarget(vec3(object->getWorldBoundSphere().getCenter()));
    _setOutline(object);

    _selectedObject = object;
    _selectedGroup = NodePtr{};

    _partNameLabel->setText(partName);

    _fitZoomAfterCamAnimStop = true;
}

void MaintenanceWorldLogic::importModel(const String &path)
{
    if(_modelDirPath.empty())
    {
        Log::error("Path to models not exist\n");
        return;
    }

    auto filesystem = FileSystem::get();
    if(!filesystem)
    {
        Log::error("FileSystem instance invalid!\n");
        return;
    }

    if(!filesystem->isFileExist(path))
    {
        Log::error("File %s doesn`t exist\n", path.basename().get());
        return;
    }

    const auto ext = path.extension();  //*.iges *.stp *.step *.stl
    if(ext.contains(CAD_IGES) || ext.contains(CAD_STEP) || ext.contains(CAD_STL) || ext.contains(CAD_STP))
        _importCAD(path);
}

void MaintenanceWorldLogic::setPartVisible(int partId, int state)
{
    auto node = Editor::get()->getNodeById(partId);
    if(!node.get())
    {
        Log::warning("Part with id = %s missed\n", String::itoa(partId).get());
        return;
    }

    state == Qt::Unchecked ? node->setEnabled(0) : node->setEnabled(1);
}

bool MaintenanceWorldLogic::setPartName(int partId, const String &name)
{
    auto node = Editor::get()->getNodeById(partId);
    if(!node.get())
    {
        Log::warning("Part with id = %s missed\n", String::itoa(partId).get());
        return false;
    }

    /* Add oldName to map if it`s initial*/
    if(_defaultNames.count(partId) == 0)
        _defaultNames[partId] = node->getName();

    node->setName(name);

    /* Check for changes */
    _isModelChanged = false;
    for(auto& pair : _defaultNames)
    {
        if(pair.second != String(Editor::get()->getNodeById(pair.first)->getName()))
        {
            _isModelChanged = true;
            break;
        }
    }

    _partNameLabel->setText(name);

    return _isModelChanged;
}

void MaintenanceWorldLogic::savePartNamesToNode()
{
    const auto name = String(_visibleModel->getName());
    const auto path = _modelDirPath + name + "/" + name + ".node";
    World::get()->saveNode(path, _visibleModel);

    _defaultNames.clear();
    _isModelChanged = false;
}


// ~~~ Utilities ~~~
bool MaintenanceWorldLogic::_getUnigineNode(NodePtr &node, const char *nodeName)
{
    node = Editor::get()->getNodeByName(nodeName);
    if(!node.get())
    {
        Log::warning("%s not found", nodeName);
        return false;
    }

    return true;
}

bool MaintenanceWorldLogic::_uiInit()
{
    auto gui = Gui::get();

    /* View cube */
    _viewcube = new ViewCube(gui);

    /* Transparent check box */
    _transparentToggle = WidgetCheckBox::create(gui, "Полупрозрачность");
    _transparentToggle->setFontSize(20);
    _transparentToggle->setCallback0(Gui::CHANGED, MakeCallback(this, &MaintenanceWorldLogic::_onTransparentToggleChanged));
    _transparentToggle->setCallback0(Gui::LEAVE, MakeCallback(this, &MaintenanceWorldLogic::_onWidgetEnterLeave));
    _transparentToggle->setCallback0(Gui::ENTER, MakeCallback(this, &MaintenanceWorldLogic::_onWidgetEnterLeave));

    /* Parent label */
    _partNameLabel = WidgetLabel::create(gui, "Импортируйте модель");
    _partNameLabel->setFontSize(40);

    /* Horizontal layout */
    _hLayout = WidgetHBox::create(gui, 5, 10);
    _hLayout->addChild(_transparentToggle->getWidget());

    /* Info panel layout */
    _hLayoutInfoPanel = WidgetHBox::create(gui, 5, 10);
    _hLayoutInfoPanel->addChild(_partNameLabel->getWidget(), Gui::ALIGN_BACKGROUND | Gui::ALIGN_CENTER | Gui::ALIGN_TOP);
    _hLayoutInfoPanel->addChild(_viewcube->getWidget(), Gui::ALIGN_RIGHT | Gui::ALIGN_TOP);

    gui->addChild(_hLayoutInfoPanel->getWidget(), Gui::ALIGN_BACKGROUND | Gui::ALIGN_TOP | Gui::ALIGN_EXPAND);
    gui->addChild(_hLayout->getWidget(), Gui::ALIGN_BOTTOM | Gui::ALIGN_BACKGROUND);
    gui->setFont("fonts/GOST_B.ttf");

    return true;
}

bool MaintenanceWorldLogic::_cameraInit()
{
    _camera = PlayerPersecutor::create();
    if(!_camera.get())
        return false;

    // Min/max distance
    _camera->setMinDistance(0.0f);
    _camera->setMaxDistance(5.0f);

    _camera->setPosition(Math::Vec3(7.0, 5.0, 5.0));

    /* Camera init */
    _camera->setFov(90.0f);
    _camera->setZNear(0.05f);
    _camera->setZFar(10000.0f);    

    // fixing or PlayerPersecutor to keep its position relative to the anchor point and angles specified
    _camera->setFixed(1);

    // setting a target node to follow
    if(!_target.get())
        return false;

    _camera->setTarget(_target);

    // setting the anchor point (in the local coordinates of the target node)
    _camera->setAnchor(Math::vec3(0.0f, 0.0f, 0.0f));

    // setting Phi and Theta angles so that out camera would have a top view    
    _camera->setPhiAngle(0.0f);

    _camera->setMaxThetaAngle(90.0f);
    _camera->setMinThetaAngle(-90.0f);
    _camera->setThetaAngle(0.0f);

    _camera->setDistance(5.0f);
    _camera->setCollision(0);

    // beautiful view
    _camera->setThetaAngle(45);

    Game::get()->setPlayer(_camera->getPlayer());

    return true;
}

WidgetButtonPtr MaintenanceWorldLogic::_createButton(QSize size, QPoint pos,
                                                     String toolTip,
                                                     String pathToIcon,
                                                     ImagePtr image,
                                                     QSize iconSize,
                                                     bool toggleable,
                                                     bool arrange)
{
    auto button = WidgetButton::create(Gui::get());
    button->setBackground(0);
    button->setToggleable(toggleable ? 1 : 0);

    if(!toolTip.empty())
        button->setToolTip(toolTip);

    if(arrange)
        button->arrange();

    if(!size.isNull())
    {
        button->setWidth(size.width());
        button->setHeight(size.height());
    }

    if(!pos.isNull())
        button->setPosition(pos.x(), pos.y());

    ImagePtr icon;
    if(pathToIcon.empty())
        icon = image;
    else if(!image.get())
        icon = Image::create(pathToIcon);

    if(icon)
    {
        if(!iconSize.isEmpty())
            icon->resize(iconSize.width(), iconSize.height());
        button->setImage(icon);
    }

    return button;
}

void MaintenanceWorldLogic::_zoomCamera()
{
    auto zoomAction = [&](float step){
        const auto distance = _camera->getDistance();
        _camera->setDistance(distance - step);
    };

    auto app = App::get();

    // Mouse wheel
    const int axis = app->getMouseAxis(App::AXIS_Y);
    if(axis != 0)
        zoomAction(axis * 0.5f);

    // Keyboard
    if(app->getKeyState(App::KEY_CTRL) && app->getKeyState('-'))
        zoomAction(-0.10f);

    if(app->getKeyState('-'))
        zoomAction(-0.02f);

    if(app->getKeyState(App::KEY_CTRL) && app->getKeyState('='))
        zoomAction(0.10f);

    if(app->getKeyState('='))
        zoomAction(0.02f);

    if(app->getKeyState(' '))
        _fitZoom(_selectedObject);
}

void MaintenanceWorldLogic::_selectElement()
{
    auto app = App::get();

    if(app->getMouseButtonState(App::BUTTON_LEFT) == 0 && _mouseLeftButtonState == 1)
    {
        //! \todo replace 35 to ifps parameter
        if(_frameCount > 35)
        {
            _mouseLeftButtonState = 0;
            return;
        }

        Vec3 p0, p1;

        PlayerPtr player = Game::get()->getPlayer();
        if (player.get() == nullptr)
            return;

        const auto width = App::get()->getWidth();
        const auto height = App::get()->getHeight();

        const auto x = App::get()->getMouseX();
        const auto y = App::get()->getMouseY();

        player->getDirectionFromScreen(p0, p1, x, y, width, height);

        auto intersection = WorldIntersection::create();
        auto object = World::get()->getIntersection(p0,p1,1,intersection);

        if(object)
        {
            _createSplineToTarget(vec3(object->getWorldBoundSphere().getCenter()));
            _setOutline(object);

            _selectedObject = object;
            _selectedGroup = NodePtr{};

            _partNameLabel->setText(object->getName());
            emit partSelected(object->getID());
        }

        _mouseLeftButtonState = 0;
    }
    else if(app->getMouseButtonState(App::BUTTON_LEFT) == 1 && _mouseLeftButtonState == 0)
    {
        _frameCount = 0;
        _mouseLeftButtonState = 1;
    }
    else if(app->getMouseButtonState(App::BUTTON_LEFT) == 1 && _mouseLeftButtonState == 1)
        ++_frameCount;
}

void MaintenanceWorldLogic::_moveTarget()
{
    if(_startTargetAnimation)
    {
        _targetAnimStatus += 1 / App::get()->getFps();
        if(_targetAnimStatus > 1)
        {
            _targetAnimStatus     = 1;
            _startTargetAnimation = false;

            if(_fitZoomAfterCamAnimStop)
            {
                _fitZoom(_selectedObject);

                _fitZoomAfterCamAnimStop = false;
            }
        }

        if(_targetSpline->getNumPoints() > 0)
            _target->setPosition(Vec3(_targetSpline->calcSegmentPoint(0, _targetAnimStatus)));
    }
}

void MaintenanceWorldLogic::_setOutline(ObjectPtr& object)
{
    auto model = object->getParent();
    while(model->getParent().get())
        model = model->getParent();

    _applyStdMat(model);

    object->setMaterial(_getMaterialName(object->getNode(), String(OUTLINE_POSTFIX)), "*");
}

void MaintenanceWorldLogic::_setOutline(NodePtr &node)
{
    _applyStdMat(_visibleModel);
    _setGroupOutline(node);
}

Vec3 MaintenanceWorldLogic::_getCoarseCenter(const NodePtr &group)
{
    auto getCenter = [](const NodePtr& node){
        return node->getWorldBoundSphere().getCenter();
    };

    auto min = getCenter(group->getChild(FIRST));
    auto max = min;

    for(int i = 0; i < group->getNumChildren(); ++i)
    {
        auto child = group->getChild(i);
        if(child->getNumChildren() > 0)
        {
            auto mid = _getCoarseCenter(child);
            min = Math::min(min, mid);
            max = Math::max(max, mid);
        }

        if(child->getType() != Node::NODE_DUMMY)
        {
            min = Math::min(min, getCenter(child));
            max = Math::max(max, getCenter(child));
        }
    }

    const auto bs = WorldBoundSphere{WorldBoundBox(min, max)};

    return bs.getCenter();
}

void MaintenanceWorldLogic::_setGroupOutline(NodePtr &parent)
{
    for(int i = 0; i < parent->getNumChildren(); ++i)
    {
        auto child = parent->getChild(i);
        if(child->getNumChildren() > 0)
            _setGroupOutline(child);

        if(child->getType() != Node::NODE_DUMMY)
            Object::cast(child)->setMaterial(_getMaterialName(child, String(OUTLINE_POSTFIX)), "*");
    }
}

void MaintenanceWorldLogic::_fitZoom(ObjectPtr &object)
{
    if(!object.get())
        return;

    const auto radius = object->getBoundSphere().getRadius();
    const auto& scaleVec = object->getScale();
    const auto scale = min({scaleVec.x, scaleVec.y, scaleVec.z}, [](float a, float b){
        return a < b;
    });

    _camera->setDistance(radius * 144.5f * scale / 100.0f);
}

void MaintenanceWorldLogic::_applyStdMat(NodePtr &model)
{
    const auto num = model->getNumChildren();
    for(int i = 0; i < num; ++i)
    {
        auto node = model->getChild(i);
        switch(node->getType())
        {
        case Node::OBJECT_MESH_STATIC:
            if(_transparentToggle->isChecked())
                Object::cast(node)->setMaterial(TRANSPARENT_MAT, "*");
            else
                Object::cast(node)->setMaterial(_getMaterialName(node), "*");
            break;
        default:
            _applyStdMat(node);
        }
    }
}

String MaintenanceWorldLogic::_getMaterialName(const NodePtr &part, const String& postfix/* = ""*/)
{
    String mat = ObjectMeshStatic::cast(part)->getSurfaceName(FIRST);
    if(!Materials::get()->findMaterial(mat).get())
        mat = DEFAULT_MAT;

    mat += postfix;

    return mat;
}

String MaintenanceWorldLogic::_getMaterialName(const ObjectMeshStaticPtr &mesh, const String &postfix)
{
    String mat = mesh->getSurfaceName(FIRST);
    if(!Materials::get()->findMaterial(mat).get())
        mat = DEFAULT_MAT;

    mat += postfix;

    return mat;
}

QVariantList MaintenanceWorldLogic::_getParts(NodePtr& parent)
{
    QVariantList names;
    QVariantList ids;

    const auto num = parent->getNumChildren();
    for(int i = 0; i < num; ++i)
    {
        auto part = parent->getChild(i);
        auto id = part->getID();
        const QString partName{part->getName()};
        if(part->getNumChildren() > 0)
            names.append(QVariantMap{{partName, _getParts(part)}});
        else
            names.append(partName);
        ids.append(QString::number(id));
    }

    return {names, ids};
}

QVariantList MaintenanceWorldLogic::_getInitParts(NodePtr &parent)
{
    auto initialName = [](const NodePtr& node){
        const auto propsNum = node->getNumProperties();
        for(auto propId = 0; propId < propsNum; ++propId)
        {
            const String propName = node->getPropertyName(propId);
            if(propName != "initial_name")
                continue;

            return QString(node->getProperty(propId)->getParameterString(0));
        }

        return QString{};
    };

    QVariantList names;
    const auto num = parent->getNumChildren();
    for(int i = 0; i < num; ++i)
    {
        auto part = parent->getChild(i);

        const auto partName = initialName(part);
        if(part->getNumChildren() > 0)
            names.append(QVariantMap{{partName, _getInitParts(part)}});
        else
            names.append(partName);
    }

    return {names, QVariantList{}};
}

void MaintenanceWorldLogic::_createSplineToTarget(const vec3 &target)
{
    if(!_target.get())
    {
        Log::error("Target %s does not exist", TARGET);
        return;
    }

    _targetSpline->clear();
    _targetSpline->addPoint(vec3(_target->getPosition()));
    _targetSpline->addPoint(target);

    _targetSpline->addSegment(0, vec3::ZERO, vec3::ZERO, 1, vec3::ZERO, vec3::ZERO);

    _targetAnimStatus     = 0;
    _startTargetAnimation = true;
}

void MaintenanceWorldLogic::_importCAD(const String &path)
{
    /* Create model folders */
    auto absoluteImportPath = String(FileSystem::get()->getMount(IMPORT_FOLDER)->getDataPath()) + "/";
    auto importModelPath = absoluteImportPath + path.filename() + "/";
    auto dataModelPath = _modelDirPath + path.filename() + "/";

    _mkdir(dataModelPath);
    _mkdir(importModelPath);

    /* Import */
    auto importer = Import::get()->createImporterByFileName(path);
    importer->removePreProcessor("MergeStaticMeshes");
    importer->addPreProcessor("MergeSurfacesByMaterials");
    importer->addPreProcessor("SplitByBound");
    importer->removePreProcessor("Repivot");
    importer->setParameterInt("need_triangulate", 1);
    importer->setParameterInt("vertex_cache", 1);
    importer->setParameterInt("create_unique_material_names", 1);
    importer->setParameterInt("workflow", 0);
    importer->setParameterFloat("scale", 3.0f);
    importer->setParameterFloat("linear_deflection", 0.2f);
    importer->setParameterFloat("angular_deflection", 0.7f);

    importer->init(path, Importer::IMPORT_MESHES | Importer::IMPORT_MATERIALS);

    auto importedMatsPath = String(IMPORT_FOLDER) + IMPORTED_MATERIALS_FOLDER;
    importer->import(importedMatsPath);

    /* Creating parent node for NodeReference */
    auto parent = NodeDummy::create();
    parent->setName(path.filename());
    parent->release();

#if 1
    /* Replace .mesh files to its model folder and remove combined.node*/
    auto dir = Dir::create();
    dir->open(/*String::joinPaths(absoluteImportPath.get(), IMPORTED_MATERIALS_FOLDER).get()*/importedMatsPath.get());
    if(!dir->isOpened())
    {
        Log::error("Failed to open %s (%s)\n", importedMatsPath.get(), String::joinPaths(absoluteImportPath.get(), IMPORTED_MATERIALS_FOLDER).get());
        return;
    }

    for(int i = 0; i < dir->getNumFiles(); ++i)
    {
        String file = dir->getFileName(i);
        const auto extension = file.extension();
        const auto fileImportPath = importModelPath + file.basename();
        if(extension == "mat")
        {
            _replace(file, fileImportPath);
            _makeOutlineMaterial(fileImportPath);
        }
        else if(extension == "mesh")
        {
            _replace(file, fileImportPath);
            auto mesh = ObjectMeshStatic::create(fileImportPath);
            mesh->setMaterial(_getMaterialName(mesh), "*");
            auto node = mesh->getNode();
            node->setName(file.filename());
            parent->addChild(node);
            mesh->release();
        }
        else if(extension == "node")
            _remove(file);
    }
#endif

    /* Creating NodeReference */
    parent->setWorldPosition(Vec3::ZERO);
    parent->setEnabled(0);

    /* Save node and reload world */
    World::get()->saveNode(dataModelPath + path.filename() + ".node", parent->getNode());

    const auto parts = getParts(dataModelPath + path.filename() + ".node");

    emit importComplete(QString(path.filename()), parts);
}

bool MaintenanceWorldLogic::_mountPointsInit()
{
    /* Create mount point */
    auto filesystem = FileSystem::get();
    if(filesystem->getMount("import/").get())
        return true;

    const auto importPath = String(filesystem->getRootMount()->getDataPath()) + String(IMPORT_PATH);

    auto mount = filesystem->createMount(importPath, "import/", FileSystemMount::ACCESS_READWRITE);
    if(!mount)
    {
        Log::error("Failed to create mount point to %s\n", importPath.get());
        return false;
    }

    Console::get()->run("filesystem_reload");

    return true;
}

bool MaintenanceWorldLogic::_mkdir(const String &path)
{
    auto engine = Engine::get();
    if(!engine)
    {
        Log::error("Failed to get Engine\n");
        return false;
    }

    if(!engine->runWorldFunction(Variable("make_dir"), Variable(path)).getInt())
    {
        Log::warning("Path '%s' exists or you have no permission on write\n", path.get());
        return false;
    }

    return true;
}

bool MaintenanceWorldLogic::_replace(const String &oldPath, const String &newPath)
{
    auto engine = Engine::get();
    if(!engine)
    {
        Log::error("Failed to get Engine\n");
        return false;
    }

    if(!engine->runWorldFunction(Variable("replace_file"), Variable(oldPath), Variable(newPath)).getInt())
    {
        Log::warning("Failed to replace %s to %s\n", oldPath.get(), newPath.get());
        return false;
    }

    return true;
}

bool MaintenanceWorldLogic::_remove(const String &path)
{
    auto engine = Engine::get();
    if(!engine)
    {
        Log::error("Failed to get Engine\n");
        return false;
    }

    if(!engine->runWorldFunction(Variable("remove_path"), Variable(path)).getInt())
    {
        Log::warning("Failed to remove %s\n", path.get());
        return false;
    }

    return true;
}

void MaintenanceWorldLogic::_makeOutlineMaterial(const String &path)
{
    auto name = path.filename();
    if(name.contains(OUTLINE_POSTFIX))
        return;

    auto materials = Materials::get();
    auto base = materials->findMaterial(name.upper());
    if(!base.get())
    {
        Log::warning("Failed to load %s\n", path.basename().get());
        return;
    }

    auto outline = base->inherit(name + OUTLINE_POSTFIX);
    if(!outline.get())
    {
        Log::warning("Failed to inherits from %s\n", path.get());
        return;
    }

    outline->setParameterSlider(outline->findParameter("metalness"), 1.0f);
    outline->setState("auxiliary", 1);
    outline->setParameter("auxiliary_color", vec4(1.0f, 0.334f, 0.0f, 1.0f));
    outline->setName(name.upper() + OUTLINE_POSTFIX);
    outline->save(path.dirname() + "/" + name + OUTLINE_POSTFIX + ".mat");
}

bool MaintenanceWorldLogic::_initModelPathDir()
{
    const auto dataPath = String(FileSystem::get()->getRootMount()->getDataPath());
    auto modelsDir = Dir::create();
    const auto modelPath = dataPath + String(MODELS_PATH);
    modelsDir->open(modelPath);
    if(!modelsDir->isOpened())
        return false;

    _modelDirPath = modelPath;

    return true;
}

void MaintenanceWorldLogic::_onTransparentToggleChanged()
{
    if(_selectedObject.get())
        _setOutline(_selectedObject);
    else if(_selectedGroup.get())
        _setOutline(_selectedGroup);
    else if(_visibleModel.get())
        _applyStdMat(_visibleModel);
}

void MaintenanceWorldLogic::_onWidgetEnterLeave()
{
    _mouseControlEnabled = !_mouseControlEnabled;
}
