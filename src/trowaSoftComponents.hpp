#ifndef TROWASOFT_COMPONENTS_HPP
#define TROWASOFT_COMPONENTS_HPP

#include <rack.hpp>
using namespace rack;

#include <string.h>
#include <stdio.h>
#include "window.hpp"
#include "ui.hpp"
#include "math.hpp"
#include "TSColors.hpp"
#include "componentlibrary.hpp"
#include "plugin.hpp"
#include "trowaSoftUtilities.hpp"
#include <color.hpp>
#include <nanovg.h>

extern Plugin* pluginInstance;

//=======================================================
// trowaSoft - TurtleMonkey Components 
//=======================================================


//::: Helpers :::::::::::::::::::::::::::::::::::::::::::
#ifndef  KNOB_SENSITIVITY
#define KNOB_SENSITIVITY 0.0015
#endif // ! KNOB_SENSITIVITY


//-----------------------------------------------------------------
// Form controls - Default colors and such
//-----------------------------------------------------------------
#define FORMS_DEFAULT_TEXT_COLOR		TSColors::COLOR_TS_TEXT
#define FORMS_DEFAULT_BORDER_COLOR		TSColors::COLOR_TS_BORDER
#define FORMS_DEFAULT_BG_COLOR			TSColors::COLOR_BLACK


//--------------------------------------------------------------
// ColorValueLight - Sorta like the old ColorValueLight that was in Rack.
//--------------------------------------------------------------
struct ColorValueLight : ModuleLightWidget {
	NVGcolor baseColor;
	// Pixels to add for outer radius (either px or relative %).
	float outerRadiusHalo = 0.35;
	bool outerRadiusRelative = true;
	ColorValueLight() : ModuleLightWidget()
	{
		bgColor = nvgRGB(0x20, 0x20, 0x20);
		borderColor = nvgRGBA(0, 0, 0, 0);
		return;
	};
	virtual ~ColorValueLight(){};
	// Set a single color
	void setColor(NVGcolor bColor)
	{
		color = bColor;
		baseColor = bColor;
		if (baseColors.size() < 1)
		{
			baseColors.push_back(bColor);			
		}
		else
		{
			baseColors[0] = bColor;
		}
	}
	void drawHalo(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + ((outerRadiusRelative) ? (radius*outerRadiusHalo) : outerRadiusHalo);

		nvgBeginPath(args.vg);
		nvgRect(args.vg, radius - oradius, radius - oradius, 2 * oradius, 2 * oradius);

		NVGpaint paint;
		NVGcolor icol = color::mult(color, 0.10);//colorMult(color, 0.10);
		NVGcolor ocol = nvgRGB(0, 0, 0);
		paint = nvgRadialGradient(args.vg, radius, radius, radius, oradius, icol, ocol);
		nvgFillPaint(args.vg, paint);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFill(args.vg);
	}
};

//------------------------------------------------------------------------------------------------
// TS_Label : Label with the trowaSoft default label font.
//------------------------------------------------------------------------------------------------
struct TS_Label : Label {
	// Font size. Default is 10.
	int fontSize = 10;
	// Font face
	std::shared_ptr<Font> font;
	// The font color. Default is Dark Gray.
	NVGcolor textColor = TSColors::COLOR_DARK_GRAY;
	enum TextAlignment {
		Left,
		Center,
		Right
	};
	// Text alignment. Default is Left.
	TextAlignment textAlign = TextAlignment::Left;
	enum VerticalAlignment {
		Baseline,
		Top,
		Middle,
		Bottom
	};
	// Vertical alignment. Default is Baseline.
	VerticalAlignment verticalAlign = VerticalAlignment::Baseline;
	// The text letter spacing (default 1.0f).
	float textLetterSpacing = 1.0f;

	bool drawBackground = false;
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	float padding = 0.f;

	TS_Label(const char* fontPath)
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, fontPath));
		return;
	}
	TS_Label() : TS_Label(TROWA_LABEL_FONT)
	{
		return;
	}
	TS_Label(const char* fontPath, Vec size) : TS_Label(fontPath)
	{
		box.size = size;
		return;
	}
	TS_Label(Vec size) : TS_Label(TROWA_LABEL_FONT)
	{
		box.size = size;
		return;
	}
	TS_Label(const char* fontPath, std::string text, Vec size) : TS_Label(fontPath, size)
	{
		this->text = text;
		return;
	}
	TS_Label(std::string text, Vec size) : TS_Label(size)
	{
		this->text = text;
		return;
	}

	void draw(const DrawArgs &args) override
	{
		if (visible)
		{
			//nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
			if (drawBackground)
			{
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
				nvgFillColor(args.vg, backgroundColor);
				nvgFill(args.vg);
				nvgStrokeWidth(args.vg, 1.0);
				nvgStrokeColor(args.vg, borderColor);
				nvgStroke(args.vg);
			}

			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, fontSize);
			nvgTextLetterSpacing(args.vg, textLetterSpacing);
			float x = 0;
			float y = 0;
			uint8_t alignment = 0x00;
			switch (textAlign)
			{
			case TextAlignment::Center:
				x = box.size.x / 2.0;
				alignment = NVGalign::NVG_ALIGN_CENTER;
				break;
			case TextAlignment::Right:
				x = box.size.x - padding;
				alignment = NVGalign::NVG_ALIGN_RIGHT;
				break;
			case TextAlignment::Left:
			default:
				x = 0.0f + padding;
				alignment = NVGalign::NVG_ALIGN_LEFT;
				break;
			}
			switch (verticalAlign)
			{
			case VerticalAlignment::Middle:
				y = box.size.y / 2.0;
				alignment |= NVGalign::NVG_ALIGN_MIDDLE;
				break;
			case VerticalAlignment::Bottom:
				y = box.size.y - padding;
				alignment |= NVGalign::NVG_ALIGN_BOTTOM;
				break;
			case VerticalAlignment::Top:
				y = 0.0f + padding;
				alignment |= NVGalign::NVG_ALIGN_TOP;
				break;
			case VerticalAlignment::Baseline:
			default:
				y = 0.0f + padding;
				alignment |= NVGalign::NVG_ALIGN_BASELINE;  // Default, align text vertically to baseline.
				break;
			}
			nvgTextAlign(args.vg, alignment);
			nvgFillColor(args.vg, textColor);
			nvgText(args.vg, x, y, text.c_str(), NULL);
		}
		return;
	}

};

