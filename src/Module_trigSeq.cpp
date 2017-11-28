#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"

#define trigSeq_GATE_ON_OUTPUT	  10.0  // If gate is on, the value to output (port Voltage)
#define trigSeq_GATE_OFF_OUTPUT	   0.0  // If gate is off, the value to output (port Voltage)


//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq Module
// trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct trigSeq : TSSequencerModuleBase
{	
	SchmittTrigger gateTriggers[TROWA_SEQ_NUM_STEPS];
	trigSeq() : TSSequencerModuleBase(false)
	{
		selectedOutputValueMode = VALUE_TRIGGER;
		lastOutputValueMode = selectedOutputValueMode;
		modeStrings[0] = "TRIG";
		modeStrings[1] = "RTRG";
		modeStrings[2] = "CONT";		
		return;
	}
	void step() override;
	// Only randomize the current gate/trigger steps.
	void randomize() override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void trigSeq::randomize()
{
	for (int s = 0; s < TROWA_SEQ_NUM_STEPS; s++) 
	{
		triggerState[currentPatternEditingIx][currentTriggerEditingIx][s] = (randomf() > 0.5);		
	}	
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq:step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void trigSeq::step() {
	bool gOn = true;
	bool pulse = false;
	bool reloadMatrix = false;
	bool valueModeChanged =  false;
	
	TSSequencerModuleBase::getStepInputs(&pulse, &reloadMatrix, &valueModeChanged);
	
	
	if (valueModeChanged)
	{
		// Gate Mode has changed
		gateMode =  static_cast<GateMode>((short)(selectedOutputValueMode));
		modeString = modeStrings[selectedOutputValueMode];
	}
	
	int r = 0;
	int c = 0;
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	if (reloadMatrix)
	{
		// Load this gate and/or pattern into our 4x4 matrix
		for (int s = 0; s < TROWA_SEQ_NUM_STEPS; s++) 
		{
			r = s / TROWA_SEQ_STEP_NUM_COLS;
			c = s % TROWA_SEQ_STEP_NUM_COLS;
			padLightPtrs[r][c]->setColor(voiceColors[currentTriggerEditingIx]);
			if (triggerState[currentPatternEditingIx][currentTriggerEditingIx][s])
			{
				//gateLights[r][c] = (triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]) ? 1.0f - stepLights[r][c] : 0.0f;
				gateLights[r][c] = 1.0f - stepLights[r][c];
				gateTriggers[s].state = SchmittTrigger::HIGH;
			}
			else
			{
				gateLights[r][c] = 0.0f; // Turn light off	
				gateTriggers[s].state = SchmittTrigger::LOW;
			}
		}		
	}
	//-- * Read the buttons
	else
	{		
		// Gate buttons (we only show one gate) - Read Inputs
		for (int s = 0; s < TROWA_SEQ_NUM_STEPS; s++) 
		{
			if (gateTriggers[s].process(params[GATE_PARAM + s].value)) 
			{
				triggerState[currentPatternEditingIx][currentTriggerEditingIx][s] = !triggerState[currentPatternEditingIx][currentTriggerEditingIx][s];
			}
			r = s / TROWA_SEQ_STEP_NUM_COLS;
			c = s % TROWA_SEQ_STEP_NUM_COLS;
			stepLights[r][c] -= stepLights[r][c] / lightLambda / engineGetSampleRate();
			
			gateLights[r][c] = (triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]) ? 1.0 - stepLights[r][c] : stepLights[r][c];
			lights[PAD_LIGHTS + s].value = gateLights[r][c];
		} // end loop through step buttons
	}
	
	// Set Outputs (16 triggers)	
	gOn = true;
	if (gateMode == TRIGGER)
		gOn = pulse;  // gateOn = gateOn && pulse;
	else if (gateMode == RETRIGGER)
		gOn = !pulse; // gateOn = gateOn && !pulse;		
	for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++) 
	{
		float gate = (running && gOn && (triggerState[currentPatternPlayingIx][g][index] >= 1.0)) ? trigSeq_GATE_ON_OUTPUT : trigSeq_GATE_OFF_OUTPUT;
		outputs[GATES_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):		
		lights[GATE_LIGHTS + g].value = (triggerState[currentPatternPlayingIx][g][index]) ? 1.0 : 0;
	}
	
	return;
} // end step()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeqWidget
// Widget for the trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
trigSeqWidget::trigSeqWidget() : TSSequencerWidgetBase()
{
	trigSeq *module = new trigSeq();
	setModule(module);
	
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/trigSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls();
	
	// (User) Input Pads ==================================================	
	int y = 115;
	int x = 79;
	int dx = 3;
	//int lightSize = 50 - 2*dx;
	Vec lSize = Vec(50 - 2*dx, 50 - 2*dx);
	//int v = 0;
	for (int r = 0; r < TROWA_SEQ_STEP_NUM_ROWS; r++) //---------THE PADS
	{
		for (int c = 0; c < TROWA_SEQ_STEP_NUM_COLS; c++)
		{			
			// Pad buttons:
			addParam(createParam<TS_PadSquare>(Vec(x, y), module, TSSequencerModuleBase::GATE_PARAM + r*TROWA_SEQ_STEP_NUM_COLS + c, 0.0, 1.0, 0.0));
			// Keep a reference to our pad lights so we can change the colors
			module->padLightPtrs[r][c] = dynamic_cast<TS_LightSquare*>(TS_createColorValueLight<TS_LightSquare>(/*pos */ Vec(x+dx, y+dx), 
				/*module*/ module, 
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*TROWA_SEQ_STEP_NUM_COLS + c,
				/* size */ lSize, /* color */ module->voiceColors[module->currentTriggerEditingIx]));
			addChild( module->padLightPtrs[r][c] );

			x+= 59;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through 4x4 grid
	
	module->modeString = module->modeStrings[module->selectedOutputValueMode];

	return;
}
