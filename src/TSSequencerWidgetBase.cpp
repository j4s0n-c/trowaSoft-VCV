#include <string.h>
#include <stdio.h>
//#include "widgets.hpp"
#include "trowaSoft.hpp"
using namespace rack;
//#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "TSSequencerWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerWidgetBase() - Base constructor.
// Instantiate a trowaSoft Sequencer widget. v0.60 must have module as param.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSSequencerWidgetBase::TSSequencerWidgetBase(TSSequencerModuleBase* seqModule) : TSSModuleWidgetBase(seqModule, /*randomizeParameters*/ false)
{
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Add common controls to the UI widget for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerWidgetBase::addBaseControls(bool addGridLines)
{
	TSSequencerModuleBase *thisModule = NULL;
	if (this->module != NULL)
	{
		thisModule = dynamic_cast<TSSequencerModuleBase*>(this->module);
	}
	bool isPreview = thisModule == NULL;
		
	////////////////////////////////////
	// DISPLAY
	////////////////////////////////////
	{
		display = new TSSeqDisplay();
		display->box.pos = Vec(13, 24);
		display->box.size = Vec(363, 48);
		display->module = thisModule;
		addChild(display);
	}
	
	
	////////////////////////////////////
	// OSC configuration screen.
	// Should be a popup but J just wants it on the screen.
	////////////////////////////////////
	if (!isPreview)
	{		
		TSOSCConfigWidget* oscConfig = new TSOSCConfigWidget(thisModule, TSSequencerModuleBase::ParamIds::OSC_SAVE_CONF_PARAM, TSSequencerModuleBase::ParamIds::OSC_AUTO_RECONNECT_PARAM,
			thisModule->oscCurrentClient,
			thisModule->currentOSCSettings.oscTxIpAddress.c_str(), thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort);
		oscConfig->setVisible(false);
		oscConfig->box.pos = display->box.pos;
		oscConfig->box.size = display->box.size;
		this->oscConfigurationScreen = oscConfig;
		addChild(oscConfig);
	}
	
	////////////////////////////////////
	// Pattern Sequencer Configuration / Auto-Play Patterns
	////////////////////////////////////	
	if (!isPreview && thisModule->allowPatternSequencing)
	{
		pattSeqConfigurationScreen = new TSSeqPatternSeqConfigWidget(thisModule);
		pattSeqConfigurationScreen->setVisible(false);
		pattSeqConfigurationScreen->box.pos = display->box.pos;
		pattSeqConfigurationScreen->box.size = display->box.size;		
		addChild(pattSeqConfigurationScreen);		
	}
	

	////////////////////////////////////
	// Labels
	////////////////////////////////////
	{
		FramebufferWidget* labelContainer = new FramebufferWidget();
		labelContainer->box.pos = Vec(0, 0);
		labelContainer->box.size = Vec(box.size.x, 380);
		addChild(labelContainer);
		
		labelArea = new TSSeqLabelArea();
		labelArea->box.pos = Vec(0, 0);
		labelArea->box.size = Vec(box.size.x, 380);
		labelArea->module = thisModule;
		labelArea->drawGridLines = addGridLines;
		labelContainer->addChild(labelArea);
		//addChild(labelArea);
	}
	// Screws:
	addChild(createWidget<ScrewBlack>(Vec(0, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 15, 0)));
	addChild(createWidget<ScrewBlack>(Vec(0, box.size.y - 15)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 15, box.size.y - 15)));

	// Input Controls ==================================================	
	// Run (Toggle)
	NVGcolor lightColor = nvgRGBAf(0.7, 0.7, 0.7, 0.8); //TSColors::COLOR_WHITE
	Vec btnSize = Vec(50,22);
	addParam(createParam<TS_PadBtn>(Vec(15, 320), thisModule, TSSequencerModuleBase::ParamIds::RUN_PARAM));//, 0.0, 1.0, 0.0));
	TS_LightString* item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 320), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::LightIds::RUNNING_LIGHT,
		/* size */ btnSize, /* color */ lightColor));
	item->lightString = "RUN";
	addChild(item);
	
	// Reset (Momentary)
	addParam(createParam<TS_PadBtn>(Vec(15, 292), thisModule, TSSequencerModuleBase::ParamIds::RESET_PARAM));//, 0.0, 1.0, 0.0));
	item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 292), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::LightIds::RESET_LIGHT,	
		/* size */ btnSize, /* color */ lightColor));
	item->lightString = "RESET";
	addChild(item);

	// Paste button:
	addParam(createParam<TS_PadBtn>(Vec(15, 115), thisModule, TSSequencerModuleBase::ParamIds::PASTE_PARAM));//, 0.0, 1.0, 0.0));
	item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 115), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::LightIds::PASTE_LIGHT,
		/* size */ btnSize, /* color */ lightColor));
	item->lightString = "PASTE";
	// if (!isPreview)
		// thisModule->pasteLight = item;
	pasteLight = item;
	addChild(item);
		
	// Top Knobs : Keep references for later
	int knobRow = 79;
	int knobStart = 27;
	int knobSpacing = 61;

	TS_RoundBlackKnob* outKnobPtr = NULL;
	
	// Pattern Playback Select  (Knob)
	outKnobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(knobStart, knobRow), thisModule, TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_PLAY_PARAM));//, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ 0.0));
	if (!isPreview)
		outKnobPtr->getParamQuantity()->randomizeEnabled = false; // outKnobPtr->allowRandomize = false;
	outKnobPtr->snap = true;	// [Rack v2]: Knob::snap doesn't seem to work anymore. Use ParamQuantity::snapEnabled.
	addParam(outKnobPtr);
	
	// Clock BPM (Knob)
	outKnobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(knobStart + (knobSpacing * 1), knobRow), thisModule, TSSequencerModuleBase::ParamIds::BPM_PARAM));//, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX, (TROWA_SEQ_BPM_KNOB_MAX + TROWA_SEQ_BPM_KNOB_MIN) / 2));
	if (!isPreview)
		outKnobPtr->getParamQuantity()->randomizeEnabled = false; // outKnobPtr->allowRandomize = false;	
	addParam(outKnobPtr);
	
	// Steps (Knob)
	outKnobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(knobStart + (knobSpacing * 2), knobRow), thisModule, TSSequencerModuleBase::ParamIds::STEPS_PARAM));//, 1.0, this->maxSteps, this->maxSteps));
	if (!isPreview)
		outKnobPtr->getParamQuantity()->randomizeEnabled = false; // outKnobPtr->allowRandomize = false;
	outKnobPtr->snap = true;	// [Rack v2]: Knob::snap doesn't seem to work anymore. Use ParamQuantity::snapEnabled.
	addParam(outKnobPtr);

	// Output Mode (Knob)
	outKnobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(knobStart + (knobSpacing * 3), knobRow), thisModule, 
		TSSequencerModuleBase::ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM));//, 0, TROWA_SEQ_NUM_MODES - 1, TSSequencerModuleBase::ValueMode::VALUE_TRIGGER));
	if (!isPreview)
		outKnobPtr->getParamQuantity()->randomizeEnabled = false; // outKnobPtr->allowRandomize = false;
	outKnobPtr->minAngle = -0.6*M_PI;
	outKnobPtr->maxAngle = 0.6*M_PI;
	outKnobPtr->snap = true;	// [Rack v2]: Knob::snap doesn't seem to work anymore. Use ParamQuantity::snapEnabled.
	addParam(outKnobPtr);
	
	// Pattern Edit Select (Knob)
	outKnobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(knobStart + (knobSpacing * 4), knobRow), thisModule, TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_EDIT_PARAM));//, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ 0));
	if (!isPreview)
		outKnobPtr->getParamQuantity()->randomizeEnabled = false; // outKnobPtr->allowRandomize = false;
	outKnobPtr->snap = true;	// [Rack v2]: Knob::snap doesn't seem to work anymore. Use ParamQuantity::snapEnabled.
	addParam(outKnobPtr);
	
	// Selected Gate/Voice/Channel (Knob)
	outKnobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(knobStart + (knobSpacing * 5), knobRow), thisModule, TSSequencerModuleBase::ParamIds::SELECTED_CHANNEL_PARAM));//, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_CHNLS - 1, /*default value*/ 0));
	if (!isPreview)
		outKnobPtr->getParamQuantity()->randomizeEnabled = false; // outKnobPtr->allowRandomize = false;
	outKnobPtr->snap = true;	// [Rack v2]: Knob::snap doesn't seem to work anymore. Use ParamQuantity::snapEnabled.
	addParam(outKnobPtr);


	//=======================================================
	// Small buttons next to knobs
	//=======================================================		
	Vec ledSize = Vec(15,15);
	int dx = 28;
	float xLightOffset = 1.5f;
	float yLightOffset = 1.5f;	
	TS_LEDButton* btn;
	knobStart += 2;	
	int y = knobRow;
	int x = knobStart;
	Vec ledBtnSize = Vec(ledSize.x - 2, ledSize.y - 2);	
	
	//--------------------------------------------------------------------------
	// PATTERN SEQ: Pattern Sequencing Configuration (+Config/Enabled Lights):
	//--------------------------------------------------------------------------
	x = knobStart + dx; // 30
	if (!isPreview && thisModule->allowPatternSequencing)
	{
		btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(x, y), module, TSSequencerModuleBase::ParamIds::PATTERN_SEQ_SHOW_CONFIG_PARAM));
		btn->setSize(ledBtnSize);
		addParam(btn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + xLightOffset, y + yLightOffset), module, TSSequencerModuleBase::LightIds::PATTERN_SEQ_CONFIGURE_LIGHT, ledSize, TSColors::COLOR_WHITE));
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + xLightOffset + 2, y + yLightOffset + 2), module, TSSequencerModuleBase::LightIds::PATTERN_SEQ_ENABLED_LIGHT, Vec(ledSize.x - 4, ledSize.y - 4), TS_PATTERN_SEQ_STATUS_COLOR));
	}
	
	//-------------------------------------------------------
	// OSC: Configuration Button (+Config/Enabled Lights):
	//-------------------------------------------------------
	x = knobStart + (knobSpacing * 3) + dx; // 30
	if (isPreview || thisModule->allowOSC)
	{
		btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(x, y), module, TSSequencerModuleBase::ParamIds::OSC_SHOW_CONF_PARAM));//, 0, 1, 0));
		btn->setSize(ledBtnSize);
		addParam(btn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + xLightOffset, y + yLightOffset), module, TSSequencerModuleBase::LightIds::OSC_CONFIGURE_LIGHT, ledSize, TSColors::COLOR_WHITE));
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + xLightOffset + 2, y + yLightOffset + 2), module, TSSequencerModuleBase::LightIds::OSC_ENABLED_LIGHT, Vec(ledSize.x - 4, ledSize.y - 4), TSOSC_STATUS_COLOR));
	}
	
	
	

	ColorValueLight* lightPtr = NULL;

	//-------------------------------------------------------
	// COPY: Pattern Copy button:
	//-------------------------------------------------------
	btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(knobStart + (knobSpacing * 4) + dx, knobRow), module, TSSequencerModuleBase::ParamIds::COPY_PATTERN_PARAM));//, 0, 1, 0));
	btn->setSize(ledSize);
	addParam(btn);
	lightPtr = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 4) + dx + xLightOffset, knobRow + yLightOffset), module, TSSequencerModuleBase::LightIds::COPY_PATTERN_LIGHT, ledSize, TSColors::COLOR_WHITE));
	copyPatternLight = lightPtr;
	addChild(lightPtr);

	//-------------------------------------------------------
	// COPY: Channel Copy button:
	//-------------------------------------------------------
	btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(knobStart + (knobSpacing * 5) + dx, knobRow), module, TSSequencerModuleBase::ParamIds::COPY_CHANNEL_PARAM));//, 0, 1, 0));
	btn->setSize(ledSize);
	addParam(btn);
	lightPtr = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 5) + dx + xLightOffset, knobRow + yLightOffset), module, TSSequencerModuleBase::LightIds::COPY_CHANNEL_LIGHT, ledSize, TSColors::COLOR_WHITE));
	copyChannelLight = lightPtr;	
	addChild(lightPtr);

	//-------------------------------------------------------
	// CHANGE BPM CALC NOTE (1/4, 1/8, 1/8T, 1/16)
	//-------------------------------------------------------
	btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(knobStart + (knobSpacing * 1) + dx, knobRow), module, TSSequencerModuleBase::ParamIds::SELECTED_BPM_MULT_IX_PARAM));//, 0, 1, 0));
	btn->setSize(ledSize);
	addParam(btn);
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 1) + dx + xLightOffset, knobRow + yLightOffset), module, TSSequencerModuleBase::LightIds::SELECTED_BPM_MULT_IX_LIGHT, ledSize, TSColors::COLOR_WHITE));
	
	// Input Jacks:
	int xStart = 10;
	int ySpacing = 28;
	int portStart = 143;
	
	// Selected Pattern Playback:
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 0)), thisModule, TSSequencerModuleBase::InputIds::SELECTED_PATTERN_PLAY_INPUT));
	
	// Clock
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 1)), thisModule, TSSequencerModuleBase::InputIds::BPM_INPUT));
	
	// Steps
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 2)), thisModule, TSSequencerModuleBase::InputIds::STEPS_INPUT));
	
	// External Clock
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 3)), thisModule, TSSequencerModuleBase::InputIds::EXT_CLOCK_INPUT));
	
	// Reset 
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 4)), thisModule, TSSequencerModuleBase::InputIds::RESET_INPUT));

	// Outputs ==================================================
	// Loop through each channel/voice/gate
	y = 115;
	x = 314;
	int v = 0;
	
	float jackDiameter = 20.5; // 28.351
	float add = 0;
	Vec outputLightSize = Vec(jackDiameter + add, jackDiameter + add);

	const NVGcolor* channelColors = NULL;
	if (!isPreview)
	{
		channelColors = thisModule->voiceColors;
	}
	else
	{
		channelColors = TSColors::CHANNEL_COLORS; // Point to our default array
	}
	
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 2; c++)
		{
			// Triggers / Gates / Output:
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), thisModule, TSSequencerModuleBase::OutputIds::CHANNELS_OUTPUT+v, /*color*/ channelColors[v]));
			// Match the color to the trigger/gate/output:
			addChild(TS_createColorValueLight<TS_LightRing>(/*position*/ Vec(x + 5, y + 5), 
				/*thisModule*/ thisModule, 
				/*lightId*/ TSSequencerModuleBase::LightIds::CHANNEL_LIGHTS+v,
				/*size*/ outputLightSize, /*lightColor*/ channelColors[v % 16], /*backColor*/ channelColors[v % 16]));
			if (!isPreview)
				thisModule->lights[TSSequencerModuleBase::LightIds::CHANNEL_LIGHTS + v].value = 0;
			x += 36;
			v++;
		} // end for
		y += 28; // Next row
		x = 314;
	} // end loop through NxM grid
	return;
} // end addBaseControls()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerWidgetBase::step()
{
	if (this->module == NULL)
		return;

	TSSequencerModuleBase* thisModule = dynamic_cast<TSSequencerModuleBase*>(module);
	
	//------------------------------------
	// Step Matrix Color
	//------------------------------------
	if (padLightPtrs && colorRGBNotEqual(lastStepMatrixColor, thisModule->currentStepMatrixColor))
	{
		for (int r = 0; r < numRows; r++)
		{
			for (int c = 0; c < numCols; c++)
			{
				padLightPtrs[r][c]->setColor(thisModule->currentStepMatrixColor);
			}
		}
	}
	lastStepMatrixColor = thisModule->currentStepMatrixColor;
	
	//------------------------------------
	// Copy & Paste
	//------------------------------------
	// Change colors of our lights here
	if (colorRGBNotEqual(thisModule->currentCopyChannelColor, copyChannelLight->color))
	{
		copyChannelLight->setColor(thisModule->currentCopyChannelColor);
	}
	if (colorRGBNotEqual(thisModule->currentPasteColor, pasteLight->color))
	{
		pasteLight->setColor(thisModule->currentPasteColor);
	}

	//------------------------------------
	// Pattern Sequencing
	//------------------------------------	
	if (thisModule->allowPatternSequencing)
	{
		if (pattSeqConfigurationScreen != NULL)				
		{
			thisModule->lastShowPatternSequencingConfig = thisModule->showPatternSequencingConfig;			
			if (thisModule->patternSeqConfigTrigger.process(thisModule->params[TSSequencerModuleBase::ParamIds::PATTERN_SEQ_SHOW_CONFIG_PARAM].getValue()))
			{
				thisModule->showPatternSequencingConfig = !thisModule->showPatternSequencingConfig;
				// Set the Configuration Visible			
				if (thisModule->showPatternSequencingConfig)
				{
					// SHOW Pattern Config
					this->display->showDisplay = false;
					pattSeqConfigurationScreen->setVisible(true);
					pattSeqConfigurationScreen->ckEnabled->checked = thisModule->patternSequencingOn;
					// Hide OSC Config				
					thisModule->oscShowConfigurationScreen = false;
					this->oscConfigurationScreen->setVisible(false);
					thisModule->lights[TSSequencerModuleBase::LightIds::OSC_CONFIGURE_LIGHT].value = 0.0f;				
				}
				else
				{
					// Hide Pattern Config
					this->display->showDisplay = !thisModule->oscShowConfigurationScreen;
					pattSeqConfigurationScreen->setVisible(false);	
				}
			} // end if show config button pressed.			
		}
	} // end if we are doing pattern sequencing
	
	//------------------------------------
	// OSC
	//------------------------------------	
	if (thisModule->oscConfigTrigger.process(thisModule->params[TSSequencerModuleBase::ParamIds::OSC_SHOW_CONF_PARAM].getValue()))
	{
		thisModule->oscShowConfigurationScreen = !thisModule->oscShowConfigurationScreen;
		thisModule->lights[TSSequencerModuleBase::LightIds::OSC_CONFIGURE_LIGHT].value = (thisModule->oscShowConfigurationScreen) ? 1.0 : 0.0;
		this->oscConfigurationScreen->setVisible(thisModule->oscShowConfigurationScreen);
		this->display->showDisplay = !thisModule->oscShowConfigurationScreen;
		if (thisModule->oscShowConfigurationScreen)
		{
			thisModule->showPatternSequencingConfig = false; // Stop showing pattern configuration if showing
			if (pattSeqConfigurationScreen != NULL)
				pattSeqConfigurationScreen->setVisible(false);			
			if (!thisModule->oscInitialized)
			{
				// Make sure the ports are available
				int p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscTxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscTxPort = TSOSCConnector::GetAvailablePortTrans(thisModule->oscId, thisModule->currentOSCSettings.oscTxPort);
				p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscRxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscRxPort = TSOSCConnector::GetAvailablePortRecv(thisModule->oscId, thisModule->currentOSCSettings.oscRxPort);

			}
			this->oscConfigurationScreen->setValues(thisModule->currentOSCSettings.oscTxIpAddress, thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort);
			this->oscConfigurationScreen->ckAutoReconnect->checked = thisModule->oscReconnectAtLoad;
			this->oscConfigurationScreen->setSelectedClient(thisModule->oscCurrentClient); // OSC Client
			this->oscConfigurationScreen->btnActionEnable = !thisModule->oscInitialized;

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
		}
	}
	if (thisModule->oscShowConfigurationScreen)
	{
		// Check for enable/disable
		if (thisModule->oscConnectTrigger.process(thisModule->params[TSSequencerModuleBase::ParamIds::OSC_SAVE_CONF_PARAM].getValue()))
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
					this->oscConfigurationScreen->errorMsg = "";
					thisModule->oscNewSettings.oscTxIpAddress = this->oscConfigurationScreen->tbIpAddress->text.c_str();
					thisModule->oscNewSettings.oscTxPort = this->oscConfigurationScreen->getTxPort();
					thisModule->oscNewSettings.oscRxPort = this->oscConfigurationScreen->getRxPort(); 
					thisModule->oscCurrentClient = this->oscConfigurationScreen->getSelectedClient();
					thisModule->oscCurrentAction = TSSequencerModuleBase::OSCAction::Enable;
					thisModule->oscReconnectAtLoad = this->oscConfigurationScreen->ckAutoReconnect->checked;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("Set osc current action to %d.", thisModule->oscCurrentAction);
#endif					
				}
			} // end if enable osc
			else
			{
				// Disable OSC ------------------------------------------------------------------
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Disable OSC clicked.");
#endif
				this->oscConfigurationScreen->errorMsg = "";
				thisModule->oscCurrentAction = TSSequencerModuleBase::OSCAction::Disable;
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
		if (thisModule->useOSC && thisModule->oscInitialized)
		{ // Now we have a checkbox up top where statusMsg used to be, so let's not use it for now.
			//this->oscConfigurationScreen->statusMsg = OSCClientAbbr[thisModule->oscCurrentClient] + " " + thisModule->currentOSCSettings.oscTxIpAddress;
			//this->oscConfigurationScreen->statusMsg2 = ":" + std::to_string(thisModule->currentOSCSettings.oscTxPort)
			//	+ " :" + std::to_string(thisModule->currentOSCSettings.oscRxPort);
			this->oscConfigurationScreen->statusMsg2 = OSCClientAbbr[thisModule->oscCurrentClient] + " " + thisModule->currentOSCSettings.oscTxIpAddress;
			this->oscConfigurationScreen->btnActionEnable = false;
		}
		else
		{
			this->oscConfigurationScreen->successMsg = "";
			this->oscConfigurationScreen->statusMsg = "";// "OSC Not Connected";
			this->oscConfigurationScreen->statusMsg2 = "OSC Not Connected"; //"";
			this->oscConfigurationScreen->btnActionEnable = true;
		}
	} // end if show OSC config screen

	ModuleWidget::step();
	return;
} // end step()

