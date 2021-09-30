#include "ViewCube.h"

#include <UnigineApp.h>
#include <UnigineGame.h>
#include <UnigineMesh.h>

using namespace Unigine;
using namespace Unigine::Math;

////////////////////////////////////////////////////////////////////////////////
namespace
{
	// Constants.
	const char TEXTURE_REGULAR[] = "viewcube/viewcube.png";
	const char TEXTURE_HIGHLIGHTED[] = "viewcube/viewcube_highlighted.png";

	const char MESH_VIEWCUBE[] = "viewcube/viewcube.mesh";
	const char MESH_VIEWCUBE_ARROWS[] = "viewcube/viewcube_arrows.mesh";

	const int WIDTH = 120;
	const int HEIGHT = 120;
	const float HALF_WIDTH = WIDTH / 2.0f;
	const float HALF_HEIGHT = HEIGHT / 2.0f;

	const float SCALE = 55.0f;
	const float FOV = 40.0f;
	const float ASPECT = 1.0f;
	const float ZNEAR = 0.0f;
	const float ZFAR = 100.0f;

	const mat4 ORTHOGRAPHIC_TRANSFORM =
		translate(HALF_WIDTH, HALF_HEIGHT, 0.0f) *
		scale(SCALE, -SCALE, SCALE);

	const mat4 PERSPECTIVE_TRANSFORM =
		ORTHOGRAPHIC_TRANSFORM *
		perspective(FOV, ASPECT, ZNEAR, ZFAR) *
		translate(0, 0, -3.3f);

	// Utils.
	vec3 get_polygon_pos(const char *name);
    quat get_direction_quat(const vec3& direction);

