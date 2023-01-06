#ifndef WIDGET_OSCCV_HPP
#define WIDGET_OSCCV_HPP

#include "TSSModuleWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"
#include "Module_oscCV.hpp"
#include <rack.hpp>
#include <vector>
using namespace rack;

#define TROWA_SCROLLING_MSG_TOTAL_SIZE		256
#define TROWA_OSCCV_NUM_COLORS				  8
#define TROWA_OSCCV_OSC_PATH_SIZE		    256 // Max path size (was hard coded for TSTextBoxes to 50)

struct oscCV;
struct TSOscCVTopDisplay;
struct TSOscCVMiddleDisplay;
struct TSOscCVLabels;
struct TSOscCVChannelConfigScreen;
struct TSPageNumberControl;

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
	// Screen for the channel configuration.
	TSOscCVChannelConfigScreen* oscChannelConfigScreen;
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
	// Flag if we have the master oscCV module loaded for configuration or an Expander.
	bool masterConfigLoaded = true;
	
	bool showConfigScreen = false;
	// -- EXPANDERS --
	int showConfigIndex = 0;
	int showConfigColumnIndex = 0;
	ColorValueLight* prevLight = NULL;
	ColorValueLight* nextLight = NULL;	
	NVGcolor showConfigColor = TSColors::COLOR_WHITE;
	// Reference to the current expander loaded.
	oscCVExpander* currentEditExpander = NULL;
	std::string configName;
	bool configNameLeft = false;
	// Text box for an expander's id/name.
	TSTextField* tbExpanderID;
	int tbExpanderXPos[2];
	// Button to renumber the expander
	TS_ScreenBtn* btnExpRenumber = NULL;
	// Which page number / column number we are editing for expanders with more than 8 channels.
	TSPageNumberControl* dialExpEditPageNumber = NULL;

	// Channel colors
	/// TODO: Move this to someplace else
	static const NVGcolor CHANNEL_COLORS[TROWA_OSCCV_NUM_COLORS];
	// Plug lights
	bool plugLightsEnabled = true;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCVWidget()
	// Instantiate a oscCV widget. v0.60 must have module as param.
	// @oscModule : (IN) Pointer to the osc module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCVWidget(oscCV* oscModule);

	~oscCVWidget()
	{
		try
		{
			oscConfigurationScreen->module = NULL;
			oscConfigurationScreen = NULL;
			display = NULL;
			middleDisplay = NULL;
			oscChannelConfigScreen = NULL;
			//btnDrawInputAdvChConfig.clear();
			//btnDrawOutputAdvChConfig.clear();
		}
		catch (std::exception& ex)
		{
			WARN("Error: %s", ex.what());
		}
		return;
	}
	// Step
	void step() override;

	// Show or hide the channel configuration
	void toggleChannelPathConfig(bool show);
	// Show or hide the channel configuration
	void toggleChannelPathConfig(bool showInput, bool showOutput);	
	// Clear the channel path text boxes.
	void clearChannelPathConfig();	
	// Set the channel path text boxes.
	void setChannelPathConfig();
	// Read the channel path configs and store in module's channels.
	// @index : (IN) The module index (0 is master, < 0 is input expander, > 0 is output expander).
	// @colIx : (IN) The column index of the module.
	void readChannelPathConfig(int index, int colIx);		
	// Read the channel path configs and store in module's channels.
	// @inputChannels : (IN/OUT) The input channels array.
	// @outputChannels : (IN/OUT) The output channels array.
	// @nChannels : (IN) Size of the channel arrays.
	// @channelColumn : (IN) Which column / page of channels are we displaying (we can only display 8 at a time).
	std::string readChannelPathConfig(TSOSCCVInputChannel* inputChannels, TSOSCCVChannel* outputChannels, int numChannels, int channelColumn = 0);
	// Set the channel path text boxes.
	// @inputChannels : (IN) The input channels array.
	// @outputChannels : (IN) The output channels array.
	// @nChannels : (IN) Size of the channel arrays.
	// @channelColumn : (IN) Which column / page of channels are we displaying (we can only display 8 at a time).
	void setChannelPathConfig(TSOSCCVInputChannel* inputChannels, TSOSCCVChannel* outputChannels, int numChannels, std::string expanderName, int channelColumn = 0);

	// Rename the advanced configuration buttons
	void renameAdvConfigBtns();

	// OnDragEnd - Revisit what expanders we may be connected to.
	void onDragEnd(const event::DragEnd &e) override;
	
	// Calc color of an expander.
	static NVGcolor calcColor(int index)
	{
		NVGcolor color;
		int add = 0;
		if (index == 0)
		{
			color = TSColors::COLOR_WHITE;
		}
		else 
		{
			if (index < 0)
				add = -index;
			else
				add = index;
			color = TSColors::CHANNEL_COLORS[(add - 1 + TSColors::NUM_CHANNEL_COLORS) % TSColors::NUM_CHANNEL_COLORS];					
		}
		return color;		
	}
	// Append custom context menu options (for the send frequency).
	void appendContextMenu(ui::Menu *menu) override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Labels for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVLabels : TransparentWidget {
	//std::shared_ptr<Font> font; // Rack v2 Conversion - Don't even store this font ptr
	std::string fontPath;// Rack v2 store font path
	int fontSize;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVLabels(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVLabels()
	{		
		fontSize = 12;
		fontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @args.vg : (IN) NVGcontext to draw on
	// Draw labels on our widget.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ const DrawArgs &args) override;
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
	//std::shared_ptr<Font> font;  // Rack v2 Conversion - Don't even store this font ptr
	Vec textOffset;
	NVGcolor color;
	// Font size
	float fontSize;
	std::string displayStr;
	int borderWidth = 0;
	NVGcolor borderColor;
	NVGcolor backgroundColor;
	int showNumChars = 15;
	TSOscCVChannelConfigScreen* parentScreen = NULL;
	std::string fontPath; // Rack v2 store font path

	TSOscCVDataTypeSelectBtn(int numVals, int* itemVals, std::string* itemStrs, int selVal) {
		fontPath = asset::plugin(pluginInstance, TROWA_MONOSPACE_FONT);  // Rack v2 store font path
		fontSize = 14.0f;
		backgroundColor = FORMS_DEFAULT_BG_COLOR;
		color = FORMS_DEFAULT_TEXT_COLOR;
		textOffset = Vec(5, 3);
		borderWidth = 1;
		borderColor = FORMS_DEFAULT_BORDER_COLOR;

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
	// When the selected index changes.
	void onSelectedIndexChanged();

	// Set the selected value.
	void setSelectedValue(int selVal) {
		this->selectedVal = selVal;
		for (int i = 0; i < numVals; i++)
		{
			if (itemVals[i] == selectedVal)
				selectedIndex = i;
		}
		onSelectedIndexChanged();
		return;
	}
	// Set the selected index.
	void setSelectedIndex(int selIx) {
		this->selectedIndex = selIx;
		this->selectedVal = itemVals[selectedIndex];
		onSelectedIndexChanged();
		return;
	}
	void step() override {
		text = string::ellipsize(itemStrs[selectedIndex], showNumChars);
	}
	void onAction(const event::Action &e) override;
	// Draw if visible
	void drawLayer(const DrawArgs &args, int layer) override;
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
	void onAction(const event::Action &e) override {
		parentButton->setSelectedIndex(index);
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Single Channel configuration (advanced) widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVChannelConfigScreen : OpaqueWidget {
	oscCVWidget* parentWidget;
	//std::shared_ptr<Font> font;  // Rack v2 Conversion - Don't even store this font ptr
	//std::shared_ptr<Font> labelFont;  // Rack v2 Conversion - Don't even store this font ptr
	std::string fontPath;
	std::string labelFontPath;
	int fontSize;
	bool visible = false;
	// If this is an input or output channel.
	bool isInput = false;
	enum TextBoxIx {
		// Minimum CV voltage
		MinCVVolt,
		// Maximum CV voltage
		MaxCVVolt,
		// Minimum OSC value
		MinOSCVal,
		// Maximum OSC value
		MaxOSCVal,
		NumTextBoxes
	};

	// The text boxes for min/max values.
	TSTextField* tbNumericBounds[TextBoxIx::NumTextBoxes];
	std::string tbErrors[TextBoxIx::NumTextBoxes];

	const int numDataTypes = 3;
	TSOSCCVChannel::ArgDataType oscDataTypeVals[3] = { TSOSCCVChannel::ArgDataType::OscFloat, TSOSCCVChannel::ArgDataType::OscInt, TSOSCCVChannel::ArgDataType::OscBool };
	std::string oscDataTypeStr[3] = { std::string("Float"), std::string("Int"), std::string("Bool") };
	// The selected data type.
	TSOSCCVChannel::ArgDataType selectedDataType = TSOSCCVChannel::ArgDataType::OscFloat;
	// Label buffer
	char buffer[50];

	// OSC Data Type select/dropdown
	TSOscCVDataTypeSelectBtn* btnSelectDataType;
	
	// Turn on / off translating values.
	bool translateValsEnabled = false;
	TS_ScreenCheckBox* btnToggleTranslateVals;	
	dsp::SchmittTrigger translateTrigger;

	// Clip values (MIN/MAX input voltage) if we are translating.	
	bool clipValsEnabled = false;
	TS_ScreenCheckBox* btnToggleTranslateClipVals;
	dsp::SchmittTrigger clipChannelInputTrigger;
	

	// Save button
	TS_ScreenBtn* btnSave;
	// Cancel button
	TS_ScreenBtn* btnCancel;
	dsp::SchmittTrigger saveTrigger;
	dsp::SchmittTrigger cancelTrigger;

	// Pointer to the current channel information.
	TSOSCCVChannel* currentChannelPtr = NULL;

	int startX = 6;
	int startY = 6;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVChannelConfigScreen(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVChannelConfigScreen() : TSOscCVChannelConfigScreen(NULL, Vec(0,0), Vec(300, 300)) {
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVChannelConfigScreen()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVChannelConfigScreen(oscCVWidget* widget, Vec pos, Vec boxSize);

	~TSOscCVChannelConfigScreen()
	{
		parentWidget = NULL;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Set visible or not.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setVisibility(bool visible) {
		this->visible = visible;
		try
		{
			if (btnToggleTranslateVals)
				btnToggleTranslateVals->visible = visible;
			if (btnToggleTranslateClipVals)
				btnToggleTranslateClipVals->visible = visible;
			this->btnSelectDataType->visible = visible;

			btnSave->visible = visible;
			btnCancel->visible = visible;
			for (int i = 0; i < TextBoxIx::NumTextBoxes; i++)
			{
				tbNumericBounds[i]->visible = visible;
			}
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
		return;
	} // end setVisibility()

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Save the values to the ptr.
	// @channelPtr : (OUT) Place to save the values.
	// @returns : True if saved, false if there was an error.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	bool saveValues(/*out*/ TSOSCCVChannel* channelPtr);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Save the values to current ptr.
	// @returns : True if saved, false if there was an error.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	bool saveValues() {
		return saveValues(this->currentChannelPtr);		
	}

	void setDataType(TSOSCCVChannel::ArgDataType dataType)
	{
		if (dataType == TSOSCCVChannel::ArgDataType::OscBool)
		{
			// Bools have to be false/true....
			tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = false;
			tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = false;
			tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string("0");
			tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string("1");
		}
		else
		{
			tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = true;
			tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = true;
		}
		return;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// validateValues(void)
	// @returns : True if valid, false if not.
	// POST CONDITION: tbErrors is set and errors may be displayed on the screen.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	bool validateValues();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// showControl()
	// @channel: (IN) The channel which we are configuring.
	// @isInput: (IN) If this an input or output.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void showControl(TSOSCCVChannel* channel, bool isInput);

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Process
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @args.vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawLayer(/*in*/ const DrawArgs &args, int layer) override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Top Display for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVTopDisplay : TransparentWidget {
	oscCVWidget* parentWidget;
	//std::shared_ptr<Font> font;  // Rack v2 Conversion - Don't even store this font ptr
	//std::shared_ptr<Font> labelFont;  // Rack v2 Conversion - Don't even store this font ptr
	std::string fontPath;
	std::string labelFontPath;
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
	~TSOscCVTopDisplay() {
		parentWidget = NULL;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSOscCVTopDisplay()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSOscCVTopDisplay(oscCVWidget* widget)
	{
		parentWidget = widget;
		fontSize = 12;
		memset(messageStr, '\0', sizeof(char)*TROWA_DISP_MSG_SIZE);
		memset(scrollingMsg, '\0', sizeof(char)*TROWA_SCROLLING_MSG_TOTAL_SIZE);
		showDisplay = true;

		fontPath = asset::plugin(pluginInstance, TROWA_DIGITAL_FONT);  // Rack v2 store font path
		labelFontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path
		return;
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

}; // end struct TSOscCVTopDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Middle Display for oscCV widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSOscCVMiddleDisplay : TransparentWidget {
	oscCVWidget* parentWidget;
	//std::shared_ptr<Font> font;   // Rack v2 Conversion - Don't even store this font ptr
	//std::shared_ptr<Font> labelFont;  // Rack v2 Conversion - Don't even store this font ptr
	std::string fontPath; // Rack v2 store font path
	std::string labelFontPath; // Rack v2 store font path
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	//bool showDisplay = true;

	enum DisplayMode {
		// None - It's in configuration mode.
		None = 0,
		// Info
		Default = 1,
		// Show only the config name
		ConfigName = 2,
		// Show the config name and label for the ID.
		ConfigNameAndLabel = 3
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
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';
		fontPath = asset::plugin(pluginInstance, TROWA_DIGITAL_FONT);  // Rack v2 store font path
		labelFontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path
		return;
	}
	~TSOscCVMiddleDisplay() {
		parentWidget = NULL;
		return;
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
	// @args.vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawLayer(/*in*/ const DrawArgs &args, int layer) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawChannelChart()
	// Draw the channel data.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawChannelChart(/*in*/ const DrawArgs &args, /*in*/ TSOSCCVChannel* channelData,  /*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height, /*in*/ NVGcolor lineColor);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// drawChannelBar()
	// Draw the channel data.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void drawChannelBar(/*in*/ const DrawArgs &args, /*in*/ TSOSCCVChannel* channelData,  /*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height, /*in*/ NVGcolor lineColor);

}; // end struct TSOscCVMiddleDisplay

struct TSPageNumberControl : TS_ScreenDial
{

	TSPageNumberControl(Vec size, Module* module, int paramId) : TS_ScreenDial(size, module, paramId)
	{
		return;
	}

	// Displays 'x of N' (i.e. Normal display string ('x') with 'of N' appended where 'N' is the maxValue + 1).
	// So for like 'Page 2 of 4'.
	std::string getDisplayText() override
	{
		ParamQuantity* pQty = getParamQuantity();
		if (pQty)
		{
			int maxNumPages = static_cast<int>(pQty->maxValue) + 1;
			return rack::string::f("%s of %d", pQty->getDisplayValueString().c_str(), maxNumPages);
		}
		else
			return std::string("");
	}
};

#endif // !WIDGET_OSCCV_HPP