struct seqRandomSubMenuItem : MenuItem {
	TSSequencerModuleBase* sequencerModule;
	bool useStucturedRandom;
	enum ShiftType {
		// Current Edit Pattern & Channel
		CurrentChannelOnly,
		// Current Edit Pattern, All Channels
		ThisPattern,
		// All patterns, all channels
		AllPatterns,
		// Song mode / Pattern sequence.
		SongMode		
	};
	ShiftType Target = ShiftType::CurrentChannelOnly;

	seqRandomSubMenuItem(std::string text, ShiftType target, bool useStructured, TSSequencerModuleBase* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->Target = target;
		this->useStucturedRandom = useStructured;
		this->sequencerModule = seqModule;
	}
	~seqRandomSubMenuItem()
	{
		sequencerModule = NULL;
	}

	void onAction(const event::Action &e) override 
	{
		switch (this->Target)
		{
			case ShiftType::CurrentChannelOnly:
				sequencerModule->randomize(sequencerModule->currentPatternEditingIx, sequencerModule->currentChannelEditingIx, useStucturedRandom);
				break;			
			case ShiftType::ThisPattern:
				sequencerModule->randomize(sequencerModule->currentPatternEditingIx, TROWA_SEQ_COPY_CHANNELIX_ALL, useStucturedRandom);
				break;
			case ShiftType::SongMode:
				// PATTERN SEQUENCE 
				sequencerModule->randomizePatternSequence(useStucturedRandom);
				break;
			default:
				// All steps
				sequencerModule->randomize(TROWA_INDEX_UNDEFINED, TROWA_SEQ_COPY_CHANNELIX_ALL, useStucturedRandom);
				break;
		}		
	}
};