//--------------------------------------------------------------
// TS_PadSwitch
// Empty momentary switch of given size.
//--------------------------------------------------------------
struct TS_PadSwitch : Switch {
	int btnId = -1;
	// Group id (to match guys that should respond to mouse down drag).
	int groupId = -1;
	TS_PadSwitch() : Switch() {
		momentary = true;
		return;
	}
	TS_PadSwitch(Vec size) {
		box.size = size;	
		return;
	}
	void setValue(float val) {
		if (paramQuantity)
		{
			paramQuantity->setValue(val);
		}		
		return;
	}
	// Allow mouse-down & drag to set buttons (i.e. on Sequencer grid where there are many buttons). 
	// Suggestion from @LKHSogpit, Solution from @AndrewBelt.
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/7
	// https://github.com/VCVRack/Rack/issues/607
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(const event::DragStart &e) override {
		if (paramQuantity)
		{
			if (momentary)
			{
				paramQuantity->setValue(paramQuantity->maxValue); // Trigger Value				
			}
			else
			{
				float newVal = (paramQuantity->getValue() < paramQuantity->maxValue) ? paramQuantity->maxValue : paramQuantity->minValue;
//DEBUG("onDragStart(%d) - Current Value is %.1f, toggling to %.1f.", btnId, paramQuantity->getValue(), newVal);			
				paramQuantity->setValue(newVal); // Toggle Value				
			}
		}
		return;
	}
	/** Called when the left button is released and this widget is being dragged */
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/12
	// Last button keeps pressed down.
	void onDragEnd(const event::DragEnd &e) override {
		if (paramQuantity) {
// DEBUG("onDragEnd(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, paramQuantity->getValue(), paramQuantity->minValue);
			// paramQuantity->setValue(paramQuantity->minValue); // Turn Off			
		}
		return;
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(const event::DragEnter &e) override {	
		// Set these no matter what because if you drag back onto your starting square, you want to toggle it again.
		TS_PadSwitch *origin = dynamic_cast<TS_PadSwitch*>(e.origin);
		if (origin && origin != this && origin->groupId == this->groupId && paramQuantity) {
			float newVal = (paramQuantity->getValue() < paramQuantity->maxValue) ? paramQuantity->maxValue : paramQuantity->minValue;
//DEBUG("onDragEnter(%d) - Current Value is %.1f, toggling to %.1f", btnId, paramQuantity->getValue(), newVal);						
			paramQuantity->setValue(newVal); // Toggle Value
		}
	}
	void onDragLeave(const event::DragLeave &e) override {
		TS_PadSwitch *origin = dynamic_cast<TS_PadSwitch*>(e.origin);
		if (origin && origin->groupId == this->groupId && paramQuantity) {
			if (momentary)
			{
//DEBUG("onDragLeave(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, paramQuantity->getValue(), paramQuantity->minValue);
				paramQuantity->setValue(paramQuantity->minValue); // Turn Off				
			}
		}
	}
	void onButton(const event::Button &e) override {
		Widget::onButton(e);
		e.stopPropagating();
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			// if (e.action == GLFW_RELEASE && paramQuantity) // onMouseUp?
				// paramQuantity->setValue(paramQuantity->minValue); // Turn Off
			e.consume(this);
		}
	}
};

//--------------------------------------------------------------
// TS_PadSwitch
//--------------------------------------------------------------
struct TS_PadSvgSwitch : SvgSwitch {
	int btnId = -1;
	// Group id (to match guys that should respond to mouse down drag).
	int groupId = -1;
	TS_PadSvgSwitch() : SvgSwitch() {
		momentary = false;
		return;
	}
	TS_PadSvgSwitch(Vec size) : TS_PadSvgSwitch() {
		box.size = size;	
		return;
	}
	void setValue(float val) {
		if (paramQuantity)
		{
			paramQuantity->setValue(val);
		}		
		return;
	}
	
	void toggleVal()
	{
		if (paramQuantity)
		{
			float newVal = (paramQuantity->getValue() < paramQuantity->maxValue) ? paramQuantity->maxValue : paramQuantity->minValue;
			paramQuantity->setValue(newVal); // Toggle Value
		}
		return;
	}
	
	// Allow mouse-down & drag to set buttons (i.e. on Sequencer grid where there are many buttons). 
	// Suggestion from @LKHSogpit, Solution from @AndrewBelt.
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/7
	// https://github.com/VCVRack/Rack/issues/607
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(const event::DragStart &e) override {
		if (paramQuantity)
		{
			float newVal = (paramQuantity->getValue() < paramQuantity->maxValue) ? paramQuantity->maxValue : paramQuantity->minValue;
DEBUG("onDragStart(%d) - Current Value is %.1f, toggling to %.1f.", btnId, paramQuantity->getValue(), newVal);			
			paramQuantity->setValue(newVal); // Toggle Value
		}
		return;
	}
	/** Called when the left button is released and this widget is being dragged */
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/12
	// Last button keeps pressed down.
	void onDragEnd(const event::DragEnd &e) override {
		if (paramQuantity) {
//DEBUG("onDragEnd(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, paramQuantity->getValue(), paramQuantity->minValue);
			// paramQuantity->setValue(paramQuantity->minValue); // Turn Off			
		}
		return;
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(const event::DragEnter &e) override {	
		// Set these no matter what because if you drag back onto your starting square, you want to toggle it again.
		TS_PadSvgSwitch *origin = dynamic_cast<TS_PadSvgSwitch*>(e.origin);
		if (origin && origin != this && origin->groupId == this->groupId && paramQuantity) {
			float newVal = (paramQuantity->getValue() < paramQuantity->maxValue) ? paramQuantity->maxValue : paramQuantity->minValue;
//DEBUG("onDragEnter(%d) - Current Value is %.1f, toggling to %.1f", btnId, paramQuantity->getValue(), newVal);						
			paramQuantity->setValue(newVal); // Toggle Value
		}
	}
	void onDragLeave(const event::DragLeave &e) override {
		SvgSwitch::onDragLeave(e);
		//TS_PadSvgSwitch *origin = dynamic_cast<TS_PadSvgSwitch*>(e.origin);
		// if (origin && origin->groupId == this->groupId && paramQuantity && !e.isConsumed()) {
// DEBUG("onDragLeave(%d) - Current Value is %.1f, setting to %.1f (off).", btnId, paramQuantity->getValue(), paramQuantity->minValue);
			// paramQuantity->setValue(paramQuantity->minValue); // Turn Off
		// }
		return;
	}
	void onButton(const event::Button &e) override {
		Widget::onButton(e);
		e.stopPropagating();
//DEBUG("onButton(%d) - Curr Value is %.1f", paramQuantity->getValue());
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			if (paramQuantity)
			{
				if (e.action == GLFW_RELEASE) // onMouseUp?
				{
					// paramQuantity->setValue(paramQuantity->minValue); // Turn Off				
//DEBUG("onButton(%d) - Left Click Release - Current Value is %.1f, setting to %.1f (off).", btnId, paramQuantity->getValue(), paramQuantity->minValue);				
				}				
			}
			e.consume(this);
		}
	}
};


//--------------------------------------------------------------
// TS_PadSquare - A Square Pad button.
//--------------------------------------------------------------
struct TS_PadSquare : TS_PadSvgSwitch {
	TS_PadSquare() 
	{
		this->SvgSwitch::addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_0.svg")));
		sw->wrap();
		SvgSwitch::box.size = sw->box.size;
	}
	TS_PadSquare(Vec size)
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ComponentLibrary/TS_pad_0.svg")));
		sw->box.size = size;
		SvgSwitch::box.size = size;
	}
};

