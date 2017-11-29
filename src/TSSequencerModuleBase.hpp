#ifndef TROWASOFT_MODULE_TSSEQUENCERBASE_HPP
#define TROWASOFT_MODULE_TSSEQUENCERBASE_HPP

#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include <chrono>
#include "TSTempoBPM.hpp"


#define TROWA_SEQ_NUM_TRIGS		16	// Num of triggers/voices
#define TROWA_SEQ_NUM_STEPS		16  // Num of steps per gate/voice
// We only show 4x4 grid of steps at time.
#define TROWA_SEQ_STEP_NUM_ROWS	4	// Num of rows for display of the Steps (single Gate displayed at a time)
#define TROWA_SEQ_STEP_NUM_COLS	(TROWA_SEQ_NUM_STEPS/TROWA_SEQ_STEP_NUM_ROWS)

#define TROWA_SEQ_NUM_MODES		3
#define TROWASEQ_STEPS_MIN_V	TROWASEQ_PATTERN_MIN_V   // Min voltage input / output for controlling # steps
#define TROWASEQ_STEPS_MAX_V	TROWASEQ_PATTERN_MAX_V   // Max voltage input / output for controlling # steps
#define TROWA_SEQ_BPM_KNOB_MIN		-2		// Was -2 to +6
#define TROWA_SEQ_BPM_KNOB_MAX		 6
#define TROWA_SEQ_SWING_ADJ_MIN			-0.5
#define TROWA_SEQ_SWING_ADJ_MAX		     0.5
#define TROWA_SEQ_SWING_STEPS		4
// 0 WILL BE NO SWING

