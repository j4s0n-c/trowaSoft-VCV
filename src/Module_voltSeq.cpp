#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"


#define voltSeq_STEP_KNOB_MIN	  -10.0  // Min Value from our knobs
#define voltSeq_STEP_KNOB_MAX	   10.0  // Maximum value from our knobs

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq Module
// trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct voltSeq : TSSequencerModuleBase	
{	
	// Lights
	//TS_LightArc* padLightPtrs[TROWA_SEQ_STEP_NUM_ROWS][TROWA_SEQ_STEP_NUM_COLS];
	// References to our pad knobs
	TS_LightedKnob* knobStepMatrix[TROWA_SEQ_STEP_NUM_ROWS][TROWA_SEQ_STEP_NUM_COLS];
	
	ValueSequencerMode* ValueModes[TROWA_SEQ_NUM_MODES] = { 
		// Voltage Mode 
		new ValueSequencerMode(/*displayName*/ "VOLT",
			/*minDisplayValue*/ -10, /*maxDisplayValue*/ 10, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ -10, /*outVoltageMax*/ 10, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ 1.5*NVG_PI, 
			/*display format String */ "%04.2f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ (voltSeq_STEP_KNOB_MAX+voltSeq_STEP_KNOB_MIN)/2.0),//
		// Sequence Mode (1-16 for the patterns)
		
		new NoteValueSequencerMode(/*displayName*/ "NOTE",			
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX),
			
		new ValueSequencerMode(/*displayName*/ "PATT",
			/*minDisplayValue*/ 1, /*maxDisplayValue*/ TROWASEQ_NUM_PATTERNS, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ TROWASEQ_PATTERN_MIN_V, /*outVoltageMax*/ TROWASEQ_PATTERN_MAX_V, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ 0.67*NVG_PI, 
			/*display format String */ "%02.0f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ voltSeq_STEP_KNOB_MIN)        
			
	};
	voltSeq() : TSSequencerModuleBase(voltSeq_STEP_KNOB_MIN)
	{
		selectedOutputValueMode = VALUE_VOLT;
		lastOutputValueMode = selectedOutputValueMode;
		modeStrings[0] = "VOLT";
		modeStrings[1] = "NOTE";
		modeStrings[2] = "PATT";		
		return;
	}
	void step() override;
	// Only randomize the current gate/trigger steps.
	void randomize() override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::randomize()
{
	int r, c;
	for (int s = 0; s < TROWA_SEQ_NUM_STEPS; s++) 
	{
		// randomf() - [0.0, 1.0)
		triggerState[currentPatternEditingIx][currentTriggerEditingIx][s] = voltSeq_STEP_KNOB_MIN + randomf()*(voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN);		
		r = s / TROWA_SEQ_STEP_NUM_COLS;
		c = s % TROWA_SEQ_STEP_NUM_COLS;
		this->params[GATE_PARAM + s].value = this->triggerState[currentPatternEditingIx][currentTriggerEditingIx][s];
		knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]);			
		//lights[PAD_LIGHTS + s].value = gateLights[r][c];		
	}	
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::step()
{
	bool gOn = true;
	bool pulse = false;
	bool reloadMatrix = false;
	bool valueModeChanged =  false;
	TSSequencerModuleBase::getStepInputs(&pulse, &reloadMatrix, &valueModeChanged);
	int r = 0;
	int c = 0;

	// Current output value mode	
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueMode];
	if (valueModeChanged)
	{
		modeString = currOutputValueMode->displayName;
		// Change our lights 
		for (r = 0; r < TROWA_SEQ_STEP_NUM_ROWS; r++)
		{
			for (c = 0; c < TROWA_SEQ_STEP_NUM_COLS; c++)
			{
				dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->zeroAnglePoint = currOutputValueMode->zeroPointAngle_radians;
				dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->valueMode = currOutputValueMode;//   setNumericBounds(currOutputValueMode->minDisplayValue, currOutputValueMode->maxDisplayValue);
			}
		}
	}
	lastOutputValueMode = selectedOutputValueMode;
		
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	if (reloadMatrix || valueModeChanged)
	{
		// Load this gate and/or pattern into our 4x4 matrix
		for (int s = 0; s < TROWA_SEQ_NUM_STEPS; s++) 
		{
			r = s / TROWA_SEQ_STEP_NUM_COLS;
			c = s % TROWA_SEQ_STEP_NUM_COLS;
			//padLightPtrs[r][c]->baseColor = voiceColors[currentTriggerEditingIx];
			padLightPtrs[r][c]->setColor(voiceColors[currentTriggerEditingIx]);
			gateLights[r][c] = 1.0 - stepLights[r][c];
			//lights[PAD_LIGHTS + s].value = gateLights[r][c];
			// float v = currOutputValueMode->GetOutputValue(triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]);
			// v = (v < 0) ? -1* v : v;
			// if (v)
			// {
				// v = 1.0 - stepLights[r][c];
				// // v = v / 10.0 - stepLights[r][c];
				// // if (v < 0)
					// // v *= -1;
			// }
			// else
			// {
				// v = stepLights[r][c];
			// }
			//gateLights[r][c] = v; // (v) ? 1.0f - stepLights[r][c] : 0.0f;
			
			// Load the position of our knob:
			//knobStepMatrix[r][c]->defaultValue = voltSeq_STEP_KNOB_MIN;
			//knobStepMatrix[r][c]->value = triggerState[currentPatternEditingIx][currentTriggerEditingIx][s];
			this->params[GATE_PARAM + s].value = this->triggerState[currentPatternEditingIx][currentTriggerEditingIx][s];
			knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]);			
			//knobStepMatrix[r][c]->defaultValue = currOutputValueMode->zeroValue;
			//knobStepMatrix[r][c]->dirty = true;			
			
			lights[PAD_LIGHTS + s].value = gateLights[r][c];	
				
		}		
	}
	//-- * Read the buttons
	else
	{		
		// Gate buttons (we only show one gate) - Read Inputs
		for (int s = 0; s < TROWA_SEQ_NUM_STEPS; s++) 
		{
			//if (gateTriggers[currentTriggerEditingIx][s].process(params[GATE_PARAM + s].value)) 
			{
				// Set the position of our knob
				this->triggerState[currentPatternEditingIx][currentTriggerEditingIx][s] = this->params[GATE_PARAM + s].value;
			}
			r = s / TROWA_SEQ_STEP_NUM_COLS;
			c = s % TROWA_SEQ_STEP_NUM_COLS;
			
			stepLights[r][c] -= stepLights[r][c] / lightLambda / engineGetSampleRate();
			gateLights[r][c] = stepLights[r][c];
			

			///gateLights[r][c] = (triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]) ? 1.0 - stepLights[r][c] : stepLights[r][c];
			// float v = currOutputValueMode->GetOutputValue(triggerState[currentPatternEditingIx][currentTriggerEditingIx][s]);
			// v = (v < 0) ? -1* v : v;
			// if (v)
			// {
				// v = 1 - stepLights[r][c];
				// // v = v / 10.0 - stepLights[r][c];
				// // if (v < 0)
					// // v *= -1;
			// }
			// else
			// {
				// v = stepLights[r][c];
			// }
			// gateLights[r][c] = v; // (v) ? 1.0f - stepLights[r][c] : 0.0f;
			lights[PAD_LIGHTS + s].value = gateLights[r][c];
		} // end loop through step buttons
	}
	
	// Set Outputs (16 triggers)	
	for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++) 
	{		
		float gate = (running && gOn) ? currOutputValueMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] ) : 0.0; //***********VOLTAGE OUTPUT
		outputs[GATES_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):
		//gateLightsOut[g] = (gate[g]) ? 1.0 : 0.0;
		gateLightsOut[g] = (gate < 0) ? -gate : gate;
		lights[GATE_LIGHTS + g].value = gate / currOutputValueMode->outputVoltageMax;
	}
	firstLoad = false;
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget
// Widget for the trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
voltSeqWidget::voltSeqWidget() : TSSequencerWidgetBase()
{		
	voltSeq *module = new voltSeq();
	setModule(module);

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/voltSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls();

	
	// (User) Input KNOBS ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	//int lightSize = 50 - 2*dx;
	Vec lSize = Vec(50, 50);
	int v = 0;
	ValueSequencerMode* currValueMode = module->ValueModes[module->selectedOutputValueMode];
	module->modeString = currValueMode->displayName;
	for (int r = 0; r < TROWA_SEQ_STEP_NUM_ROWS; r++) //---------THE KNOBS
	{
		for (int c = 0; c < TROWA_SEQ_STEP_NUM_COLS; c++)
		{						
			// Pad Knob:
			TS_LightedKnob* knobPtr = dynamic_cast<TS_LightedKnob*>(createParam<TS_LightedKnob>(Vec(x, y), module, 
				TSSequencerModuleBase::GATE_PARAM + r*TROWA_SEQ_STEP_NUM_COLS + c, 
				/*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX, /*default*/ voltSeq_STEP_KNOB_MIN));
			module->knobStepMatrix[r][c] = knobPtr;
			knobPtr->value = voltSeq_STEP_KNOB_MIN;
			//knobPtr->value = module->triggerState[module->currentPatternEditingIx][module->currentTriggerEditingIx][v];
			
			// Keep a reference to our pad lights so we can change the colors			
			TS_LightArc* lightPtr = dynamic_cast<TS_LightArc*>(TS_createColorValueLight<TS_LightArc>(/*pos */ Vec(x+dx, y+dx), 
				/*module*/ module,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*TROWA_SEQ_STEP_NUM_COLS + c,								
				/* size */ lSize, /* color */ module->voiceColors[module->currentTriggerEditingIx]));
			
			lightPtr->numericValue = &(knobPtr->value);
			lightPtr->currentAngle_radians = &(knobPtr->currentAngle);
			lightPtr->zeroAnglePoint = currValueMode->zeroPointAngle_radians;
			lightPtr->valueMode = currValueMode;			
			
			module->padLightPtrs[r][c] = lightPtr;			
			addChild( module->padLightPtrs[r][c] );
			
			addParam(knobPtr);
			knobPtr->dirty = true;

			x+= 59;
			v++;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through 4x4 grid

	return;
}