//--------------------------------------------------------------
// TS_PadBtn - A wide Pad button. (Empty text)
//--------------------------------------------------------------
struct TS_PadBtn : SvgSwitch { // MomentarySwitch
	
	TS_PadBtn() 
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_btn_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_btn_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//--------------------------------------------------------------
// TS_PadRun - A wide Pad button. (RUN >)
//--------------------------------------------------------------
struct TS_Pad_Run : SvgSwitch { // MomentarySwitch
	
	TS_Pad_Run() 
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_run_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_run_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//--------------------------------------------------------------
// TS_PadReset - A wide Pad button. (< RST)
//--------------------------------------------------------------
struct TS_Pad_Reset : SvgSwitch { // MomentarySwitch
	
	TS_Pad_Reset() 
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_reset_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/TS_pad_reset_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

struct HideableLEDButton : LEDButton
{

	void draw(const DrawArgs &args) override {
		if (visible) {
			LEDButton::draw(args);
		}
	}
	void onButton(const event::Button &e) override {
		if (visible){
			LEDButton::onButton(e);
		}
		return;
	}
	// void onMouseDown(const event::MouseDown &e) override {
		// if (visible) {
			// LEDButton::onMouseDown(e);
		// }
	// };
	// void onMouseUp(const event::MouseUp &e) override {
		// if (visible) {
			// LEDButton::onMouseUp(e);
		// }
	// };
	// /** Called on every frame, even if mouseRel = Vec(0, 0) */
	// void onMouseMove(const event::MouseMove &e) override {
		// if (visible) {
			// LEDButton::onMouseMove(e);
		// }
	// }
	void onHoverKey(const event::HoverKey &e) override {
		if (visible) {
			LEDButton::onHoverKey(e);
		}
	};
	///** Called when this widget begins responding to `onMouseMove` events */
	//virtual void onMouseEnter(const event::MouseEnter &e) {}
	///** Called when another widget begins responding to `onMouseMove` events */
	//virtual void onMouseLeave(const event::MouseLeave &e) {}
	//virtual void onFocus(const event::Focus &e) {}
	//virtual void onDefocus(const event::Defocus &e) {}
	//virtual void onScroll(const event::Scroll &e);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(const event::DragStart &e) override {
		if (visible) {
			LEDButton::onDragStart(e);
		}
	}
	/** Called when the left button is released and this widget is being dragged */
	void onDragEnd(const event::DragEnd &e) override {
		if (visible) {
			LEDButton::onDragEnd(e);
		}
	}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	void onDragMove(const event::DragMove &e) override {
		if (visible) {
			LEDButton::onDragMove(e);
		}
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(const event::DragEnter &e) override {
		if (visible) {
			LEDButton::onDragEnter(e);
		}
	}
	void onDragLeave(const event::DragLeave &e) override {
		if (visible) {
			LEDButton::onDragLeave(e);
		}
	}
	void onDragDrop(const event::DragDrop &e) override {
		if (visible) {
			LEDButton::onDragDrop(e);
		}
	}
	void onPathDrop(const event::PathDrop &e)override {
		if (visible) {
			LEDButton::onPathDrop(e);
		}
	}

	void onAction(const event::Action &e) override {
		if (visible) {
			LEDButton::onAction(e);
		}
	}


};
//--------------------------------------------------------------
// TS_ScreenBtn - Screen button.
//--------------------------------------------------------------
struct TS_ScreenBtn : Switch {
	bool visible = true;
	// Text to display on the btn.
	std::string btnText;
	// Background color
	NVGcolor backgroundColor = nvgRGBA(0, 0, 0, 0);
	// Text color
	NVGcolor color = TSColors::COLOR_TS_GRAY;
	// Border color
	NVGcolor borderColor = TSColors::COLOR_TS_GRAY;
	// Border width
	int borderWidth = 1;
	// Corner radius. 0 for straight corners.
	int cornerRadius = 5;
	// Font size for our display numbers
	int fontSize = 10;
	// Font face
	std::shared_ptr<Font> font = NULL;
	// Padding
	int padding = 1;

	enum TextAlignment {
		Left,
		Center,
		Right
	};

	TextAlignment textAlign = TextAlignment::Center;

	//TS_ScreenBtn()
	//{		
	//	return;
	//}
	TS_ScreenBtn(Vec size, Module* module, int paramId, std::string text) : Switch()
	{
		box.size = size;
		font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
		fontSize = 10;
		btnText = text;
		if (module) {
			if (this->paramQuantity == NULL)
				this->paramQuantity = module->paramQuantities[paramId];
			// this->paramQuantity->setValue(0.0);
			// this->paramQuantity->minValue = minVal;
			// this->paramQuantity->maxValue = maxVal;
			// this->paramQuantity->defaultValue = defVal;			
		}
		return;
	}	
	TS_ScreenBtn(Vec size, Module* module, int paramId, std::string text, float minVal, float maxVal, float defVal)
	{
		box.size = size;
		font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
		fontSize = 10;
		btnText = text;
		if (module) {
			if (this->paramQuantity == NULL)
				this->paramQuantity = module->paramQuantities[paramId];
			// this->paramQuantity->setValue(0.0);
			// this->paramQuantity->minValue = minVal;
			// this->paramQuantity->maxValue = maxVal;
			// this->paramQuantity->defaultValue = defVal;			
		}
		return;
	}
	
	void setValue(float val){
		if (paramQuantity){
			paramQuantity->setValue(val);
		}
	}
	float getValue() {
		return (paramQuantity) ? paramQuantity->getValue() : 0.0;
	}
	
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(const event::DragStart &e) override {
		if (visible) {
			Switch::onDragStart(e);
		}
		return;
	}
	/** Called when the left button is released and this widget is being dragged */
	void onDragEnd(const event::DragEnd &e) override {
		if (visible) {
			Switch::onDragEnd(e);
		}
		return;
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(const event::DragEnter &e) override {
		if (visible) {
			Switch::onDragEnter(e);
		}
	}
	// void onMouseDown(const event::MouseDown &e) override {
		// if (visible) {
			// Switch::onMouseDown(e);
		// }
	// }
	void onButton(const event::Button &e) override{
		if (visible){
			Switch::onButton(e);
		}
		return;
	}

	void setVisible(bool visible) {
		this->visible = visible;
		return;
	}

	virtual void draw(const DrawArgs &args) override
	{
		if (!visible)
			return;
		// Background
		nvgBeginPath(args.vg);
		if (cornerRadius > 0)
			nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		else
			nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		// Background - border.
		if (borderWidth > 0) {
			nvgStrokeWidth(args.vg, borderWidth);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);
		}
		// Text
		nvgBeginPath(args.vg);
		nvgScissor(args.vg, padding, padding, box.size.x - 2*padding, box.size.y - 2*padding);
		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, color);

