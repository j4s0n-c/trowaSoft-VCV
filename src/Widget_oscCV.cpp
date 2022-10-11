#include "Widget_oscCV.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "Module_oscCV.hpp"
#include <stdlib.h>
#include "TSOSCCV_Common.hpp"
#include <vector>
#include <string>

#define OSCCV_CHOOSE_UNUSED_PORTS		0

// Channel colors
const NVGcolor oscCVWidget::CHANNEL_COLORS[TROWA_OSCCV_NUM_COLORS] = {
	TSColors::COLOR_TS_C01, TSColors::COLOR_TS_C02, TSColors::COLOR_TS_C03, TSColors::COLOR_TS_C04, 
	TSColors::COLOR_TS_C05, TSColors::COLOR_TS_C06, TSColors::COLOR_TS_C07, TSColors::COLOR_TS_C08
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVWidget()
// Instantiate a oscCV widget. v0.60 must have module as param.
// @oscModule : (IN) Pointer to the osc module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVWidget::oscCVWidget(oscCV* oscModule) : TSSModuleWidgetBase(oscModule, false)
{
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	if (!isPreview && oscModule == NULL)
	{
		oscModule = dynamic_cast<oscCV*>(this->module);
	}
	this->numberChannels = (isPreview) ? TROWA_OSCCV_DEFAULT_NUM_CHANNELS : oscModule->numberChannels; 

	Vec topScreenSize = Vec(363, 48);

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/cvOSCcv.svg")));
		addChild(panel);
	}

	////////////////////////////////////
	// Top Screen
	////////////////////////////////////
	{
		this->display = new TSOscCVTopDisplay(this);
		this->display->showDisplay = true;
		display->box.pos = Vec(TROWA_HORIZ_MARGIN, 24);
		display->box.size = topScreenSize;
		addChild(display);
	}

	////////////////////////////////////
	// OSC configuration screen.
	////////////////////////////////////
	if (!isPreview)
	{
		TSOSCConfigWidget* oscConfig = new TSOSCConfigWidget(oscModule, oscCV::ParamIds::OSC_SAVE_CONF_PARAM, oscCV::ParamIds::OSC_AUTO_RECONNECT_PARAM,
			oscModule->currentOSCSettings.oscTxIpAddress.c_str(), oscModule->currentOSCSettings.oscTxPort, oscModule->currentOSCSettings.oscRxPort,
			false, OSCClient::GenericClient, true, TROWA_OSCCV_DEFAULT_NAMESPACE);
		oscConfig->setVisible(false);
		oscConfig->box.pos = Vec(TROWA_HORIZ_MARGIN, 24);
		oscConfig->box.size = topScreenSize;

		this->oscConfigurationScreen = oscConfig;
		addChild(oscConfig);
	}
	
	//////////////////////////////////////
	// Labels
	//////////////////////////////////////
	{
		TSOscCVLabels* labelArea = new TSOscCVLabels();
		labelArea->box.pos = Vec(TROWA_HORIZ_MARGIN, topScreenSize.y + 24);
		labelArea->box.size = Vec(box.size.x - TROWA_HORIZ_MARGIN * 2, box.size.y - labelArea->box.pos.y - 15);
		addChild(labelArea);
	}

	int x, y, dx, dy;
	int xStart, yStart;


	//////////////////////////////////////
	// Parameters / UI Buttons
	//////////////////////////////////////
	Vec ledSize = Vec(15, 15);
	dx = 28;

	//---------------------------
	// Button: Show Config
	//---------------------------
	TS_LEDButton* btn;
	Vec btnSize = Vec(ledSize.x - 2, ledSize.y - 2);	
	y = topScreenSize.y + 30;
	x = (box.size.x - ledSize.x)/2; //76; // 80
	btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(x, y), oscModule, oscCV::ParamIds::OSC_SHOW_CONF_PARAM));//, 0, 1, 0));
	btn->setSize(btnSize);
	addParam(btn);
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + 1.5, y + 1.5), oscModule, oscCV::LightIds::OSC_CONFIGURE_LIGHT, ledSize, TSColors::COLOR_WHITE));
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + 3.5, y + 3.5), oscModule, oscCV::LightIds::OSC_ENABLED_LIGHT, Vec(ledSize.x - 4, ledSize.y - 4), TSOSC_STATUS_COLOR));

	// Previous
	x = (box.size.x - ledSize.x)/2.f - 110;
	btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(x, y), oscModule, oscCV::ParamIds::OSC_EXPANDER_CONFIG_PREV_PARAM));
	btn->setSize(btnSize);
	addParam(btn);
	prevLight = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(x + 1.5, y + 1.5), oscModule, oscCV::LightIds::OSC_CONFIGURE_PREV_LIGHT, ledSize, TSColors::COLOR_RED));
	addChild(prevLight);
	
	// Next
	x = (box.size.x - ledSize.x)/2.f + 110;
	btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(x, y), oscModule, oscCV::ParamIds::OSC_EXPANDER_CONFIG_NEXT_PARAM));
	btn->setSize(btnSize);
	addParam(btn);
	nextLight = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(x + 1.5, y + 1.5), oscModule, oscCV::LightIds::OSC_CONFIGURE_NEXT_LIGHT, ledSize, TSColors::COLOR_RED));
	addChild(nextLight);


	xStart = TROWA_HORIZ_MARGIN;
	yStart = 98; // 88
	dx = 28;
	dy = 30; // 32
	const float tbYOffset = 6.0;
	const float tbXOffset = 6.0;

	////////////////////////////////////
	// Middle Screen
	////////////////////////////////////
	x = xStart + dx * 2 + tbXOffset / 2.0;
	y = yStart;
	{
		int xEnd = box.size.x - xStart - dx * 2 - tbXOffset / 2.0;
		middleDisplay = new TSOscCVMiddleDisplay(this);
		middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::Default);
		middleDisplay->box.size = Vec(xEnd - x, numberChannels* dy);
		middleDisplay->box.pos = Vec(x, y);
		addChild(middleDisplay);
	}

	////////////////////////////////////
	// Channel Configuration
	////////////////////////////////////
	{
		this->oscChannelConfigScreen = new TSOscCVChannelConfigScreen(this, Vec(x, y), middleDisplay->box.size);
		oscChannelConfigScreen->setVisibility(false);
		addChild(oscChannelConfigScreen);
	}

	////////////////////////////////////
	// Input Ports
	////////////////////////////////////
	// Trigger and Value CV inputs for each channel
	y = yStart;
	ledSize = Vec(5, 5);
	float ledYOffset = 12.5;
#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
	Vec tbPathSize = Vec(92, 20); // 88, 20 // 108, 20
#else
	Vec tbPathSize = Vec(108, 20); // 88, 20 // 108, 20
#endif
	btnSize = Vec(24, 20); 
	for (int r = 0; r < numberChannels; r++)
	{
		// Light (to indicate when we send OSC)
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(xStart - ledSize.x, y + ledYOffset), oscModule, oscCV::LightIds::CH_LIGHT_START + r * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL, ledSize, CHANNEL_COLORS[r]));

		// Trigger Input:
		x = xStart;
		if (colorizeChannels)
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled));

		// Value input:
		x += dx;
		if (colorizeChannels)
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled));


		//---* OSC Channel Configuration *---
		// OSC Input Path (this is OSC outgoing message, our input)
		x += dx + tbXOffset;
		tbExpanderXPos[0] = x;
		std::string path = (isPreview) ? "/ch/" + std::to_string(r + 1) : oscModule->inputChannels[r].path;
		TSTextField* txtField = new TSTextField(TSTextField::TextType::Any, TROWA_OSCCV_OSC_PATH_SIZE);
		txtField->box.size = tbPathSize;
		txtField->box.pos = Vec(x, y + tbYOffset);
		txtField->text = path;
		if (colorizeChannels) {
			txtField->borderColor = CHANNEL_COLORS[r];
			txtField->caretColor = CHANNEL_COLORS[r];
			txtField->caretColor.a = 0.70;
		}
		tbOscInputPaths.push_back(txtField);
		addChild(txtField);