    vec3 getPolygonAngles(const char *name);

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
ViewCube::ViewCube(const GuiPtr &gui)
    : gui_(gui)
    , _player(PlayerPersecutor::cast(Game::get()->getPlayer()))
    , _wasPressed(false)
    , _mouseOver(false)
    , _stepTheta(0.0f)
    , _stepPhi(0.0f)
    , _count(0)
{
    create_widgets();

    if(!_player.get())
        Log::error("PlayerPersecutor is NULL. Initialize it first.");
}

ViewCube::~ViewCube()
{
	_mainHbox.destroy();
}

void ViewCube::update(const vec3& target)
{
    if(!isVisible() || !_player.get())
        return;

    if(_count > 0)
    {
        _player->setThetaAngle(_player->getThetaAngle() - _stepTheta);
        _player->setPhiAngle(_stepPhi);
        --_count;
    }

    rotation_ = get_direction_quat(_player->getViewDirection());

	int mouse_x = _mainHbox->getMouseX();
	int mouse_y = _mainHbox->getMouseY();
    _mouseOver = static_cast<bool>(_mainHbox->getIntersection(mouse_x, mouse_y));

    float mouse_distance = static_cast<float>(sqrt(pow(mouse_x - HALF_WIDTH, 2) + pow(mouse_y - HALF_HEIGHT, 2)));
	vec4 color = vec4::ONE;
	color.w = 1.0f - saturate(mouse_distance / WIDTH / 4.0f - 0.1f);
    if(arrows_color_ != color)
	{
		arrows_color_ = color;
        for(int i = 0; i < _arrowsCanvas->getNumPolygons(); i++)
            _arrowsCanvas->setPolygonColor(i, color);
	}

	// cube
	int cube_polygon = -1;

    if(_mouseOver)
	{
        for(int i = 0; i < _cubeCanvas->getNumPolygons(); i++)
            _cubeCanvas->setPolygonTexture(i, TEXTURE_REGULAR);
        cube_polygon = _cubeCanvas->getPolygonIntersection(mouse_x, mouse_y);
        if(cube_polygon != -1 && _cubeCanvas->getPolygonTexture(cube_polygon) != TEXTURE_HIGHLIGHTED)
            _cubeCanvas->setPolygonTexture(cube_polygon, TEXTURE_HIGHLIGHTED);

        for(int i = 0; i < _cubeCanvas->getNumPolygons(); i++)
		{
            vec3 pos = rotation_ * cube_polygon_positions_[i];
            int order = static_cast<int>((1.0f + pos.z) * 100.0f);
            _cubeCanvas->setPolygonOrder(i, order);
		}
	}

	// arrows
    if(color.w <= 0.0f && _arrowsCanvas->isHidden())
		return;

	int arrows_hidden = 1;

	vec3 dir = normalize(conjugate(rotation_) * vec3(0, 0, 1));
	dir.x = Unigine::Math::abs(dir.x);
	dir.y = Unigine::Math::abs(dir.y);
	dir.z = Unigine::Math::abs(dir.z);
    if(length(dir - vec3(1, 0, 0)) < 0.1f)
		arrows_hidden = 0;
    else if(length(dir - vec3(0, 1, 0)) < 0.1f)
		arrows_hidden = 0;
    else if(length(dir - vec3(0, 0, 1)) < 0.1f)
		arrows_hidden = 0;

    _arrowsCanvas->setHidden(arrows_hidden);

	int arrow_polygon = -1;
    if(!arrows_hidden && _mouseOver)
	{
        for(int i = 0; i < _arrowsCanvas->getNumPolygons(); i++)
            _arrowsCanvas->setPolygonTexture(i, TEXTURE_REGULAR);

        arrow_polygon = _arrowsCanvas->getPolygonIntersection(mouse_x, mouse_y);
        if(arrow_polygon != -1 && _arrowsCanvas->getPolygonTexture(arrow_polygon) != TEXTURE_HIGHLIGHTED)
            _arrowsCanvas->setPolygonTexture(arrow_polygon, TEXTURE_HIGHLIGHTED);
	}

	// mouse click
    if(_mouseOver)
	{
        bool pressed = static_cast<bool>(App::get()->getMouseButtonState(App::BUTTON_LEFT));

        if(pressed && !_wasPressed)
		{
            if(cube_polygon != -1)
			{
				_mainHbox->setFocus();
                applyRotation(_cubePolygonRotations.at(cube_polygon), target);
			}
            else if(arrow_polygon != -1)
			{
				_mainHbox->setFocus();
                applyRotation(_arrowsPolygonRotations.at(arrow_polygon), target);
			}
			else
			{
				gui_->removeFocus();
			}
		}

        _wasPressed = pressed;
	}
	else
	{
		_mainHbox->removeFocus();
        _wasPressed = false;
	}
}

void ViewCube::render()
{
    if(!isVisible())
		return;

    _cubeCanvas->setTransform(PERSPECTIVE_TRANSFORM * mat4(rotation_));
    _arrowsCanvas->setTransform(ORTHOGRAPHIC_TRANSFORM * mat4(rotation_));
}

void ViewCube::create_widgets()
{
	_mainHbox = WidgetHBox::create(gui_);
	_mainHbox->setWidth(WIDTH);
	_mainHbox->setHeight(HEIGHT);

	MeshPtr mesh = Mesh::create(MESH_VIEWCUBE);
    _cubeCanvas = WidgetCanvas::create(gui_);

    for(int surface = 0; surface < mesh->getNumSurfaces(); ++surface)
	{
        const char *name = mesh->getSurfaceName(surface);
        vec3 pos = get_polygon_pos(name);
        cube_polygon_positions_.push_back(pos);

        const auto angles = getPolygonAngles(name);
        _cubePolygonRotations.push_back(angles);

		mesh->remapCVertex(surface);

        int polygon = _cubeCanvas->addPolygon();

        _cubeCanvas->setPolygonTwoSided(polygon, 0);

        for(int i = 0; i < mesh->getNumVertex(surface); ++i)
		{
			vec3 point = mesh->getVertex(i, surface);
            _cubeCanvas->addPolygonPoint(polygon, point);

			vec2 uv = mesh->getTexCoord0(i, surface);
            _cubeCanvas->setPolygonTexCoord(polygon, uv);
		}

        for(int i = 0; i < mesh->getNumCIndices(surface); ++i)
		{
			int index = mesh->getCIndex(i, surface);
            _cubeCanvas->addPolygonIndex(polygon, index);
		}
	}

	mesh = Mesh::create(MESH_VIEWCUBE_ARROWS);
    _arrowsCanvas = WidgetCanvas::create(gui_);

    for(int surface = 0; surface < mesh->getNumSurfaces(); ++surface)
	{
		const char *name = mesh->getSurfaceName(surface);
        const auto angles = getPolygonAngles(name);
        _arrowsPolygonRotations.push_back(angles);

		mesh->remapCVertex(surface);

        int polygon = _arrowsCanvas->addPolygon();

        for(int i = 0; i < mesh->getNumVertex(surface); ++i)
		{
			vec3 point = mesh->getVertex(i, surface);
            _arrowsCanvas->addPolygonPoint(polygon, point);

			vec2 uv = mesh->getTexCoord0(i, surface);
            _arrowsCanvas->setPolygonTexCoord(polygon, uv);
		}

        for(int i = 0; i < mesh->getNumCIndices(surface); ++i)
		{
			int index = mesh->getCIndex(i, surface);
            _arrowsCanvas->addPolygonIndex(polygon, index);
		}
	}

    for(int i = 0; i < _cubeCanvas->getNumPolygons(); ++i)
        _cubeCanvas->setPolygonTexture(i, TEXTURE_REGULAR);
    for(int i = 0; i < _arrowsCanvas->getNumPolygons(); ++i)
        _arrowsCanvas->setPolygonTexture(i, TEXTURE_REGULAR);

    _arrowsCanvas->setHidden(1);

    _mainHbox->addChild(_arrowsCanvas->getWidget(), Gui::ALIGN_OVERLAP);
    _mainHbox->addChild(_cubeCanvas->getWidget(), Gui::ALIGN_OVERLAP);

    _cubeCanvas->setTransform(PERSPECTIVE_TRANSFORM);
    _arrowsCanvas->setTransform(ORTHOGRAPHIC_TRANSFORM);
}

void ViewCube::applyRotation(const vec3& eulerAngles, const vec3& targetPos)
{
    if(!_player.get())
        return;

    const auto cameraPos = vec3(_player->getWorldPosition());
    const auto rot = lookAt(cameraPos, targetPos, _player->getUp()).getRotate();
    const auto currentEuler = decomposeRotationXYZ(rot.getMat3());

    auto delta = currentEuler - eulerAngles;

    _count = 200;
    _stepPhi = delta.z / 200;
    _stepTheta = delta.x / 200;
}

////////////////////////////////////////////////////////////////////////////////
namespace
{
	vec3 get_polygon_pos(const char *name)
	{
		vec3 pos = vec3::ZERO;

        if(strstr(name, "px"))
			pos.x = 1;
        if(strstr(name, "py"))
			pos.y = 1;
        if(strstr(name, "pz"))
			pos.z = 1;
        if(strstr(name, "nx"))
			pos.x = -1;
        if(strstr(name, "ny"))
			pos.y = -1;
        if(strstr(name, "nz"))
			pos.z = -1;

		return pos;
	}