		float x, y;
		NVGalign nvgAlign;
		y = box.size.y / 2.0f;
		switch (textAlign) {
			case TextAlignment::Left:
				nvgAlign = NVG_ALIGN_LEFT;
				x = box.size.x + padding;
				break;
			case TextAlignment::Right:
				nvgAlign = NVG_ALIGN_RIGHT;
				x = box.size.x - padding;
				break;
			case TextAlignment::Center:
			default:
				nvgAlign = NVG_ALIGN_CENTER;
				x = box.size.x / 2.0f;
				break;
		}

		nvgTextAlign(args.vg, nvgAlign | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, x, y, btnText.c_str(), NULL);
		nvgResetScissor(args.vg);

		return;
	}
};
//--------------------------------------------------------------
// TS_ScreenCheckBox : Screen checkbox button
//--------------------------------------------------------------
struct TS_ScreenCheckBox : TS_ScreenBtn {
	// If the check box should show as checked.
	bool checked = false;
	float checkBoxWidth = 14;
	float checkBoxHeight = 14;

	TS_ScreenCheckBox(Vec size, Module* module, int paramId, std::string text, float minVal, float maxVal, float defVal)
		: TS_ScreenBtn(size, module, paramId, text, minVal, maxVal, defVal)
	{
		return;
	}
	TS_ScreenCheckBox(Vec size, Module* module, int paramId, std::string text)
		: TS_ScreenBtn(size, module, paramId, text)
	{
		return;
	}	

	void draw(const DrawArgs &args) override
	{
		if (!visible)
			return;
		// Background
		nvgBeginPath(args.vg);
		if (cornerRadius > 0)
			nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		else
			nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		// Background - border.
		if (borderWidth > 0) {
			nvgStrokeWidth(args.vg, borderWidth);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);
		}

		// Text
		nvgBeginPath(args.vg);
		nvgScissor(args.vg, padding, padding, box.size.x - 2 * padding, box.size.y - 2 * padding);
		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgFillColor(args.vg, color);

		float x, y;
		NVGalign nvgAlign;
		y = box.size.y / 2.0f;
		switch (textAlign) {
		case TextAlignment::Left:
			nvgAlign = NVG_ALIGN_LEFT;
			x = box.size.x + padding;
			break;
		case TextAlignment::Right:
			nvgAlign = NVG_ALIGN_RIGHT;
			x = box.size.x - padding;
			break;
		case TextAlignment::Center:
		default:
			nvgAlign = NVG_ALIGN_CENTER;
			x = box.size.x / 2.0f;
			break;
		}
		nvgTextAlign(args.vg, nvgAlign | NVG_ALIGN_MIDDLE);
		float txtBounds[4] = { 0,0,0,0 };
		nvgTextBounds(args.vg, x, y, btnText.c_str(), NULL, txtBounds);
		nvgText(args.vg, x, y, btnText.c_str(), NULL);
		nvgResetScissor(args.vg);

		// Check box ::::::::::::::::::::::::::::::::::::::::::::::

		float boxX = txtBounds[0] - checkBoxWidth - padding;
		float boxY = y - checkBoxHeight / 2.0 - padding;
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, boxX, boxY, checkBoxWidth, checkBoxHeight, 3);
		nvgStrokeColor(args.vg, color);
		nvgStrokeWidth(args.vg, 1.0f);
		nvgStroke(args.vg);

		if (checked)
		{
			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, boxX + padding, boxY + padding, checkBoxWidth - padding * 2, checkBoxHeight - padding*2, 3);
			nvgFillColor(args.vg, color);
			nvgFill(args.vg);
		}

		return;
	}
};


//--------------------------------------------------------------
// Base knob class with wrappers:
// 1. getValue() and setValue() accessors.
// 2. getDirty() and setDirty() accessors.
//--------------------------------------------------------------
struct TS_BaseKnob : Knob {
	int size = 20;
	
	TS_BaseKnob()
	{
		return;
	}
	
	TS_BaseKnob(int s)
	{
		size = s;
		box.size = Vec(size, size);		
		return;
	}
		
	float getValue()
	{
		if (paramQuantity)
		{
			return paramQuantity->getValue();
		}
		else{
			return 0;
		}
	}
	void setValue(float val)
	{
		if (paramQuantity)
		{
			paramQuantity->setValue(val);
		}
	}
	
	float getDefaultValue()
	{
		if (paramQuantity){
			return paramQuantity->defaultValue;
		}
		else {
			return 0;
		}
	}
};


//--------------------------------------------------------------
// Base knob class with wrappers:
// 1. getValue() and setValue() accessors.
// 2. getDirty() and setDirty() accessors.
//--------------------------------------------------------------
struct TS_Knob : RoundKnob {
	int size = 20;
	
	TS_Knob()
	{
		return;
	}
	
	TS_Knob(int s, const char* svgPath)
	{
		size = s;
		this->SvgKnob::setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, svgPath)));
		box.size = Vec(size, size);		
		return;
	}	
	void setDirty(bool isDirty)
	{
		if (fb){
			fb->dirty = isDirty;			
		}
		return;
	}
	
	bool getDirty()
	{
		if (fb) {
			return fb->dirty;
		}
		else {
			return false;
		}
	}
	
	float getValue()
	{
		if (paramQuantity)
		{
			return paramQuantity->getValue();
		}
		else{
			return 0;
		}
	}
	void setValue(float val)
	{
		if (paramQuantity)
		{
			paramQuantity->setValue(val);
		}
	}
	
	float getDefaultValue()
	{
		if (paramQuantity){
			return paramQuantity->defaultValue;
		}
		else {
			return 0;
		}
	}
};
//--------------------------------------------------------------
// TS_RoundBlackKnob - 30x30 RoundBlackKnob
//--------------------------------------------------------------
struct TS_RoundBlackKnob : RoundBlackKnob {
	bool allowRandomize = true;
	 TS_RoundBlackKnob() : RoundBlackKnob() {
		 return;
	 }
	 
	// Override randomize. Only do randomize if set to true.
	void randomize() override
	{
		if (allowRandomize) {
			this->ParamWidget::randomize();
		}
		return;
	}
	 
	void setDirty(bool isDirty)
	{
		if (fb){
			fb->dirty = isDirty;			
		}
		return;
	}
	
	
	bool getDirty()
	{
		if (fb) {
			return fb->dirty;
		}
		else {
			return false;
		}
	}
	
	float getValue()
	{
		if (paramQuantity)
		{
			return paramQuantity->getValue();
		}
		else{
			return 0;
		}
	}
	void setValue(float val)
	{
		if (paramQuantity)
		{
			paramQuantity->setValue(val);
		}
	}
	
	float getDefaultValue()
	{
		if (paramQuantity){
			return paramQuantity->defaultValue;
		}
		else {
			return 0;
		}
	}
	 
 };