#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		// OSC Advanced Channel Config Button (INPUTS)
		x += txtField->box.size.x;
		int paramId = oscCV::ParamIds::CH_PARAM_START 
			+ r*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS
			+ TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
		// Make sure it is set to momentary
		TS_ScreenBtn* btn = new TS_ScreenBtn(btnSize, oscModule, paramId, std::string( "ADV" ), /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f, true);
		btn->box.pos = Vec(x, y + tbYOffset);
		btn->borderColor = CHANNEL_COLORS[r];
		btn->color = CHANNEL_COLORS[r];
		btn->borderWidth = 1;
		btn->backgroundColor = nvgRGB(clamp(CHANNEL_COLORS[r].r - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].g - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].b - 0.3f, 0.0f, 1.0f));
		btn->setVisible(false);
		addParam(btn);	
		btnDrawInputAdvChConfig.push_back(btn);
#endif

		y += dy;
	} // end input channels
	
	////////////////////////////////////
	// Output Ports
	////////////////////////////////////
	// Trigger and Value CV inputs for each channel
	xStart = box.size.x - TROWA_HORIZ_MARGIN - dx * 2;
	y = yStart;
	for (int r = 0; r < numberChannels; r++)
	{
		//---* OSC Channel Configuration *---
		// OSC Output Path (this is OSC incoming message, our output)
		x = xStart - tbXOffset - tbPathSize.x;
		tbExpanderXPos[1] = x;
		std::string path = (isPreview) ? "/ch/" + std::to_string(r + 1) : oscModule->outputChannels[r].path;
		TSTextField* txtField = new TSTextField(TSTextField::TextType::Any, TROWA_OSCCV_OSC_PATH_SIZE);
		txtField->box.size = tbPathSize;
		txtField->box.pos = Vec(x, y + tbYOffset);
		txtField->text = path;
		if (colorizeChannels) {
			txtField->borderColor = CHANNEL_COLORS[r];
			txtField->caretColor = CHANNEL_COLORS[r];
			txtField->caretColor.a = 0.70;
		}
		tbOscOutputPaths.push_back(txtField);
		addChild(txtField);

#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		// OSC Advanced Channel Config (OUTPUT)
		x -= btnSize.x;
		tbExpanderXPos[1] = x;		
		int paramId = oscCV::ParamIds::CH_PARAM_START
			+ (numberChannels + r) * TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
		// Make sure it is set to momentary			
		TS_ScreenBtn* btn = new TS_ScreenBtn(btnSize, oscModule, paramId, std::string("ADV"), /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f, true);
		btn->box.pos = Vec(x, y + tbYOffset);
		btn->borderColor = CHANNEL_COLORS[r];
		btn->color = CHANNEL_COLORS[r];
		btn->borderWidth = 1;
		btn->backgroundColor = nvgRGB(clamp(CHANNEL_COLORS[r].r - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].g - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].b - 0.3f, 0.0f, 1.0f));
		btn->setVisible(false);
		addParam(btn);
		btnDrawOutputAdvChConfig.push_back(btn);
