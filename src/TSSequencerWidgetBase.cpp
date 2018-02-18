#include <string.h>
#include <stdio.h>
#include "widgets.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
//#include "TSTextField.hpp"
#include "TSOSCConfigWidget.hpp"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Base constructor.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSSequencerWidgetBase::TSSequencerWidgetBase()
{
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Add common controls to the UI widget for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerWidgetBase::addBaseControls(bool addGridLines)
{
	TSSequencerModuleBase *thisModule = dynamic_cast<TSSequencerModuleBase*>(this->module);
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
	// Should be a popup but J just wants it of the screen.
	////////////////////////////////////
	{
		TSOSCConfigWidget* oscConfig = new TSOSCConfigWidget(thisModule, TSSequencerModuleBase::ParamIds::OSC_SAVE_CONF_PARAM, TSSequencerModuleBase::ParamIds::OSC_DISABLE_PARAM,
			thisModule->oscCurrentClient,
			thisModule->currentOSCSettings.oscTxIpAddress.c_str(), thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort);
		oscConfig->visible = false;
		oscConfig->box.pos = display->box.pos;
		oscConfig->box.size = display->box.size;
		//oscConfig->module = thisModule;
		this->oscConfigurationScreen = oscConfig;
		addChild(oscConfig);
	}



	////////////////////////////////////
	// Labels
	////////////////////////////////////
	{
		TSSeqLabelArea *area = new TSSeqLabelArea();
		area->box.pos = Vec(0, 0);
		area->box.size = Vec(box.size.x, 380);
		area->module = thisModule;
		area->drawGridLines = addGridLines;
		addChild(area);
	}
	// Screws:
	addChild(createScrew<ScrewBlack>(Vec(0, 0)));
	addChild(createScrew<ScrewBlack>(Vec(box.size.x - 15, 0)));
	addChild(createScrew<ScrewBlack>(Vec(0, box.size.y - 15)));
	addChild(createScrew<ScrewBlack>(Vec(box.size.x - 15, box.size.y - 15)));

	// Inputs ==================================================	
	// Run (Toggle)
	Vec btnSize = Vec(50,22);
	addParam(createParam<TS_PadBtn>(Vec(15, 320), thisModule, TSSequencerModuleBase::ParamIds::RUN_PARAM, 0.0, 1.0, 0.0));
	TS_LightString* item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 320), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::LightIds::RUNNING_LIGHT,
		/* size */ btnSize, /* color */ COLOR_WHITE));
	item->lightString = "RUN";
	addChild(item);
	
	// Reset (Momentary)
	addParam(createParam<TS_PadBtn>(Vec(15, 292), thisModule, TSSequencerModuleBase::ParamIds::RESET_PARAM, 0.0, 1.0, 0.0));
	item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 292), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::LightIds::RESET_LIGHT,	
		/* size */ btnSize, /* color */ COLOR_WHITE));
	item->lightString = "RESET";
	addChild(item);
	
	// Paste button:
	addParam(createParam<TS_PadBtn>(Vec(15, 115), thisModule, TSSequencerModuleBase::ParamIds::PASTE_PARAM, 0.0, 1.0, 0.0));
	thisModule->pasteLight = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 115), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::LightIds::PASTE_LIGHT,
		/* size */ btnSize, /* color */ COLOR_WHITE));
	thisModule->pasteLight->lightString = "PASTE";
	addChild(thisModule->pasteLight);
		
	// Top Knobs : Keep references for later
	int knobRow = 79;
	int knobStart = 27;
	int knobSpacing = 61;

	// Pattern Playback Select  (Knob)
	thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::PlayPatternKnob] = dynamic_cast<SVGKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart, knobRow), thisModule, TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_PLAY_PARAM, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ thisModule->currentPatternPlayingIx));
	addParam(thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::PlayPatternKnob]);
	
	// Clock BPM (Knob)
	thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::BPMKnob] = dynamic_cast<SVGKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 1), knobRow), thisModule, TSSequencerModuleBase::ParamIds::BPM_PARAM, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX, (TROWA_SEQ_BPM_KNOB_MAX + TROWA_SEQ_BPM_KNOB_MIN) / 2));
	addParam(thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::BPMKnob]);
	
	// Steps (Knob)
	thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::StepLengthKnob] = dynamic_cast<SVGKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 2), knobRow), thisModule, TSSequencerModuleBase::ParamIds::STEPS_PARAM, 1.0, thisModule->maxSteps, thisModule->maxSteps));
	addParam(thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::StepLengthKnob]);
	
	// Output Mode (Knob)
	RoundSmallBlackKnob* outKnobPtr = dynamic_cast<RoundSmallBlackKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 3), knobRow), thisModule, 
		TSSequencerModuleBase::ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM, 0, TROWA_SEQ_NUM_MODES - 1, TSSequencerModuleBase::ValueMode::VALUE_TRIGGER));
	outKnobPtr->minAngle = -0.6*M_PI;
	outKnobPtr->maxAngle = 0.6*M_PI;
	thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::OutputModeKnob] = outKnobPtr;
	addParam(outKnobPtr);
	
	// Pattern Edit Select (Knob)
	thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::EditPatternKnob] = dynamic_cast<SVGKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 4), knobRow), thisModule, TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_EDIT_PARAM, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ thisModule->currentPatternEditingIx));
	addParam(thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::EditPatternKnob]);
	
	// Selected Gate/Voice/Channel (Knob)
	thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::EditChannelKnob] = dynamic_cast<SVGKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 5), knobRow), thisModule, TSSequencerModuleBase::ParamIds::SELECTED_CHANNEL_PARAM, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_CHNLS - 1, /*default value*/ thisModule->currentChannelEditingIx));
	addParam(thisModule->controlKnobs[TSSequencerModuleBase::KnobIx::EditChannelKnob]);
	
	Vec ledSize = Vec(15,15);
	int dx = 28;

	// OSC: Enable OSC button:
	LEDButton* btn;
	int y = knobRow;
	int x = knobStart + (knobSpacing * 3) + dx; // 30
	if (thisModule->allowOSC)
	{
		Vec btnSize = Vec(ledSize.x - 2, ledSize.y - 2);
		btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(x, y), module, TSSequencerModuleBase::ParamIds::OSC_SHOW_CONF_PARAM, 0, 1, 0));
		btn->box.size = btnSize;
		addParam(btn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x, y), module, TSSequencerModuleBase::LightIds::OSC_CONFIGURE_LIGHT, ledSize, COLOR_WHITE));
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + 2, y + 2), module, TSSequencerModuleBase::LightIds::OSC_ENABLED_LIGHT, Vec(ledSize.x - 4, ledSize.y - 4), TSOSC_STATUS_COLOR));
		//Vec smallLedSize = Vec(ledSize.x - 6, ledSize.y - 6);
		//addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + 3, y + 3), module, TSSequencerModuleBase::LightIds::OSC_CONFIGURE_LIGHT, smallLedSize, COLOR_WHITE));
	}

	// COPY: Pattern Copy button:
	btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(knobStart + (knobSpacing * 4) + dx, knobRow), module, TSSequencerModuleBase::ParamIds::COPY_PATTERN_PARAM, 0, 1, 0));
	btn->box.size = ledSize;
	addParam(btn);
	thisModule->copyPatternLight = TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 4) + dx, knobRow), module, TSSequencerModuleBase::LightIds::COPY_PATTERN_LIGHT, ledSize, COLOR_WHITE);
	addChild(thisModule->copyPatternLight);
	// COPY: Gate Copy button:
	btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(knobStart + (knobSpacing * 5) + dx, knobRow), module, TSSequencerModuleBase::ParamIds::COPY_CHANNEL_PARAM, 0, 1, 0));
	btn->box.size = ledSize;
	addParam(btn);
	thisModule->copyGateLight = TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 5) + dx, knobRow), module, TSSequencerModuleBase::LightIds::COPY_CHANNEL_LIGHT, ledSize, COLOR_WHITE);
	addChild(thisModule->copyGateLight);
	// CHANGE BPM CALC NOTE (1/4, 1/8, 1/8T, 1/16)
	//SELECTED_BPM_MULT_IX_PARAM
	btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(knobStart + (knobSpacing * 1) + dx, knobRow), module, TSSequencerModuleBase::ParamIds::SELECTED_BPM_MULT_IX_PARAM, 0, 1, 0));
	btn->box.size = ledSize;
	addParam(btn);
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 1) + dx, knobRow), module, TSSequencerModuleBase::LightIds::SELECTED_BPM_MULT_IX_LIGHT, ledSize, COLOR_WHITE));
	
	// Swing Adjustment Knob:
	//addParam(createParam<RoundSmallBlackKnob>(Vec(20, 270), thisModule, 
		//TSSequencerModuleBase::SWING_ADJ_PARAM, /*min*/ TROWA_SEQ_SWING_ADJ_MIN, /*max*/ TROWA_SEQ_SWING_ADJ_MAX, 
	//	/*default value*/ 0));

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
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 2; c++)
		{
			// Triggers / Gates / Output:
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), thisModule, TSSequencerModuleBase::OutputIds::CHANNELS_OUTPUT+v, /*color*/ thisModule->voiceColors[v]));
			// Match the color to the trigger/gate/output:
			addChild(TS_createColorValueLight<TS_LightRing>(/*position*/ Vec(x + 5, y + 5), 
				/*thisModule*/ thisModule, 
				/*lightId*/ TSSequencerModuleBase::LightIds::CHANNEL_LIGHTS+v,
				/*size*/ outputLightSize, /*lightColor*/ thisModule->voiceColors[v], /*backColor*/ thisModule->voiceColors[v]));
			thisModule->lights[TSSequencerModuleBase::LightIds::CHANNEL_LIGHTS + v].value = 0;
			x += 36;
			v++;
		} // end for
		y += 28; // Next row
		x = 314;
	} // end loop through NxM grid

	info("Base Controls added.");
	return;
} // end addBaseControls()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerWidgetBase::step()
{
	TSSequencerModuleBase* thisModule = dynamic_cast<TSSequencerModuleBase*>(module);

	if (thisModule->oscConfigTrigger.process(thisModule->params[TSSequencerModuleBase::ParamIds::OSC_SHOW_CONF_PARAM].value))
	{
		thisModule->oscShowConfigurationScreen = !thisModule->oscShowConfigurationScreen;
		thisModule->lights[TSSequencerModuleBase::LightIds::OSC_CONFIGURE_LIGHT].value = (thisModule->oscShowConfigurationScreen) ? 1.0 : 0.0;
		this->oscConfigurationScreen->setVisible(thisModule->oscShowConfigurationScreen);
		this->display->showDisplay = !thisModule->oscShowConfigurationScreen;
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
			this->oscConfigurationScreen->setValues(thisModule->currentOSCSettings.oscTxIpAddress, thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort);
			this->oscConfigurationScreen->setSelectedClient(thisModule->oscCurrentClient); // OSC Client
			this->oscConfigurationScreen->btnActionEnable = !thisModule->oscInitialized;

			this->oscConfigurationScreen->errorMsg = "";
			if (thisModule->oscError)
			{
				this->oscConfigurationScreen->errorMsg = "Error connecting to " + thisModule->currentOSCSettings.oscTxIpAddress;
			}
		}
	}
	if (thisModule->oscShowConfigurationScreen)
	{
		// Check for enable/disable
		if (thisModule->oscConnectTrigger.process(thisModule->params[TSSequencerModuleBase::ParamIds::OSC_SAVE_CONF_PARAM].value))
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
					this->oscConfigurationScreen->errorMsg = "";
					thisModule->oscNewSettings.oscTxIpAddress = this->oscConfigurationScreen->tbIpAddress->text.c_str();
					thisModule->oscNewSettings.oscTxPort = this->oscConfigurationScreen->getTxPort();
					thisModule->oscNewSettings.oscRxPort = this->oscConfigurationScreen->getRxPort(); 
					thisModule->oscCurrentClient = this->oscConfigurationScreen->getSelectedClient();
					thisModule->oscCurrentAction = TSSequencerModuleBase::OSCAction::Enable;
				}
			} // end if enable osc
			else
			{
				// Disable OSC ------------------------------------------------------------------
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Disable OSC clicked.");
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
		{
			this->oscConfigurationScreen->statusMsg = OSCClientAbbr[thisModule->oscCurrentClient] + " " + thisModule->currentOSCSettings.oscTxIpAddress;
			this->oscConfigurationScreen->statusMsg2 = ":" + std::to_string(thisModule->currentOSCSettings.oscTxPort)
				+ " :" + std::to_string(thisModule->currentOSCSettings.oscRxPort);
			this->oscConfigurationScreen->btnActionEnable = false;
		}
		else
		{
			this->oscConfigurationScreen->successMsg = "";
			this->oscConfigurationScreen->statusMsg = "OSC Not Connected";
			this->oscConfigurationScreen->statusMsg2 = "";
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
		AllPatterns
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

	void onAction(EventAction &e) override {
		if (this->Target == ShiftType::AllPatterns)
		{
			sequencerModule->randomize(TROWA_INDEX_UNDEFINED, TROWA_SEQ_COPY_CHANNELIX_ALL, useStucturedRandom);
		}
		else if (this->Target == ShiftType::ThisPattern)
		{
			sequencerModule->randomize(sequencerModule->currentPatternEditingIx, TROWA_SEQ_COPY_CHANNELIX_ALL, useStucturedRandom);
		}
		else //if (this->Target == ShiftType::CurrentChannelOnly)
		{
			sequencerModule->randomize(sequencerModule->currentPatternEditingIx, sequencerModule->currentChannelEditingIx, useStucturedRandom);
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

	void createChildren()
	{
		addChild(new seqRandomSubMenuItem("Current Edit Channel", seqRandomSubMenuItem::ShiftType::CurrentChannelOnly, this->useStucturedRandom, this->sequencerModule));
		addChild(new seqRandomSubMenuItem("Current Edit Pattern", seqRandomSubMenuItem::ShiftType::ThisPattern, this->useStucturedRandom, this->sequencerModule));
		addChild(new seqRandomSubMenuItem("ALL Patterns", seqRandomSubMenuItem::ShiftType::AllPatterns, this->useStucturedRandom, this->sequencerModule));
		return;
	}
};
// First tier menu item. Create Submenu
struct seqRandomMenuItem : MenuItem {
	TSSequencerModuleBase* sequencerModule;
	bool useStucturedRandom;

	seqRandomMenuItem(std::string text, bool useStructured, TSSequencerModuleBase* seqModule)
	{
		this->text = text;
		this->useStucturedRandom = useStructured;
		this->sequencerModule = seqModule;
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


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// createContextMenu()
// Create context menu with more random options for sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Menu *TSSequencerWidgetBase::createContextMenu()
{
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	TSSequencerModuleBase* sequencerModule = dynamic_cast<TSSequencerModuleBase*>(module);

	//-------- Random ------- //
	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Random Options";
	menu->addChild(modeLabel); //menu->pushChild(modeLabel);
	menu->addChild(new seqRandomMenuItem("> All Steps Random", false, sequencerModule));
	menu->addChild(new seqRandomMenuItem("> Structured Random", true, sequencerModule));
	return menu;
}