//--------------------------------------------------------------
// TS_TinyBlackKnob - 20x20 RoundBlackKnob
//--------------------------------------------------------------
struct TS_TinyBlackKnob : TS_Knob {
	 TS_TinyBlackKnob() : TS_Knob(20, "res/ComponentLibrary/TS_RoundBlackKnob_20.svg") {
		 return;
	 }
 };
//--------------------------------------------------------------
// TS_15_BlackKnob - 15x15 RoundBlackKnob
// This is a little too tiny
//--------------------------------------------------------------
struct TS_15_BlackKnob : TS_Knob {
	 TS_15_BlackKnob() : TS_Knob(15, "res/ComponentLibrary/TS_RoundBlackKnob_15.svg"){
		 return;
	 }
 };
//--------------------------------------------------------------
// TS_20_BlackEncoder - 20x20 Encoder
// Pseudo continuous.... Still enforces the limits but allows the knob rotate 360.
//--------------------------------------------------------------
struct TS_20_BlackEncoder : TS_Knob { //RoundKnob {
	int c = 0;
	float rotationRangeMin = -1.f;
	float rotationRangeMax = 1.f;
	float fineControlMult = 1. / 16.f;
	float coarseControlMult = 16.f;

	TS_20_BlackEncoder() : TS_Knob(20, "res/ComponentLibrary/TS_RoundBlackEncoder_20.svg") {
		// box.size = Vec(size, size);
		// setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ComponentLibrary/TS_RoundBlackEncoder_20.svg")));
		minAngle = 0;
		maxAngle = 2 * M_PI;
		return;
	}
	// Set the amount a full rotation should yield.
	void setRotationAmount(float amt) {
		rotationRangeMin = -0.5f*amt;
		rotationRangeMax = 0.5f*amt;
		return;
	}
	// Override to allow pseudo endless encoding (still bound by some real values).
	void onDragMove(const event::DragMove &e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;		
		if (paramQuantity)
		{
			oldValue = paramQuantity->getSmoothValue();
			//if (snap) {
				snapValue = paramQuantity->getValue();
			//}
			float range = rotationRangeMax - rotationRangeMin;
			// e.mouseRel --> e.mouseDelta?
			float delta = KNOB_SENSITIVITY * -e.mouseDelta.y * speed * range;
			// Drag slower if mod is held
			int mods = APP->window->getMods();
			if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				delta *= fineControlMult; // Finer control
			}
			// Drag even slower if mod+shift is held
			else if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
				delta *= coarseControlMult; // Coarser control
			}		
			snapValue += delta;
			snapValue = clampSafe(snapValue, paramQuantity->minValue, paramQuantity->maxValue);
			if (snap)
				paramQuantity->setValue(roundf(snapValue));
			// else if (smooth)
				// paramQuantity->setSmoothValue(paramQuantity->getSmoothValue() + delta);
			else
				paramQuantity->setValue(snapValue);			
		} // end if there is a quantity to change
		ParamWidget::onDragMove(e);
		return;
	}
	//void step() override {
	void onChange(const event::Change &e) override {
		// Re-transform TransformWidget if dirty
		//if (fb->dirty && paramQuantity) {
		if (paramQuantity) {
			// Allow rotations 360 degrees:
			float angle = rescale(paramQuantity->getValue(), rotationRangeMin, rotationRangeMax, minAngle, maxAngle);
			angle = fmodf(angle, 2 * M_PI);

			tw->identity();
			// Rotate Svg
			Vec center = sw->box.getCenter();
			tw->translate(center);
			tw->rotate(angle);
			tw->translate(center.neg());
			
			fb->dirty = true;
		}
		Knob::onChange(e);
		return;
	}
	
};

//------------------------------------------------------------------------------------------------
// TS_LightArc - Lighted arc for light knobs
//------------------------------------------------------------------------------------------------
struct TS_LightArc : ColorValueLight {
	// The inner radius 
	float innerRadius = 22;	
	// Pointer to current angle in radians. This is the differential like from a knob.
	float* currentAngle_radians;
	// Font size for our display numbers
	int fontSize;
	// Font face
	std::shared_ptr<Font> font;
	// Numeric value to print out
	//float* numericValue;
	//ParamQuantity* pValue;
	ParamWidget* paramWidget = NULL;
	
	// Buffer for our light string.
	char lightString[10];
	// The point where the angle is considered 0 degrees / radians.
	float zeroAnglePoint;
	// Pointer to the Sequencer Value mode information.
	ValueSequencerMode* valueMode;
	