#endif

		// Trigger Input:
		x = xStart;
		if (colorizeChannels)
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2, !plugLightsEnabled));

		// Value input:
		x += dx;
		if (colorizeChannels)
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2 + 1, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2 + 1, !plugLightsEnabled));

		// Light (to indicate when we receive OSC)
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + dx + ledSize.x/2.0, y + ledYOffset), oscModule, oscCV::LightIds::CH_LIGHT_START + r * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1, ledSize, CHANNEL_COLORS[r]));


		y += dy;
	} // end output channels
	
	// Expander Name
	tbExpanderID = new TSTextField(TSTextField::TextType::Any, TROWA_OSCCV_OSC_PATH_SIZE);
	tbExpanderID->box.size = tbPathSize;
	tbExpanderID->box.size.x += btnSize.x;
	tbExpanderID->box.pos = Vec(tbOscInputPaths[0]->box.pos.x, tbOscInputPaths[0]->box.pos.y + 14);//Vec(x, y + tbYOffset);
	tbExpanderID->visible = false;
	tbExpanderID->maxLength = 50;
	if (colorizeChannels) {
		tbExpanderID->borderColor = CHANNEL_COLORS[0];
		tbExpanderID->caretColor = CHANNEL_COLORS[0];
		tbExpanderID->caretColor.a = 0.70;
	}
	addChild(tbExpanderID);	

	// Set Tab Order:
	for (int c = 0; c < numberChannels; c++)
	{
		tbOscInputPaths[c]->nextField = tbOscOutputPaths[c];
		tbOscOutputPaths[c]->prevField = tbOscInputPaths[c];
		if (c > 0)
		{
			tbOscInputPaths[c]->prevField = tbOscOutputPaths[c - 1];
		}
		else
		{
			// Loop back around.
			//tbOscInputPaths[c]->prevField = tbOscOutputPaths[numberChannels - 1];			
			// Inject expander id.
			tbOscInputPaths[c]->prevField = tbExpanderID;
			tbExpanderID->prevField = tbOscOutputPaths[numberChannels - 1];
		}
		if (c < numberChannels - 1)
		{
			tbOscOutputPaths[c]->nextField = tbOscInputPaths[c + 1];
		}
		else
		{
			// Loop back around.
			//tbOscOutputPaths[c]->nextField = tbOscInputPaths[0]; // Loop back around
			// Inject expander id.			
			tbOscOutputPaths[c]->nextField = tbExpanderID;			
			tbExpanderID->nextField = tbOscInputPaths[0];
		}
	} // end loop through channels - put tab order on text fields
	
	
	// Screws:
	addChild(createWidget<ScrewBlack>(Vec(0, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 15, 0)));
	addChild(createWidget<ScrewBlack>(Vec(0, box.size.y - 15)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 15, box.size.y - 15)));

	if (oscModule != NULL)
	{
		oscModule->isInitialized = true;
	}

	toggleChannelPathConfig(false);
	return;
} //end oscCVWidget()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
// Handle UI controls.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVWidget::step()
{
	if (this->module == NULL)
		return;

	oscCV* thisModule = dynamic_cast<oscCV*>(module);
	
	bool loadModuleToConfig = false;
	if (showConfigScreen != thisModule->oscShowConfigurationScreen)
	{
		// State change
		if (thisModule->oscShowConfigurationScreen)
		{
			// Show screens:
			loadModuleToConfig = true;
#if OSCCV_CHOOSE_UNUSED_PORTS			
			if (!thisModule->oscInitialized)
			{
				// Make sure the ports are available
				int p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscTxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscTxPort = TSOSCConnector::GetAvailablePortTrans(thisModule->oscId, thisModule->currentOSCSettings.oscTxPort, /*sharingAllowed*/ true);
				p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscRxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscRxPort = TSOSCConnector::GetAvailablePortRecv(thisModule->oscId, thisModule->currentOSCSettings.oscRxPort, /*sharingAllowed*/ true);

			}
#endif			
			this->oscConfigurationScreen->setValues(thisModule->currentOSCSettings.oscTxIpAddress, thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort, thisModule->oscNamespace);
			this->oscConfigurationScreen->ckAutoReconnect->checked = thisModule->oscReconnectAtLoad;
			this->oscConfigurationScreen->btnActionEnable = !thisModule->oscInitialized;			
		}
		else 
		{
			// Hide configuration screens:
			this->oscConfigurationScreen->setVisible(false);
			readChannelPathConfig(showConfigIndex); // Read any changes that may have happened. (we don't have room for a save button)			
			toggleChannelPathConfig(false);
			oscChannelConfigScreen->setVisibility(false); // Hide
			//--------------------------------------------------------------------
			// [2019-09-02] Save the ip, ports, and namespace even if we haven't successfully connected, so that presets and copying will work 
			// even if user doesn't actually connect at least once.
			thisModule->setOscNamespace(this->oscConfigurationScreen->tbNamespace->text);			
			// Save IP only if it is a valid IP
			if (this->oscConfigurationScreen->isValidIpAddress())
			{
				thisModule->oscNewSettings.oscTxIpAddress = this->oscConfigurationScreen->tbIpAddress->text.c_str();
			}
			// Save Tx Port only if it is a valid port
			if (this->oscConfigurationScreen->isValidTxPort())
			{
				thisModule->oscNewSettings.oscTxPort = this->oscConfigurationScreen->getTxPort();
			}
			// Save Rx Port only if it is valid port
			if (this->oscConfigurationScreen->isValidRxPort())
			{
				thisModule->oscNewSettings.oscRxPort = this->oscConfigurationScreen->getRxPort();				
			}
			// Go ahead and save auto-reconnect on load on hide screen (Issue https://github.com/j4s0n-c/trowaSoft-VCV/issues/55)
			thisModule->oscReconnectAtLoad = this->oscConfigurationScreen->ckAutoReconnect->checked;
		} // end else (hide config)
		thisModule->lights[oscCV::LightIds::OSC_CONFIGURE_LIGHT].value = (thisModule->oscShowConfigurationScreen) ? 1.0 : 0.0;
		this->oscConfigurationScreen->setVisible(thisModule->oscShowConfigurationScreen);
		this->display->showDisplay = !thisModule->oscShowConfigurationScreen;
		if (thisModule->oscShowConfigurationScreen) {
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::None);
		}
		else {
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::Default);
		}		
	}
	else if (thisModule->oscShowConfigurationScreen)
	{
		// Configuration screen is showing, but we have changed which channels we are configuring.
		if (thisModule->expCurrentEditExpanderIx != showConfigIndex)
		{	
			// Module being configured has changed.
			loadModuleToConfig = true;
			// Read any changes that may have happened. (we don't have room for a save button)
			// For the old module:
			readChannelPathConfig(showConfigIndex); 
		}
	}
	
	
	if (loadModuleToConfig)// (thisModule->oscConfigTrigger.process(thisModule->params[oscCV::ParamIds::OSC_SHOW_CONF_PARAM].getValue()))
	{
		masterConfigLoaded = thisModule->expCurrentEditExpanderIx == 0 || thisModule->expCurrentEditExpander == NULL;
		currentEditExpander = thisModule->expCurrentEditExpander; // Save reference
		setChannelPathConfig(); // Set the channel text boxes
		oscChannelConfigScreen->setVisibility(false); // Hide
		if (masterConfigLoaded)
		{
			toggleChannelPathConfig(true);
			thisModule->expCurrentEditExpanderIx = 0;
			showConfigColor = TSColors::COLOR_WHITE;
			configName = std::string("");
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::None);			
		}
		else
		{
			// Show only input or output
			showConfigColor = calcColor(thisModule->expCurrentEditExpanderIx);			
			toggleChannelPathConfig(thisModule->expCurrentEditExpanderIx < 0, thisModule->expCurrentEditExpanderIx > 0);
			configName = (thisModule->expCurrentEditExpanderIx < 0) ? "CV -> OSC" : "OSC -> CV";
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::ConfigNameAndLabel);			
			configNameLeft = thisModule->expCurrentEditExpanderIx > 0;
		}
		this->oscConfigurationScreen->errorMsg = "";
		if (thisModule->oscError)
		{
			this->oscConfigurationScreen->errorMsg = "Error connecting to " + thisModule->currentOSCSettings.oscTxIpAddress;
		}
		this->oscConfigurationScreen->setVisible(true);
		
		// Buttons for Next/Previous - Change color if there is a next/previous
		prevLight->setColor(calcColor(thisModule->expCurrentEditExpanderIx - 1));
		nextLight->setColor(calcColor(thisModule->expCurrentEditExpanderIx + 1));
		
		// Rename the Advanced Configuration Buttons:
		int numChannels = thisModule->numberChannels;
		int baseChNum = abs( thisModule->expCurrentEditExpanderIx ) * numChannels;
		char buffer[50];
		for (int i = 0; i < numChannels; i++)
		{			
			int ch = baseChNum + i + 1;
			sprintf(buffer, "Configure %s Channel %d", "Input", ch);
			btnDrawInputAdvChConfig[i]->getParamQuantity()->name = std::string(buffer);
			sprintf(buffer, "Configure %s Channel %d", "Output", ch);
			btnDrawOutputAdvChConfig[i]->getParamQuantity()->name = std::string(buffer);			
		}
	}
	
	if (thisModule->oscShowConfigurationScreen)
	{
		//------------------------------------------
		// Check for show channel config buttons
		//------------------------------------------
		TSOSCCVChannel* editChannelPtr = NULL;
		bool isInput = false;
		/// TODO: Accomodate if Expanders ever have more or less # channels than the main module.
		for (int c = 0; c < thisModule->numberChannels; c++) {
			// Input Channel:
			int paramId = oscCV::ParamIds::CH_PARAM_START 
				+ c*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
			if (thisModule->inputChannels[c].showChannelConfigTrigger.process(thisModule->params[paramId].getValue())) {
				//DEBUG("btnClick: Input Ch %d, paramId = %d", c, paramId);				
				// Read any changes that may have happened. (we don't have room for a save button) on the previous screen
				readChannelPathConfig(showConfigIndex); 			
				if (masterConfigLoaded)			
					editChannelPtr = &(thisModule->inputChannels[c]);
				else if (thisModule->expCurrentEditExpander != NULL)
				{
					try
					{
						editChannelPtr = &(thisModule->expCurrentEditExpander->inputChannels[c]);
					}
					catch (const std::exception& expanderNull)
					{
						WARN("Error %s - Expander - ", expanderNull.what());
					}
				}
				isInput = true;
				break;
			}
			paramId = oscCV::ParamIds::CH_PARAM_START
				+ (thisModule->numberChannels + c) * TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
			// Output Channel:
			if (thisModule->outputChannels[c].showChannelConfigTrigger.process(thisModule->params[paramId].getValue())) {
				//DEBUG("btnClick: Output Ch %d, paramId = %d", c, paramId);
				// Read any changes that may have happened. (we don't have room for a save button) on the previous screen
				readChannelPathConfig(showConfigIndex);				
				if (masterConfigLoaded)
					editChannelPtr = &(thisModule->outputChannels[c]);
				else if (thisModule->expCurrentEditExpander != NULL)
				{
					try
					{
						editChannelPtr = &(thisModule->expCurrentEditExpander->outputChannels[c]);
					}
					catch (const std::exception& expanderNull)
					{
						WARN("Error %s - Expander - ", expanderNull.what());
					}
				}				
				isInput = false;
				break;
			}
		} // end for (loop through channels)
			
		if (editChannelPtr != NULL) 
		{
			this->toggleChannelPathConfig(false); // Hide the channel paths
			this->oscChannelConfigScreen->showControl(editChannelPtr, isInput);
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::ConfigName);
			configNameLeft = false;
		}
		else if (this->oscChannelConfigScreen->visible)
		{
			// Check for enable/disable data massaging
			// Check for Save or Cancel
			bool screenDone = false;
			if (oscChannelConfigScreen->saveTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_SAVE_PARAM].getValue())) {
				// Validate form & save
				screenDone = oscChannelConfigScreen->saveValues();
			}
			else if (oscChannelConfigScreen->cancelTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_CANCEL_PARAM].getValue())) {
				// Just hide and go back to paths
				screenDone = true;
				oscChannelConfigScreen->setVisibility(false); // Hide
				this->toggleChannelPathConfig(true); // Show paths again
			}
			if (screenDone) {
				oscChannelConfigScreen->setVisibility(false); // Hide
				this->toggleChannelPathConfig(thisModule->expCurrentEditExpanderIx <= 0, thisModule->expCurrentEditExpanderIx >= 0); // Show paths again
				configNameLeft = thisModule->expCurrentEditExpanderIx > 0;
				this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::ConfigNameAndLabel);				
			}
		}

		//------------------------------------------
		// Check for enable/disable OSC
		//------------------------------------------
		if (thisModule->oscConnectTrigger.process(thisModule->params[oscCV::ParamIds::OSC_SAVE_CONF_PARAM].getValue()))
		{
			if (oscConfigurationScreen->btnActionEnable)
			{
				// Enable OSC ------------------------------------------------------------------------
				// User checked to connect
				if (!this->oscConfigurationScreen->isValidIpAddress())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("IP Address is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid IP Address.";
					this->oscConfigurationScreen->tbIpAddress->requestFocus();
				}
				else if (!this->oscConfigurationScreen->isValidTxPort())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("Tx Port is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid Output Port (0-" + std::to_string(0xFFFF) + ").";
					this->oscConfigurationScreen->tbTxPort->requestFocus();

				}
				else if (!this->oscConfigurationScreen->isValidRxPort())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("Rx Port is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid Input Port (0-" + std::to_string(0xFFFF) + ").";
					this->oscConfigurationScreen->tbRxPort->requestFocus();
				}
				else
				{
					// Try to connect
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("Save OSC Configuration clicked, save information for module.");
#endif
					readChannelPathConfig(thisModule->expCurrentEditExpanderIx);
					this->oscConfigurationScreen->errorMsg = "";
					thisModule->oscNewSettings.oscTxIpAddress = this->oscConfigurationScreen->tbIpAddress->text.c_str();
					thisModule->oscNewSettings.oscTxPort = this->oscConfigurationScreen->getTxPort();
					thisModule->oscNewSettings.oscRxPort = this->oscConfigurationScreen->getRxPort();
					//thisModule->oscCurrentClient = this->oscConfigurationScreen->getSelectedClient();
					//DEBUG("Setting namespace");
					thisModule->setOscNamespace(this->oscConfigurationScreen->tbNamespace->text);
					thisModule->oscCurrentAction = oscCV::OSCAction::Enable;
					thisModule->oscReconnectAtLoad = this->oscConfigurationScreen->ckAutoReconnect->checked;
				}
			} // end if enable osc
			else
			{
				// Disable OSC ------------------------------------------------------------------
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Disable OSC clicked.");
#endif
				this->oscConfigurationScreen->errorMsg = "";
				thisModule->oscCurrentAction = oscCV::OSCAction::Disable;
			} // end else disable OSC
		} // end if OSC Save btn pressed
		else
		{
			if (thisModule->oscError)
			{
				if (this->oscConfigurationScreen->errorMsg.empty())
					this->oscConfigurationScreen->errorMsg = "Error connecting to " + thisModule->currentOSCSettings.oscTxIpAddress + ".";
			}
		}
		// Current status of OSC		
		if (thisModule->oscInitialized)
		{
			this->oscConfigurationScreen->statusMsg2 = thisModule->currentOSCSettings.oscTxIpAddress;
			this->oscConfigurationScreen->btnActionEnable = false;
		}
		else
		{
			this->oscConfigurationScreen->successMsg = "";
			this->oscConfigurationScreen->statusMsg2 = "OSC Not Connected";
			this->oscConfigurationScreen->btnActionEnable = true;
		}
	} // end if show OSC config screen
	
	if (thisModule->debugOSCConsoleOn)
	{
		
	}
		
	showConfigIndex = thisModule->expCurrentEditExpanderIx; // Save this for next time.
	showConfigScreen = thisModule->oscShowConfigurationScreen;	
	
	ModuleWidget::step();
	return;
} // end oscCVWidget()