struct seqRandomSubMenu : Menu {
	TSSequencerModuleBase* sequencerModule;
	bool useStucturedRandom;

	seqRandomSubMenu(bool useStructured, TSSequencerModuleBase* seqModule)
	{
		this->box.size = Vec(200, 60);
		this->useStucturedRandom = useStructured;
		this->sequencerModule = seqModule;
		return;
	}
	~seqRandomSubMenu()
	{
		sequencerModule = NULL;
	}

	void createChildren()
	{
		addChild(new seqRandomSubMenuItem("Current Edit Channel", seqRandomSubMenuItem::ShiftType::CurrentChannelOnly, this->useStucturedRandom, this->sequencerModule));
		addChild(new seqRandomSubMenuItem("Current Edit Pattern", seqRandomSubMenuItem::ShiftType::ThisPattern, this->useStucturedRandom, this->sequencerModule));
		addChild(new seqRandomSubMenuItem("ALL Patterns", seqRandomSubMenuItem::ShiftType::AllPatterns, this->useStucturedRandom, this->sequencerModule));
		if (sequencerModule->allowPatternSequencing)
			addChild(new seqRandomSubMenuItem("Song Mode", seqRandomSubMenuItem::ShiftType::SongMode, this->useStucturedRandom, this->sequencerModule));		
		return;
	}
};
// First tier menu item. Create Submenu
struct seqRandomMenuItem : MenuItem {
	TSSequencerModuleBase* sequencerModule;
	bool useStucturedRandom;