	TS_LightArc()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
		fontSize = 10;
		bgColor = nvgRGBAf(0.0, 0, 0, /*alpha */ 1.0);
		baseColor = TSColors::COLOR_WHITE;
		zeroAnglePoint = TROWA_ANGLE_STRAIGHT_UP_RADIANS;
	}	
	void draw(const DrawArgs &args) override
	{
		float oradius = box.size.x / 2.0; // 25
		float radius = oradius - 3; // 23
		
		float angle = *currentAngle_radians;
		zeroAnglePoint = valueMode->zeroPointAngle_radians;
		int dir = (angle < zeroAnglePoint) ? NVG_CCW : NVG_CW;

		// Background - Solid
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, oradius, oradius, innerRadius);
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);
		
		nvgStrokeWidth(args.vg, radius - innerRadius);
		NVGcolor borderColor = color;// bgColor;
		borderColor.a *= 0.5;//1.0;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
		
		// svg Angles go clockwise from positive x -->
		
		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgCircle(args.vg, oradius, oradius, radius);		
		borderColor = color;
		borderColor.a = 0.25;
		nvgStrokeWidth(args.vg, oradius - radius);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
		
		
		nvgBeginPath(args.vg);
		//nvgArcTo(args.vg, oradius, oradius, float x2, float y2, float radius);
		// Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
		// and the arc is drawn from angle a0 to a1, and swept in direction dir (NVG_CCW, or NVG_CW).
		// Angles are specified in radians.
		// nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
		nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, /*radius*/ innerRadius, 
			/*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
		nvgStrokeWidth(args.vg, oradius - innerRadius);
		borderColor = baseColor;
		borderColor.a *= 0.7;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
		
		// Outer glow
		nvgBeginPath(args.vg);
		nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, innerRadius - 3, 
			 /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
	
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= 0.8;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(args.vg, oradius, oradius, innerRadius, oradius, icol, ocol);
		nvgStrokeWidth(args.vg, oradius - innerRadius + 3);	
		nvgStrokePaint(args.vg, paint);
		nvgStroke(args.vg);
					
		//if (numericValue != NULL)
		//if (pValue != NULL)
		if (paramWidget != NULL && paramWidget->paramQuantity)
		{
			nvgBeginPath(args.vg);
			nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
			NVGcolor textColor = TSColors::COLOR_WHITE;
			nvgFontSize(args.vg, fontSize); 	
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			float v = valueMode->GetOutputValue(paramWidget->paramQuantity->getValue()); //(*numericValue);
			valueMode->GetDisplayString(v, lightString);
			nvgFillColor(args.vg, textColor);
			nvgText(args.vg, oradius, oradius, lightString, NULL);
		}
		return;
	}
}; // end TS_LightArc

//------------------------------------------------------------------------------------------------
// TS_LightedKnob - Knob to be used with light arcs. (not actually lit itself)
//------------------------------------------------------------------------------------------------
struct TS_LightedKnob : TS_BaseKnob { // SvgKnob
	float currentAngle;
	float differentialAngle;
	const float zeroAnglePoint = TROWA_ANGLE_STRAIGHT_UP_RADIANS;
	int id = 0; // For debugging.
	float minAngle;
	float maxAngle;
		
	TS_LightedKnob() : TS_BaseKnob(50) {
		minAngle = -0.83*NVG_PI;
		maxAngle = 0.83*NVG_PI;
		box.size = Vec(50, 50);
		currentAngle = 0;		
		snap = false;
		return;
	}
	TS_LightedKnob(int id) : TS_LightedKnob() {
		this->id = id;
	}
	void randomize() override { return; }	
	void setKnobValue(float val)
	{
		if (paramQuantity) {						
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			float prevVal = paramQuantity->getValue();
#endif
			paramQuantity->setValue(val);
			differentialAngle = rescale(val, paramQuantity->minValue, paramQuantity->maxValue, minAngle, maxAngle);
			currentAngle = zeroAnglePoint + differentialAngle;		
			//this->dirty = true;
			//fb->dirty = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Knob %d: Set value to %.2f (from %.2f). Current Value is now %.2f.", id, val, prevVal, paramQuantity->getValue());
#endif
		}
		return;
	}
	
	void onChange(const event::Change &e) override {
		if (paramQuantity) {
			differentialAngle = rescale(paramQuantity->getValue(), paramQuantity->minValue, paramQuantity->maxValue, minAngle, maxAngle);
			currentAngle = zeroAnglePoint + differentialAngle;			
		}
		TS_BaseKnob::onChange(e);
		return;
	}
	
	//
	// void step() override {
		// // Re-transform TransformWidget if dirty
		// if (fb->dirty && paramQuantity != NULL) {
			// differentialAngle = rescale(paramQuantity->getValue(), paramQuantity->minValue, paramQuantity->maxValue, minAngle, maxAngle);
			// currentAngle = zeroAnglePoint + differentialAngle;
			// tw->identity();
			// // Scale Svg to box
			// tw->scale(box.size.div(sw->box.size));
			// // Rotate Svg
			// Vec center = sw->box.getCenter();
			// tw->translate(center);
			// tw->rotate(currentAngle);
			// tw->translate(center.neg());
		// }
		// SvgKnob::step();
	// }
	
}; // end TS_LightedKnob

//--------------------------------------------------------------
// TS_LightString - A light with a string (message/text).
//--------------------------------------------------------------
struct TS_LightString : ColorValueLight
{
	const char * lightString;
	float cornerRadius = 3.0;	
	std::shared_ptr<Font> font;
	int fontSize;
	TS_LightString()
	{
		font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
		fontSize = 14;
		bgColor = nvgRGBAf(0.2, 0.2, 0.2, /*alpha */ 1);
		baseColor = TSColors::COLOR_WHITE;		
	}
	void draw(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 20.0;
		float radiusY = box.size.y / 2.0;
		float oradiusY = radiusY + 20.0;

		NVGcolor outerColor = color;
		
		// Solid
		nvgBeginPath(args.vg);
		// Border
		nvgStrokeWidth(args.vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
		
		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);

		// Outer glow
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, /*x*/ radius - oradius, /*y*/ radiusY - oradiusY, /*w*/ 3*oradius, /*h*/ 2*oradiusY, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.5;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		float feather = 3;
		// Feather defines how blurry the border of the rectangle is.
		paint = nvgBoxGradient(args.vg, /*x*/ 0, /*y*/ 0, /*w*/ box.size.x, /*h*/ oradiusY - 10, 
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(args.vg, paint);
		nvgFill(args.vg);
				
		nvgBeginPath(args.vg);
		nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
		NVGcolor textColor = baseColor;
		nvgFillColor(args.vg, textColor);
		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		if (lightString != NULL)
			nvgText(args.vg, box.size.x / 2, box.size.y / 2, lightString, NULL);
		return;
	}	
}; // end TS_LightString

//--------------------------------------------------------------
// TS_LightSquare - Square light. 
//--------------------------------------------------------------
struct TS_LightSquare : ColorValueLight 
{
	// Radius on corners
	float cornerRadius = 5.0;
	TS_LightSquare()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		baseColor = TSColors::COLOR_WHITE;
	}
	void draw(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius*1.1;

		NVGcolor backColor = bgColor;
		NVGcolor outerColor = color;
		// Solid
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		nvgFillColor(args.vg, backColor);
		nvgFill(args.vg);

		// Border
		nvgStrokeWidth(args.vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);

		// Outer glow
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2*oradius, /*h*/ 2*oradius, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.25;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		float feather = 2;
		// Feather defines how blurry the border of the rectangle is. // Fixed 01/19/2018, made it too tiny before
		paint = nvgBoxGradient(args.vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2 * oradius, /*h*/ 2 * oradius,  //args.vg, /*x*/ -5, /*y*/ -5, /*w*/ 2*oradius + 10, /*h*/ 2*oradius + 10, 
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(args.vg, paint);
		nvgFill(args.vg);
		return;
	}
}; // end TS_LightSquare

//--------------------------------------------------------------
// TS_LightRing - Light to be used around ports.
//--------------------------------------------------------------
struct TS_LightRing : ColorValueLight 
{
	// The inner radius 
	float innerRadius = 6.8;
	
	TS_LightRing()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.2);
		baseColor = TSColors::COLOR_WHITE;
	}
	void draw(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 10.0;
		

		// Solid
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, radius, radius, radius);

		// Border
		nvgStrokeWidth(args.vg, radius - innerRadius);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 1.0;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		//nvgFillColor(args.vg, color);
		//nvgFill(args.vg);

		// Outer glow
		nvgBeginPath(args.vg);
		nvgRect(args.vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= (module != NULL) ? module->lights[firstLightId].value : 0;
		//icol.a *= value;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(args.vg, radius, radius, innerRadius, oradius, icol, ocol);
		nvgFillPaint(args.vg, paint);
		nvgFill(args.vg);
		return;
	}
};


//--------------------------------------------------------------
// TS_Port - Smaller port with set light color and light disable
// (by just making the lights transparent... TODO: get rid of light completely.)
//--------------------------------------------------------------
struct TS_Port : SvgPort {
	NVGcolor negColor;
	NVGcolor posColor;
	