#define TROWA_SEQ_COPY_GATEIX_ALL		-1 // To copy all gates/triggers in the selected target Pattern

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerModuleBase
// Sequencer Base Class
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSequencerModuleBase : Module {
	enum ParamIds {
		// BPM Knob
		BPM_PARAM,
		// Run toggle		
		RUN_PARAM,
		// Reset Trigger (Momentary)		
		RESET_PARAM,
		STEPS_PARAM,
		SELECTED_PATTERN_PLAY_PARAM,  // What pattern we are playing
		SELECTED_PATTERN_EDIT_PARAM,  // What pattern we are editing
		SELECTED_GATE_PARAM,	 // Which gate is selected for editing		
		SELECTED_OUTPUT_VALUE_MODE_PARAM,     // Which value mode we are doing	
		SWING_ADJ_PARAM, // Amount of swing adjustment (-0.1 to 0.1)
		COPY_PATTERN_PARAM, // Copy the current editting Pattern
		COPY_GATE_PARAM, // Copy the current GATE/trigger in the current Pattern only.
		PASTE_PARAM, // Paste what is on our clip board to the now current editing.
		SELECTED_BPM_MULT_IX_PARAM, // Selected index into our BPM calculation multipliers (for 1/4, 1/8, 1/8T, 1/16 note calcs)
		GATE_PARAM,
		// We only show 4x4 (one gate at a time)
		//NUM_PARAMS = GATE_PARAM + TROWA_SEQ_NUM_STEPS // Num Steps is not fixed at compile time anymore
		NUM_PARAMS = GATE_PARAM // Add the number of steps separately...
	};
	enum InputIds {
		// BPM Input
		BPM_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		SELECTED_PATTERN_PLAY_INPUT,  // What pattern we are editing and/or playing		
		SELECTED_PATTERN_EDIT_INPUT,  // What pattern we are editing and/or playing
		SELECTED_GATE_INPUT,
		NUM_INPUTS
	};
	// Each of the 16 voices need a gate output
	enum OutputIds {
		GATES_OUTPUT,
		NUM_OUTPUTS = GATES_OUTPUT + TROWA_SEQ_NUM_TRIGS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		COPY_PATTERN_LIGHT,
		COPY_GATE_LIGHT,
		PASTE_LIGHT,
		SELECTED_BPM_MULT_IX_LIGHT,
		GATE_LIGHTS,
		PAD_LIGHTS = GATE_LIGHTS + TROWA_SEQ_NUM_TRIGS,
		// Num Steps is not fixed at compile time anymore
		//PAD_LIGHTS,	// One light per step
		//GATE_LIGHTS = PAD_LIGHTS + TROWA_SEQ_NUM_STEPS,
		//NUM_LIGHTS = GATE_LIGHTS + TROWA_SEQ_NUM_TRIGS
		NUM_LIGHTS = PAD_LIGHTS // Add the number of steps separately...
	};

	// If this module is running.
	bool running = true;
	SchmittTrigger clockTrigger; 		// for external clock
	SchmittTrigger runningTrigger;		// Detect running btn press
	SchmittTrigger resetTrigger;		// Detect reset btn press
	float realPhase = 0.0;
	int index = 0; // Index into the sequence (step)
	
	enum GateMode : short {
		TRIGGER = 0,
		RETRIGGER = 1,
		CONTINUOUS = 2,
	};
	GateMode gateMode = TRIGGER;
	PulseGenerator gatePulse;
	
	enum ValueMode : short {
		VALUE_TRIGGER = 0,
		VALUE_RETRIGGER = 1,
		VALUE_CONTINUOUS = 2,	
		VALUE_VOLT = 0,
		VALUE_MIDINOTE = 1,		
		VALUE_PATTERN = 2
	};
	// Selected output value mode.
	ValueMode selectedOutputValueMode = VALUE_TRIGGER;
	ValueMode lastOutputValueMode = VALUE_TRIGGER;
	
	int maxSteps = 16;
	int numRows = 4;
	int numCols = 4;
	//float triggerState[TROWASEQ_NUM_PATTERNS][TROWA_SEQ_NUM_TRIGS][TROWA_SEQ_NUM_STEPS]={};
	float * triggerState[TROWASEQ_NUM_PATTERNS][TROWA_SEQ_NUM_TRIGS];

	int currentPatternEditingIx = 0; 	// Index of which pattern we are playing
	int currentPatternPlayingIx = 0; 	// Index of which pattern we are editing 
	int currentTriggerEditingIx = 0; 	// Index of which get is currently displayed/edited.
	
	int currentNumberSteps = TROWA_SEQ_NUM_STEPS; // The current number of steps to play
	float currentBPM = 0.0f; // Calculated current BPM 
	bool lastStepWasExternalClock = false; // If the last step was the external clock
	std::chrono::high_resolution_clock::time_point lastExternalStepTime;
	
	// Lights
	// We only show 4x4 (16 steps) for one gate at a time.
	//float stepLights[TROWA_SEQ_STEP_NUM_ROWS][TROWA_SEQ_STEP_NUM_COLS];
	//float gateLights[TROWA_SEQ_STEP_NUM_ROWS][TROWA_SEQ_STEP_NUM_COLS];	
	float** stepLights;
	float** gateLights;	
	
	// Default values for our pads/knobs:
	float defaultStateValue = 0.0;
	
	// References to our pad lights
	//ColorValueLight * padLightPtrs[TROWA_SEQ_STEP_NUM_ROWS][TROWA_SEQ_STEP_NUM_COLS];
	ColorValueLight*** padLightPtrs;
	
	// Output lights (for triggers/gate jacks)
	float gateLightsOut[TROWA_SEQ_NUM_TRIGS]; 
	
	NVGcolor voiceColors[TROWA_SEQ_NUM_TRIGS] = { 
		COLOR_TS_RED, COLOR_DARK_ORANGE, COLOR_YELLOW, COLOR_TS_GREEN,
		COLOR_CYAN, COLOR_TS_BLUE, COLOR_PURPLE, COLOR_PINK,
		COLOR_TS_RED, COLOR_DARK_ORANGE, COLOR_YELLOW, COLOR_TS_GREEN,
		COLOR_CYAN, COLOR_TS_BLUE, COLOR_PURPLE, COLOR_PINK
	};
	
	// Swing ////////////////////////////////
	float swingAdjustment = 0.0; // Amount of swing adjustment (i.e. -0.1 to 0.1)
	const int swingResetSteps = TROWA_SEQ_SWING_STEPS; // These many steps need to be adjusted.
	float swingAdjustedPhase = 0.0;
	int swingRealSteps = 0;
	
	// Copy & Paste /////////////////////////
	int copySourcePatternIx = -1;
	int copySourceGateIx = TROWA_SEQ_COPY_GATEIX_ALL; // Which gate/trigger we are copying, -1 for all
	//float copyBuffer[TROWA_SEQ_NUM_TRIGS][TROWA_SEQ_NUM_STEPS];
	float* copyBuffer[TROWA_SEQ_NUM_TRIGS];
	
	SchmittTrigger copyPatternTrigger;
	SchmittTrigger copyGateTrigger;
	SchmittTrigger pasteTrigger;
	TS_LightString* pasteLight;
	ColorValueLight* copyPatternLight;
	ColorValueLight* copyGateLight;
	
	// BPM Calculation //////////////
	// Index into the array BPMOptions
	int selectedBPMNoteIx = 1; // 1/8th
	SchmittTrigger selectedBPMNoteTrigger;
	
	
	// Mode /////////////////////////	
	// The mode string.
	const char* modeString;
	const char* modeStrings[3]; // Mode strings

	bool firstLoad = false;		
	const float lightLambda = 0.05;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSequencerModuleBase()
	// Instantiate the abstract base class.
	// @numSteps: (IN) Maximum number of steps
	// @numRows: (IN) The number of rows (for layout).
	// @numCols: (IN) The number of columns (for layout).
	// @numRows * @numCols = @numSteps
	// @defStateVal : (IN) The default state value (i.e. 0/false for a boolean step sequencer or whatever float value you want).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSequencerModuleBase(/*in*/ int numSteps, /*in*/ int numRows, /*in*/ int numCols, /*in*/ float defStateVal) : Module(NUM_PARAMS + numSteps, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS + numSteps) 
	{
		lastStepWasExternalClock = false;
		defaultStateValue = defStateVal;
		currentTriggerEditingIx = 0;
		currentPatternEditingIx = 0;
		currentPatternPlayingIx = 0;
		// Number of steps in not static at compile time anymore...
		maxSteps = numSteps; // Num Steps may vary now up to 64
		currentNumberSteps = maxSteps; 
		this->numRows = numRows;
		this->numCols = numCols;

		stepLights = new float*[numRows];
		gateLights = new float*[numRows];
		padLightPtrs = new ColorValueLight**[numRows];
		
		for (int r = 0; r < numRows; r++)
		{
			stepLights[r] = new float[numCols];
			gateLights[r] = new float[numCols];
			padLightPtrs[r] = new ColorValueLight*[numCols];
			for (int c = 0; c < numCols; c++)
			{
				stepLights[r][c] = 0;
				gateLights[r][c] = 0;
			}
		}		
		for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++)
		{				
			copyBuffer[g] = new float[maxSteps];
		}			
		
		for (int p = 0;  p < TROWASEQ_NUM_PATTERNS; p++)
		{
			for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++)
			{				
				triggerState[p][g] = new float[maxSteps];
				for (int s = 0; s < maxSteps; s++)
				{
					triggerState[p][g][s] = defaultStateValue;
				}
			}			
		}		
		modeStrings[0] = "TRIG";
		modeStrings[1] = "RTRG";
		modeStrings[2] = "CONT";	
		
		copySourcePatternIx = -1;
		copySourceGateIx = TROWA_SEQ_COPY_GATEIX_ALL; // Which trigger we are copying, -1 for all		
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Delete our goodies.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	~TSSequencerModuleBase()
	{
		for (int r = 0; r < numRows; r++)
		{
			delete [] stepLights[r];
			delete [] gateLights[r];
			delete [] padLightPtrs[r];
		}
		delete [] stepLights; stepLights = NULL;
		delete [] gateLights; gateLights = NULL;
		delete [] padLightPtrs;	padLightPtrs = NULL;
		
		for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++)
		{				
			delete [] copyBuffer[g];
			copyBuffer[g] = NULL; // We should be totally dead & unreferenced anyway, so I'm not sure we have NULL our ptrs???
		}
		
		for (int p = 0;  p < TROWASEQ_NUM_PATTERNS; p++)
		{
			for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++)
			{				
				delete [] triggerState[p][g];
				triggerState[p][g] = NULL;
			}
		}
		return;
	}
	
	// Get the inputs for this step.
	void getStepInputs(bool* pulse, bool* reloadMatrix, bool* valueModeChanged);
	// Paste the clipboard pattern and/or specific gate to current selected pattern and/or gate.
	bool paste();
	// Copy the contents:
	void copy(int patternIx, int gateIx);
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize(void)
	// Only randomize the current gate/trigger steps.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void randomize() override
	{
		for (int s = 0; s < maxSteps; s++) 
		{
			triggerState[currentPatternEditingIx][currentTriggerEditingIx][s] = (randomf() > 0.5);		
		}	
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// clearClipboard(void)
	// Shallow clear of clipboard and reset the Copy/Paste lights.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	void clearClipboard()
	{
		copySourcePatternIx = -1;
		copySourceGateIx = TROWA_SEQ_COPY_GATEIX_ALL; // Which trigger we are copying, -1 for all		
		lights[COPY_GATE_LIGHT].value = 0;		
		pasteLight->setColor(COLOR_WHITE); // Return the paste light to white
		lights[COPY_PATTERN_LIGHT].value = 0;		
		lights[PASTE_LIGHT].value = 0;			
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// toJson(void)
	// Save our junk to json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));
		
		// Current Items:
		json_object_set_new(rootJ, "currentPatternEditIx", json_integer((int) currentPatternEditingIx));
		json_object_set_new(rootJ, "currentTriggerEditIx", json_integer((int) currentTriggerEditingIx));
		// The current output / knob mode.
		json_object_set_new(rootJ, "selectedOutputValueMode", json_integer((int) selectedOutputValueMode));
		// Current BPM calculation note (i.e. 1/4, 1/8, 1/8T, 1/16)
		json_object_set_new(rootJ, "selectedBPMNoteIx",  json_integer((int) selectedBPMNoteIx));
		
		// triggers
		json_t *triggersJ = json_array();
		for (int p = 0; p < TROWASEQ_NUM_PATTERNS; p++)
		{
			for (int t = 0; t < TROWA_SEQ_NUM_TRIGS; t++)
			{
				for (int s = 0; s < maxSteps; s++)
				{
					json_t *gateJ = json_real((float) triggerState[p][t][s]);
					json_array_append_new(triggersJ, gateJ);					
				} // end for (steps)
			} // end for (triggers)
		} // end for (patterns)
		json_object_set_new(rootJ, "triggers", triggersJ);

		// gateMode
		json_t *gateModeJ = json_integer((int) gateMode);
		json_object_set_new(rootJ, "gateMode", gateModeJ);

		return rootJ;
	} // end toJson()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// fromJson(void)
	// Read in our junk from json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void fromJson(json_t *rootJ) override {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);
		
		// Current Items:
		json_t *currJ = NULL;
		currJ = json_object_get(rootJ, "currentPatternEditIx");
		if (currJ)
			currentPatternEditingIx = json_integer_value(currJ);
		currJ = json_object_get(rootJ, "currentTriggerEditIx");
		if (currJ)
			currentTriggerEditingIx = json_integer_value(currJ);
		currJ = json_object_get(rootJ, "selectedOutputValueMode");
		if (currJ)
		{			
			selectedOutputValueMode = static_cast<ValueMode>( json_integer_value(currJ) );
			modeString = modeStrings[selectedOutputValueMode];
		}
		// Current BPM calculation note (i.e. 1/4, 1/8, 1/8T, 1/16)
		currJ = json_object_get(rootJ, "selectedBPMNoteIx");
		if (currJ)
			selectedBPMNoteIx = json_integer_value(currJ);
		
		// triggers
		json_t *triggersJ = json_object_get(rootJ, "triggers");
		if (triggersJ)
		{
			int i = 0;
			for (int p = 0; p < TROWASEQ_NUM_PATTERNS; p++)
			{
				for (int t = 0; t < TROWA_SEQ_NUM_TRIGS; t++)
				{
					for (int s = 0; s < maxSteps; s++)
					{
						json_t *gateJ = json_array_get(triggersJ, i++);
						if (gateJ)
							triggerState[p][t][s] = (float)json_real_value(gateJ);					
					} // end for (steps)
				} // end for (triggers)
			} // end for (patterns)			
		}

		// gateMode
		json_t *gateModeJ = json_object_get(rootJ, "gateMode");
		if (gateModeJ)
			gateMode = (GateMode)json_integer_value(gateModeJ);
		return;
	} // end fromJson()
}; // end struct TSSequencerModuleBase

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqDisplay
// A top digital display for trowaSoft sequencers.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSeqDisplay : TransparentWidget {
	TSSequencerModuleBase *module;
	std::shared_ptr<Font> font;
	std::shared_ptr<Font> labelFont;
	int fontSize;
	char messageStr[TROWA_DISP_MSG_SIZE]; // tmp buffer for our strings.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqDisplay(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSeqDisplay() {
		font = Font::load(assetPlugin(plugin, "res/Fonts/Digital dream Fat.ttf"));
		labelFont = Font::load(assetPlugin(plugin, "res/Fonts/ZeroesThree-Regular.ttf"));
		fontSize = 12;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';		
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void draw(/*in*/ NVGcontext *vg) override {
		int currPlayPattern = module->currentPatternPlayingIx + 1;
		int currEditPattern = module->currentPatternEditingIx + 1;
		int currentGate = module->currentTriggerEditingIx + 1;
		int currentNSteps = module->currentNumberSteps;
		float currentBPM = module->currentBPM;
		
		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.5);

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
		

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		NVGcolor currColor = module->voiceColors[module->currentTriggerEditingIx];
		
		int y1 = 42;
		int y2 = 27;
		int dx = 0;
		int x = 0;
		int spacing = 61;
				
		nvgTextAlign(vg, NVG_ALIGN_CENTER);

		// Current Playing Pattern
		nvgFillColor(vg, textColor);
		x = 5 + 21;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);		
		nvgText(vg, x, y1, "PATT", NULL);
		sprintf(messageStr, "%02d", currPlayPattern);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);		
		nvgText(vg, x + dx, y2, messageStr, NULL);
		
		// Current Playing Speed
		nvgFillColor(vg, textColor);
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);				
		//nvgText(vg, x, y1, "BPM", NULL);		
		sprintf(messageStr, "BPM/%s", BPMOptions[module->selectedBPMNoteIx]->label);		
		nvgText(vg, x, y1, messageStr, NULL);		
		// // BPM Note:
		// nvgFontSize(vg, fontSize * 0.6); // Tiny font
		// nvgTextLetterSpacing(vg, 1.0);
		// nvgText(vg, x + 14, y1 - 1, BPMOptions[module->selectedBPMNoteIx]->label, NULL);						
		// nvgTextLetterSpacing(vg, 2.5);						
		if (module->lastStepWasExternalClock)
		{
			sprintf(messageStr, "%s", "CLK");
		}
		else
		{
			sprintf(messageStr, "%03.0f", currentBPM);
		}
		nvgFontFaceId(vg, font->handle);
		nvgFontSize(vg, fontSize * 1.5); // Large font		
		nvgText(vg, x + dx, y2, messageStr, NULL);
		
		
		// Current Playing # Steps
		nvgFillColor(vg, textColor);
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);		
		nvgText(vg, x, y1, "LENG", NULL);
		sprintf(messageStr, "%02d", currentNSteps);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);				
		nvgText(vg, x + dx, y2, messageStr, NULL);
		
		// Current Mode:
		nvgFillColor(vg, nvgRGB(0xda, 0xda, 0xda));
		x += spacing + 5;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);						
		nvgText(vg, x, y1, "MODE", NULL);
		nvgFontSize(vg, fontSize);	// Small font
		if (module->modeString != NULL)
		{
			nvgFontFaceId(vg, font->handle);					
			//nvgText(vg, x + dx -6, y2, module->modeString, NULL);					
			nvgText(vg, x + dx, y2, module->modeString, NULL);			
		}

		nvgTextAlign(vg, NVG_ALIGN_CENTER);

		// Current Edit Pattern
		nvgFillColor(vg, textColor);
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);						
		nvgText(vg, x, y1, "PATT", NULL);
		sprintf(messageStr, "%02d", currEditPattern);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);		
		nvgText(vg, x + dx, y2, messageStr, NULL);

		// Current Edit Gate/Trigger
		nvgFillColor(vg, currColor); // Match the Gate/Trigger color
		x += spacing;
		nvgFontSize(vg, fontSize); // Small font
		nvgFontFaceId(vg, labelFont->handle);						
		nvgText(vg, x, y1, "CHNL", NULL);
		sprintf(messageStr, "%02d", currentGate);
		nvgFontSize(vg, fontSize * 1.5);	// Large font
		nvgFontFaceId(vg, font->handle);				
		nvgText(vg, x + dx, y2, messageStr, NULL);
		
		// [[[[[[[[[[[[[[[[ EDIT Box Group ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
		nvgTextAlign(vg, NVG_ALIGN_LEFT);		
		NVGcolor groupColor = nvgRGB(0xDD, 0xDD, 0xDD);
		nvgFillColor(vg, groupColor);
		int labelX = 297;
		x = labelX; // 289
		nvgFontSize(vg, fontSize-5); // Small font
		nvgFontFaceId(vg, labelFont->handle);				
		nvgText(vg, x, 8, "EDIT", NULL);
				
		// Edit Label Line ---------------------------------------------------------------
		nvgBeginPath(vg);
		// Start top to the left of the text "Edit"
		int y = 5;
		nvgMoveTo(vg, /*start x*/ x - 3, /*start y*/ y);// Starts new sub-path with specified point as first point.s
		x = 256;// x - 35;//xOffset + 3 * spacing - 3 + 60;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to Left (Line Start)
		
		x = labelX + 22;
		y = 5;
		nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Right of "Edit"
		x = box.size.x - 6;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // RHS of box

		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, groupColor);
		nvgStroke(vg);
		
		// [[[[[[[[[[[[[[[[ PLAYBACK Box Group ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
		groupColor = nvgRGB(0xEE, 0xEE, 0xEE);
		nvgFillColor(vg, groupColor);
		labelX = 64;
		x = labelX;
		nvgFontSize(vg, fontSize-5); // Small font
		nvgText(vg, x, 8, "PLAYBACK", NULL);
				
		// Play Back Label Line ---------------------------------------------------------------
		nvgBeginPath(vg);
		// Start top to the left of the text "Play"
		y = 5;
		nvgMoveTo(vg, /*start x*/ x - 3, /*start y*/ y);// Starts new sub-path with specified point as first point.s
		x = 6;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to the left
		
		x = labelX+49; 
		y = 5;
		nvgMoveTo(vg, /*x*/ x, /*y*/ y); // To the Right of "Playback"
		x = 165; //x + 62 ;
		nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go Right 
		
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, groupColor);
		nvgStroke(vg);
		return;
	} // end draw()
}; // end struct TSSeqDisplay

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSeqLabelArea
// Draw labels on our sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSeqLabelArea : TransparentWidget {
	TSSequencerModuleBase *module;
	std::shared_ptr<Font> font;
	int fontSize;
	bool drawGridLines = false;
	char messageStr[TROWA_DISP_MSG_SIZE];
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSeqLabelArea()
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSeqLabelArea() {
		font = Font::load(assetPlugin(plugin, "res/Fonts/ZeroesThree-Regular.ttf"));
		fontSize = 13;
		for (int i = 0; i < TROWA_DISP_MSG_SIZE; i++)
			messageStr[i] = '\0';		
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// draw()
	// @vg : (IN) NVGcontext to draw on
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void draw(NVGcontext *vg) override {		
		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);

		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(vg, textColor);
		nvgFontSize(vg, fontSize); 
		
		/// MAKE LABELS HERE
		int x = 45;
		int y = 163;
		int dy = 28;
		
		// Selected Pattern Playback:
		nvgText(vg, x, y, "PAT", NULL);

		// Clock		
		y += dy;
		nvgText(vg, x, y, "BPM ", NULL);		

		// Steps
		y += dy;
		nvgText(vg, x, y, "LNG", NULL);		
		
		// Ext Clock 
		y += dy;
		nvgText(vg, x, y, "CLK", NULL);
		
		// Reset
		y += dy;		
		nvgText(vg, x, y, "RST", NULL);
		
		// Outputs
		nvgFontSize(vg, fontSize * 0.95);
		x = 320;
		y = 350;		
		nvgText(vg, x, y, "OUTPUTS", NULL);
		
		// Copy btn labels
		nvgFontSize(vg, fontSize * 0.6);
		x = 300;
		y = 103;		
		nvgText(vg, x, y, "CPY", NULL);
		x = 362;		
		nvgText(vg, x, y, "CPY", NULL);
		// BPM divisor/note label:
		x = 118;		
		nvgText(vg, x, y, "DIV", NULL);
		
		
		if (drawGridLines)
		{
			NVGcolor gridColor = nvgRGB(0x66, 0x66, 0x66);
			nvgBeginPath(vg);
			x = 80;
			y = 228;
			nvgMoveTo(vg, /*start x*/ x, /*start y*/ y);// Starts new sub-path with specified point as first point
			x += 225;			
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to the left
			
			nvgStrokeWidth(vg, 1.0);
			nvgStrokeColor(vg, gridColor);
			nvgStroke(vg);

						
			// Vertical
			nvgBeginPath(vg);
			x = 192;
			y = 117;
			nvgMoveTo(vg, /*start x*/ x, /*start y*/ y);// Starts new sub-path with specified point as first point
			y += 225;			
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Go to the left			
			
			nvgStrokeWidth(vg, 1.0);
			nvgStrokeColor(vg, gridColor);
			nvgStroke(vg);

		}
		
		return;
	} // end draw()
}; // end struct TSSeqLabelArea
#endif