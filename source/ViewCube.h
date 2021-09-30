#pragma once

#include <vector>
#include <UnigineGui.h>
#include <UnigineWidgets.h>
#include <UniginePlayers.h>

class ViewCube final
{
public:
	ViewCube(const Unigine::GuiPtr &gui);
	~ViewCube();

	ViewCube(const ViewCube &) = delete;
	ViewCube(ViewCube &&) = delete;
	ViewCube &operator=(const ViewCube &) = delete;
	ViewCube &operator=(ViewCube &&) = delete;

    bool isVisible() const { return !_mainHbox->isHidden(); }
    void setVisible(bool value) { _mainHbox->setHidden(!value); }

    Unigine::WidgetPtr getWidget() { return _mainHbox->getWidget(); }
    Unigine::WidgetCanvasPtr getCubeWidget() { return _cubeCanvas; }
    Unigine::WidgetCanvasPtr getArrowsWidget() { return _arrowsCanvas; }
    bool isMouseOver() { return _mouseOver; }

    void update(const Unigine::Math::vec3& target);
	void render();

private:
	void create_widgets();

    void applyRotation(const Unigine::Math::vec3& eulerAngles, const Unigine::Math::vec3& targetPos);

private:
	Unigine::GuiPtr gui_;

    Unigine::PlayerPersecutorPtr _player;

	Unigine::Math::quat rotation_;

	Unigine::Math::vec4 arrows_color_;

    std::vector<Unigine::Math::vec3> cube_polygon_positions_;

    std::vector<Unigine::Math::vec3> _cubePolygonRotations;
    std::vector<Unigine::Math::vec3> _arrowsPolygonRotations;

    bool _wasPressed;
    bool _mouseOver;

    Unigine::WidgetHBoxPtr _mainHbox;
    Unigine::WidgetCanvasPtr _cubeCanvas;
    Unigine::WidgetCanvasPtr _arrowsCanvas;

    // Custom
    float _stepTheta;
    float _stepPhi;
    int _count;
};