	TS_Port() : SvgPort() {
		//background->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/ComponentLibrary/TS_Port.svg"));
		//background->wrap();
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ComponentLibrary/TS_Port.svg")));
		box.size = sw->box.size;
		if (plugLight)
		{
			negColor = plugLight->baseColors[1];
			posColor = plugLight->baseColors[0];
		}
	}
	void disableLights()
	{
		// Save our colors:
		if (plugLight)
		{
			negColor = plugLight->baseColors[1];
			posColor = plugLight->baseColors[0];		
			plugLight->baseColors[0] = nvgRGBAf(0,0,0,0);
			plugLight->baseColors[1] = nvgRGBAf(0,0,0,0);		
		}
	}
	void enableLights()
	{
		if (plugLight)
		{
			plugLight->baseColors[1] = negColor;
			plugLight->baseColors[0] = posColor;		
		}
	}
	void setLightColor(NVGcolor color)
	{		
		negColor = color;
		posColor = color;
		if (plugLight)
		{
			plugLight->baseColors[0] = color;
			plugLight->baseColors[1] = color;
		}
	}
	void setLightColor(NVGcolor negativeColor, NVGcolor positiveColor)
	{
		negColor = negativeColor;
		posColor = positiveColor;
		if (plugLight)
		{
			plugLight->baseColors[1] = negativeColor;
			plugLight->baseColors[2] = positiveColor;			
		}
	}	
};

//--------------------------------------------------------------
// TS_Panel - Panel with controllable borders on all sides.
//--------------------------------------------------------------
struct TS_Panel : TransparentWidget  
{
	NVGcolor backgroundColor;
	std::shared_ptr<Image> backgroundImage;
	NVGcolor originalBackgroundColor;
	NVGcolor borderColor = TSColors::COLOR_BLACK;
	float borderWidth = 0;
	float borderTop = 0;
	float borderLeft = 0;
	float borderRight = 0;
	float borderBottom = 0;
	
	void setBorderWidth(float top, float right, float bottom, float left)
	{
		borderTop = top;
		borderLeft = left;
		borderRight = right;
		borderBottom = bottom;
		return;
	}
	//void invertBackgroundColor()
	//{
	//	backgroundColor = ColorInvertToNegative(originalBackgroundColor);
	//	return;
	//}	
	void draw(const DrawArgs &args) override
	{
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);

		// Background color
		if (backgroundColor.a > 0) {
			nvgFillColor(args.vg, backgroundColor);
			nvgFill(args.vg);
		}

		// Background image
		if (backgroundImage) {
			int width, height;
			nvgImageSize(args.vg, backgroundImage->handle, &width, &height);
			NVGpaint paint = nvgImagePattern(args.vg, 0.0, 0.0, width, height, 0.0, backgroundImage->handle, 1.0);
			nvgFillPaint(args.vg, paint);
			nvgFill(args.vg);
		}

		// Border		
		if (borderWidth > 0)
		{
			nvgBeginPath(args.vg);
			nvgRect(args.vg, borderWidth / 2.0, borderWidth / 2.0, box.size.x - borderWidth, box.size.y - borderWidth);
			nvgStrokeColor(args.vg, borderColor);
			nvgStrokeWidth(args.vg, borderWidth);
			nvgStroke(args.vg);			
		}
		int x, y;
		
		if (borderTop > 0)
		{
			// Line at top
			nvgBeginPath(args.vg);
			x = 0;
			y = borderTop / 2.0;
			nvgMoveTo(args.vg, /*start x*/ x, /*start y*/ y); // Top Left
			x = box.size.x;
			nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Top Right
			nvgStrokeColor(args.vg, borderColor);
			nvgStrokeWidth(args.vg, borderTop);
			nvgStroke(args.vg);				
		}
		if (borderRight > 0)
		{
			nvgBeginPath(args.vg);
			x = box.size.x - borderRight / 2.0;
			y = 0;
			nvgMoveTo(args.vg, /*x*/ x, /*y*/ y); // Top Right					
			y = box.size.y;// - borderRight;
			nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Bottom Right								
			nvgStrokeColor(args.vg, borderColor);
			nvgStrokeWidth(args.vg, borderRight);
			nvgStroke(args.vg);							
		}
		if (borderBottom > 0)
		{
			nvgBeginPath(args.vg);
			x = box.size.x;// - borderBottom;
			y = box.size.y - borderBottom / 2.0;// - borderBottom;
			nvgMoveTo(args.vg, /*x*/ x, /*y*/ y); // Bottom Right					
			x = 0;// borderBottom / 2.0;
			nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Bottom Left			
			nvgStrokeColor(args.vg, borderColor);
			nvgStrokeWidth(args.vg, borderBottom);
			nvgStroke(args.vg);										
		}
		if (borderLeft > 0)
		{
			nvgBeginPath(args.vg);
			x = borderLeft / 2.0;
			y = box.size.y;// - borderLeft;
			nvgMoveTo(args.vg, /*x*/ x, /*y*/ y); // Bottom Left					
			y = 0;//borderLeft / 2.0;
			nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Top Left						
			nvgStrokeColor(args.vg, borderColor);
			nvgStrokeWidth(args.vg, borderLeft);
			nvgStroke(args.vg);													
		}
		this->Widget::draw(args);
	} // end draw()
}; // end TS_Panel

//--------------------------------------------------------------
// TS_SvgPanel - Svg Panel without mandatory border on LHS
//--------------------------------------------------------------
//--------------------------------------------------------------
struct TS_SvgPanel : SvgPanel
{
	NVGcolor borderColor = TSColors::COLOR_BLACK;
	float borderTop = 0;
	float borderLeft = 0;
	float borderRight = 0;
	float borderBottom = 0;
	
	TS_SvgPanel() : SvgPanel()
	{
		return;
	}
	TS_SvgPanel(float borderTop, float borderRight, float borderBottom, float borderLeft) : TS_SvgPanel()
	{
		this->borderTop = borderTop;
		this->borderRight = borderRight;
		this->borderBottom = borderBottom;
		this->borderLeft = borderLeft;
		return;
	}	
	void setBackground(std::shared_ptr<Svg> svg)  {
		SvgWidget *sw = new SvgWidget();
		sw->setSvg(svg);
		addChild(sw);

		// Set size
		box.size = sw->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);

		if (borderTop + borderRight + borderLeft + borderBottom > 0)
		{
			TS_Panel* pb = new TS_Panel();
			pb->setBorderWidth(borderTop, borderRight, borderBottom, borderLeft);
			pb->borderColor = this->borderColor;
			pb->box.size = box.size;
			addChild(pb);
		}
		
		// PanelBorder *pb = new PanelBorder();
		// pb->box.size = box.size;
		// addChild(pb);
	} // end setBackground()
}; // end TS_SvgPanel