	seqRandomMenuItem(std::string text, bool useStructured, TSSequencerModuleBase* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->useStucturedRandom = useStructured;
		this->sequencerModule = seqModule;
		return;
	}
	~seqRandomMenuItem() {
		sequencerModule = NULL;
		return;
	}
	Menu *createChildMenu() override {
		seqRandomSubMenu* menu = new seqRandomSubMenu(useStucturedRandom, sequencerModule);
		menu->sequencerModule = this->sequencerModule;
		menu->createChildren();
		menu->box.size = Vec(200, 60);
		return menu;
	}
};


// Initialize menu items.
struct seqInitializeMenuItem : MenuItem {
	TSSequencerModuleBase* sequencerModule;
	enum InitType {
		// Current Edit Pattern & Channel
		CurrentChannelOnly,
		// Current Edit Pattern, All Channels
		ThisPattern,
		// All patterns, all channels
		AllPatterns,
		// Song mode / Pattern sequence.
		SongMode
	};
	InitType Target = InitType::CurrentChannelOnly;

	seqInitializeMenuItem(std::string text, InitType target, TSSequencerModuleBase* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->Target = target;
		this->sequencerModule = seqModule;
	}
	~seqInitializeMenuItem()
	{
		sequencerModule = NULL;
	}

	void onAction(const event::Action &e) override 
	{
		switch (this->Target)
		{
			case InitType::ThisPattern:
				sequencerModule->reset(sequencerModule->currentPatternEditingIx, TROWA_INDEX_UNDEFINED);			
				break;
			case InitType::CurrentChannelOnly:
				sequencerModule->reset(sequencerModule->currentPatternEditingIx, sequencerModule->currentChannelEditingIx);			
				break;
			case InitType::SongMode:
				// PATTERN SEQUENCE 			
				sequencerModule->resetPatternSequence();
				break;
			default:
				// All steps
				sequencerModule->reset(TROWA_INDEX_UNDEFINED, TROWA_INDEX_UNDEFINED);				
				break;
		}
		return;
	}
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// createContextMenu()
// Create context menu with more random options for sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerWidgetBase::appendContextMenu(ui::Menu *menu)
{
	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	TSSequencerModuleBase* sequencerModule = dynamic_cast<TSSequencerModuleBase*>(module);
	MenuLabel* modeLabel = NULL;
	
	//-------- Initialize ----//
	modeLabel = new MenuLabel();
	modeLabel->text = "Initialize Options";
	menu->addChild(modeLabel);
	menu->addChild(new seqInitializeMenuItem("Current Edit Channel", seqInitializeMenuItem::InitType::CurrentChannelOnly, sequencerModule));
	menu->addChild(new seqInitializeMenuItem("Current Edit Pattern", seqInitializeMenuItem::InitType::ThisPattern, sequencerModule));
	menu->addChild(new seqInitializeMenuItem("ALL Patterns", seqInitializeMenuItem::InitType::AllPatterns, sequencerModule));	
	if (sequencerModule->allowPatternSequencing)
	{
		menu->addChild(new seqInitializeMenuItem("Song Mode", seqInitializeMenuItem::InitType::SongMode, sequencerModule));		
	}
	
	//-------- Spacer -------- //
	modeLabel = new MenuLabel();
	modeLabel->text = "";
	menu->addChild(modeLabel); 
	
	//-------- Random ------- //
	modeLabel = new MenuLabel();
	modeLabel->text = "Random Options";
	menu->addChild(modeLabel); //menu->pushChild(modeLabel);
	menu->addChild(new seqRandomMenuItem("> All Steps Random", false, sequencerModule));
	menu->addChild(new seqRandomMenuItem("> Structured Random", true, sequencerModule));
	return;
}


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqLabelArea
// Draw labels on our sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
// Draw labels on a trowaSoft sequencer.
// (Common across trigSeq, voltSeq, multiSeq).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSeqLabelArea::draw(const DrawArgs &args) 
{
	// Default Font:
	nvgFontSize(args.vg, fontSize);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 1);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(args.vg, textColor);
	nvgFontSize(args.vg, fontSize);

