#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"

TSSequencerWidgetBase::TSSequencerWidgetBase()
{
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Add common controls to the UI widget for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerWidgetBase::addBaseControls()
{
	TSSequencerModuleBase *thisModule = dynamic_cast<TSSequencerModuleBase*>(this->module);
	//const int margin = 0;	// Margin (away from edge of thisModule)

	////////////////////////////////////
	// DISPLAY
	////////////////////////////////////
	{
		TSSeqDisplay *display = new TSSeqDisplay();
		display->box.pos = Vec(13, 24);
		display->box.size = Vec(363, 48);
		display->module = thisModule;
		addChild(display);
	}	
	////////////////////////////////////
	// Labels
	////////////////////////////////////
	{
		TSSeqLabelArea *area = new TSSeqLabelArea();
		area->box.pos = Vec(0, 0);
		area->box.size = Vec(box.size.x, 380);
		area->module = thisModule;
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
	addParam(createParam<TS_PadBtn>(Vec(15, 320), thisModule, TSSequencerModuleBase::RUN_PARAM, 0.0, 1.0, 0.0));
	TS_LightString* item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 320), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::RUNNING_LIGHT,
		/* size */ btnSize, /* color */ COLOR_WHITE));
	item->lightString = "RUN";
	addChild(item);
	
	// Reset (Momentary)
	addParam(createParam<TS_PadBtn>(Vec(15, 292), thisModule, TSSequencerModuleBase::RESET_PARAM, 0.0, 1.0, 0.0));
	item = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 292), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::RESET_LIGHT,	
		/* size */ btnSize, /* color */ COLOR_WHITE));
	item->lightString = "RESET";
	addChild(item);
	
	// Paste button:
	addParam(createParam<TS_PadBtn>(Vec(15, 115), thisModule, TSSequencerModuleBase::PASTE_PARAM, 0.0, 1.0, 0.0));
	thisModule->pasteLight = dynamic_cast<TS_LightString*>(TS_createColorValueLight<TS_LightString>(/*pos */ Vec(15, 115), 
		/*thisModule*/ thisModule,
		/*lightId*/ TSSequencerModuleBase::PASTE_LIGHT,
		/* size */ btnSize, /* color */ COLOR_WHITE));
	thisModule->pasteLight->lightString = "PASTE";
	addChild(thisModule->pasteLight);
	
		
	int knobRow = 79;
	int knobStart = 27;
	int knobSpacing = 61;
	
	// Pattern Playback Select  (Knob)
	addParam(createParam<RoundSmallBlackKnob>(Vec(knobStart, knobRow), thisModule, TSSequencerModuleBase::SELECTED_PATTERN_PLAY_PARAM, /*min*/ 0.0, /*max*/ TROWASEQ_NUM_PATTERNS - 1, /*default value*/ thisModule->currentPatternPlayingIx));
	
	// Clock BPM (Knob)
	//addParam(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 1), knobRow), thisModule, TSSequencerModuleBase::BPM_PARAM, -2.0, 6.0, 2.0));
	addParam(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 1), knobRow), thisModule, TSSequencerModuleBase::BPM_PARAM, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX, (TROWA_SEQ_BPM_KNOB_MAX+TROWA_SEQ_BPM_KNOB_MIN)/2));	
	
	// Steps (Knob)
	addParam(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 2), knobRow), thisModule, TSSequencerModuleBase::STEPS_PARAM, 1.0, TROWA_SEQ_NUM_STEPS, TROWA_SEQ_NUM_STEPS));
	
	// Output Mode (Knob)
	RoundSmallBlackKnob* outKnobPtr = dynamic_cast<RoundSmallBlackKnob*>(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 3), knobRow), thisModule, 
		TSSequencerModuleBase::SELECTED_OUTPUT_VALUE_MODE_PARAM, 0, TROWA_SEQ_NUM_MODES - 1, TSSequencerModuleBase::VALUE_TRIGGER));
	outKnobPtr->minAngle = -0.6*M_PI;
	outKnobPtr->maxAngle = 0.6*M_PI;
	addParam(outKnobPtr);
	
	// Pattern Edit Select (Knob)
	addParam(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 4), knobRow), thisModule, TSSequencerModuleBase::SELECTED_PATTERN_EDIT_PARAM, /*min*/ 0.0, /*max*/ TROWASEQ_NUM_PATTERNS - 1, /*default value*/ thisModule->currentPatternEditingIx));
	
	// Selected Gate/Voice/Channel (Knob)
	addParam(createParam<RoundSmallBlackKnob>(Vec(knobStart + (knobSpacing * 5), knobRow), thisModule, TSSequencerModuleBase::SELECTED_GATE_PARAM, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_TRIGS - 1, /*default value*/ thisModule->currentTriggerEditingIx));
	
	Vec ledSize = Vec(15,15);
	int dx = 28;
	// COPY: Pattern Copy button:
	LEDButton* btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(knobStart + (knobSpacing * 4) + dx, knobRow), module, TSSequencerModuleBase::COPY_PATTERN_PARAM, 0, 1, 0));
	btn->box.size = ledSize;
	addParam(btn);
	thisModule->copyPatternLight = TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 4) + dx, knobRow), module, TSSequencerModuleBase::COPY_PATTERN_LIGHT, ledSize, COLOR_WHITE);
	addChild(thisModule->copyPatternLight);
	// COPY: Gate Copy button:
	btn = dynamic_cast<LEDButton*>(createParam<LEDButton>(Vec(knobStart + (knobSpacing * 5) + dx, knobRow), module, TSSequencerModuleBase::COPY_GATE_PARAM, 0, 1, 0));
	btn->box.size = ledSize;
	addParam(btn);
	thisModule->copyGateLight = TS_createColorValueLight<ColorValueLight>(Vec(knobStart + (knobSpacing * 5) + dx, knobRow), module, TSSequencerModuleBase::COPY_GATE_LIGHT, ledSize, COLOR_WHITE);
	addChild(thisModule->copyGateLight);
	
	
	
	// Swing Adjustment Knob:
	//#define TROWA_SEQ_SWING_ADJ_MIN			-0.1
	//#define TROWA_SEQ_SWING_ADJ_MAX		     0.1
	//addParam(createParam<RoundSmallBlackKnob>(Vec(20, 270), thisModule, 
		//TSSequencerModuleBase::SWING_ADJ_PARAM, /*min*/ TROWA_SEQ_SWING_ADJ_MIN, /*max*/ TROWA_SEQ_SWING_ADJ_MAX, 
	//	/*default value*/ 0));

		
	// Input Jacks:
	int xStart = 10;
	int ySpacing = 28;
	int portStart = 143;
	
	// Selected Pattern Playback:
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 0)), thisModule, TSSequencerModuleBase::SELECTED_PATTERN_PLAY_INPUT));
	
	// Clock
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 1)), thisModule, TSSequencerModuleBase::BPM_INPUT));
	
	// Steps
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 2)), thisModule, TSSequencerModuleBase::STEPS_INPUT));
	
	// External Clock
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 3)), thisModule, TSSequencerModuleBase::EXT_CLOCK_INPUT));
	
	// Reset 
	addInput(TS_createInput<TS_Port>(Vec(xStart, portStart + (ySpacing * 4)), thisModule, TSSequencerModuleBase::RESET_INPUT));
		
	// Outputs ==================================================
	// Loop through each channel/voice/gate
	int y = 115;
	int x = 314;
	int v = 0;
	//int ix = 0;
	
	float jackDiameter = 20.5; // 28.351
	float add = 0;
	Vec outputLightSize = Vec(jackDiameter + add, jackDiameter + add);
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 2; c++)
		{
			// Triggers / Gates / Output:
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), thisModule, TSSequencerModuleBase::GATES_OUTPUT+v, /*color*/ thisModule->voiceColors[v]));
			// Match the color to the trigger/gate/output:
			addChild(TS_createColorValueLight<TS_LightRing>(/*position*/ Vec(x + 5, y + 5), 
				/*thisModule*/ thisModule, 
				/*lightId*/ TSSequencerModuleBase::GATE_LIGHTS+v,
				/*size*/ outputLightSize, /*lightColor*/ thisModule->voiceColors[v], /*backColor*/ thisModule->voiceColors[v]));
			
			x += 36;
			v++;
		}
		y += 28; // Next row
		x = 314;
	} // end loop through 4x4 grid
}