//--------------------------------------------------------------
// TS_ColorSlider - Horizontal color slider control 'knob'.
// Meant for picking colors via Hue, Saturation, Lightness.
//--------------------------------------------------------------
struct TS_ColorSlider : Knob {
	// If this control should be rendered
	bool visible = true;
	// Starting color.
	TSColorHSL startColorHSL;
	// Ending color.
	TSColorHSL endColorHSL;
	// HSL representation of the selected color.
	TSColorHSL selectedColorHSL;
	// NVGcolor of the selected color (RGBA)
	NVGcolor selectedColor;
	// Number of stops for the gradients (doing them manually since nanovg only lets you have 2 colors)
	int numStops = 24;
	float handleWidth = 15.0;
	float handleMargin = 3.0;
	TS_ColorSlider() : Knob()
	{
		this->horizontal = true;
		this->snap = false;
		startColorHSL.h = 0.0;
		startColorHSL.s = 1.0;
		startColorHSL.lum = 0.5;
		endColorHSL.h = 1.0;
		endColorHSL.s = 1.0;
		endColorHSL.lum = 0.5;
		selectedColorHSL.h = 1.0;
		selectedColorHSL.s = 1.0;
		selectedColorHSL.lum = 0.5;
		return;
	}
	TS_ColorSlider(Vec size) : TS_ColorSlider()
	{
		box.size = size;
		return;
	}
	
	void setValue(float val)
	{
		if (paramQuantity)
		{
			paramQuantity->setValue(val);
		}
	}
	float getValue()
	{
		if (paramQuantity)
		{
			return paramQuantity->getValue();
		}
		else
		{
			return 0;
		}
	}
	
	// Set the component value for start and end
	void setComponent(int index, float val)
	{
		startColorHSL.hsl[index] = val;
		endColorHSL.hsl[index] = val;
		return;
	}
	void onDragStart(const event::DragStart &e) override {
		if (visible)
			this->Knob::onDragStart(e);
	}
	
	void onDragMove(const event::DragMove &e) override {
		if (visible) {
			this->Knob::onDragMove(e);
		}
		// if (visible && paramQuantity)
		// {
			// // Drag slower if Mod
			// float delta = KNOB_SENSITIVITY * (paramQuantity->maxValue - paramQuantity->minValue) * e.mouseDelta.x;
			// int mods = APP->window->getMods();
			// if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				// delta /= 16.f;
			// }
			// // Drag even slower if mod+shift is held
			// else if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
				// delta /= 256.f;
			// }			
			// snapValue += delta;
			// if (snap)
				// paramQuantity->setValue(roundf(snapValue));
			// else
				// paramQuantity->setValue(snapValue);

		// }
		return;
	}
	void onDragEnd(const event::DragEnd &e) override {
		if (visible)
			this->Knob::onDragEnd(e);
	}
	void onChange(const event::Change &e) override {
		if (visible)
			this->Knob::onChange(e);
	}
	//void step() override {
	//	return;
	//}
	//void onChange(const event::Change &e) override {
	//	//dirty = true;
	//	ParamWidget::onChange(e);
	//}
	void draw(const DrawArgs &args) override {
		if (!visible)
			return;

		// Draw the background:
		float x = 0;
		float y = 0;
		float dx = box.size.x / numStops;
		float deltaComponents[3];
		// Calculate the delta/interval for each component (HSL)
		for (int i = 0; i < 3; i++)
		{
			deltaComponents[i] = (endColorHSL.hsl[i] - startColorHSL.hsl[i]) / numStops;
		}
		float hue, sat, lht;
		hue = startColorHSL.h; sat = startColorHSL.s; lht = startColorHSL.lum;
		NVGcolor sColor;
		NVGcolor eColor = nvgHSLA(hue, sat, lht, 0xFF);
		y = box.size.y / 2.0;
		for (int i = 0; i < numStops; i++)
		{
			sColor = eColor;
			eColor = nvgHSLA(hue += deltaComponents[0], sat += deltaComponents[1], lht += deltaComponents[2], 0xFF);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, x, 0, dx + 1, box.size.y);
			nvgStrokeWidth(args.vg, 0.0);
			NVGpaint paint = nvgLinearGradient(args.vg, x, y, x + dx + 1, y, sColor, eColor);
			nvgFillPaint(args.vg, paint);
			nvgFill(args.vg);
			x += dx;
		}
		float val = 0.5;
		float min = 0;
		float max = 1.0;
		if (paramQuantity)
		{
			val = paramQuantity->getValue();
			min = paramQuantity->minValue;
			max = paramQuantity->maxValue;
		}
		for (int i = 0; i < 3; i++)
		{
			selectedColorHSL.hsl[i] = startColorHSL.hsl[i] + (endColorHSL.hsl[i] - startColorHSL.hsl[i]) * val;
		}
		selectedColor = nvgHSL(selectedColorHSL.hsl[0], selectedColorHSL.hsl[1], selectedColorHSL.hsl[2]);
		float handleHeight = box.size.y + 2 * handleMargin;
		float handleX = rescale(val, min, max, 0, box.size.x) - handleWidth / 2.0;
		float handleY = -handleMargin;// rescale(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y);
		// Draw handle
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, handleX, handleY, handleWidth, handleHeight, 5);
		nvgFillColor(args.vg, selectedColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		NVGcolor strokeColor = ((val < 0.5) ? TSColors::COLOR_WHITE : TSColors::COLOR_BLACK);
		nvgStrokeColor(args.vg, strokeColor);
		nvgStroke(args.vg);
	}
}; // end TS_ColorSlider

//:::-:::-:::-:::- Helpers -:::-:::-:::-:::
template <class TModuleLightWidget>
ColorValueLight * TS_createColorValueLight(Vec pos,  Module *module, int lightId, Vec size, NVGcolor lightColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;	
	//light->value = value;
	light->box.size = size;
	light->setColor(lightColor);
	//light->baseColor = lightColor;
	return light;
}
template <class TModuleLightWidget>
ColorValueLight * TS_createColorValueLight(Vec pos, Module *module, int lightId, Vec size, NVGcolor lightColor, NVGcolor backColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;	
	light->box.size = size;
	light->setColor(lightColor);	
	light->bgColor = backColor;
	return light;
}

template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::INPUT;
	port->portId = inputId;
	port->disableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::INPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, NVGcolor negColor, NVGcolor posColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::INPUT;
	port->portId = inputId;
	port->setLightColor(negColor, posColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, bool disableLight) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::INPUT;
	port->portId = inputId;
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, bool disableLight, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::INPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}


template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::OUTPUT;
	port->portId = inputId;
	port->disableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::OUTPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, NVGcolor negColor, NVGcolor posColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::OUTPUT;
	port->portId = inputId;
	port->setLightColor(negColor, posColor);
	port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, bool disableLight) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::OUTPUT;
	port->portId = inputId;
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, bool disableLight, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = PortWidget::OUTPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	if (disableLight)
		port->disableLights();
	else
		port->enableLights();
	return port;
}



#endif // end if not defined
