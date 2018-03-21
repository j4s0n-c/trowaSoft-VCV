#include "Widget_oscCV.hpp"

#include "widgets.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "Module_oscCV.hpp"

// Channel colors
const NVGcolor oscCVWidget::CHANNEL_COLORS[TROWA_OSCCV_NUM_COLORS] = {
	COLOR_TS_RED, COLOR_DARK_ORANGE, COLOR_YELLOW, COLOR_TS_GREEN,
	COLOR_CYAN, COLOR_TS_BLUE, COLOR_PURPLE, COLOR_PINK
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVWidget()
// Instantiate a oscCV widget. v0.60 must have module as param.
// @oscModule : (IN) Pointer to the osc module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVWidget::oscCVWidget(oscCV* oscModule) : TSSModuleWidgetBase(oscModule)
{
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	bool isPreview = oscModule == NULL; // If this is null, this for a preview??? Just get the controls layed out
	this->module = oscModule;
	this->numberChannels = (isPreview) ? TROWA_OSCCV_DEFAULT_NUM_CHANNELS : oscModule->numberChannels; 

	Vec topScreenSize = Vec(363, 48);

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/cvOSCcv.svg")));
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
		TSOSCConfigWidget* oscConfig = new TSOSCConfigWidget(oscModule, oscCV::ParamIds::OSC_SAVE_CONF_PARAM, oscCV::ParamIds::OSC_DISABLE_PARAM,
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
	// Button: Enable OSC button
	//---------------------------
	LEDButton* btn;
	y = topScreenSize.y + 30;
	x = 76; // 80
	Vec btnSize = Vec(ledSize.x - 2, ledSize.y - 2);
	btn = dynamic_cast<LEDButton*>(ParamWidget::create<LEDButton>(Vec(x, y), oscModule, oscCV::ParamIds::OSC_SHOW_CONF_PARAM, 0, 1, 0));
	btn->box.size = btnSize;
	addParam(btn);
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(x, y), oscModule, oscCV::LightIds::OSC_CONFIGURE_LIGHT, ledSize, COLOR_WHITE));
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + 2, y + 2), oscModule, oscCV::LightIds::OSC_ENABLED_LIGHT, Vec(ledSize.x - 4, ledSize.y - 4), TSOSC_STATUS_COLOR));


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
		std::string path = (isPreview) ? "/ch/" + std::to_string(r + 1) : oscModule->inputChannels[r].path;
		TSTextField* txtField = new TSTextField(TSTextField::TextType::Any, 50);
		txtField->box.size = tbPathSize;
		txtField->box.pos = Vec(x, y + tbYOffset);
		txtField->text = path;
		if (colorizeChannels) {
			txtField->borderColor = CHANNEL_COLORS[r];
		}
		tbOscInputPaths.push_back(txtField);
		addChild(txtField);

#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		// OSC Advanced Channel Config Button
		x += txtField->box.size.x;
		TS_ScreenBtn* btn = new TS_ScreenBtn(btnSize, oscModule, oscCV::ParamIds::CH_PARAM_START + r * 2, std::string( "ADV" ), /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
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
	}


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
		std::string path = (isPreview) ? "/ch/" + std::to_string(r + 1) : oscModule->outputChannels[r].path;
		TSTextField* txtField = new TSTextField(TSTextField::TextType::Any, 50);
		txtField->box.size = tbPathSize;
		txtField->box.pos = Vec(x, y + tbYOffset);
		txtField->text = path;
		if (colorizeChannels) {
			txtField->borderColor = CHANNEL_COLORS[r];
		}
		tbOscOutputPaths.push_back(txtField);
		addChild(txtField);