	/// MAKE LABELS HERE
	int x = 45;
	int y = 163;
	int dy = 28;

	// Selected Pattern Playback:
	nvgText(args.vg, x, y, "PAT", NULL);

	// Clock		
	y += dy;
	nvgText(args.vg, x, y, "BPM ", NULL);

	// Steps
	y += dy;
	nvgText(args.vg, x, y, "LNG", NULL);

	// Ext Clock 
	y += dy;
	nvgText(args.vg, x, y, "CLK", NULL);

	// Reset
	y += dy;
	nvgText(args.vg, x, y, "RST", NULL);

	// Outputs
	nvgFontSize(args.vg, fontSize * 0.95);
	x = 320;
	y = 350;
	nvgText(args.vg, x, y, "OUTPUTS", NULL);

	// TINY btn labels
	nvgFontSize(args.vg, fontSize * 0.6);
	// OSC Labels
	y = 103;
	if (module == NULL || module->allowOSC)
	{
		x = 242; //240
		nvgText(args.vg, x, y, "OSC", NULL);
	}
	// Copy button labels:
	x = 304; // 302
	nvgText(args.vg, x, y, "CPY", NULL);
	x = 364; // 364
	nvgText(args.vg, x, y, "CPY", NULL);
	// BPM divisor/note label:
	x = 120; //118
	nvgText(args.vg, x, y, "DIV", NULL);
	// Pattern sequence label
	if (allowPatternSequencing)
	{
		x = 60;
		nvgText(args.vg, x, y, "SEQ", NULL);			
	}


