#ifndef WIDGET_MULTISCOPE_HPP
#define WIDGET_MULTISCOPE_HPP

#include "Features.hpp"

#if USE_NEW_SCOPE


#include <rack.hpp>
using namespace rack;

#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSModuleWidgetBase.hpp"
#include "Module_multiScope.hpp"

/// TODO: Widget/Module wide invert setting



struct TSScopeModuleResizeHandle : Widget {
	float minWidth;
	float defWidth;
	bool right = false;
	float dragX;
	Rect originalBox;
	bool dragStarted = false;

	TS_PadSwitch* displayToggleBtn = NULL;
	ColorValueLight* displayToggleLED = NULL;
	TS_PadSwitch* colorDisplayToggleBtn = NULL;
	ColorValueLight* colorDisplayToggleLED = NULL;

	TSScopeModuleResizeHandle(float minWidth, float defWidth) {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
		this->minWidth = minWidth;
		this->defWidth = defWidth;
		return;
	}
	TSScopeModuleResizeHandle(float minWidth, float defWidth, SvgPanel* bgPanel) : TSScopeModuleResizeHandle(minWidth, defWidth)
	{
		addChild(bgPanel);
		return;
	}
	TSScopeModuleResizeHandle(float minWidth, float defWidth, SvgPanel* bgPanel, TS_PadSwitch* displayBtn, ColorValueLight* displayLED) : TSScopeModuleResizeHandle(minWidth, defWidth, bgPanel)
	{
		this->displayToggleBtn = displayBtn;
		this->displayToggleLED = displayLED;
		return;
	}
	// Set positions of controls relative to our position (even though we don't own these).
	void setChildPositions()
	{
		if (displayToggleBtn)
		{
			float margin = (this->box.size.x - displayToggleBtn->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			displayToggleBtn->box.pos.x = this->box.pos.x + margin;
		}
		if (displayToggleLED)
		{
			float margin = (this->box.size.x - displayToggleLED->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			displayToggleLED->box.pos.x = this->box.pos.x + margin;
		}
		if (colorDisplayToggleBtn)
		{
			float margin = (this->box.size.x - colorDisplayToggleBtn->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			colorDisplayToggleBtn->box.pos.x = this->box.pos.x + margin;
		}
		if (colorDisplayToggleLED)
		{
			float margin = (this->box.size.x - colorDisplayToggleLED->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			colorDisplayToggleLED->box.pos.x = this->box.pos.x + margin;
		}
		return;
	}

	void setNewSizeAndPos(Rect& newBox) {
		ModuleWidget* m = getAncestorOfType<ModuleWidget>();
		if (m) {
			m->box = newBox;
			if (!APP->scene->rack->requestModulePos(m, newBox.pos)) {
				m->box = originalBox;
			}
		}
		return;
	}

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			e.consume(this);
		}		
	}
	void onDoubleClick(const DoubleClickEvent& e) override {
		ModuleWidget* m = getAncestorOfType<ModuleWidget>();
		dragStarted = false;
		if (m) {
			originalBox = m->box;
			// Return to default width
			Rect newBox = originalBox;
			newBox.size.x = defWidth;
			if (!right) {
				newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
			}
			this->setNewSizeAndPos(newBox);
			e.consume(this);
		}
		return;
	}
	void onDragStart(const event::DragStart &e) override {
		// [Rack v2] mousePos no longer accessible. Now accessor getMousePos().
		dragX = APP->scene->rack->getMousePos().x; // APP->scene->rack->mousePos.x;//gRackWidget->lastMousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		if (m) {
			originalBox = m->box;
			dragStarted = true;
			e.consume(this);
		}
		return;
	}
	void onDragMove(const event::DragMove &e) override {
		if (!dragStarted)
			return;
		e.consume(this);
		// [Rack v2] mousePos no longer accessible. Now accessor getMousePos().
		float newDragX = APP->scene->rack->getMousePos().x;
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			if (newBox.size.x < minWidth)
				newBox.size.x = minWidth;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			if (newBox.size.x < minWidth)
				newBox.size.x = minWidth;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		this->setNewSizeAndPos(newBox);
	}
	void onDragEnd(const DragEndEvent& e) override 
	{
		if (dragStarted) {
			e.consume(this);
			dragStarted = false;
		}
		return;
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScopeWidget
// Widget for the scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct multiScopeWidget : TSSModuleWidgetBase {
	//Panel* panel;
	TSScopeModuleResizeHandle* rightHandle;
	TransparentWidget* display[TROWA_SCOPE_NUM_WAVEFORMS];
	TS_Panel* screenContainer;
	TS_ColorSlider* colorSliders[3];
	TSScopeDisplay* scopeInfoDisplay;
	// Keep screw references to move them.
	TS_DEFAULT_SCREW* rhsScrews[2];
	int inputAreaWidth;
	bool plugLightsEnabled = true;
	// Keep references to our ports to disable lights or change the colors.
	TS_Port* inputPorts[multiScope::NUM_INPUTS];
	// Keep references to our scale knobs [0: x, 1: y]
	TS_TinyBlackKnob* scaleKnobs[TROWA_SCOPE_NUM_WAVEFORMS][2];
	ColorValueLight* fillColorLEDs[TROWA_SCOPE_NUM_WAVEFORMS];
	bool sizeLoaded = false;
	int lastWidth;
	NVGcolor* lastEditColorPtr;
	TSColorHSL _lastBgColor;
#if DEBUG_COLOR_SLIDER
	int debugCount = 0;
#endif
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiScopeWidget()
	// Instantiate a multiScope widget. v0.60 must have module as param.
	// @scopeModule : (IN) Pointer to the multiScope module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	multiScopeWidget(multiScope* scopeModule);
	void step() override;
	// /** 
	// Overriding these is deprecated.
	// Use Module::dataToJson() and dataFromJson() instead
	// */	
	// json_t *toJson() override;
	// void fromJson(json_t *rootJ) override;	
	//Menu *createContextMenu() override;
};

#endif // use new scope

#endif // end if not defined