#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		// OSC Advanced Channel Config
		x -= btnSize.x;
		TS_ScreenBtn* btn = new TS_ScreenBtn(btnSize, oscModule, oscCV::ParamIds::CH_PARAM_START + r * 2 + 1, std::string("ADV"), /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
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
	}

	// Screws:
	addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(0, box.size.y - 15)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, box.size.y - 15)));

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

	if (thisModule->oscConfigTrigger.process(thisModule->params[oscCV::ParamIds::OSC_SHOW_CONF_PARAM].value))
	{
		//debug("Button clicked");
		thisModule->oscShowConfigurationScreen = !thisModule->oscShowConfigurationScreen;
		thisModule->lights[oscCV::LightIds::OSC_CONFIGURE_LIGHT].value = (thisModule->oscShowConfigurationScreen) ? 1.0 : 0.0;
		this->oscConfigurationScreen->setVisible(thisModule->oscShowConfigurationScreen);
		this->display->showDisplay = !thisModule->oscShowConfigurationScreen;
		if (thisModule->oscShowConfigurationScreen) {
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::None);
		}
		else {
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::Default);
		}
		if (thisModule->oscShowConfigurationScreen)
		{
			if (!thisModule->oscInitialized)
			{
				// Make sure the ports are available
				int p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscTxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscTxPort = TSOSCConnector::GetAvailablePort(thisModule->oscId, thisModule->currentOSCSettings.oscTxPort);
				p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscRxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscRxPort = TSOSCConnector::GetAvailablePort(thisModule->oscId, thisModule->currentOSCSettings.oscRxPort);

			}
			this->oscConfigurationScreen->setValues(thisModule->currentOSCSettings.oscTxIpAddress, thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort, thisModule->oscNamespace);
			//this->oscConfigurationScreen->setSelectedClient(thisModule->oscCurrentClient); // OSC Client
			this->oscConfigurationScreen->btnActionEnable = !thisModule->oscInitialized;

			setChannelPathConfig(); // Set the channel text boxes
			toggleChannelPathConfig(true);

			this->oscConfigurationScreen->errorMsg = "";
			if (thisModule->oscError)
			{
				this->oscConfigurationScreen->errorMsg = "Error connecting to " + thisModule->currentOSCSettings.oscTxIpAddress;
			}
			this->oscConfigurationScreen->setVisible(true);
		}
		else
		{
			this->oscConfigurationScreen->setVisible(false);
			toggleChannelPathConfig(false);
			readChannelPathConfig(); // Read any changes that may have happened.
		}
	}
	if (thisModule->oscShowConfigurationScreen)
	{
		// Check for enable/disable
		if (thisModule->oscConnectTrigger.process(thisModule->params[oscCV::ParamIds::OSC_SAVE_CONF_PARAM].value))
		{
			if (oscConfigurationScreen->btnActionEnable)
			{
				// Enable OSC ------------------------------------------------------------------------
				// User checked to connect
				if (!this->oscConfigurationScreen->isValidIpAddress())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("IP Address is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid IP Address.";
					this->oscConfigurationScreen->tbIpAddress->requestFocus();
				}
				else if (!this->oscConfigurationScreen->isValidTxPort())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Tx Port is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid Output Port (0-" + std::to_string(0xFFFF) + ").";
					this->oscConfigurationScreen->tbTxPort->requestFocus();

				}
				else if (!this->oscConfigurationScreen->isValidRxPort())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Rx Port is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid Input Port (0-" + std::to_string(0xFFFF) + ").";
					this->oscConfigurationScreen->tbRxPort->requestFocus();
				}
				else
				{
					// Try to connect
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Save OSC Configuration clicked, save information for module.");
#endif
					readChannelPathConfig();
					this->oscConfigurationScreen->errorMsg = "";
					thisModule->oscNewSettings.oscTxIpAddress = this->oscConfigurationScreen->tbIpAddress->text.c_str();
					thisModule->oscNewSettings.oscTxPort = this->oscConfigurationScreen->getTxPort();
					thisModule->oscNewSettings.oscRxPort = this->oscConfigurationScreen->getRxPort();
					//thisModule->oscCurrentClient = this->oscConfigurationScreen->getSelectedClient();
					//debug("Setting namespace");
					thisModule->setOscNamespace(this->oscConfigurationScreen->tbNamespace->text);
					thisModule->oscCurrentAction = oscCV::OSCAction::Enable;
				}
			} // end if enable osc
			else
			{
				// Disable OSC ------------------------------------------------------------------
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Disable OSC clicked.");
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
			//this->oscConfigurationScreen->statusMsg = thisModule->currentOSCSettings.oscTxIpAddress;
			//this->oscConfigurationScreen->statusMsg2 = ":" + std::to_string(thisModule->currentOSCSettings.oscTxPort)
			//	+ " :" + std::to_string(thisModule->currentOSCSettings.oscRxPort);
			this->oscConfigurationScreen->btnActionEnable = false;
		}
		else
		{
			this->oscConfigurationScreen->successMsg = "";
			//this->oscConfigurationScreen->statusMsg = "OSC Not Connected";
			//this->oscConfigurationScreen->statusMsg2 = "";
			this->oscConfigurationScreen->statusMsg2 = "OSC Not Connected";
			this->oscConfigurationScreen->btnActionEnable = true;
		}
	} // end if show OSC config screen

	ModuleWidget::step();
	return;
}