//-- Events --//
// onDragEnd() - See if are still 
void oscCVWidget::onDragEnd(const event::DragEnd &e)
{
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;	
	ModuleWidget::onDragEnd(e);
	if (this->module != NULL && showConfigScreen)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(this->module);	
		if (!masterConfigLoaded && showConfigIndex != 0)
		{
			oscCVExpander* expander = thisModule->getExpansionModule(showConfigIndex);
			if (expander != currentEditExpander)
			{
				// We were configuring an Expander, but we *just* moved. 
				// This guy is NOT valid anymore.
				// Reset
				thisModule->expCurrentEditExpanderIx = 0; // Reset, our process() function should detect this is different and reload the main.
				thisModule->expCurrentEditExpander = NULL;
				this->oscChannelConfigScreen->visible = false;
				this->clearChannelPathConfig();			
			}
		}
	}
	return;
} // end onDragEnd()


// Show or hide the channel configuration
void oscCVWidget::toggleChannelPathConfig(bool show)
{
	toggleChannelPathConfig(show, show);
	return;
}
// Show or hide the channel configuration
void oscCVWidget::toggleChannelPathConfig(bool showInput, bool showOutput)
{
	for (int i = 0; i < this->numberChannels; i++)
	{
		this->tbOscInputPaths[i]->visible = showInput;
		this->tbOscOutputPaths[i]->visible = showOutput;
#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		btnDrawInputAdvChConfig[i]->setVisible(showInput);
		btnDrawOutputAdvChConfig[i]->setVisible(showOutput);		
#endif
	}
	
	// EXPANDERS ================================
	if (showInput == showOutput)
	{
		// Hide our expander text box.
		// (Everything is hidden OR every channel text box is show).
		tbExpanderID->visible = false;
	}
	else
	{
		tbExpanderID->visible = true;		
		if (showInput)
		{
			// Move expander to the right
			tbExpanderID->box.pos.x = tbExpanderXPos[1];
		}
		else 
		{
			tbExpanderID->box.pos.x = tbExpanderXPos[0];			
		}
		if (colorizeChannels) {
			tbExpanderID->borderColor = showConfigColor;
			tbExpanderID->caretColor = showConfigColor;
			tbExpanderID->caretColor.a = 0.70;
		}
	}
	return;
}
// Read the channel path configs and store in module's channels.
std::string oscCVWidget::readChannelPathConfig(TSOSCCVInputChannel* inputChannels, TSOSCCVChannel* outputChannels, int nChannels)
{
	std::string expanderName = std::string("");
	if (tbExpanderID->visible)
	{
		expanderName = tbExpanderID->text;
	}	
	if (inputChannels != NULL || outputChannels != NULL)
	{
		try
		{
			for (int i = 0; i < nChannels; i++)
			{
				if (inputChannels)
					inputChannels[i].setPath(this->tbOscInputPaths[i]->text);
				if (outputChannels)
					outputChannels[i].setPath(this->tbOscOutputPaths[i]->text);					
			}
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
	}
	return expanderName;
} // end readChannelPathConfig()

// Read the channel path configs and store in module's channels.
void oscCVWidget::readChannelPathConfig(int index)
{
	if (module != NULL)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(module);		
		try
		{
			if (masterConfigLoaded)
			{
				readChannelPathConfig(thisModule->inputChannels, thisModule->outputChannels, this->numberChannels);	
				thisModule->renamePorts(); // [Rack v2] Ports can have labels.
			}
			else if (index != 0)
			{
				oscCVExpander* exp = thisModule->getExpansionModule(index);
				if (exp)
				{
					exp->displayName = readChannelPathConfig(exp->inputChannels, exp->outputChannels, exp->numberChannels);
					exp->renamePorts(); // [Rack v2] Ports can have labels.
				}
			}
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
	}
	return;
} // end readChannelPathConfig()

// Set the channel path text boxes.
void oscCVWidget::setChannelPathConfig(TSOSCCVInputChannel* inputChannels, TSOSCCVChannel* outputChannels, int nChannels, std::string expanderName) 
{
	if (inputChannels != NULL || outputChannels != NULL)
	{
		try
		{
			for (int i = 0; i < nChannels; i++)
			{
				if (inputChannels)
					this->tbOscInputPaths[i]->text = inputChannels[i].getPath();
				else
					this->tbOscInputPaths[i]->text = std::string("");
				if (outputChannels)
					this->tbOscOutputPaths[i]->text = outputChannels[i].getPath();
				else
					this->tbOscOutputPaths[i]->text = std::string("");					
			}
			tbExpanderID->text = expanderName;
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
	}
	return;
} // end setChannelPathConfig()
// Set the channel path text boxes.
void oscCVWidget::setChannelPathConfig() {
	if (module != NULL)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(module);
		try
		{
			if (masterConfigLoaded)
			{
				setChannelPathConfig(thisModule->inputChannels, thisModule->outputChannels, this->numberChannels, std::string(""));				
			}
			else if (thisModule->expCurrentEditExpanderIx != 0 && thisModule->expCurrentEditExpander)
			{
				setChannelPathConfig(thisModule->expCurrentEditExpander->inputChannels, thisModule->expCurrentEditExpander->outputChannels, thisModule->expCurrentEditExpander->numberChannels, thisModule->expCurrentEditExpander->displayName);
			}
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
	}
	return;
} // end setChannelPathConfig()

// Clear the channel paths in the configuration window..
void oscCVWidget::clearChannelPathConfig() 
{
	for (int i = 0; i < numberChannels; i++)
	{
		this->tbOscInputPaths[i]->text = std::string("");
		this->tbOscOutputPaths[i]->text = std::string("");
	}
	return;
} // end clearChannelPathConfig()

// This is not used right now:
// struct oscCVMenuItemDebug : MenuItem
// {
	// oscCV* oscModule = NULL;
	// bool debugOn = true;
	// oscCVMenuItemDebug(std::string text, bool debug, oscCV* oscModule)
	// {
		// this->box.size.x = 200;
		// this->text = text;
		// this->debugOn = debug;
		// this->oscModule = oscModule;
	// }

	// void onAction(const event::Action &e) override 
	// {
		// oscModule->debugOSCConsoleOn = this->debugOn;
		// if (this->debugOn)
			// oscModule->oscShowConfigurationScreen = false; // Turn off showing the configuration screen
	// }
	// void step() override {
		// rightText = (oscModule->debugOSCConsoleOn == this->debugOn) ? "✔" : "";
	// }
// };

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Create context menu with choice of osc output/send rate (Hz).
// (QUICK and dirty: Options list instead of making a text box).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVWidget::appendContextMenu(ui::Menu *menu)
{	
	oscCV* thisModule = dynamic_cast<oscCV*>(module);
	assert(thisModule);
	
	menu->addChild(new MenuSeparator);
	
	std::vector<std::string> optLabels;
	for (int i = 0; i < TROWA_OSCCV_NUM_SEND_HZ_OPTS; i++) {
		optLabels.push_back(std::to_string(TROWA_OSCCV_Send_Freq_Opts_Hz[i]));
	}	
	menu->addChild(createIndexSubmenuItem("Send Frequency (Hz)",
		optLabels,
		[=]() {
			return thisModule->getSendFrequencyIx();
		},
		[=](int ix) {
			thisModule->setSendFrequencyIx(ix);
		}
	));	

	return;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
// Draw labels on our widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVLabels::draw(/*in*/ const DrawArgs &args) {	
	//std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT)); // Rack v2 load font each time
	std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time

	// Default Font:
	nvgFontSize(args.vg, fontSize);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 1);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(args.vg, textColor);
	nvgFontSize(args.vg, fontSize);

	float x, y;
	int dx;
	int xStart, yStart;
	xStart = 0;
	yStart = 25; // 17
	dx = 28;
	//dy = 32;

	//-- * Top Buttons *--//
	x = 84;
	y = 18.5;
	// nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
	// nvgText(args.vg, x, y, "CONFIG", NULL);
	
	Vec ledSize = Vec(15, 15);	
	int offset = 5;
	x = (box.size.x - ledSize.x)/2.f - offset + 1;
	nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
	nvgText(args.vg, x, y, "MASTER", NULL);
	
	x = (box.size.x + ledSize.x)/2.f + offset;
	nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
	nvgText(args.vg, x, y, "CONFIG", NULL);
	
	offset = 4;
	//x = (box.size.x)/2.f + ledSize.x - offset;
	x = (box.size.x + ledSize.x) /2.f - 110 + 6;
	nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
	nvgText(args.vg, x, y, "LEFT", NULL);

	//x = (box.size.x - ledSize.x)/2.f + offset;
	x = (box.size.x - ledSize.x)/2.f + 110 - 4;
	nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
	nvgText(args.vg, x, y, "RIGHT", NULL);
	

	//--- * Inputs *---//
	// (Left hand side)
	// TRIG:
	x = xStart + dx / 2;
	y = yStart;
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgText(args.vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgText(args.vg, x, y, "VAL", NULL);
	// Bottom (INPUTS)
	x = xStart + dx;
	y = box.size.y - 13;
	nvgText(args.vg, x, y, "INPUTS", NULL);

	//--- * Outputs *---//
	// (Right hand side)
	// TRIG:
	x = box.size.x - dx / 2 - dx;
	y = yStart;
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgText(args.vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgText(args.vg, x, y, "VAL", NULL);
	// Bottom (INPUTS)
	x = box.size.x - dx;
	y = box.size.y - 13;
	nvgText(args.vg, x, y, "OUTPUTS", NULL);
} // end TSOscCVLabels::draw()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step()
// Calculate scrolling and stuff?
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVTopDisplay::step() {
	bool connected = false;
	bool isPreview = parentWidget->module == NULL;
	std::string thisIp = std::string("NO CONNECTION ");
	oscCV* thisModule = NULL;
	if (!isPreview)
	{
		thisModule = dynamic_cast<oscCV*>(parentWidget->module);
		connected = thisModule->oscInitialized;
		if (connected)
		{
			thisIp = thisModule->currentOSCSettings.oscTxIpAddress
				+ std::string(" Tx:") + std::to_string(thisModule->currentOSCSettings.oscTxPort)
				+ std::string(" Rx:") + std::to_string(thisModule->currentOSCSettings.oscRxPort);
			if (!thisModule->oscNamespace.empty())
			   thisIp = thisIp + ((thisModule->oscNamespace.at(0) == '/') ? " " : " /") + thisModule->oscNamespace + " ";				
		}
	}
	if (thisIp.compare(lastIp) != 0)
	{
		sprintf(scrollingMsg, "trowaSoft - %s - cv<->OSC<->cv - ", thisIp.c_str());
	}

	//dt += engineGetSampleTime() / scrollTime_sec;
	//if (dt > 1.0f)
	dt += 100.0 / APP->engine->getSampleRate();
	if (dt > scrollTime_sec) 
	{
		//DEBUG("Dt has grown. Increment Ix: %d", scrollIx);
		dt = 0.0f;
		if (static_cast<size_t>(scrollIx) == strlen(scrollingMsg) - 1)
			scrollIx = 0;
		else
			scrollIx++;
	}

	lastIp = thisIp;
	TransparentWidget::step(); // parent whatever he does
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
// Top display
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVTopDisplay::drawLayer(/*in*/ const DrawArgs &args, int layer)
{
	std::shared_ptr<Font> font = APP->window->loadFont(fontPath);  // Rack v2 load font each time
	std::shared_ptr<Font> labelFont = APP->window->loadFont(labelFontPath); // Rack v2 load font each time
	// Fonts don't show up when you close the VST and re-open???????? Even though they are loaded everytime....
	if (visible)
	{
		if (layer == 1)
		{
			// Background Colors:
			NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
			NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

			// Screen:
			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
			nvgFillColor(args.vg, backgroundColor);
			nvgFill(args.vg);
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);

			if (!showDisplay)
				return;

			Rect b = Rect(Vec(0, 0), Vec(box.size.x - 13, box.size.y));
			nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

			int x, y;

			// Default Font:
			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 2.5);
			NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
			nvgFillColor(args.vg, textColor);

			// SCROLLING MESSAGE ====================================
			x = 13;// -scrollIx * 0.5;
			y = 13;
			nvgFontSize(args.vg, fontSize * 1.5);	// Large font
			//nvgFontFaceId(args.vg, font->handle);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			
			if (parentWidget->module == NULL)
				sprintf(scrollingMsg, "trowaSoft - cv<->OSC<->cv - %s", "NO CONNECTION");

			// Start (left on screen) of scrolling message:
			const char * subStr = scrollingMsg + scrollIx;
			nvgText(args.vg, x, y, subStr, NULL);
			// Get circular wrap (right part of screen) - start of message again:
			float txtBounds[4] = { 0,0,0,0 };
			float nextX = nvgTextBounds(args.vg, x, y, subStr, NULL, txtBounds);
			x = nextX; // +24
			if (x < b.size.x) {
				// Wrap the start of the string around
				nvgText(args.vg, x, y, scrollingMsg, subStr);
			}
			nvgResetScissor(args.vg);			
		} // end if layer == 1
		
		this->Widget::drawLayer(args, layer); // Children if we have any
	} // end if visible	
	return;
} // end TSOscCVTopDisplay::draw()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Process
// Middle display -- If we have to do scrolling, calculate it.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::step() {
	if (displayMode == DisplayMode::Default) {
		dt += 100.0 / APP->engine->getSampleRate();
		if (dt > scrollTime)
		{
			dt = 0.0f;
			chPathPosition += 0.05f;
			if (chPathPosition > 1.0f)
				chPathPosition = 0.0f;
		}

	}
	TransparentWidget::step(); // parent whatever he does
	return;
} // end step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
// Middle display drawing.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawLayer(/*in*/ const DrawArgs &args, int layer) {	
	if (layer == 1)
	{
		bool isPreview = parentWidget->module == NULL; // May get a NULL module for preview
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
		std::shared_ptr<Font> labelFont = APP->window->loadFont(labelFontPath); // Rack v2 load font each time

		// Background Colors:
		NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

		// Screen:
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		if (!displayMode)
			return;
		
		if (displayMode == DisplayMode::Default)
		{
			// Default Display Mode
			oscCV* thisModule = (isPreview) ? NULL : dynamic_cast<oscCV*>(parentWidget->module);
			// Default Font:
			nvgFontSize(args.vg, 9);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 1);
			NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
			nvgFillColor(args.vg, textColor);

			int dy = 2;
			int dx = 4;
			int height = 28;
			int width = box.size.x / 2.0 - 4;
			int y = 2;
			float txtBounds[4] = { 0,0,0,0 };
			const int txtPadding = 4;
			int txtWidth = width - txtPadding;
			bool drawBoxes = false;
			int numChannels = (isPreview) ? TROWA_OSCCV_DEFAULT_NUM_CHANNELS : thisModule->numberChannels;
			char buffer[50];
			for (int c = 0; c < numChannels; c++) {
				int ix = 0;
				float nextX;

				//---- INPUT ----
				int x = 2;
				if (drawBoxes) {
					// Debug draw box:
					nvgBeginPath(args.vg);
					nvgRect(args.vg, x, y, width, height);
					nvgStrokeColor(args.vg, oscCVWidget::CHANNEL_COLORS[c]);
					nvgStrokeWidth(args.vg, 1.0);
					nvgStroke(args.vg);
				}
				// Label:
				const char* lbl = NULL;
				int len = 0;
				if (isPreview)
				{
					sprintf(buffer, "/ch/%d", c + 1);
					lbl = buffer;
					len = strlen(lbl);
				}
				else
				{
					lbl = thisModule->inputChannels[c].path.c_str();
					// Chart:
					drawChannelChart(args, &(thisModule->inputChannels[c]), x, y, width, height, oscCVWidget::CHANNEL_COLORS[c]);
					len = thisModule->inputChannels[c].path.length();
				}
				nextX = nvgTextBounds(args.vg, x, y, lbl, NULL, txtBounds);
				ix = 0;
				if (nextX > txtWidth) {
					ix = chPathPosition * len;
				}
				nvgScissor(args.vg, x, y, txtWidth, height);
				nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
				nvgText(args.vg, x, y + 1, &(lbl[ix]), NULL);
				nvgResetScissor(args.vg);

				//---- OUTPUT ----
				x += dx + width;
				if (drawBoxes) {
					// Debug draw box:
					nvgBeginPath(args.vg);
					nvgRect(args.vg, x, y, width, height);
					nvgStrokeColor(args.vg, oscCVWidget::CHANNEL_COLORS[thisModule->numberChannels - c - 1]);
					nvgStrokeWidth(args.vg, 1.0);
					nvgStroke(args.vg);
				}
				// Label:
				if (isPreview)
				{
					sprintf(buffer, "/ch/%d", c + 1);
					lbl = buffer;
					len = strlen(lbl);			
				}
				else
				{
					lbl = thisModule->outputChannels[c].path.c_str();
					// Chart:
					drawChannelChart(args, &(thisModule->outputChannels[c]), x, y, width, height, oscCVWidget::CHANNEL_COLORS[c]);
					len = thisModule->outputChannels[c].path.length();
				}
				nextX = nvgTextBounds(args.vg, x, y, lbl, NULL, txtBounds);
				ix = len;
				if (nextX > txtWidth) {
					ix = len - chPathPosition * len;
				}
				nvgScissor(args.vg, x + txtPadding, y, txtWidth, height);
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
				nvgText(args.vg, x + width, y + 1, lbl, &(lbl[ix]));
				nvgResetScissor(args.vg);

				y += dy + height;
			} // end loop		
		} // end if default display
		else 
		{		
			// Just show the config name
			const int margin = 20;
			int mult = 1;
			//int xOffset = box.size.y/2;
			//int yOffset = box.size.x/2;
			int x = box.size.y - margin;
			int y = box.size.x - margin;//(parentWidget->configNameLeft) ? box.size.x - margin : margin ;
			int align = NVG_ALIGN_RIGHT;
			if (!parentWidget->configNameLeft)
			{
				x = margin;
				align = NVG_ALIGN_LEFT;
				mult = -1;
			}

			nvgSave(args.vg);
			nvgTranslate(args.vg, box.size.x / 2.0, box.size.y / 2.0);		
			nvgRotate(args.vg, mult*NVG_PI*0.5);
			nvgTranslate(args.vg, -box.size.y / 2.0, -box.size.x / 2.0);		
			
			// Default Font:
			nvgFontSize(args.vg, 14);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0.2);
			//NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
			nvgTextAlign(args.vg, align | NVG_ALIGN_MIDDLE);
			nvgFillColor(args.vg, parentWidget->showConfigColor);		
			nvgText(args.vg, x, y, parentWidget->configName.c_str(), NULL); 
			
			nvgRestore(args.vg);
			
			if (displayMode == DisplayMode::ConfigNameAndLabel)
			{
				// Label for ID textbox:
				x = (parentWidget->configNameLeft) ? 3 : box.size.x/2.0 + 4;  //parentWidget->tbExpanderID->box.pos.x;
				y = 5;
				nvgFontSize(args.vg, 11);
				nvgFontFaceId(args.vg, labelFont->handle);
				nvgFillColor(args.vg, parentWidget->showConfigColor);				
				nvgTextLetterSpacing(args.vg, 0);
				nvgTextAlign(args.vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
				nvgText(args.vg, x, y, "Expander Name", NULL); 			
			}
		} // end if show config name		
	} // end if layer == 1
	
	this->Widget::drawLayer(args, layer);
	return;
} // end draw()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawChannelChart()
// Draw the channel data.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawChannelChart(/*in*/ const DrawArgs &args, /*in*/ TSOSCCVChannel* channelData,  
	/*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height,
	/*in*/ NVGcolor lineColor)
{
	nvgScissor(args.vg, x, y, width, height);
	nvgBeginPath(args.vg);
	float dx = static_cast<float>(width) / TROWA_OSCCV_VAL_BUFFER_SIZE;
	//if (dx < 0.5)
	//	dx = 0.5;
	float px = x;
	nvgMoveTo(args.vg, x, y);
	float startY = y + height;
	for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
	{
		float py = startY - rescale(channelData->valBuffer[i], channelData->minVoltage, channelData->maxVoltage, 0, height);
		nvgLineTo(args.vg, px, py);
		px += dx;
	}
	nvgStrokeColor(args.vg, lineColor);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);

	nvgResetScissor(args.vg);
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawChannelChart()
// Draw the channel data.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawChannelBar(/*in*/ const DrawArgs &args, /*in*/ TSOSCCVChannel* channelData,
	/*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height,
	/*in*/ NVGcolor lineColor)
{
	nvgScissor(args.vg, x, y, width, height);
	nvgBeginPath(args.vg);
	float dx = static_cast<float>(width) / TROWA_OSCCV_VAL_BUFFER_SIZE;
	float px = x;
	nvgMoveTo(args.vg, x, y);
	for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
	{
		float py = y + rescale(channelData->valBuffer[i], channelData->minVoltage, channelData->maxVoltage, 0, height);
		nvgLineTo(args.vg, px, py);
		px += dx;
	}
	nvgStrokeColor(args.vg, lineColor);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);

	nvgResetScissor(args.vg);
	return;
}
// On click
void TSOscCVDataTypeSelectBtn::onAction(const event::Action &e) {
	if (visible)
	{
		Menu *menu = createMenu();// gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;
		for (int i = 0; i < numVals; i++) {
			TSOscCVDataTypeItem *option = new TSOscCVDataTypeItem(this, i);
			option->itemVal = itemVals[i];
			option->text = itemStrs[i];
			menu->addChild(option);
		}
	}
	return;
}


