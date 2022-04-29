#ifndef WIDGET_OSCCVEXPANDER_HPP
#define WIDGET_OSCCVEXPANDER_HPP

#include "TSSModuleWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"
#include "Module_oscCVExpander.hpp"
#include "TSOSCCV_Common.hpp"
#include <vector>
#include <rack.hpp>
using namespace rack;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVExpanderWidget
// Open Sound Control <==> Control Voltage EXPANDER Widget
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct oscCVExpanderWidget : TSSModuleWidgetBase {
	// Number of channels. Should be in the module instance, but since we are no longer guaranteed a non-NULL reference, we will store the # channels here.
	int numberChannels = TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS;
	// If we should color the channels.
	bool colorizeChannels = true;
	// Plug lights
	bool plugLightsEnabled = true;
	// The expander color.
	NVGcolor expanderColor;
	const NVGcolor defaultExpanderColor = TSColors::COLOR_TS_TEXT;
	// The expander type (INPUT/OUTPUT). Should be on the module.
	TSOSCCVExpanderDirection expanderType = TSOSCCVExpanderDirection::Unknown;	
	// The expander level from master.
	int expanderLvl = 0;
	
	float bandXStart = 0.f;
	float bandWidth = 10.f;
	// If we're being configed.
	bool lastConfigStatus = false;
	// Alpha channel 
	float lastConfigA = 0.0f;	
	bool dir = false;
	
	ColorValueLight* lightMasterConnected = NULL;
	ColorValueLight* lightRightConnected = NULL;
	ColorValueLight* lightLeftConnected = NULL;
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpanderWidget()
	// Instantiate a oscCVExpander widget. 
	// @oscExpanderModule : (IN) Pointer to the osc module.
	// @expanderDirection : (IN) What direction (IN/OUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderWidget(oscCVExpander* oscExpanderModule, TSOSCCVExpanderDirection expanderDirection);
	~oscCVExpanderWidget()
	{
		return;
	}
	// Step
	void step() override;		
	void onDragEnd(const event::DragEnd &e) override;

};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVExpanderInputWidget
// CV -> OSC
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct oscCVExpanderInputWidget : oscCVExpanderWidget {
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpanderInputWidget()
	// Instantiate a oscCVExpanderInput widget. 
	// @oscExpanderModule : (IN) Pointer to the osc module.
	// @expanderDirection : (IN) What direction (IN/OUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderInputWidget(oscCVExpander* oscExpanderModule) : oscCVExpanderWidget(oscExpanderModule, TSOSCCVExpanderDirection::Input)
	{
		return;
	}
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVExpanderOutputWidget
// OSC -> CV
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct oscCVExpanderOutputWidget : oscCVExpanderWidget {
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVExpanderOutputWidget()
	// Instantiate a oscCVExpanderOutput widget. 
	// @oscExpanderModule : (IN) Pointer to the osc module.
	// @expanderDirection : (IN) What direction (IN/OUT).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVExpanderOutputWidget(oscCVExpander* oscExpanderModule) : oscCVExpanderWidget(oscExpanderModule, TSOSCCVExpanderDirection::Output)
	{
		return;
	}
};

struct oscCVExpanderSideIndicator : TransparentWidget {	
	oscCVExpanderWidget* parent = NULL;
	float strokeWidth = 4.0f;
	oscCVExpanderSideIndicator(oscCVExpanderWidget* parentWidget, Vec size)
	{
		this->parent = parentWidget;
		this->box.size = size;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @args.vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ const DrawArgs &args) override;	
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Top Display for oscCV Expander widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVExpanderTopDisplay : TransparentWidget {
	oscCVExpanderWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	bool showDisplay = true;
	std::string displayName;
	std::string directionName;
	char scrollingMsg[100];
	int scrollIx = 0;
	float dt = 0.0f;
	float scrollTime_sec = 0.05f;
	std::string lastName = std::string("");
	bool lastConnectedToMaster = false;
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVExpanderTopDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVExpanderTopDisplay() : TSOscCVExpanderTopDisplay(NULL) {
		return;
	}
	~TSOscCVExpanderTopDisplay() {
		parentWidget = NULL;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVExpanderTopDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVExpanderTopDisplay(oscCVExpanderWidget* widget)
	{
		parentWidget = widget;
		fontSize = 10;
		showDisplay = true;
		displayName = "Input";
		directionName = "CV->OSC";
		if (widget)
		{
			if (widget->expanderType == TSOSCCVExpanderDirection::Output)
			{
				displayName = "Output";
				directionName = "OSC->CV";
			}
		}
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// step()
	// Calculate scrolling and stuff?
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @args.vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawLayer(/*in*/ const DrawArgs &args, int layer) override;

}; // end struct TSOscCVExpanderTopDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Labels for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVExpanderLabels : TransparentWidget {
	std::shared_ptr<Font> font;
	int fontSize;
	TSOSCCVExpanderDirection expanderType;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVExpanderLabels(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVExpanderLabels()
	{
		fontSize = 12;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVExpanderLabels()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVExpanderLabels(TSOSCCVExpanderDirection dir) : TSOscCVExpanderLabels()
	{
		expanderType = dir;
		return;
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @args.vg : (IN) NVGcontext to draw on
	// Draw labels on our widget.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ const DrawArgs &args) override;
}; // end TSOscCVLabels

#endif // !WIDGET_OSCCVEXPANDER_HPP