	if (drawGridLines)
	{
		NVGcolor gridColor = nvgRGB(0x44, 0x44, 0x44);
		nvgBeginPath(args.vg);
		x = 80;
		y = 228;
		nvgMoveTo(args.vg, /*start x*/ x, /*start y*/ y);// Starts new sub-path with specified point as first point
		x += 225;
		nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Go to the left

		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, gridColor);
		nvgStroke(args.vg);


		// Vertical
		nvgBeginPath(args.vg);
		x = 192;
		y = 116;
		nvgMoveTo(args.vg, /*start x*/ x, /*start y*/ y);// Starts new sub-path with specified point as first point
		y += 225;
		nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Go to the left			

		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, gridColor);
		nvgStroke(args.vg);
	}
	return;
} // end draw()

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqPatternSeqConfigWidget
// Pattern sequencing configuration.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqPatternSeqConfigWidget();
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSSeqPatternSeqConfigWidget::TSSeqPatternSeqConfigWidget(TSSequencerModuleBase* module)
{
	this->box.size = Vec(400, 50);	
	seqModule = module;
	font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_DIGITAL_FONT));
	labelFont = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
	fontSize = 12;
	memset(messageStr, '\0', TROWA_DISP_MSG_SIZE);	
	
	ckEnabled = new TS_ScreenCheckBox(Vec(50, 14), seqModule, TSSequencerModuleBase::ParamIds::PATTERN_SEQ_ON_PARAM, "Enable");
	ckEnabled->momentary = false;
	ckEnabled->box.pos = Vec(250, 20);
	ckEnabled->checkBoxWidth = 10;
	ckEnabled->checkBoxHeight = 10;
	ckEnabled->fontSize = 9;
	ckEnabled->borderWidth = 0;
	ckEnabled->padding = 1;	
	ckEnabled->color = TS_PATTERN_SEQ_STATUS_COLOR;
	addChild(ckEnabled);
		
	TS_KnobColored* knobPtr = dynamic_cast<TS_KnobColored*>(createParam<TS_KnobColored>(Vec(200, 15), seqModule, TSSequencerModuleBase::ParamIds::PATTERN_SEQ_LENGTH_PARAM));
	knobPtr->init(TS_KnobColored::SizeType::Small, TS_KnobColored::KnobColor::MedGray);
	knobPtr->getParamQuantity()->randomizeEnabled = false; //knobPtr->allowRandomize = false;
	knobPtr->snap = true;		
	addChild(knobPtr);	
	//parentWidget->addParam(knobPtr);
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSeqPatternSeqConfigWidget::draw(const DrawArgs &args)
{
	if (!visible || seqModule == NULL)
		return;
	
	OpaqueWidget::draw(args);
	
	// Show the current pattern value
	int currentPatternIndex = seqModule->currentPatternDataBeingEditedIx; //0-63
	int paramId = seqModule->currentPatternDataBeingEditedParamId;
	int currentPatternVal = (paramId > -1) ? seqModule->params[paramId].getValue() + 1 : -1; // 1-64	
	int currentPatternLength = seqModule->numPatternsInSequence;
	

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

	int y1 = 42;
	int y2 = 27;
	int dx = 0;
	float x = 0;
	int spacing = 61;
	
	///---------------///
	/// * Edit Line * ///
	///---------------///	
	float xl_0 = 36.0f; //41.0f;
	float xl_1 = xl_0 + spacing * 1.5f + 10.0f;
	const float yline = 5.0f;
	NVGcolor groupColor = nvgRGB(0xDD, 0xDD, 0xDD);
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, xl_0, yline);
	nvgLineTo(args.vg, xl_1, yline); 
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, groupColor);
	nvgStroke(args.vg);	

	nvgBeginPath(args.vg);
	nvgFillColor(args.vg, backgroundColor);
	x = (xl_0 + xl_1) / 2.0f - 11;
	nvgRect(args.vg, x, yline - 3, 22, 6);
	nvgFill(args.vg);
	
	nvgFillColor(args.vg, groupColor);		
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgFontSize(args.vg, fontSize - 5); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);	
	nvgText(args.vg, (xl_0 + xl_1) / 2.0f, 8, "EDIT", NULL);	

	
	///-------------///
	/// * DISPLAY * ///
	///-------------///		
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	// Default Font:
	nvgFontSize(args.vg, fontSize);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 2.5);	
	
	// Current Edit Pattern Index (Sequence #/Step #)
	nvgFillColor(args.vg, textColor);
	x = 56;// 26;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "SEQ", NULL);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	if (currentPatternIndex > -1)
	{
		sprintf(messageStr, "%02d", (currentPatternIndex + 1));		
		nvgText(args.vg, x + dx, y2, messageStr, NULL);		
	}
	else
	{
		nvgText(args.vg, x + dx, y2, "--", NULL);		
	}

	// Current PATTERN Value (control being dragged)
	nvgFillColor(args.vg, textColor); // Maybe highlight this and the pad being edited?
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "PATT", NULL);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	if (currentPatternVal > -1)
	{
		sprintf(messageStr, "%02d", currentPatternVal);	
		nvgText(args.vg, x + dx, y2, messageStr, NULL);		
	}
	else
	{
		nvgText(args.vg, x + dx, y2, "--", NULL);				
	}

	// Pattern Length
	nvgFillColor(args.vg, textColor); // Maybe highlight this and the pad being edited?
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "PLEN", NULL);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	sprintf(messageStr, "%02d", currentPatternLength);	
	nvgText(args.vg, x + dx, y2, messageStr, NULL);
	
	return;
} // Pattern Sequence Config Widget draw()