    quat get_direction_quat(const vec3& direction)
    {
        auto up = vec3::UP;
        if(Math::abs(dot(direction, up) - 1.0f) < UNIGINE_EPSILON)
            up = vec3::BACK;

        return quat(lookAt(vec3::ZERO, direction, up));
    }

    vec3 getPolygonAngles(const char *name)
    {
        vec3 angles = vec3::ZERO;

        if(strstr(name, "px"))
            angles = vec3(-90.0f, 0.0f, -90.0f);
        if(strstr(name, "py"))
            angles = vec3(-90.0f, 0.0f, -180.0f);
        if(strstr(name, "pz"))
            angles = vec3(0.0f, 0.0f, 0.0f);
        if(strstr(name, "nx"))
            angles = vec3(-90.0f, 0.0f, 90.0f);
        if(strstr(name, "ny"))
            angles = vec3(-90.0f, 0.0f, 0.0f);
        if(strstr(name, "nz"))
            angles = vec3(-180.0f, 0.0f, -180.0f);
        if(strstr(name, "ny") && strstr(name, "px"))
            angles = vec3(-90.0f, 0.0f, -45.0f);
        if(strstr(name, "px") && strstr(name, "py"))
            angles = vec3(-90.0f, 0.0f, -135.0f);
        if(strstr(name, "py") && strstr(name, "nx"))
            angles = vec3(-90.0f, 0.0f, 135.0f);
        if(strstr(name, "nx") && strstr(name, "ny"))
            angles = vec3(-90.0f, 0.0f, 45.0f);
        if(strstr(name, "pz") && strstr(name, "py"))
            angles = vec3(-45.0f, 0.0f, -180.0f);
        if(strstr(name, "pz") && strstr(name, "px"))
            angles = vec3(-45.0f, 0.0f, -90.0f);
        if(strstr(name, "pz") && strstr(name, "ny"))
            angles = vec3(-45.0f, 0.0f, 0.0f);
        if(strstr(name, "pz") && strstr(name, "nx"))
            angles = vec3(-45.0f, 0.0f, 90.0f);
        if(strstr(name, "ny") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, 0.0f);
        if(strstr(name, "nx") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, 90.0f);
        if(strstr(name, "py") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, 180.0f);
        if(strstr(name, "px") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, -90.0f);
        if(strstr(name, "ny") && strstr(name, "px") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, -45.0f);
        if(strstr(name, "px") && strstr(name, "py") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, -135.0f);
        if(strstr(name, "py") && strstr(name, "nx") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, 135.0f);
        if(strstr(name, "nx") && strstr(name, "ny") && strstr(name, "nz"))
            angles = vec3(-135.0f, 0.0f, 45.0f);
        if(strstr(name, "ny") && strstr(name, "px") && strstr(name, "pz"))
            angles = vec3(-45.0f, 0.0f, -45.0f);
        if(strstr(name, "px") && strstr(name, "py") && strstr(name, "pz"))
            angles = vec3(-45.0f, 0.0f, -135.0f);
        if(strstr(name, "py") && strstr(name, "nx") && strstr(name, "pz"))
            angles = vec3(-45.0f, 0.0f, 135.0f);
        if(strstr(name, "nx") && strstr(name, "ny") && strstr(name, "pz"))
            angles = vec3(-45.0f, 0.0f, 45.0f);

        return angles;
    }
} // anonymous namespace
