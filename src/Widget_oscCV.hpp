#ifndef WIDGET_OSCCV_HPP
#define WIDGET_OSCCV_HPP

#include "widgets.hpp"
#include "TSSModuleWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"
#include "Module_oscCV.hpp"
#include "rack.hpp"
#include <vector>
using namespace rack;

#define TROWA_SCROLLING_MSG_TOTAL_SIZE		256
#define TROWA_OSCCV_NUM_COLORS				  8

struct oscCV;
struct TSOscCVTopDisplay;
struct TSOscCVMiddleDisplay;
struct TSOscCVLabels;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVWidget
// Open Sound Control <==> Control Voltage widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct oscCVWidget : TSSModuleWidgetBase {
	// OSC configuration widget.
	TSOSCConfigWidget* oscConfigurationScreen;
	// The top display.
	TSOscCVTopDisplay* display;
	// The middle display
	TSOscCVMiddleDisplay* middleDisplay;
	// Number of channels. Should be in the module instance, but since we are no longer guaranteed a non-NULL reference, we will store the # channels here.
	int numberChannels;
	// Rack CV Input (osc trans messge path)
	std::vector<TSTextField*> tbOscInputPaths;
	// Rack CV Output (osc recv message path)
	std::vector<TSTextField*> tbOscOutputPaths;
	// If we should color the channels.
	bool colorizeChannels = true;
	// Advanced channel configs
	std::vector<TS_ScreenBtn*> btnDrawInputAdvChConfig;
	// Advanced channel configs
	std::vector<TS_ScreenBtn*> btnDrawOutputAdvChConfig;

	// Channel colors
	static const NVGcolor CHANNEL_COLORS[TROWA_OSCCV_NUM_COLORS];
	// Plug lights
	bool plugLightsEnabled = true;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVWidget()
	// Instantiate a oscCV widget. v0.60 must have module as param.
	// @oscModule : (IN) Pointer to the osc module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVWidget(oscCV* oscModule);
	// Step
	void step() override;

	// Show or hide the channel configuration
	void toggleChannelPathConfig(bool show);
	// Read the channel path configs and store in module's channels.
	void readChannelPathConfig();
	// Set the channel path text boxes.
	void setChannelPathConfig();
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Labels for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVLabels : TransparentWidget {
	std::shared_ptr<Font> font;
	int fontSize;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVLabels(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVLabels()
	{
		font = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	// Draw labels on our widget.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;
}; // end TSOscCVLabels

// Button to chose OSC data type
struct TSOscCVDataTypeSelectBtn : ChoiceButton {
	// The selected value.
	int selectedVal;
	int selectedIndex;
	int numVals;
	int* itemVals;
	std::string* itemStrs;
	bool visible = false;
	std::shared_ptr<Font> font;
	Vec textOffset;
	NVGcolor color;
	// Font size
	float fontSize;
	std::string displayStr;
	int borderWidth = 0;
	NVGcolor borderColor;
	NVGcolor backgroundColor;
	int showNumChars = 15;

	TSOscCVDataTypeSelectBtn(int numVals, int* itemVals, std::string* itemStrs, int selVal) {
		this->numVals = numVals;
		this->selectedVal = selVal;
		this->itemVals = itemVals;
		this->itemStrs = itemStrs;
		for (int i = 0; i < numVals; i++)
		{
			if (itemVals[i] == selectedVal)
				selectedIndex = i;
		}
		return;
	}
	void step() override {
		text = ellipsize(itemStrs[selectedIndex], showNumChars);
	}
	void onAction(EventAction &e) override;
	// Draw if visible
	void draw(NVGcontext *vg) override;
};
// An OSC client option in dropdown.
struct TSOscCVDataTypeItem : MenuItem {
	int itemVal;
	int index;
	TSOscCVDataTypeSelectBtn* parentButton;
	TSOscCVDataTypeItem(TSOscCVDataTypeSelectBtn* parent, int index)
	{
		parentButton = parent;
		this->index = index;
		return;
	}
	void onAction(EventAction &e) override {
		parentButton->selectedVal = itemVal;
		parentButton->selectedIndex = index;
	}
};

struct TSOscCVChannelConfigScreen : OpaqueWidget {
	oscCVWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	bool visible = false;

	TSTextField* tbCVMinVolt;
	TSTextField* tbCVMaxVolt;
	TSTextField* tbOSCMinVal;
	TSTextField* tbOSCMaxVal;
	// OSC Data Type select/dropdown
	TSOscCVDataTypeSelectBtn* btnSelectDataType;
	// Save button
	TS_ScreenBtn* btnSave;
	// Cancel button
	TS_ScreenBtn* btnCancel;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVChannelConfigScreen(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVChannelConfigScreen() : TSOscCVChannelConfigScreen(NULL) {
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVChannelConfigScreen()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVChannelConfigScreen(oscCVWidget* widget)
	{
		parentWidget = widget;
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		visible = true;
		return;
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Top Display for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVTopDisplay : TransparentWidget {
	oscCVWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	bool showDisplay = true;
	char scrollingMsg[TROWA_SCROLLING_MSG_TOTAL_SIZE];
	int scrollIx = 0;
	std::string lastIp = std::string("");
	float dt = 0.0f;
	float scrollTime_sec = 0.05f;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVTopDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVTopDisplay() : TSOscCVTopDisplay(NULL) {
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVMiddleDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVTopDisplay(oscCVWidget* widget)
	{
		parentWidget = widget;
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		memset(messageStr, '\0', sizeof(char)*TROWA_DISP_MSG_SIZE);
		memset(scrollingMsg, '\0', sizeof(char)*TROWA_SCROLLING_MSG_TOTAL_SIZE);
		showDisplay = true;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// step()
	// Calculate scrolling and stuff?
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;

}; // end struct TSOscCVTopDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Middle Display for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVMiddleDisplay : TransparentWidget {
	oscCVWidget* parentWidget;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	//bool showDisplay = true;

	enum DisplayMode {
		None = 0,
		Default = 1
	};

	DisplayMode displayMode = DisplayMode::Default;
	// Current channel path position (for paths that are too large and need some scrolling).
	float chPathPosition = 0.0f;
	// Amt of time that has passed.
	float dt = 0.0f;
	// Scrolling time
	float scrollTime = 0.05f;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVMiddleDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVMiddleDisplay(oscCVWidget* widget)
	{
		parentWidget = widget;
		font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
		labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		return;
	}
	~TSOscCVMiddleDisplay() {
	}

	void setDisplayMode(DisplayMode mode) {
		displayMode = mode;
		if (displayMode == DisplayMode::Default)
		{
			chPathPosition = 0.0f; // reset
			dt = 0.0f; // reset
		}
		return;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Process
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawChannelChart()
	// Draw the channel data.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawChannelChart(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,  /*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height, /*in*/ NVGcolor lineColor);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawChannelBar()
	// Draw the channel data.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawChannelBar(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,  /*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height, /*in*/ NVGcolor lineColor);

}; // end struct TSOscCVMiddleDisplay

#endif // !WIDGET_OSCCV_HPP