//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqDisplay
// A top digital display for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Draw the normal default overview.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSeqDisplay::drawNormalView(const DrawArgs &args)
{
	bool isPreview = module == NULL; // May get a NULL module for preview		
	int currPlayPattern = 1;
	int currEditPattern = 1;
	int currentChannel = 1;
	int currentNSteps = 16;
	float currentBPM = 120;
	
	TSSequencerModuleBase::ControlSource currPlayPatternCtrlSrc = TSSequencerModuleBase::ControlSource::UserParameterSrc;

	NVGcolor currColor = TSColors::COLOR_TS_RED;

	if (!isPreview)
	{
		currColor = module->voiceColors[module->currentChannelEditingIx];
		currPlayPattern = module->currentPatternPlayingIx + 1;
		currEditPattern = module->currentPatternEditingIx + 1;
		currentChannel = module->currentChannelEditingIx + 1;
		currentNSteps = module->currentNumberSteps;
		currentBPM = module->currentBPM;
		currPlayPatternCtrlSrc = module->patternPlayingControlSource;
	}

	// Default Font:
	nvgFontSize(args.vg, fontSize);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 2.5);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

	int y1 = 42;
	int y2 = 27;
	int dx = 0;
	int x = 0;
	int spacing = 61;

	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

	// Current Playing Pattern
	nvgFillColor(args.vg, textColor);
	x = 5 + 21;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "PATT", NULL);
	sprintf(messageStr, "%02d", currPlayPattern);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);
	
	// Little indicator for what is controlling the current playing pattern
	// // Highest Priority Source (0) - External message (OSC or in future direct MIDI integration)
	// ExternalMsgSrc = 0,
	// // Control Voltage Input - Priority (1)
	// CVInputSrc = 1,
	// // For like Auto-Pattern-Sequencing - Priority (2)
	// AutoSrc = 2,
	// // User Parameter (UI) Control - Priority (3 = Last)
	// UserParameterSrc = 3
	const char *indicator[] = { "EXT", "CV", "SEQ", "USR" };
	int x_ind = x + spacing / 2.0f - 12;
	float smFontSize = fontSize - 5.5;
	nvgFontSize(args.vg, smFontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);		
	int y_ind = 17;
	//const float bWidth = 16;
	//const float bHeight = 8;
	nvgFillColor(args.vg, textColor);
	nvgTextAlign(args.vg, NVG_ALIGN_LEFT);	
	//nvgText(args.vg, x_ind, y_ind, indicator[currPlayPatternCtrlSrc], NULL);	
	// Make more room (if you put it at the highest BPM, it will cut it off), so we'll display it vertically
	const char* lbl = indicator[currPlayPatternCtrlSrc];
	int len = static_cast<int>(strlen(lbl));
	for (int i = 0; i < len; i++)
	{
		const char* end = (i < len-1) ? &(lbl[i + 1]) : NULL;
		nvgText(args.vg, x_ind, y_ind + i * smFontSize, &(lbl[i]), end);		
	}
	
	// for (int s = 0; s < 4; s++)
	// {
		// if (s == currPlayPatternCtrlSrc)
		// {
			// nvgBeginPath(args.vg);
			// nvgRect(args.vg, x_ind - bWidth/2.0f, y_ind - bHeight / 2.0f, bWidth, bHeight);
			// nvgFillColor(args.vg, textColor); 
			// nvgFill(args.vg);
			// nvgFillColor(args.vg, backgroundColor);				
		// }
		// else
		// {
			// nvgFillColor(args.vg, textColor);				
		// }
		// nvgText(args.vg, x_ind, y_ind, indicator[s], NULL);	
		// y_ind += 9;
	// } // end for
	
	// Current Playing Speed
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);		
	nvgFillColor(args.vg, textColor);
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	if (isPreview)
		sprintf(messageStr, "BPM/%s", BPMOptions[1]->label);
	else
		sprintf(messageStr, "BPM/%s", BPMOptions[module->selectedBPMNoteIx]->label);
	nvgText(args.vg, x, y1, messageStr, NULL);
	if (!isPreview && module->lastStepWasExternalClock)
	{
		sprintf(messageStr, "%s", "CLK");
	}
	else
	{
		sprintf(messageStr, "%03.0f", currentBPM);
	}
	nvgFontFaceId(args.vg, font->handle);
	nvgFontSize(args.vg, fontSize * 1.5); // Large font		
	nvgText(args.vg, x + dx, y2, messageStr, NULL);


	// Current Playing # Steps
	nvgFillColor(args.vg, textColor);
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "LENG", NULL);
	sprintf(messageStr, "%02d", currentNSteps);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);

	// Current Mode:
	nvgFillColor(args.vg, nvgRGB(0xda, 0xda, 0xda));
	x += spacing + 5;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "MODE", NULL);
	nvgFontSize(args.vg, fontSize);	// Small font
	nvgFontFaceId(args.vg, font->handle);	
	// Mode is now associated to the Channel, so match the channel color:
	nvgFillColor(args.vg, currColor); // Match the Gate/Trigger color	
	if (!isPreview && module->modeString != NULL)
	{
		nvgText(args.vg, x + dx, y2, module->modeString, NULL);
	}
	else 
	{
		nvgText(args.vg, x + dx, y2, "TRIG", NULL);			
	}

	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

	// Current Edit Pattern
	nvgFillColor(args.vg, textColor);
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "PATT", NULL);
	sprintf(messageStr, "%02d", currEditPattern);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);

	// Current Edit Gate/Trigger
	nvgFillColor(args.vg, currColor); // Match the Gate/Trigger color
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "CHNL", NULL);
	sprintf(messageStr, "%02d", currentChannel);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);

	// [[[[[[[[[[[[[[[[ EDIT Box Group ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
	nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
	NVGcolor groupColor = nvgRGB(0xDD, 0xDD, 0xDD);
	nvgFillColor(args.vg, groupColor);
	int labelX = 297;
	x = labelX; // 289
	nvgFontSize(args.vg, fontSize - 5); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, 8, "EDIT", NULL);

	// Edit Label Line ---------------------------------------------------------------
	nvgBeginPath(args.vg);
	// Start top to the left of the text "Edit"
	int y = 5;
	nvgMoveTo(args.vg, /*start x*/ x - 3, /*start y*/ y);// Starts new sub-path with specified point as first point.s
	x = 256;// x - 35;//xOffset + 3 * spacing - 3 + 60;
	nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Go to Left (Line Start)

	x = labelX + 22;
	y = 5;
	nvgMoveTo(args.vg, /*x*/ x, /*y*/ y); // Right of "Edit"
	x = box.size.x - 6;
	nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // RHS of box

	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, groupColor);
	nvgStroke(args.vg);

	// [[[[[[[[[[[[[[[[ PLAYBACK Box Group ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
	groupColor = nvgRGB(0xEE, 0xEE, 0xEE);
	nvgFillColor(args.vg, groupColor);
	labelX = 64;
	x = labelX;
	nvgFontSize(args.vg, fontSize - 5); // Small font
	nvgText(args.vg, x, 8, "PLAYBACK", NULL);

	// Play Back Label Line ---------------------------------------------------------------
	nvgBeginPath(args.vg);
	// Start top to the left of the text "Play"
	y = 5;
	nvgMoveTo(args.vg, /*start x*/ x - 3, /*start y*/ y);// Starts new sub-path with specified point as first point.s
	x = 6;
	nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Go to the left

	x = labelX + 52;
	y = 5;
	nvgMoveTo(args.vg, /*x*/ x, /*y*/ y); // To the Right of "Playback"
	x = 165; //x + 62 ;
	nvgLineTo(args.vg, /*x*/ x, /*y*/ y); // Go Right 

	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, groupColor);
	nvgStroke(args.vg);
	
	return;
} // end drawNormalView()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Draw the edit step view.
// @currEditStep : Step number (1 - N).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSeqDisplay::drawEditStepView(const DrawArgs &args, int currEditStep)
{
	bool isPreview = module == NULL; // May get a NULL module for preview		
	int currEditPattern = 1;
	int currentChannel = 1;
	float currEditStepVal = 0.0f;
	int currEditStepParamId = -1;
	NVGcolor currColor = TSColors::COLOR_TS_RED;

	if (!isPreview)
	{
		currColor = module->voiceColors[module->currentChannelEditingIx];
		currEditPattern = module->currentPatternEditingIx + 1;
		currentChannel = module->currentChannelEditingIx + 1;
		currEditStepParamId = module->currentStepBeingEditedParamId;
		currEditStepVal = module->params[currEditStepParamId].getValue();
	}

	// Default Font:
	nvgFontSize(args.vg, fontSize);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 2.5);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

	int y1 = 42;
	int y2 = 27;
	int dx = 0;
	float x = 0;
	int spacing = 61;

	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

	// Current Edit Pattern
	nvgFillColor(args.vg, textColor);
	x = 26;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "PATT", NULL);
	sprintf(messageStr, "%02d", currEditPattern);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);

	// Current Edit Channel
	nvgFillColor(args.vg, currColor); // Match the Gate/Trigger color
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "CHNL", NULL);
	sprintf(messageStr, "%02d", currentChannel);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);
			
	// Current Edit Step
	nvgFillColor(args.vg, currColor); // Match the Gate/Trigger color
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	nvgText(args.vg, x, y1, "STEP", NULL);
	sprintf(messageStr, "%02d", currEditStep);
	nvgFontSize(args.vg, fontSize * 1.5);	// Large font
	nvgFontFaceId(args.vg, font->handle);
	nvgText(args.vg, x + dx, y2, messageStr, NULL);

	// Current Edit Step Value
	nvgFillColor(args.vg, textColor);
	x += spacing;
	nvgFontSize(args.vg, fontSize); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);
	if (sequencerValueModePtr != NULL)
	{
		nvgText(args.vg, x, y1, sequencerValueModePtr->displayName, NULL);
	}
	else
	{
		nvgText(args.vg, x, y1, "VAL", NULL);
	}
	nvgFontSize(args.vg, fontSize * 1.3);	// Large font
	nvgFontFaceId(args.vg, font->handle);		
	if (sequencerValueModePtr != NULL)
	{
		sequencerValueModePtr->GetDisplayString(currEditStepVal, messageStr);
	}
	else
	{
		sprintf(messageStr, "%4.1f", currEditStepVal);			
	}
	nvgText(args.vg, x + dx, y2, messageStr, NULL);
	
	///---------------///
	/// * Edit Line * ///
	///---------------///	
	float xl_0 = 5.0f;
	float mult = 3.7f;
	if (module != NULL)
	{
		if (module->selectedOutputValueMode == TSSequencerModuleBase::ValueMode::VALUE_VOLT)
			mult = 4.0f; // Longer values
	}	
	float xl_1 = xl_0 + spacing * mult;
	const float yline = 5.0f;
	NVGcolor groupColor = nvgRGB(0xDD, 0xDD, 0xDD);
	nvgBeginPath(args.vg);
	nvgMoveTo(args.vg, xl_0, yline);
	nvgLineTo(args.vg, xl_1, yline); 
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, groupColor);
	nvgStroke(args.vg);	

	nvgBeginPath(args.vg);
	nvgFillColor(args.vg, backgroundColor);
	x = (xl_0 + xl_1) / 2.0f - 11;
	nvgRect(args.vg, x, yline - 3, 22, 6);
	nvgFill(args.vg);
	
	nvgFillColor(args.vg, groupColor);		
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgFontSize(args.vg, fontSize - 5); // Small font
	nvgFontFaceId(args.vg, labelFont->handle);	
	nvgText(args.vg, (xl_0 + xl_1) / 2.0f, 8, "EDIT", NULL);	
	
	// if (lastStepEditShownParamId != currEditStepParamId || lastStepEditShownValue != currEditStepVal)
	// {
		// DEBUG("Param Value Change. Step #%d. Id = %d, Value = %4.1f", currEditStep, currEditStepParamId, currEditStepVal);
	// }		
	lastStepEditShownParamId = currEditStepParamId;
	lastStepEditShownValue = currEditStepVal;						
	return;
} // end drawEditStepView()	