// Draw if visible
void TSOscCVDataTypeSelectBtn::drawLayer(const DrawArgs &args, int layer) {
	if (visible) {
		if (layer == 1)
		{
			std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
			nvgScissor(args.vg, 0, 0, box.size.x, box.size.y);

			// Background
			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5.0);
			nvgFillColor(args.vg, backgroundColor);
			nvgFill(args.vg);

			// Border
			if (borderWidth > 0) {
				nvgStrokeWidth(args.vg, borderWidth);
				nvgStrokeColor(args.vg, borderColor);
				nvgStroke(args.vg);
			}
			nvgResetScissor(args.vg);

			if (font->handle >= 0) {
				nvgScissor(args.vg, 0, 0, box.size.x - 5, box.size.y);

				nvgFillColor(args.vg, color);
				nvgFontFaceId(args.vg, font->handle);
				//nvgTextLetterSpacing(args.vg, 0.0);

				nvgFontSize(args.vg, fontSize);
				nvgText(args.vg, textOffset.x, textOffset.y, text.c_str(), NULL);

				nvgResetScissor(args.vg);

				bndUpDownArrow(args.vg, box.size.x - 10, 10, 5, color);
			}			
		} // end layer == 1
		this->Widget::drawLayer(args, layer);
	} // end if visible
	return;
} // end draw()
// When the selected item chagnes.
void TSOscCVDataTypeSelectBtn::onSelectedIndexChanged() {
	if (parentScreen != NULL)
	{
		parentScreen->setDataType(static_cast<TSOSCCVChannel::ArgDataType>(this->selectedVal));
	}
	return;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSOscCVChannelConfigScreen()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSOscCVChannelConfigScreen::TSOscCVChannelConfigScreen(oscCVWidget* widget, Vec pos, Vec boxSize)
{
	fontPath = asset::plugin(pluginInstance, TROWA_DIGITAL_FONT); // Rack v2 store font path
	labelFontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path

	visible = false;
	box.size = boxSize;
	parentWidget = widget;
	Module* thisModule = (widget != NULL) ? widget->module : NULL;
	fontSize = 10;
	box.pos = pos;
	//visible = true;

	//DEBUG("Init Channel Config screen");
	int x, y;
	Vec tbSize = Vec(180, 20);
	int dx = 20;
	int dy = 15;

	// -- Toggle Translate Buton and Light
	x = startX;
	y = startY;
	Vec ledSize = Vec(15, 15);

	Vec btnSize = Vec(100, 20);
	x = box.size.x - startX - btnSize.x;
	// Translate values
	btnToggleTranslateVals = new TS_ScreenCheckBox(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM, /*text*/ "Convert Values", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnToggleTranslateVals->fontSize = 9;
	btnToggleTranslateVals->color = TSColors::COLOR_TS_GRAY;
	btnToggleTranslateVals->borderWidth = 0;
	btnToggleTranslateVals->padding = 2;
	btnToggleTranslateVals->textAlign = TS_ScreenBtn::TextAlignment::Right;
	btnToggleTranslateVals->box.pos = Vec(x, y);
	addChild(btnToggleTranslateVals);
	
	// Clip values
	btnToggleTranslateClipVals = new TS_ScreenCheckBox(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_CLIP_CV_VOLT_PARM, /*text*/ "Clip Values", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnToggleTranslateClipVals->fontSize = 9;
	btnToggleTranslateClipVals->color = TSColors::COLOR_TS_GRAY;
	btnToggleTranslateClipVals->borderWidth = 0;
	btnToggleTranslateClipVals->padding = 2;
	btnToggleTranslateClipVals->textAlign = TS_ScreenBtn::TextAlignment::Right;
	y = startY + ledSize.y + dy + fontSize * 2 + 5; // 13 + 15 + 20 + fontSize
	btnToggleTranslateClipVals->box.pos = Vec(x, y);
	addChild(btnToggleTranslateClipVals);


	// -- Min/Max Text Boxes
	btnSize = Vec(70, 20);
	tbSize = Vec(70, 20);
	dx = 15;
	x = startX;
	y = startY + ledSize.y + dy + fontSize * 2 + 5; // 13 + 15 + 20 + fontSize
	for (int i = 0; i < TextBoxIx::NumTextBoxes; i++) {
		tbNumericBounds[i] = new TSTextField(TSTextField::TextType::RealNumberOnly, 25);
		tbNumericBounds[i]->setRealPrecision(3); // mV? why not
		tbNumericBounds[i]->box.size = tbSize;
		tbNumericBounds[i]->box.pos = Vec(x, y);
		tbNumericBounds[i]->id = i;
		//DEBUG("TextBox %d at (%d, %d)", i, x, y);

		// Next Field
		if (i > 0)
		{
			tbNumericBounds[i]->prevField = tbNumericBounds[i-1];
			tbNumericBounds[i-1]->nextField = tbNumericBounds[i];
		}
		addChild(tbNumericBounds[i]);

		// Position:
		if (i % 2)
		{
			if (i < TextBoxIx::NumTextBoxes - 1) {
				x = startX; // New row
				y += dy + tbSize.y + fontSize * 2 + 5;
			}
		}
		else
		{
			x += tbSize.x + dx; // Next column
		}
	} // end for
	//TextBoxIx::NumTextBoxes
	tbNumericBounds[TextBoxIx::NumTextBoxes - 1]->nextField = tbNumericBounds[0]; // Loop back around


	// Data Type
	x = startX;
	y += tbSize.y + dy + dy;
	//DEBUG("Select at (%d, %d). %.2fx%.2f.", x, y, btnSize.x, btnSize.y);
	btnSelectDataType = new TSOscCVDataTypeSelectBtn(numDataTypes, reinterpret_cast<int*>(oscDataTypeVals), oscDataTypeStr, static_cast<int>(selectedDataType));
	btnSelectDataType->box.size = btnSize;
	btnSelectDataType->box.pos = Vec(x, y);
	btnSelectDataType->parentScreen = this;
	btnSelectDataType->visible = false;
	addChild(btnSelectDataType);

	// Save Button
	y += btnSelectDataType->box.size.y + dy;
	btnSave = new TS_ScreenBtn(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_SAVE_PARAM, /*text*/ "Save", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnSave->box.pos = Vec(x, y);
	btnSave->visible = false;
	btnSave->momentary = true; // This should be momentary (fix clicking buttons twice)
	addChild(btnSave);

	// Cancel button
	x += btnSize.x + dx * 2;
	//DEBUG("Cancel Button at (%d, %d). %.2fx%.2f.", x, y, btnSize.x, btnSize.y);
	btnCancel = new TS_ScreenBtn(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_CANCEL_PARAM, /*text*/ "Cancel", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnCancel->box.pos = Vec(x, y);
	btnCancel->visible = false;
	btnCancel->momentary = true; // This should be momentary (fix clicking buttons twice)	
	addChild(btnCancel);
	return;
} // end TSOscCVChannelConfigScreen()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVChannelConfigScreen::drawLayer(/*in*/ const DrawArgs &args, int layer)
{
	if (this->visible) 
	{		
		if (layer == 1)
		{
			std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
			std::shared_ptr<Font> labelFont = APP->window->loadFont(labelFontPath); // Rack v2 load font each time
			
			// Default Font:
			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 1);
			NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
			NVGcolor errorColor = TSColors::COLOR_TS_RED;

			// Draw labels
			int x, y;
			Vec tbSize = Vec(70, 20);
			//Vec ledSize = Vec(15, 15);
			int dx = 20;
			int dy = 15;
			const int errorDy = 32;

			int cIx = (this->currentChannelPtr->channelNum - 1) % TROWA_OSCCV_NUM_COLORS;
			NVGcolor channelColor = this->parentWidget->CHANNEL_COLORS[cIx];

			// -- Heading --
			// Channel Number and address
			x = startX;// +ledSize.x + 10;
			y = startY;
			sprintf(buffer, "CH %d %sPUT", this->currentChannelPtr->channelNum, (isInput) ? "IN" : "OUT");
			float txtBounds[4] = { 0,0,0,0 };
			const float padding = 2.0f;
			nvgFontSize(args.vg, fontSize*1.1);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextBounds(args.vg, x, y, buffer, NULL, txtBounds); //float txtWidth = 
			nvgBeginPath(args.vg);
			float rectHeight = txtBounds[3] - txtBounds[1] + padding*2;
			nvgRect(args.vg, txtBounds[0], y, txtBounds[2] - txtBounds[0] + padding*2, rectHeight);
			nvgFillColor(args.vg, channelColor);
			nvgFill(args.vg);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgFillColor(args.vg, TSColors::COLOR_BLACK);
			nvgText(args.vg, x + padding, y + padding, buffer, NULL);
			
			// Add the address:
			nvgFontSize(args.vg, fontSize*0.9);
			nvgFontFaceId(args.vg, labelFont->handle);
			nvgFillColor(args.vg, channelColor);			
			nvgText(args.vg, x, y + rectHeight + padding, this->currentChannelPtr->path.c_str(), NULL);			

			// Control Labels:
			nvgFillColor(args.vg, textColor);
			// -- Min/Max Text Boxes
			x = startX;
			y = startY + 15 + dy;
			//	y = startY + ledSize.y + dy + fontSize*2; // 13 + 15 + 20 + fontSize
			nvgFontSize(args.vg, fontSize*0.9);
			nvgFontFaceId(args.vg, font->handle);			
			sprintf(buffer, "Control Voltage (%s)", (isInput) ? "IN" : "OUT");
			nvgText(args.vg, x, y, buffer, NULL);
			y += fontSize + 1;
			const char* labels[] = { "Min", "Max", "Min", "Max" };
			for (int i = 0; i < TextBoxIx::NumTextBoxes; i++) {
				if (i == 2) {
					nvgFontSize(args.vg, fontSize*0.9);
					nvgFontFaceId(args.vg, font->handle);

					sprintf(buffer, "OSC Value (%s)", (isInput) ? "OUT" : "IN");
					nvgText(args.vg, x, y, buffer, NULL);
					y += fontSize + 1;
				}

				nvgFontSize(args.vg, fontSize);
				nvgFontFaceId(args.vg, labelFont->handle);
				nvgText(args.vg, x, y, labels[i], NULL);

				// Errors if any
				if (tbErrors[i].length() > 0) {
					nvgFontFaceId(args.vg, labelFont->handle);
					nvgFillColor(args.vg, errorColor);
					nvgText(args.vg, x, y + errorDy, tbErrors[i].c_str(), NULL);
					nvgFillColor(args.vg, textColor);
				}

				// Position:
				if (i % 2)
				{
					x = startX; // New row
					y += dy + tbSize.y + fontSize + 5;
				}
				else
				{
					x += tbSize.x + dx; // Next column
				}
			} // end for

			// Data Type
			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, labelFont->handle);
			nvgText(args.vg, x, y, "Data Type", NULL);

			
		} // end if layer == 1
		this->Widget::drawLayer(args, layer); // Parent
	}
	return;
} // end draw()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// showControl()
// @channel: (IN) The channel which we are configuring.
// @isInput: (IN) If this an input or output.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVChannelConfigScreen::showControl(TSOSCCVChannel* channel, bool isInput)
{
	this->currentChannelPtr = channel;
	this->isInput = isInput;

	try
	{
		// Translation On/Off
		translateValsEnabled = currentChannelPtr->convertVals;
		if (currentChannelPtr->convertVals)
		{		
			this->btnToggleTranslateVals->setValue(1.0f);
		}
		else
		{
			this->btnToggleTranslateVals->setValue(0.0f);
		}
		btnToggleTranslateVals->checked = translateValsEnabled;
		
		clipValsEnabled = currentChannelPtr->clipVals;
		this->btnToggleTranslateClipVals->setValue((clipValsEnabled) ? 1.0f : 0.0f);
		this->btnToggleTranslateClipVals->checked = clipValsEnabled;

		for (int i = 0; i < TextBoxIx::NumTextBoxes; i++)
		{
			tbErrors[i] = std::string("");
		}

		// Text Box Values
		char buffer[50] = { '\0' };
		sprintf(buffer, "%.3f", currentChannelPtr->minVoltage);
		tbNumericBounds[TextBoxIx::MinCVVolt]->text = std::string(buffer);
		sprintf(buffer, "%.3f", currentChannelPtr->maxVoltage);
		tbNumericBounds[TextBoxIx::MaxCVVolt]->text = std::string(buffer);

		tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = true;
		tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = true;
		switch (currentChannelPtr->dataType)
		{
		case TSOSCCVChannel::ArgDataType::OscInt:
		{
			const char * format = "%.0f";
			sprintf(buffer, format, currentChannelPtr->minOscVal);
			tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string(buffer);
			sprintf(buffer, format, currentChannelPtr->maxOscVal);
			tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string(buffer);
			break;
		}
		case TSOSCCVChannel::ArgDataType::OscBool:
		{
			tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = false;
			tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = false;

			tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string("0");
			tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string("1");
			break;
		}
		case TSOSCCVChannel::ArgDataType::OscFloat:
		default:
		{
			const char * format = "%.3f";
			sprintf(buffer, format, currentChannelPtr->minOscVal);
			tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string(buffer);
			sprintf(buffer, format, currentChannelPtr->maxOscVal);
			tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string(buffer);
			break;
		}
		} // end switch (data type)

		// Data Type
		selectedDataType = currentChannelPtr->dataType;
		this->btnSelectDataType->setSelectedValue(static_cast<int>(currentChannelPtr->dataType));

		setVisibility(true);
		
	}
	catch (const std::exception& chEx)
	{
		WARN("Error Configuring Channel: %s.", chEx.what());
	}
	return;
} // end showControl()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Process
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVChannelConfigScreen::step()
{
	if (this->visible && parentWidget != NULL) {
		oscCV* thisModule = dynamic_cast<oscCV*>( parentWidget->module );

		if (thisModule) {
			// Check for enable/disable data massaging
			// Save the values directly:
			translateValsEnabled = thisModule->params[oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM].getValue() > 0;
			// if (translateTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM].getValue())) {
				// //DEBUG("Translate button clicked");
				// translateValsEnabled = !translateValsEnabled;
			// }
			// Check for enable/disable clipping in data massaging:
			clipValsEnabled = thisModule->params[oscCV::ParamIds::OSC_CH_CLIP_CV_VOLT_PARM].getValue() > 0;
			// if (clipChannelInputTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_CLIP_CV_VOLT_PARM].getValue())){
				// clipValsEnabled = !clipValsEnabled;
			// }
		}
		btnToggleTranslateVals->checked = translateValsEnabled;
		btnToggleTranslateClipVals->checked = clipValsEnabled;
		OpaqueWidget::step(); // Parent
	}
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// validateValues(void)
// @returns : True if valid, false if not.
// POST CONDITION: tbErrors is set and errors may be displayed on the screen.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool TSOscCVChannelConfigScreen::validateValues()
{
	//DEBUG("TSOscCVChannelConfigScreen::validateValues()");
	bool isValid = true;
	for (int i = 0; i < TextBoxIx::NumTextBoxes; i++)
	{
		bool valid = tbNumericBounds[i]->isValid();
		tbErrors[i] = (valid) ? std::string("") : std::string("Invalid value.");
		isValid = isValid && valid;
	}
	if (isValid)
	{
		//---------------------
		// Check Min < Max
		//---------------------
		isValid = false;
		try
		{
			// 1. Rack voltage
			bool valid = std::stof(tbNumericBounds[TextBoxIx::MinCVVolt]->text, NULL) < std::stof(tbNumericBounds[TextBoxIx::MaxCVVolt]->text, NULL);
			if (valid)
				isValid = true;
			else
				tbErrors[TextBoxIx::MinCVVolt] = std::string("Min should be < Max.");
			// 2. OSC Values
			valid = std::stof(tbNumericBounds[TextBoxIx::MinOSCVal]->text, NULL) < std::stof(tbNumericBounds[TextBoxIx::MaxOSCVal]->text, NULL);
			if (!valid)
				tbErrors[TextBoxIx::MinOSCVal] = std::string("Min should be < Max.");
			isValid = isValid && valid;
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
	} // end if valid values
	return isValid;
} // end validateValues()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Save the values to the ptr.
// @channelPtr : (OUT) Place to save the values.
// @returns : True if saved, false if there was an error.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool TSOscCVChannelConfigScreen::saveValues(/*out*/ TSOSCCVChannel* channelPtr)
{
	bool saved = false;
		
	
	if (channelPtr != NULL && validateValues())
	{
		try
		{
			channelPtr->convertVals = translateValsEnabled;
			channelPtr->clipVals = clipValsEnabled;
			channelPtr->dataType = static_cast<TSOSCCVChannel::ArgDataType>(btnSelectDataType->selectedVal);			
			channelPtr->minVoltage = std::stof(tbNumericBounds[TextBoxIx::MinCVVolt]->text);
			channelPtr->maxVoltage = std::stof(tbNumericBounds[TextBoxIx::MaxCVVolt]->text);
			channelPtr->minOscVal = std::stof(tbNumericBounds[TextBoxIx::MinOSCVal]->text);
			channelPtr->maxOscVal = std::stof(tbNumericBounds[TextBoxIx::MaxOSCVal]->text);
			saved = true;
		}
		catch (const std::exception& e)
		{
			WARN("Error %s.", e.what());
		}
	}
	return saved;
} // end saveValues()