// Show or hide the channel configuration
void oscCVWidget::toggleChannelPathConfig(bool show)
{
	for (int i = 0; i < this->numberChannels; i++)
	{
		this->tbOscInputPaths[i]->visible = show;
		this->tbOscOutputPaths[i]->visible = show;
#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		btnDrawInputAdvChConfig[i]->setVisible(show);
		btnDrawOutputAdvChConfig[i]->setVisible(show);
#endif
	}
	return;
}
// Read the channel path configs and store in module's channels.
void oscCVWidget::readChannelPathConfig()
{
	if (module != NULL)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(module);
		try
		{
			for (int i = 0; i < this->numberChannels; i++)
			{
				thisModule->inputChannels[i].setPath(this->tbOscInputPaths[i]->text);
				thisModule->outputChannels[i].setPath(this->tbOscOutputPaths[i]->text);
			}
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	}
	return;
} // end readChannelPathConfig()
// Set the channel path text boxes.
void oscCVWidget::setChannelPathConfig() {
	if (module != NULL)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(module);
		try
		{
			for (int i = 0; i < this->numberChannels; i++)
			{
				this->tbOscInputPaths[i]->text = thisModule->inputChannels[i].getPath();
				this->tbOscOutputPaths[i]->text = thisModule->outputChannels[i].getPath();
			}
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	}
	return;
} // end setChannelPathConfig()



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @vg : (IN) NVGcontext to draw on
// Draw labels on our widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVLabels::draw(/*in*/ NVGcontext *vg) {
	// Default Font:
	nvgFontSize(vg, fontSize);
	nvgFontFaceId(vg, font->handle);
	nvgTextLetterSpacing(vg, 1);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(vg, textColor);
	nvgFontSize(vg, fontSize);

	int x, y, dx;// , dy;
	int xStart, yStart;
	xStart = 0;
	yStart = 25; // 17
	dx = 28;
	//dy = 32;

	//-- * Top Buttons *--//
	x = 84;
	y = 18;
	nvgTextAlign(vg, NVG_ALIGN_LEFT);
	nvgText(vg, x, y, "CONFIG", NULL);


	//--- * Inputs *---//
	// (Left hand side)
	// TRIG:
	x = xStart + dx / 2;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "VAL", NULL);
	// Bottom (INPUTS)
	x = xStart + dx;
	y = box.size.y - 13;
	nvgText(vg, x, y, "INPUTS", NULL);

	//--- * Outputs *---//
	// (Right hand side)
	// TRIG:
	x = box.size.x - dx / 2 - dx;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "VAL", NULL);
	// Bottom (INPUTS)
	x = box.size.x - dx;
	y = box.size.y - 13;
	nvgText(vg, x, y, "OUTPUTS", NULL);
} // end TSOscCVLabels::draw()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step()
// Calculate scrolling and stuff?
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVTopDisplay::step() {
	//debug("Top Display: step(%8.6f)", dt);

	bool connected = false;
	bool isPreview = parentWidget->module == NULL;
	std::string thisIp = std::string("NO CONNECTION ");
	oscCV* thisModule = NULL;
	if (!isPreview)
	{
		thisModule = dynamic_cast<oscCV*>(parentWidget->module);
		connected = thisModule->oscInitialized;
		if (connected)
			thisIp = thisModule->currentOSCSettings.oscTxIpAddress
			+ std::string(" Tx:") + std::to_string(thisModule->currentOSCSettings.oscTxPort)
			+ std::string(" Rx:") + std::to_string(thisModule->currentOSCSettings.oscRxPort)
			+ ((thisModule->oscNamespace.at(0) == '/') ? " " : " /") + thisModule->oscNamespace + " ";
	}



	if (thisIp.compare(lastIp) != 0)
	{
		sprintf(scrollingMsg, "trowaSoft - %s - cv<->OSC<->cv - ", thisIp.c_str());
	}

	//dt += engineGetSampleTime() / scrollTime_sec;
	//if (dt > 1.0f)
	dt += 100.0 / engineGetSampleRate();
	if (dt > scrollTime_sec) 
	{
		//debug("Dt has grown. Increment Ix: %d", scrollIx);
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
// @vg : (IN) NVGcontext to draw on
// Top display
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVTopDisplay::draw(/*in*/ NVGcontext *vg)
{
	//bool isPreview = parentWidget->module == NULL; // May get a NULL module for preview

	// Background Colors:
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

	// Screen:
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);

	if (!showDisplay)
		return;

	Rect b = Rect(Vec(0, 0), Vec(box.size.x - 13, box.size.y));
	nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

	//oscCV* thisModule = NULL;
	//if (!isPreview)
	//	thisModule = dynamic_cast<oscCV*>(parentWidget->module);

	int x, y;

	// Default Font:
	nvgFontSize(vg, fontSize);
	nvgFontFaceId(vg, font->handle);
	nvgTextLetterSpacing(vg, 2.5);
	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(vg, textColor);

	// SCROLLING MESSAGE ====================================
	x = 13;// -scrollIx * 0.5;
	y = 13;
	nvgFontSize(vg, fontSize * 1.5);	// Large font
	//nvgFontFaceId(vg, font->handle);
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

	// Start (left on screen) of scrolling message:
	const char * subStr = scrollingMsg + scrollIx;
	nvgText(vg, x, y, subStr, NULL);
	// Get circular wrap (right part of screen) - start of message again:
	float txtBounds[4] = { 0,0,0,0 };
	float nextX = nvgTextBounds(vg, x, y, subStr, NULL, txtBounds);
	x = nextX; // +24
	if (x < b.size.x) {
		// Wrap the start of the string around
		nvgText(vg, x, y, scrollingMsg, subStr);
	}
	//// Measures the specified text string. Parameter bounds should be a pointer to float[4],
	//// if the bounding box of the text should be returned. The bounds value are [xmin,ymin, xmax,ymax]
	//// Returns the horizontal advance of the measured text (i.e. where the next character should drawn).
	//// Measured values are returned in local coordinate space.
	//float nvgTextBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds);
	nvgResetScissor(vg);

	//const char* asciiArt[] = {
	//	"♫♪.ılılıll|̲̅̅●̲̅̅|̲̅̅=̲̅̅|̲̅̅●̲̅̅|llılılı.♫♪",
	//	"°º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸"
	//}


	// Always display connected 

	return;
} // end TSOscCVTopDisplay::draw()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Process
// Middle display -- If we have to do scrolling, calculate it.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::step() {
	if (displayMode == DisplayMode::Default) {
		dt += 100.0 / engineGetSampleRate();
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
// @vg : (IN) NVGcontext to draw on
// Middle display drawing.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::draw(/*in*/ NVGcontext *vg) {
	bool isPreview = parentWidget->module == NULL; // May get a NULL module for preview

	// Background Colors:
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

	// Screen:
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);

	if (!displayMode)
		return;

	if (!isPreview)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(parentWidget->module);
		// Default Font:
		nvgFontSize(vg, 9);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);
		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(vg, textColor);

		int dy = 2;
		int dx = 4;
		int height = 28;
		int width = box.size.x / 2.0 - 4;
		int y = 2;
		float txtBounds[4] = { 0,0,0,0 };
		const int txtPadding = 4;
		int txtWidth = width - txtPadding;
		bool drawBoxes = false;
		for (int c = 0; c < thisModule->numberChannels; c++) {
			int ix = 0;
			float nextX;

			//---- INPUT ----
			int x = 2;
			drawChannelChart(vg, &(thisModule->inputChannels[c]), x, y, width, height, oscCVWidget::CHANNEL_COLORS[c]);
			if (drawBoxes) {
				// Debug draw box:
				nvgBeginPath(vg);
				nvgRect(vg, x, y, width, height);
				nvgStrokeColor(vg, oscCVWidget::CHANNEL_COLORS[c]);
				nvgStrokeWidth(vg, 1.0);
				nvgStroke(vg);
			}

			// Label:
			nextX = nvgTextBounds(vg, x, y, thisModule->inputChannels[c].path.c_str(), NULL, txtBounds);
			ix = 0;
			if (nextX > txtWidth) {
				ix = chPathPosition * thisModule->inputChannels[c].path.length();
			}
			nvgScissor(vg, x, y, txtWidth, height);
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgText(vg, x, y + 1, &(thisModule->inputChannels[c].path.c_str()[ix]), NULL);
			nvgResetScissor(vg);

			//---- OUTPUT ----
			x += dx + width;
			drawChannelChart(vg, &(thisModule->outputChannels[c]), x, y, width, height, oscCVWidget::CHANNEL_COLORS[c]);
			if (drawBoxes) {
				// Debug draw box:
				nvgBeginPath(vg);
				nvgRect(vg, x, y, width, height);
				nvgStrokeColor(vg, oscCVWidget::CHANNEL_COLORS[thisModule->numberChannels - c - 1]);
				nvgStrokeWidth(vg, 1.0);
				nvgStroke(vg);
			}

			// Label:
			nextX = nvgTextBounds(vg, x, y, thisModule->outputChannels[c].path.c_str(), NULL, txtBounds);
			ix = thisModule->outputChannels[c].path.length();
			if (nextX > txtWidth) {
				ix = thisModule->outputChannels[c].path.length() - chPathPosition * thisModule->outputChannels[c].path.length();
			}
			nvgScissor(vg, x + txtPadding, y, txtWidth, height);
			nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
			nvgText(vg, x + width, y + 1, thisModule->outputChannels[c].path.c_str(), &(thisModule->outputChannels[c].path.c_str()[ix]));
			nvgResetScissor(vg);

			y += dy + height;
		}
	} // end if we actually have a module
	return;
} // end draw()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawChannelChart()
// Draw the channel data.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawChannelChart(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,  
	/*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height,
	/*in*/ NVGcolor lineColor)
{
	nvgScissor(vg, x, y, width, height);
	nvgBeginPath(vg);
	float dx = static_cast<float>(width) / TROWA_OSCCV_VAL_BUFFER_SIZE;
	//if (dx < 0.5)
	//	dx = 0.5;
	float px = x;
	nvgMoveTo(vg, x, y);
	for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
	{
		float py = y + rescale(channelData->valBuffer[i], channelData->minVoltage, channelData->maxVoltage, 0, height);
		nvgLineTo(vg, px, py);
		px += dx;
	}
	nvgStrokeColor(vg, lineColor);
	nvgStrokeWidth(vg, 1.0);
	nvgStroke(vg);

	nvgResetScissor(vg);
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawChannelChart()
// Draw the channel data.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawChannelBar(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,
	/*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height,
	/*in*/ NVGcolor lineColor)
{
	nvgScissor(vg, x, y, width, height);
	nvgBeginPath(vg);
	float dx = static_cast<float>(width) / TROWA_OSCCV_VAL_BUFFER_SIZE;
	//if (dx < 0.5)
	//	dx = 0.5;
	float px = x;
	nvgMoveTo(vg, x, y);
	for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
	{
		float py = y + rescale(channelData->valBuffer[i], channelData->minVoltage, channelData->maxVoltage, 0, height);
		nvgLineTo(vg, px, py);
		px += dx;
	}
	nvgStrokeColor(vg, lineColor);
	nvgStrokeWidth(vg, 1.0);
	nvgStroke(vg);

	nvgResetScissor(vg);
	return;
}

void TSOscCVDataTypeSelectBtn::onAction(EventAction &e) {
	if (visible)
	{
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;
		for (int i = 0; i < numVals; i++) {
			TSOscCVDataTypeItem *option = new TSOscCVDataTypeItem(this, i);
			option->itemVal = itemVals[i];
			option->text = itemStrs[i];
			menu->addChild(option);
		}
	}
}
// Draw if visible
void TSOscCVDataTypeSelectBtn::draw(NVGcontext *vg) {
	if (visible) {
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);

		// Background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);

		// Border
		if (borderWidth > 0) {
			nvgStrokeWidth(vg, borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStroke(vg);
		}
		nvgResetScissor(vg);

		if (font->handle >= 0) {
			nvgScissor(vg, 0, 0, box.size.x - 5, box.size.y);

			nvgFillColor(vg, color);
			nvgFontFaceId(vg, font->handle);
			nvgTextLetterSpacing(vg, 0.0);

			nvgFontSize(vg, fontSize);
			nvgText(vg, textOffset.x, textOffset.y, text.c_str(), NULL);

			nvgResetScissor(vg);

			bndUpDownArrow(vg, box.size.x - 10, 10, 5, color);
		}
	}
	return;
}

