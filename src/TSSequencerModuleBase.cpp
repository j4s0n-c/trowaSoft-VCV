#include <chrono>
#include <string.h>
#include <exception>
#include "trowaSoft.hpp"
//#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "TSOSCSequencerListener.hpp"
#include "TSOSCSequencerOutputMessages.hpp"
#include "TSOSCCommunicator.hpp"
#include "TSSequencerWidgetBase.hpp"
#include "TSParamQuantity.hpp"



// Static Variables:

// For now all these colors are the same...
const NVGcolor TSSequencerModuleBase::COPY_PATTERN_COLOR = nvgRGBAf(0.7, 0.7, 0.7, 0.8);
const NVGcolor TSSequencerModuleBase::RUNNING_COLOR = nvgRGBAf(0.7, 0.7, 0.7, 0.8);
const NVGcolor TSSequencerModuleBase::RESET_COLOR = nvgRGBAf(0.7, 0.7, 0.7, 0.8);

RandStructure TSSequencerModuleBase::RandomPatterns[TROWA_SEQ_NUM_RANDOM_PATTERNS] = {
	{ 1,{ 0 } },
	{ 2,{ 0, 1 } },
	{ 2,{ 0,0,0,1 } },
	{ 2,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } },
	{ 2,{ 0,0,1,1 } },
	{ 2,{ 0,1,1,1 } },
	{ 2,{ 0,1,1,0 } },
	{ 3,{ 0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,2 } },
	{ 3,{ 0,0,1,2 } },
	{ 3,{ 0,0,0,0,0,0,1,2 } },
	{ 3,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2 } },
	{ 3,{ 0,1,0,2 } },
	{ 3,{ 0,1,2,0 } },
	{ 3,{ 0,1,1,2 } },
	{ 3,{ 0,2,1,2 } },
	{ 4,{ 0,1,2,3 } },
	{ 4,{ 0,0,1,1,2,2,3,3 } },
	{ 4,{ 0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3 } },
	{ 5,{ 0,1,0,1,0,1,0,2,0,3,0,3,0,3,0,4 } },
	{ 6,{ 0,1,0,2,0,1,0,3,0,1,0,4,0,1,0,5 } },
	{ 8,{ 0,0,0,1,2,3,4,5,0,0,0,1,2,3,6,7 } },
	{ 9,{ 0,1,0,2,0,1,0,3,0,1,0,4,5,6,7,8 } },
	{ 10,{ 0,1,1,2,3,4,5,6,0,1,1,2,3,7,8,9 } },
	{ 10,{ 0,1,1,2,3,4,5,6,1,1,1,2,3,7,8,9 } },
	{ 10,{ 0,1,2,3,4,5,6,7,0,1,2,3,4,5,8,9 } },
	{ 10,{ 0,1,2,3,4,5,6,7,0,1,8,3,4,5,6,9 } },
	{ 11,{ 0,1,2,3,4,5,6,7,8,1,2,3,4,5,9,10 } }
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerModuleBase()
// Instantiate the abstract base class.
// @numSteps: (IN) Maximum number of steps
// @numRows: (IN) The number of rows (for layout).
// @numCols: (IN) The number of columns (for layout).
// @numRows * @numCols = @numSteps
// @defStateVal : (IN) The default state value (i.e. 0/false for a boolean step sequencer or whatever float value you want).
// @defChannelValMode : (IN) The default value mode for channels. (v1.0.4 now that VALUE_TRIGGER and VALUE_VOLT are unique).
// @enablePatternSequencing: (IN) (Opt'l) Enable pattern sequencing for this type of module. Default is false.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSSequencerModuleBase::TSSequencerModuleBase(/*in*/ int numSteps, /*in*/ int numRows, /*in*/ int numCols, /*in*/ float defStateVal, 
	/*in*/ TSSequencerModuleBase::ValueMode defChannelValMode, bool enablePatternSequencing)
{
	// # Parameters and # Lights need to be increased by numSteps.
	// Also, if pattern sequencing is a feature, then they should be increased again by numSteps.
	int numParams = NUM_PARAMS + numSteps;
	int numLights = NUM_LIGHTS + numSteps;
	if (enablePatternSequencing)
	{
		numParams += numSteps;
		PATTERN_SEQ_PARAM_START = NUM_PARAMS + numSteps;
		
		numLights += numSteps;
		PATTERN_SEQ_LIGHT_START = NUM_LIGHTS + numSteps;;		
	}
	config(numParams, NUM_INPUTS, NUM_OUTPUTS, numLights);
	
	defaultChannelValueMode = defChannelValMode;
	defaultStateValue = defStateVal;	
	// Number of steps in not static at compile time anymore...
	maxSteps = numSteps; // Num Steps may vary now up to 64	
	
	// INTERNAL PATTERN SEQUENCING //////
	// If this module has internal pattern sequencing (no to most, only tsSeq should have it for now).
	allowPatternSequencing = enablePatternSequencing;	
	// Which pattern to play (for pattern sequencing).
	patternPlayHeadIx = -1;
	// Flag if pattern sequencing is on.
	patternSequencingOn = false;	
	// Show the pattern seqeuncing configuration.
	showPatternSequencingConfig = false;	
	lastShowPatternSequencingConfig = false;
	if (allowPatternSequencing)
	{
		// The patterns to play.
		patternPlayHeadIx = 0;
		patternData = new short[maxSteps];
		for (int p = 0; p < maxSteps; p++)
		{
			patternData[p] = p % TROWA_SEQ_NUM_PATTERNS;
			// Values will be indices (0-63). Display should be 1-64.
			// Default to 0-63 (play in order).
			configParam(PATTERN_SEQ_PARAM_START + p, /*min*/ 0.0f, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ patternData[p], 
				/*label*/ "Sequence " + std::to_string(p+1) + " Pattern #", /*unit*/ "", /*displayBase*/ 0.0f, /*displayMult*/ 1.0f, /*displayOffset*/ 1);			
		}
		numPatternsInSequence = maxSteps;
	}
	else
	{
		// The patterns to play.
		patternData = NULL;		
	}
	
	// Configure Parameters:
	// BPM Knob
	// BPM_PARAM,
	configParam(/*id*/ TSSequencerModuleBase::ParamIds::BPM_PARAM, /*min*/ TROWA_SEQ_BPM_KNOB_MIN, /*max*/ TROWA_SEQ_BPM_KNOB_MAX, /*def*/ (TROWA_SEQ_BPM_KNOB_MAX + TROWA_SEQ_BPM_KNOB_MIN) / 2, 
		/*label*/ "Tempo", /*unit*/ " BPM (1/" + std::string(BPMOptions[selectedBPMNoteIx]->label) + ")" , /*displayBase*/ 2.0f, /*displayMult*/ BPMOptions[selectedBPMNoteIx]->multiplier );
	// Run toggle		
	// RUN_PARAM,
	configButton(TSSequencerModuleBase::ParamIds::RUN_PARAM, "Run");
	// Reset Trigger (Momentary)		
	// RESET_PARAM,
	configButton(TSSequencerModuleBase::ParamIds::RESET_PARAM, "Reset");
	// Step length
	// STEPS_PARAM,
	configParam(TSSequencerModuleBase::ParamIds::STEPS_PARAM, 1.0, numSteps, numSteps, "Step Length");	
	paramQuantities[TSSequencerModuleBase::ParamIds::STEPS_PARAM]->snapEnabled = true;	
	// SELECTED_PATTERN_PLAY_PARAM,  // What pattern we are playing
	configParam(TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_PLAY_PARAM, /*min*/ 0.0f, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ 0.0f, 
		/*label*/ "Play Pattern", /*unit*/ std::string(""), /*displayBase*/ 0.0f, /*displayMult*/ 1.0f, /*displayOffset*/ 1);
	paramQuantities[TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_PLAY_PARAM]->snapEnabled = true;			
	// SELECTED_PATTERN_EDIT_PARAM,  // What pattern we are editing
	configParam(TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_EDIT_PARAM, /*min*/ 0.0f, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1, /*default value*/ 0.0f,
		/*label*/ "Edit Pattern", /*unit*/ std::string(""), /*displayBase*/ 0.0f, /*displayMult*/ 1.0f, /*displayOffset*/ 1);	
	paramQuantities[TSSequencerModuleBase::ParamIds::SELECTED_PATTERN_EDIT_PARAM]->snapEnabled = true;					
	// SELECTED_CHANNEL_PARAM,	 // Which gate is selected for editing		
	configParam(TSSequencerModuleBase::ParamIds::SELECTED_CHANNEL_PARAM, /*min*/ 0.0, /*max*/ TROWA_SEQ_NUM_CHNLS - 1, /*default value*/ 0, 
		/*label*/ "Edit Channel", /*unit*/ std::string(""), /*displayBase*/ 0.0f, /*displayMult*/ 1.0f, /*displayOffset*/ 1);
	paramQuantities[TSSequencerModuleBase::ParamIds::SELECTED_CHANNEL_PARAM]->snapEnabled = true;							
	// SELECTED_OUTPUT_VALUE_MODE_PARAM,     // Which value mode we are doing	
	configParam<TS_ParamQuantityEnum>(TSSequencerModuleBase::ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM, /*min*/ 0, /*max*/ TROWA_SEQ_NUM_MODES - 1, /*default*/ TSSequencerModuleBase::ValueMode::VALUE_TRIGGER, /*label*/ "Mode");
	dynamic_cast<TS_ParamQuantityEnum*>(this->paramQuantities[TSSequencerModuleBase::ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM])->valMult = 1000.f;
	// SWING_ADJ_PARAM, // Amount of swing adjustment (-0.1 to 0.1)
	configParam(TSSequencerModuleBase::ParamIds::SWING_ADJ_PARAM, -0.1, 0.1, 0.0); // Currently not used.		
	// COPY_PATTERN_PARAM, // Copy the current editing Pattern
	//configParam(TSSequencerModuleBase::ParamIds::COPY_PATTERN_PARAM, 0, 1, 0, /*label*/ "Copy Pattern");
	configButton(TSSequencerModuleBase::ParamIds::COPY_PATTERN_PARAM, /*label*/ "Copy Pattern");	
	// COPY_CHANNEL_PARAM, // Copy the current Channel/gate/trigger in the current Pattern only.
	//configParam(TSSequencerModuleBase::ParamIds::COPY_CHANNEL_PARAM, 0, 1, 0, /*label*/ "Copy Channel");
	configButton(TSSequencerModuleBase::ParamIds::COPY_CHANNEL_PARAM, /*label*/ "Copy Channel");	
	// PASTE_PARAM, // Paste what is on our clip board to the now current editing.
	//configParam(TSSequencerModuleBase::ParamIds::PASTE_PARAM, 0.0, 1.0, 0.0, "Paste");
	configButton(TSSequencerModuleBase::ParamIds::PASTE_PARAM, "Paste");	
	// SELECTED_BPM_MULT_IX_PARAM, // Selected index into our BPM calculation multipliers (for 1/4, 1/8, 1/8T, 1/16 note calcs)
	//configParam(TSSequencerModuleBase::ParamIds::SELECTED_BPM_MULT_IX_PARAM, 0, 1, 0, /*label*/ "Next BPM Note");
	configButton(TSSequencerModuleBase::ParamIds::SELECTED_BPM_MULT_IX_PARAM, "Next BPM Note");	
	// OSC_SAVE_CONF_PARAM, // ENABLE and Save the configuration for OSC
	//configParam(TSSequencerModuleBase::ParamIds::OSC_SAVE_CONF_PARAM, 0, 1, 0, /*label*/ "Enable/Disable OSC");	
	configButton(TSSequencerModuleBase::ParamIds::OSC_SAVE_CONF_PARAM, /*label*/ "Enable/Disable OSC");		
	// OSC_AUTO_RECONNECT_PARAM,   // Auto-reconnect OSC on load from save file.
	//configParam(TSSequencerModuleBase::ParamIds::OSC_AUTO_RECONNECT_PARAM, 0, 1, 0, /*label*/ "Auto-connect OSC");		
	configButton(TSSequencerModuleBase::ParamIds::OSC_AUTO_RECONNECT_PARAM, /*label*/ "Auto-connect OSC");			
	// OSC_SHOW_CONF_PARAM, // Configure OSC toggle
	//configParam(TSSequencerModuleBase::ParamIds::OSC_SHOW_CONF_PARAM, 0, 1, 0, "Toggle OSC Config Screen");
	configButton(TSSequencerModuleBase::ParamIds::OSC_SHOW_CONF_PARAM, "Toggle OSC Config Screen");

	// CHANNEL_PARAM, // Edit Channel/Step Buttons/Knobs	
	
	////////////////////////////////////////////////
	// INTERNAL PATTERN SEQUENCING /////////////////
	////////////////////////////////////////////////
	configButton(TSSequencerModuleBase::ParamIds::PATTERN_SEQ_SHOW_CONFIG_PARAM, /*label*/ "Show Song Mode Config");
	// configParam(TSSequencerModuleBase::ParamIds::PATTERN_SEQ_ON_PARAM, /*min*/ 0.0f, /*max*/ 1.0f, /*default value*/ 0.0f, 
		// /*label*/ "Song Mode Active", /*unit*/ std::string(""), /*displayBase*/ 0.0f, /*displayMult*/ 1.0f, /*displayOffset*/ 1);	
	configButton(TSSequencerModuleBase::ParamIds::PATTERN_SEQ_ON_PARAM, /*label*/ "Song Mode Active");		
	// Pattern Sequence Length - We can only have as many as we have steps (because of room).
	configParam(TSSequencerModuleBase::ParamIds::PATTERN_SEQ_LENGTH_PARAM, /*min*/ 1.0f, /*max*/ maxSteps, /*default value*/ 1.0f, 
		/*label*/ "Songe Mode/Pattern Sequence Length", /*unit*/ std::string(""), /*displayBase*/ 0.0f, /*displayMult*/ 1.0f, /*displayOffset*/ 0);	
	paramQuantities[TSSequencerModuleBase::ParamIds::PATTERN_SEQ_LENGTH_PARAM]->snapEnabled = true;
		
	// Copy the default colors over. ? Or just reference them. This leaves it open to customize the colors eventually.
	for (int i = 0; i < TROWA_SEQ_NUM_CHNLS; i++){
		voiceColors[i] = TSColors::CHANNEL_COLORS[i % TSColors::NUM_CHANNEL_COLORS];
		// Initialize channel modes:
		// v1.0.1 - Each channel will have its own mode.		
		// v1.0.4 - Sequencer will have different default value modes (Value modes are not unique not overriden).
		channelValueModes[i] = defaultChannelValueMode;//ValueMode::VALUE_TRIGGER;
	}
	useOSC = false;
	oscInitialized = false;
	oscBuffer = NULL;
	oscTxSocket = NULL;
	oscListener = NULL;
	oscRxSocket = NULL;
	oscNamespace = OSC_DEFAULT_NS;
	oscId = TSOSCConnector::GetId();

	for (int i = 0; i < SeqOSCOutputMsg::NUM_OSC_OUTPUT_MSGS; i++)
	{
		for (int j = 0; j < OSC_ADDRESS_BUFFER_SIZE; j++)
			oscAddrBuffer[i][j] = '\0';
	}

	prevIndex = TROWA_INDEX_UNDEFINED;

	gateTriggers = NULL;

	lastStepWasExternalClock = false;
	currentChannelEditingIx = 0;
	currentPatternEditingIx = 0;
	currentPatternPlayingIx = 0;
	currentNumberSteps = maxSteps;
	storedNumberSteps = maxSteps;
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
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
	{
		copyBuffer[g] = new float[maxSteps];
	}

	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			triggerState[p][g] = new float[maxSteps];
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[p][g][s] = defaultStateValue;
			}
		}
	}
	// modeStrings[0] = "TRIG";
	// modeStrings[1] = "RTRG";
	// modeStrings[2] = "GATE"; // CONT/GATE

	copySourcePatternIx = -1;
	copySourceChannelIx = TROWA_SEQ_COPY_CHANNELIX_ALL; // Which trigger we are copying, -1 for all		
	currentCopyChannelColor = TSColors::COLOR_WHITE;
	currentPasteColor = TSColors::COLOR_WHITE;

	initialized = false;
	firstLoad = true;
	
	// [Rack v2] Add labels for inputs and outputs
	configInput(InputIds::BPM_INPUT, "Tempo/BPM");
	configInput(InputIds::EXT_CLOCK_INPUT, "External Clock");
	configInput(InputIds::RESET_INPUT, "Reset");
	configInput(InputIds::STEPS_INPUT, "Pattern (Step) Length");
	configInput(InputIds::SELECTED_PATTERN_PLAY_INPUT, "Play Pattern");
	
	char buffer[50];
	for (int ch = 0; ch < TROWA_SEQ_NUM_CHNLS ; ch++)
	{
		sprintf(buffer, "Channel %d", ch + 1);
		configOutput(OutputIds::CHANNELS_OUTPUT + ch, buffer);		
	}
	
#if TROWA_SEQ_USE_INTERNAL_DIVISOR
	// Calculate our initial divisor and initial light lambda
	onSampleRateChange();
#endif 	
	return;
} // end TSSequencerModuleBase()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Delete our goodies.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
TSSequencerModuleBase::~TSSequencerModuleBase()
{
	initialized = false; // Stop doing stuff
	cleanupOSC();
	for (int r = 0; r < numRows; r++)
	{
		if (stepLights[r])
			delete[] stepLights[r];
		if (gateLights[r])
			delete[] gateLights[r];
	}
	if (stepLights != NULL)
	{
		delete[] stepLights; stepLights = NULL;
	}
	if (gateLights != NULL)
	{
		delete[] gateLights; gateLights = NULL;
	}
	if (valueModesSupported != NULL)
	{
		delete[] valueModesSupported; valueModesSupported = NULL;
	}
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
	{
		delete[] copyBuffer[g];
		copyBuffer[g] = NULL; // We should be totally dead & unreferenced anyway, so I'm not sure we have NULL our ptrs???
	}
	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			delete[] triggerState[p][g];
			triggerState[p][g] = NULL;
		}
	}	
	if (patternData != NULL)
	{
		delete[] patternData;
	}
	
	// Free our buffer if we had initialized it
	oscMutex.lock();
	if (oscBuffer != NULL)
	{
		free(oscBuffer);
		oscBuffer = NULL;
	}
	oscMutex.unlock();
	return;
} // end ~TSSequencerModuleBase()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Reset ALL step values to default.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::onReset()
{
	valuesChanging = true;
	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[p][c][s] = defaultStateValue;
			}
		}
	}
	// [v1.0.1] Also reset all channel value modes to default:
	for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
	{
		// [v1.0.4] Sequencers can have different default output modes.
		channelValueModes[c] = defaultChannelValueMode; //ValueMode::VALUE_TRIGGER;
	}
		
	// [v1.0.4] Reset pattern sequencing data to default:
	resetPatternSequence();
	
	/// TODO: Also clear our clipboard and turn off OSC?
	reloadEditMatrix = true;
	valuesChanging = false;
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset()
// Only the given pattern and channel.
// @patternIx: (IN) Pattern. TROWA_INDEX_UNDEFINED is all.
// @channelIx: (IN) Channel. TROWA_INDEX_UNDEFINED is all.
// @resetChannelMode: (IN)(Opt'l) Reset the channel mode also (to default like TRIG or VOLT). 
//                    Default is true.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
void TSSequencerModuleBase::reset(int patternIx, int channelIx, bool resetChannelMode)
{
	//DEBUG("reset(%d, %d) = Start Current Shown is (%d, %d)", patternIx, channelIx, currentPatternEditingIx, currentChannelEditingIx);
	if (patternIx == TROWA_INDEX_UNDEFINED)
	{
		// All patterns:
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			reset(p, TROWA_INDEX_UNDEFINED, resetChannelMode); // All channels
		}
	}
	else if (channelIx == TROWA_INDEX_UNDEFINED)
	{
		// This pattern:
		for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
		{
			reset(patternIx, c, resetChannelMode);
		}
	}
	else
	{
		valuesChanging = true;

		bool isCurrentEditChannelShown = patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx;		
		
		// Channel Modes are reset too (for example back to TRIG)
		if (resetChannelMode)
		{
			// [v1.0.4] Sequencers can have different default output modes.
			channelValueModes[channelIx] = defaultChannelValueMode;			
			
			// Reload the knob and such:
			if (channelIx == currentChannelEditingIx && selectedOutputValueMode != defaultChannelValueMode)
			{
				DEBUG("reset(%d, %d) - Set the value mode of the current edit channel to default (%s)!!!", patternIx, channelIx, modeStrings[defaultChannelValueMode]);
				// Reload the selected output value mode (TRIG, RTRIG, GATE, VOLT, NOTE, PATT)
				selectedOutputValueMode = defaultChannelValueMode;
				selectedOutputValueModeIx = getSupportedValueModeIndex(defaultChannelValueMode);
				modeString = modeStrings[defaultChannelValueMode];
				// Modify the knob 
				// [v1.0.4] (now with the index not the mode value).
				this->paramQuantities[ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM]->setValue(selectedOutputValueModeIx);
			}			
		}		
		
		// -- Reset Channel Values to Default Value --
		// Get channel specific default value
		float defVal = defaultStateValue;
		ValueSequencerMode** chModes = getValueSeqChannelModes();
		if (chModes != NULL) {
			int ix = getSupportedValueModeIndex(channelValueModes[channelIx]);
			defVal = chModes[ix]->zeroValue;
			DEBUG("reset(patt=%d, ch=%d) - Default Zero Value for Channel (Mode %s) is %3.1f", patternIx, channelIx, modeStrings[channelValueModes[channelIx]], defVal); 
		}
		for (int s = 0; s < maxSteps; s++)
		{
			triggerState[patternIx][channelIx][s] = defVal;// defaultStateValue;
			if (isCurrentEditChannelShown)
				onShownStepChange(s, defVal);
		}
		
		if (isCurrentEditChannelShown)
			reloadEditMatrix = true;
		valuesChanging = false;		
	} // end else (channel and pattern specified)		
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// resetPatternSequence()
// Reset the pattern sequence / songmode steps only.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::resetPatternSequence()
{
	// [v1.0.4] Reset pattern sequencing data to default:
	if (allowPatternSequencing && patternData != NULL)
	{
		// Default to incrementing each time.
		for (int p = 0; p < maxSteps; p++)
		{
			int val = p % TROWA_SEQ_NUM_PATTERNS;
			this->params[PATTERN_SEQ_PARAM_START + p].setValue(val);
			patternData[p] = val;
		}		
	}	
	return;
} // end resetPatternSequence()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// randomizePatternSequence()
// Randomize just the pattern sequence/song mode.
// @useStructured: (IN) Create a random sequence/pattern of random values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::randomizePatternSequence(bool useStructured)
{
	if (allowPatternSequencing && patternData != NULL)	
	{
		int val = 0;
		if (useStructured)
		{
			// Use a pattern
			// A, AB, ABBA, ABAC
			int rIx = rand() % numStructuredRandomPatterns;
			int n = RandomPatterns[rIx].numDiffVals;
			int* randVals = new int[n];
			int patternLen = RandomPatterns[rIx].pattern.size();
			// Every Channel should get its own random pattern
			// random::uniform() - [0.0, 1.0)
			///*min*/ 0.0f, /*max*/ TROWA_SEQ_NUM_PATTERNS - 1			
			for (int i = 0; i < n; i++)
			{
				randVals[i] = static_cast<int>(random::uniform() * TROWA_SEQ_NUM_PATTERNS);
			}

			for (int p = 0; p < maxSteps; p++)
			{
				val = randVals[RandomPatterns[rIx].pattern[p % patternLen]];
				this->params[PATTERN_SEQ_PARAM_START + p].setValue(val);
				patternData[p] = val;
			}
			delete[] randVals;
		} // end if random pattern/structure
		else
		{
			// Every value is random
			for (int p = 0; p < maxSteps; p++)
			{
				// random::uniform() - [0.0, 1.0)				
				val = static_cast<int>(random::uniform() * TROWA_SEQ_NUM_PATTERNS);
				this->params[PATTERN_SEQ_PARAM_START + p].setValue(val);
				patternData[p] = val;
			}
		} // end else (normal Rand -- all values random)			
	} // end if we allow pattern sequencing anyway
	return;
} // end randomizePatternSequence()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// randomize()
// @patternIx : (IN) The index into our pattern matrix (0-63). Or TROWA_INDEX_UNDEFINED for all patterns.
// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
// @useStructured: (IN) Create a random sequence/pattern of random values.
// Random all from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/8
// Structured from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/10
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::randomize(int patternIx, int channelIx, bool useStructured)
{
	if (patternIx == TROWA_INDEX_UNDEFINED)
	{
		// All patterns:
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			randomize(p, TROWA_INDEX_UNDEFINED, useStructured); // All channels
		}
	}
	else if (channelIx == TROWA_INDEX_UNDEFINED)
	{
		// This pattern:
		for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
		{
			randomize(patternIx, c, useStructured);
		}
	}
	else
	{
		valuesChanging = true;
		// -- Randomize Channel Specified --
		float val;
		bool isCurrentEditChannelShown = patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx;
		if (useStructured)
		{
			// Use a pattern
			// A, AB, ABBA, ABAC
			int rIx = rand() % numStructuredRandomPatterns;// TROWA_SEQ_NUM_RANDOM_PATTERNS;
			int n = RandomPatterns[rIx].numDiffVals;
			float* randVals = new float[n];
			int patternLen = RandomPatterns[rIx].pattern.size();
			// Every Channel should get its own random pattern
			for (int i = 0; i < n; i++)
				randVals[i] = getRandomValue(channelIx);

			for (int s = 0; s < maxSteps; s++)
			{
				val = randVals[RandomPatterns[rIx].pattern[s % patternLen]];
				triggerState[patternIx][channelIx][s] = val;
				if (isCurrentEditChannelShown)
					onShownStepChange(s, val);
			}
			delete[] randVals;
		} // end if random pattern/structure
		else
		{
			// Every value is random
			for (int s = 0; s < maxSteps; s++)
			{
				val = getRandomValue(channelIx);
				triggerState[patternIx][channelIx][s] = val;
				if (isCurrentEditChannelShown)
					onShownStepChange(s, val);
			}
		} // end else (normal Rand -- all values random)
		if (isCurrentEditChannelShown)
			reloadEditMatrix = true;
		valuesChanging = false;
	} // end else (channel and pattern specified)
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Set the OSC namespace.
// @oscNs: (IN) The namespace for OSC.
// Sets the command address strings too.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::setOSCNamespace(const char* oscNs)
{
	this->oscNamespace = oscNs;
	for (int i = 0; i < SeqOSCOutputMsg::NUM_OSC_OUTPUT_MSGS; i++)
	{
		// Create our array of output addresses based on the base format and the osc name space.
		sprintf(this->oscAddrBuffer[i], TSSeqOSCOutputFormats[i], oscNamespace.c_str());
	}
	// Add %d (all this was changed for touchOSC's limitations)
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::EditStepString], "%d");
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::EditStep], "%d");
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], "%d");
	// [touchOSC] Add some <row>/<col>
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::EditTOSC_GridStep], "%d/%d");

	return;
} // end setOSCNameSpace()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Initialize OSC on the given ip and ports.
// @ipAddress: (IN) The ip address.
// @outputPort: (IN) The output port.
// @inputPort: (IN) The input port.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::initOSC(const char* ipAddress, int outputPort, int inputPort)
{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	DEBUG("TSSequencerModuleBase::initOSC() - Initializing OSC");
#endif	
	oscMutex.lock();
	try
	{
		// Try to register these ports:
		if (TSOSCConnector::RegisterPorts(oscId, outputPort, inputPort))
		{
			oscError = false;
			this->currentOSCSettings.oscTxIpAddress = ipAddress;
			this->setOSCNamespace(this->oscNamespace.c_str());
			if (oscBuffer == NULL)
			{
				oscBuffer = (char*)malloc(OSC_OUTPUT_BUFFER_SIZE * sizeof(char));
			}
			if (oscTxSocket == NULL)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				DEBUG("TSSequencerModuleBase::initOSC() - Create TRANS socket at %s, port %d.", ipAddress, outputPort);
#endif
				oscTxSocket = new UdpTransmitSocket(IpEndpointName(ipAddress, outputPort));
				this->currentOSCSettings.oscTxPort = outputPort;
			}
			if (oscRxSocket == NULL)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				DEBUG("TSSequencerModuleBase::initOSC() - Create RECV socket at any address, port %d.", inputPort);
#endif
				oscListener = new TSOSCSequencerListener();
				oscListener->sequencerModule = this;
				oscListener->oscNamespace = this->oscNamespace;
				oscRxSocket = new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, inputPort), oscListener);
				this->currentOSCSettings.oscRxPort = inputPort;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				DEBUG("TSSequencerModuleBase::initOSC() - Starting listener thread...");
#endif
				oscListenerThread = std::thread(&UdpListeningReceiveSocket::Run, oscRxSocket);
			}
			INFO("TSSequencerModuleBase::initOSC() - OSC Initialized : %s :%d :%d", ipAddress, outputPort, inputPort);
			oscInitialized = true;
		}
		else
		{
			oscError = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("TSSequencerModuleBase::initOSC() - Ports in use already.");
#endif
		}
	}
	catch (const std::exception& ex)
	{
		oscError = true;
		WARN("TSSequencerModuleBase::initOSC() - Error initializing: %s.", ex.what());
	}
	oscMutex.unlock();
	return;
} // end initOSC()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::cleanupOSC()
{
	oscMutex.lock();
	try
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("TSSequencerModuleBase::cleanupOSC() - Cleaning up OSC");
#endif
		oscInitialized = false;
		oscError = false;
		TSOSCConnector::ClearPorts(oscId, currentOSCSettings.oscTxPort, currentOSCSettings.oscRxPort);
		if (oscRxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("TSSequencerModuleBase::cleanupOSC() - Cleaning up RECV socket.");
#endif
			oscRxSocket->AsynchronousBreak();
			oscListenerThread.join(); // Wait for him to finish
			delete oscRxSocket;
			oscRxSocket = NULL;
		}
		if (oscTxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("TSSequencerModuleBase::cleanupOSC() - Cleanup TRANS socket.");
#endif
			delete oscTxSocket;
			oscTxSocket = NULL;
		}
		//if (oscBuffer != NULL)
		//{
		//	free(oscBuffer);
		//	oscBuffer = NULL;
		//}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("TSSequencerModuleBase::cleanupOSC() - OSC cleaned");
#endif
	}
	catch (const std::exception& ex)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("TSSequencerModuleBase::cleanupOSC() - Exception caught:\n%s", ex.what());
#endif
	}
	oscMutex.unlock();
	return;
} // end cleanupOSC()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// copy()
// @patternIx : (IN) The index into our pattern matrix (0-15).
// @channelIx : (IN) The index of the channel (gate/trigger/voice) to copy if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL for all).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::copy(/*in*/ int patternIx, /*in*/ int channelIx)
{
	copySourceChannelIx = channelIx;
	copySourcePatternIx = patternIx;
	if (copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
	{
		// Copy entire pattern (all gates/triggers/voices)
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				copyBuffer[g][s] = triggerState[copySourcePatternIx][g][s];
			}
		}
	}
	else
	{
		// Copy just the channel:
		for (int s = 0; s < maxSteps; s++)
		{
			copyBuffer[copySourceChannelIx][s] = triggerState[copySourcePatternIx][copySourceChannelIx][s];
		}
	}
	return;
} // end copy()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// paste(void)
// Paste our current clipboard Pattern/Gate to the currently selected Pattern/Gate.
// @returns: True if the values were copied, false if not.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool TSSequencerModuleBase::paste()
{
	if (copySourcePatternIx < 0) // Nothing to copy
		return false;
	valuesChanging = true;
	if (copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
	{
		// Paste entire pattern (all gates/triggers/voices)
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[currentPatternEditingIx][g][s] = copyBuffer[g][s];
			}
		}
	}
	else
	{
		// Paste just the channel:
		for (int s = 0; s < maxSteps; s++)
		{
			triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = copyBuffer[copySourceChannelIx][s];
		}
	}
	valuesChanging = false;
	return true;
} // end paste()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Set a single the step value
// (i.e. this command probably comes from an external source)
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::setStepValue(int step, float val, int channel, int pattern)
{
	int r, c;
	if (channel == CURRENT_EDIT_CHANNEL_IX)
	{
		channel = currentChannelEditingIx;
	}
	if (pattern == CURRENT_EDIT_PATTERN_IX)
	{
		pattern = currentPatternEditingIx;
	}
	triggerState[pattern][channel][step] = val;
	r = step / this->numCols;
	c = step % this->numCols;
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		if (triggerState[pattern][channel][step])
		{
			gateLights[r][c] = 1.0f - stepLights[r][c];
			if (gateTriggers != NULL)
				gateTriggers[step].state = TriggerSignal::HIGH;
		}
		else
		{
			gateLights[r][c] = 0.0f; // Turn light off	
			if (gateTriggers != NULL)
				gateTriggers[step].state = TriggerSignal::LOW;
		}
		paramQuantities[ParamIds::CHANNEL_PARAM + step]->setValue(val);
	}
	oscMutex.lock();
	if (useOSC && oscInitialized)
	{
		try
		{
			char addrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 };
			// Send the result back
			if (this->oscCurrentClient == OSCClient::touchOSCClient)
			{
				int gridRow, gridCol;
				touchOSC::stepIndex_to_mcRowCol(step, numRows, numCols, &gridRow, &gridCol);
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditTOSC_GridStep], gridRow, gridCol); // Grid's /<row>/<col> to accomodate touchOSC's lack of multi-parameter support.
			}
			else
			{
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], step + 1); // Changed to /<step> to accomodate touchOSC's lack of multi-parameter support.
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("setStepValue() - Received a msg (s=%d, v=%0.2f, c=%d, p=%d), sending back (%s).",
				step, val, channel, pattern,
				addrBuff);
#endif
			osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
			oscStream << osc::BeginBundleImmediate
				<< osc::BeginMessage(addrBuff)
				<< triggerState[pattern][channel][step]
				<< osc::EndMessage
				<< osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());

		}
		catch (const std::exception &e)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("seStepValue - Error sending back msg: %s.", e.what());
#endif
		}
	}
	oscMutex.unlock();
	return;
} // end setStepValue()

#if TROWA_SEQ_USE_INTERNAL_DIVISOR
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
// onSampleRateChange()
// User changes the engine sampling rate. Adjust our internal sampling rate 
// such that we run still at least every 0.5 ms (triggers should be 1 ms).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
void TSSequencerModuleBase::onSampleRateChange() 
{
	float sampleRate = APP->engine->getSampleRate();
	IDLETHRESHOLD = 0.0005f * sampleRate;
	if (IDLETHRESHOLD > MAX_ILDETHRESHOLD)
		IDLETHRESHOLD = MAX_ILDETHRESHOLD;
	lightLambda = baseLightLambda / IDLETHRESHOLD;
#if DEUBG_TROWA_SEQ_SAMPLE_DIVISOR
	DEBUG("SampleRateChange - Rack sample rate is %6.1f kHz, ILDETHRESHOLD is %d, lightLambda is %6.5f.", sampleRate, IDLETHRESHOLD, lightLambda);
#endif		
	return;
}
#endif 


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// isNewStep()
// We advance to new step or not.
// @sampleRate : (IN) Current sample rate.
// @clockTime : (OUT) The calculated internal clock time.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
bool TSSequencerModuleBase::isNewStep(float sampleRate, float* clockTime)
{
	bool nextStep = false;
	
	// Run
	if (runningTrigger.process(params[RUN_PARAM].getValue())) {
		running = !running;
	}
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

	if (running)
	{
		if (inputs[EXT_CLOCK_INPUT].isConnected())
		{
			// External clock input
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].getVoltage()))
			{
				realPhase = 0.0;
				nextStep = true;
				lastStepWasExternalClock = true;
			}
		}
		else
		{
			// Internal clock
			*clockTime = 1.0f;
			float input = 1.0f;
			if (inputs[BPM_INPUT].isConnected())
			{
				// Use whatever voltage we are getting (-10 TO 10 input)
				input = rescale(inputs[BPM_INPUT].getVoltage(), TROWA_SEQ_PATTERN_MIN_V, TROWA_SEQ_PATTERN_MAX_V,
					TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
			}
			else
			{
				// Otherwise read our knob
				input = params[BPM_PARAM].getValue(); // -2 to 6
			}
			*clockTime = powf(2.0, input); // -2 to 6			
			
			lastStepWasExternalClock = false;
			if (resetPaused)
			{
				realPhase = 0.0f;
				nextStep = true;
				resetPaused = false;
				index = -1;
				nextIndex = TROWA_INDEX_UNDEFINED; // Reset our jump to index
			}
			else
			{				
				float dt = (*clockTime) / sampleRate; // Real dt
				realPhase += dt; // Real Time no matter what
				if (realPhase >= 1.0)
				{
					realPhase -= 1.0;
					nextStep = true;
				}				
			}
		}
	} // end if running
	return nextStep;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getStepInputs()
// Get the inputs shared between our Sequencers.
// Now also processes our external message queue.
// @pulse : (OUT) If gate pulse
// @reloadMatrix: (OUT) If the edit matrix should be refreshed.
// @valueModeChanged: (OUT) If the output value mode has changed.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::getStepInputs(const ProcessArgs &args, /*out*/ bool* pulse, /*out*/ bool* reloadMatrix, /*out*/ bool* valueModeChanged, bool nextStep, float clockTime)
{
	// Track if we have changed these
	bool editPatternChanged = false;
	bool editChannelChanged = false;
	float lastBPM = currentBPM;
	bool playBPMChanged = false;
	bool lastRunning = running;
	int lastBPMNoteIx = this->selectedBPMNoteIx;
	int lastStepIndex = index;
	
	lastPatternSequencingOn = patternSequencingOn; // Save before we read it in

	// // Run
	// if (runningTrigger.process(params[RUN_PARAM].getValue())) {
		// running = !running;
	// }
	// lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

	bool oscStarted = false; // If OSC just started to a new address this step.
	switch (this->oscCurrentAction)
	{
	case OSCAction::Disable:
		this->cleanupOSC(); // Try to clean up OSC
		break;
	case OSCAction::Enable:
		this->cleanupOSC(); // Try to clean up OSC if we already have something
		this->initOSC(this->oscNewSettings.oscTxIpAddress.c_str(), this->oscNewSettings.oscTxPort, this->oscNewSettings.oscRxPort);
		this->useOSC = true;
		oscStarted = this->useOSC && this->oscInitialized;
		break;
	case OSCAction::None:
	default:
		break;
	}
	this->oscCurrentAction = OSCAction::None;

	// OSC is Enabled and Active light
	lights[LightIds::OSC_ENABLED_LIGHT].value = (useOSC && oscInitialized) ? 1.0 : 0.0;
	

	// if (!firstLoad)
		// lastPatternPlayingIx = currentPatternPlayingIx;

	

	//bool nextStep = false;
	// Now calculate BPM even if we are paused:
	// BPM calculation selection
	if (selectedBPMNoteTrigger.process(params[SELECTED_BPM_MULT_IX_PARAM].getValue())) {
		if (selectedBPMNoteIx < TROWA_TEMP_BPM_NUM_OPTIONS - 1)
			selectedBPMNoteIx++;
		else
			selectedBPMNoteIx = 0; // Wrap around
		lights[SELECTED_BPM_MULT_IX_LIGHT].value = 1.0;
		// Adjust the BPM Knob multiplier for the built-in display/text input:
		this->paramQuantities[BPM_PARAM]->unit = " BPM (1/" + std::string(BPMOptions[selectedBPMNoteIx]->label) + ")";
		this->paramQuantities[BPM_PARAM]->displayMultiplier = BPMOptions[selectedBPMNoteIx]->multiplier;
	}
	// float clockTime = 1.0;
	// float input = 1.0;
	// if (inputs[BPM_INPUT].isConnected())
	// {
		// // Use whatever voltage we are getting (-10 TO 10 input)
		// input = rescale(inputs[BPM_INPUT].getVoltage(), TROWA_SEQ_PATTERN_MIN_V, TROWA_SEQ_PATTERN_MAX_V,
			// TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
	// }
	// else
	// {
		// // Otherwise read our knob
		// input = params[BPM_PARAM].getValue(); // -2 to 6
	// }
	// clockTime = powf(2.0, input); // -2 to 6
		
	// Calculate his all the time now instead of just on next step:
	currentBPM = roundf(clockTime * BPMOptions[selectedBPMNoteIx]->multiplier);
	playBPMChanged = lastBPM != currentBPM;
	
/*	if (running)
	{
		if (inputs[EXT_CLOCK_INPUT].isConnected())
		{
			// External clock input
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].getVoltage()))
			{
				realPhase = 0.0;
				nextStep = true;
				lastStepWasExternalClock = true;
			}
		}
		else
		{
			// Internal clock
			lastStepWasExternalClock = false;
			if (resetPaused)
			{
				realPhase = 0.0f;
				nextStep = true;
				resetPaused = false;
				index = -1;
				nextIndex = TROWA_INDEX_UNDEFINED; // Reset our jump to index
			}
			else
			{
				float dt = clockTime / args.sampleRate; // Real dt
				realPhase += dt; // Real Time no matter what
				if (realPhase >= 1.0)
				{
					realPhase -= 1.0;
					nextStep = true;
				}				
			}
			//if (nextStep)
			//{	
			//currentBPM = roundf(clockTime * BPMOptions[selectedBPMNoteIx]->multiplier);
			//playBPMChanged = lastBPM != currentBPM;
			//}
		}
	} // end if running
*/	

	
	//=======================================================
	// Pattern Sequencing
	//=======================================================		
	if (allowPatternSequencing)
	{
		// Show Configuration View:
		// Read from widget
		//showPatternSequencingConfig = params[ParamIds::PATTERN_SEQ_SHOW_CONFIG_PARAM].getValue() > 0;		
		if (showPatternSequencingConfig)
		{
			// Pattern Sequencing turned on/off:
			patternSequencingOn = params[ParamIds::PATTERN_SEQ_ON_PARAM].getValue() > 0;
			// Number of Patterns in the seqeuence:
			numPatternsInSequence = params[ParamIds::PATTERN_SEQ_LENGTH_PARAM].getValue();
			// The patterns
			for (int p = 0; p < maxSteps; p++)
			{
				patternData[p] = (short)clamp(static_cast<short>(roundf(params[PATTERN_SEQ_PARAM_START + p].getValue())), 0, TROWA_SEQ_NUM_PATTERNS - 1);
			}
		} // end if configuratio is showing
	} // end if we are doing Pattern Sequencing
	

	//=======================================================
	// Current Playing Pattern
	//=======================================================	
	// Priority:
	// 1. External Message
	// 2. CV Input
	// 3. Internal Pattern Sequencing
	// 4. Knob 	
	//=======================================================	
	lastPatternPlayingIx = currentPatternPlayingIx; // Save the last one we had.	
	// If we get an input, then use that:
	if (inputs[SELECTED_PATTERN_PLAY_INPUT].isConnected())
	{
		patternSequencingOn = false; // CV Input overrides internal pattern sequencing.
		patternPlayingControlSource = ControlSource::CVInputSrc;
		//currentPatternPlayingIx = VoltsToPattern(inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage() + volts2PatternAdj) - 1;
		currentPatternPlayingIx = VoltsToPattern(inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage()) - 1;
		
#if DEBUG_PATT_SEQ

		if (debugLastPatternInputVoltage != inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage())
		{
			DEBUG("IN PATT Voltage: %10.8f, PatternIx: %d, PATT #%d", inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage(), currentPatternPlayingIx, currentPatternPlayingIx + 1);		
			debugLastPatternInputVoltage = inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage();			
		}

		// float range = (float)(TROWA_SEQ_PATTERN_MAX_V - TROWA_SEQ_PATTERN_MIN_V);
		// float dV = range / TROWA_SEQ_NUM_PATTERNS;
		// float v = inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage() - TROWA_SEQ_PATTERN_MIN_V;
		// float n = v / dV;

		// if (debugLastPatternInputVoltage != inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage())
		// {
			// currentPatternPlayingIx = static_cast<int>(n);
			// DEBUG("IN PATT Voltage: %10.8f, Pos Volt: %10.8f, dV: %10.8f, PATT Index (float): %4.2f, PatternIx: %d.", inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage(), v, dV, n, static_cast<int>(n));			
			// debugLastPatternInputVoltage = inputs[SELECTED_PATTERN_PLAY_INPUT].getVoltage();
		// }
#endif
	}
	else if (allowPatternSequencing && patternSequencingOn)
	{
		// INTERNAL PATTERN SEQUENCING ///////
		if (patternPlayHeadIx < 0)
			patternPlayHeadIx = 0;
		else if (patternPlayHeadIx >= numPatternsInSequence)
			patternPlayHeadIx = numPatternsInSequence - 1;
		patternPlayingControlSource = ControlSource::AutoSrc;		
		currentPatternPlayingIx = patternData[patternPlayHeadIx];
	}
	else
	{
		// Otherwise read our knob parameter and use that
		currentPatternPlayingIx = (int)clamp(static_cast<int>(roundf(params[SELECTED_PATTERN_PLAY_PARAM].getValue())), 0, TROWA_SEQ_NUM_PATTERNS - 1);
		patternPlayingControlSource = ControlSource::UserParameterSrc;		
	}
	if (currentPatternPlayingIx < 0)
		currentPatternPlayingIx = 0;
	else if (currentPatternPlayingIx > TROWA_SEQ_NUM_PATTERNS - 1)
		currentPatternPlayingIx = TROWA_SEQ_NUM_PATTERNS - 1;


	// Current Edit Pattern
	int lastEditPatternIx = currentPatternEditingIx;
	// From User Knob:
	currentPatternEditingIx = (int)clamp(static_cast<int>(roundf(params[SELECTED_PATTERN_EDIT_PARAM].getValue())), 0, TROWA_SEQ_NUM_PATTERNS - 1);
	if (currentPatternEditingIx < 0)
		currentPatternEditingIx = 0;
	else if (currentPatternEditingIx > TROWA_SEQ_NUM_PATTERNS - 1)
		currentPatternEditingIx = TROWA_SEQ_NUM_PATTERNS - 1;


	// Channel inputs (which channel we are displaying & editing)
	int lastChannelIx = currentChannelEditingIx;
	currentChannelEditingIx = (int)clamp(static_cast<int>(roundf(params[SELECTED_CHANNEL_PARAM].getValue())), 0, TROWA_SEQ_NUM_CHNLS - 1);
	if (currentChannelEditingIx < 0)
		currentChannelEditingIx = 0;
	else if (currentChannelEditingIx > TROWA_SEQ_NUM_CHNLS - 1)
		currentChannelEditingIx = TROWA_SEQ_NUM_CHNLS - 1;
	editChannelChanged = lastChannelIx != currentChannelEditingIx;


	int r = 0;
	int c = 0;

	if (!editChannelChanged) // [v1.1] Only read in if edit channel hasn't changed
	{
		// If the channel hasn't changed, read this in
		// Current output value mode	
		//selectedOutputValueMode = static_cast<ValueMode>((int)clamp(static_cast<int>(roundf(params[SELECTED_OUTPUT_VALUE_MODE_PARAM].getValue())), 0, TROWA_SEQ_NUM_MODES - 1));		
		// selectedOutputValueMode = static_cast<ValueMode>((int)clamp(static_cast<int>(roundf(params[SELECTED_OUTPUT_VALUE_MODE_PARAM].getValue())), 
			// ValueMode::MIN_VALUE_MODE, 
			// ValueMode::MAX_VALUE_MODE));	

		// [v1.0.4] Knob is now just index into our array.
		selectedOutputValueModeIx = (int)(clamp(static_cast<int>(roundf(params[SELECTED_OUTPUT_VALUE_MODE_PARAM].getValue())), 0, numValueModesSupported - 1));
		selectedOutputValueMode = valueModesSupported[selectedOutputValueModeIx];
	}
	else if (selectedOutputValueMode != channelValueModes[currentChannelEditingIx])
	{
		// If the channel changed, then set the selected output mode to this channel's
		selectedOutputValueMode = channelValueModes[currentChannelEditingIx];		
		// Find the index
		selectedOutputValueModeIx = getSupportedValueModeIndex(selectedOutputValueMode);
		if (selectedOutputValueModeIx < 0)
		{
			selectedOutputValueMode = defaultChannelValueMode;
			selectedOutputValueModeIx = getSupportedValueModeIndex(defaultChannelValueMode);
		}	
		// Modify the knob 
		// [v1.0.4] (now with the index not the mode value).
		this->paramQuantities[ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM]->setValue(selectedOutputValueModeIx);
	}

	int lastNumberSteps = currentNumberSteps;
	if (inputs[STEPS_INPUT].isConnected())
	{
		// Use the input if something is connected.
		// Some seqeuencers go to 64 steps, we want the same voltage to mean the same step number no matter how many max steps this one takes.
		// so voltage input is normalized to indicate step 1 to step 64, but we'll limit it to maxSteps.
		currentNumberSteps = (int)clamp(static_cast<int>(roundf(rescale(inputs[STEPS_INPUT].getVoltage(), TROWA_SEQ_STEPS_MIN_V, TROWA_SEQ_STEPS_MAX_V, 1.0, (float)TROWA_SEQ_MAX_NUM_STEPS))), 1, maxSteps);
	}
	else
	{
		// Otherwise read our knob
		currentNumberSteps = (int)clamp(static_cast<int>(roundf(params[STEPS_PARAM].getValue())), 1, maxSteps);
	}

	//------------------------------------------------------------
	// Check if we have any eternal messages.
	// (i.e. from OSC)
	//------------------------------------------------------------
	/// TODO: Check performance hit from sending OSC in general
	/// TODO: Some thread safety and make sure that this queue never gets unruly? such that we start getting out of time.
	bool resetMsg = false;
	bool doPaste = false;
	int prevCopyPatternIx = copySourcePatternIx;
	int prevCopyChannelIx = copySourceChannelIx;
	bool storedPatternChanged = false;
	bool storedLengthChanged = false;
	bool storedBPMChanged = false;
	bool playPatternSetFromExternalMsg = false; // Keep track if an external message has changed the current playing pattern.
	while (ctlMsgQueue.size() > 0)
	{
		TSExternalControlMessage recvMsg = (TSExternalControlMessage)(ctlMsgQueue.front());
		ctlMsgQueue.pop();
		float tmp;
		/// TODO: redorder switch for most common cases first.
		switch (recvMsg.messageType)
		{
		case TSExternalControlMessage::MessageType::ToggleEditStepValue:
		case TSExternalControlMessage::MessageType::SetEditStepValue:
			if (currentCtlMode == ExternalControllerMode::EditMode)
			{
				int p = (recvMsg.pattern == CURRENT_EDIT_PATTERN_IX) ? currentPatternEditingIx : recvMsg.pattern;
				int c = (recvMsg.channel == CURRENT_EDIT_CHANNEL_IX) ? currentChannelEditingIx : recvMsg.channel;
				float oldVal = this->triggerState[p][c][recvMsg.step];
				float val = (recvMsg.messageType == TSExternalControlMessage::MessageType::ToggleEditStepValue) ? getToggleStepValue(recvMsg.step, recvMsg.val, /*channel*/ c, /*pattern*/ p) : recvMsg.val;
				if (oldVal != val)
				{
					// Value has changed:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("[%d] Set Step Value (value changed): %d (P %d, C %d) = %.4f.", recvMsg.messageType, recvMsg.step, p, c, val);
#endif
					this->setStepValue(recvMsg.step, val, /*channel*/ c, /*pattern*/ p);
				}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				else
				{
					DEBUG("[%d] Step value did not change -- Ignore : %d (P %d, C %d) = %.4f.", recvMsg.messageType, recvMsg.step, p, c, val);
				}
#endif
			}
			else
			{
				// In performance mode, this will be interupted as jump to (playing):
				if (recvMsg.pattern != CURRENT_EDIT_PATTERN_IX)
				{
					currentPatternPlayingIx = recvMsg.pattern; // Jump to this pattern if sent
					playPatternSetFromExternalMsg = true;
					// Update our knob
					// /// TODO: This should be moved to Widget for future headless modules.
					// if (controlKnobs[KnobIx::PlayPatternKnob]) {
						// controlKnobs[KnobIx::PlayPatternKnob]->setValue(currentPatternPlayingIx);
						// controlKnobs[KnobIx::PlayPatternKnob]->setDirty(true);						
					// }
					params[ParamIds::SELECTED_PATTERN_PLAY_PARAM].setValue(currentPatternPlayingIx);
				}
				// Jump to this step:
				if (nextStep)
				{
					// We are already at beginning of a step, so we can sneak this in now.
					index = recvMsg.step - 1; // We will set nextStep to true to go to this step fresh
					nextIndex = TROWA_INDEX_UNDEFINED;
				}
				else
				{
					// Next time we are ready to go to the next step, this should be it.
					nextIndex = recvMsg.step;
				}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Performance Mode: Jump to Step (index): %d (Pattern %d).", index, currentPatternPlayingIx);
#endif
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayCurrentStep:
			// We want to wait until the 'next step' (finish the one we are currently doing so things are still in time).
			if (nextStep)
			{
				// We are already at beginning of a step, so we can sneak this in now.
				index = recvMsg.step - 1; // We will set nextStep to true to go to this step fresh
				nextIndex = TROWA_INDEX_UNDEFINED;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Set Play Step (index): %d [Immediate].", recvMsg.step);
#endif
			}
			else
			{
				// Next time we are ready to go to the next step, this should be it.
				nextIndex = recvMsg.step;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Set Play Step (index): %d [Next, curr is %d].", nextIndex, index);
#endif
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayReset:
			resetMsg = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Reset.");
#endif
			break;
		case TSExternalControlMessage::MessageType::SetPlayPattern:
			if (recvMsg.pattern == TROWA_INDEX_UNDEFINED)
			{
				recvMsg.pattern = storedPatternPlayingIx; // Check our stored pattern
			}
			if (recvMsg.pattern > -1 && recvMsg.pattern < TROWA_SEQ_NUM_PATTERNS)
			{
				currentPatternPlayingIx = recvMsg.pattern;
				playPatternSetFromExternalMsg = true;
				patternPlayingControlSource = ControlSource::ExternalMsgSrc;				
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Set Play Pattern: %d.", currentPatternPlayingIx);
#endif
				params[ParamIds::SELECTED_PATTERN_PLAY_PARAM].setValue(currentPatternPlayingIx);
			}
			break;
		case TSExternalControlMessage::MessageType::StorePlayPattern:
			if (storedPatternPlayingIx != recvMsg.pattern)
			{
				storedPatternPlayingIx = recvMsg.pattern;
				storedPatternChanged = true;
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayOutputMode:
			// -- Set Ouput Mode: (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT) --
			// [v1.0.4] This is an index into our supported modes array now, not the actual value to keep it backwards compatible.			
			selectedOutputValueModeIx = (int)(recvMsg.mode);
			selectedOutputValueMode = valueModesSupported[selectedOutputValueModeIx]; // (ValueMode)(recvMsg.mode);
			channelValueModes[currentChannelEditingIx] = selectedOutputValueMode; // Set if for the current channel mode we are editing
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Output Mode: %d (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT).", selectedOutputValueMode);
#endif
			// /// TODO: This should be moved to Widget for future headless modules.
			// if (controlKnobs[KnobIx::OutputModeKnob]) {
				// controlKnobs[KnobIx::OutputModeKnob]->setValue(selectedOutputValueMode);
				// controlKnobs[KnobIx::OutputModeKnob]->setDirty(true);				
			// }
			params[ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM].setValue(selectedOutputValueModeIx);//selectedOutputValueMode);
			break;
		case TSExternalControlMessage::MessageType::SetEditPattern:
			currentPatternEditingIx = recvMsg.pattern;
			*reloadMatrix = true; // Refresh our display matrix
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Edit Pattern: %d.", currentPatternEditingIx);
#endif
			// Update our knob
			/// TODO: This should be moved to Widget for future headless modules.
			// if (controlKnobs[KnobIx::EditPatternKnob]) {
				// controlKnobs[KnobIx::EditPatternKnob]->setValue(currentPatternEditingIx);
				// controlKnobs[KnobIx::EditPatternKnob]->setDirty(true);				
			// }
			params[ParamIds::SELECTED_PATTERN_EDIT_PARAM].setValue(currentPatternEditingIx);
			break;
		case TSExternalControlMessage::MessageType::SetEditChannel:
			currentChannelEditingIx = recvMsg.channel;
			*reloadMatrix = true; // Refresh our display matrix
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Edit Channel: %d.", currentChannelEditingIx);
#endif
			// Update our knob
			// /// TODO: This should be moved to Widget for future headless modules.
			// if (controlKnobs[KnobIx::EditChannelKnob]) {
				// controlKnobs[KnobIx::EditChannelKnob]->setValue(currentChannelEditingIx);
				// controlKnobs[KnobIx::EditChannelKnob]->setValue(true);		
			// }
			params[ParamIds::SELECTED_CHANNEL_PARAM].setValue(currentChannelEditingIx);
			break;
		case TSExternalControlMessage::MessageType::TogglePlayMode:
			// -- Control Mode: Edit / Performance Mode --
			currentCtlMode = (currentCtlMode == ExternalControllerMode::EditMode) ? ExternalControllerMode::PerformanceMode : ExternalControllerMode::EditMode;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Toggle Control Mode: %d.", currentCtlMode);
#endif
			break;
		case TSExternalControlMessage::MessageType::SetPlayMode:
			// -- Control Mode: Edit / Performance Mode --
			currentCtlMode = static_cast<ExternalControllerMode>(recvMsg.mode);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Control Mode: %d.", currentCtlMode);
#endif
			break;
		case TSExternalControlMessage::MessageType::StorePlayBPM:
			if (storedBPM != recvMsg.mode)
			{
				storedBPMChanged = true;
				storedBPM = recvMsg.mode;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Store BPM (%d): %.2f.", recvMsg.mode, storedBPM);
#endif
			}
			break;
			// Now BPM and Tempo are separate in OSC.
			/// TODO: Maybe move the calcs to the listener thread? But if we have other external messages from non-OSC, then it would have to repeated somewhere...
		case TSExternalControlMessage::MessageType::SetPlayBPM: // "BPM" is relative to the note
																//float bpm = recvMsg.mode;
																// currentBPM = 2^knob * mult
																// currentBPM / mult = 2 ^ knob
																// log2(currentBPM / mult) = knob
			if (recvMsg.mode == TROWA_INDEX_UNDEFINED)
				recvMsg.mode = storedBPM;
			tmp = clamp(std::log2f(recvMsg.mode / BPMOptions[selectedBPMNoteIx]->multiplier), static_cast<float>(TROWA_SEQ_BPM_KNOB_MIN), static_cast<float>(TROWA_SEQ_BPM_KNOB_MAX));
			// /// TODO: This should be moved to Widget for future headless modules.
			// if (controlKnobs[KnobIx::BPMKnob]) {
				// controlKnobs[KnobIx::BPMKnob]->setValue(tmp);
				// controlKnobs[KnobIx::BPMKnob]->setDirty(true);
			// }
			params[ParamIds::BPM_PARAM].setValue(tmp);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set BPM (%d): %.2f.", recvMsg.mode, tmp);
#endif
			break;
		case TSExternalControlMessage::MessageType::AddPlayBPM: // "BPM" is relative to the note
			// /// TODO: This should be moved to Widget for future headless modules.
			// tmp = pow(2, controlKnobs[KnobIx::BPMKnob]->getValue()) // Current BPM
				// + recvMsg.mode / BPMOptions[selectedBPMNoteIx]->multiplier;			
			tmp = pow(2, params[ParamIds::BPM_PARAM].value) // Current BPM
				+ recvMsg.mode / BPMOptions[selectedBPMNoteIx]->multiplier;
			tmp = std::log2f(tmp);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Add BPM (%d): Knob %.2f, End is %.2f", recvMsg.mode, params[ParamIds::BPM_PARAM].value, tmp);
#endif
			// /// TODO: This should be moved to Widget for future headless modules.
			// if (controlKnobs[KnobIx::BPMKnob]) {
				// controlKnobs[KnobIx::BPMKnob]->setValue(clamp(tmp, static_cast<float>(TROWA_SEQ_BPM_KNOB_MIN), static_cast<float>(TROWA_SEQ_BPM_KNOB_MAX)));
				// controlKnobs[KnobIx::BPMKnob]->setDirty(true);
			// }
			params[ParamIds::BPM_PARAM].setValue(tmp);
			break;
		case TSExternalControlMessage::MessageType::SetPlayTempo: // Tempo goes from 0 to 1
			tmp = rescale(recvMsg.val, 0.0, 1.0, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
			// /// TODO: This should be moved to Widget for future headless modules.			
			// if (controlKnobs[KnobIx::BPMKnob]) {
				// controlKnobs[KnobIx::BPMKnob]->setValue(tmp);
				// controlKnobs[KnobIx::BPMKnob]->setDirty(true);				
			// }
			params[ParamIds::BPM_PARAM].setValue(tmp);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Tempo (%.2f): Knob %.2f.", recvMsg.val, tmp);
#endif
			break;
		case TSExternalControlMessage::MessageType::AddPlayTempo: // Tempo goes from 0 to 1
			tmp = rescale(recvMsg.val, 0.0, 1.0, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
			// /// TODO: This should be moved to Widget for future headless modules.			
			// if (controlKnobs[KnobIx::BPMKnob]) {			
				// controlKnobs[KnobIx::BPMKnob]->setValue(clamp(tmp + controlKnobs[KnobIx::BPMKnob]->getValue(), static_cast<float>(TROWA_SEQ_BPM_KNOB_MIN), static_cast<float>(TROWA_SEQ_BPM_KNOB_MAX)));
				// controlKnobs[KnobIx::BPMKnob]->setDirty(true);
			// }
			params[ParamIds::BPM_PARAM].setValue(tmp);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Add Tempo (%.2f): Knob %.2f.", recvMsg.val, controlKnobs[KnobIx::BPMKnob]->getValue());
#endif
			break;
		case TSExternalControlMessage::MessageType::AddPlayBPMNote:
			selectedBPMNoteIx = (selectedBPMNoteIx + recvMsg.step) % TROWA_TEMP_BPM_NUM_OPTIONS;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Add %d to BPM Note Ix: %d.", recvMsg.step, selectedBPMNoteIx);
#endif
			break;
		case TSExternalControlMessage::MessageType::SetPlayBPMNote:
			selectedBPMNoteIx = recvMsg.step % TROWA_TEMP_BPM_NUM_OPTIONS;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set BPM Note Ix: %d.", selectedBPMNoteIx);
#endif
			break;
		case TSExternalControlMessage::MessageType::StorePlayLength:
			if (recvMsg.step != storedNumberSteps)
			{
				storedNumberSteps = recvMsg.step;
				storedLengthChanged = true;
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayLength:
			// Use our stored value if sent is -1
			recvMsg.step = (recvMsg.step == TROWA_INDEX_UNDEFINED) ? storedNumberSteps : recvMsg.step;
			if (recvMsg.step > 0 && recvMsg.step <= maxSteps)
			{
				currentNumberSteps = recvMsg.step;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Set Play Step Length: %d.", currentNumberSteps);
#endif
				// Update our knob
				// /// TODO: This should be moved to Widget for future headless modules.
				// if (controlKnobs[KnobIx::StepLengthKnob]){
					// controlKnobs[KnobIx::StepLengthKnob]->setValue(currentNumberSteps);
					// controlKnobs[KnobIx::StepLengthKnob]->setDirty(true);					
				// }
				params[ParamIds::STEPS_PARAM].setValue(currentNumberSteps);
			}
			break;
		case TSExternalControlMessage::MessageType::PasteEditClipboard:
			doPaste = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Paste message.");
#endif
			break;
		case TSExternalControlMessage::MessageType::CopyEditPattern:
		{
			int pat = (recvMsg.pattern == CURRENT_EDIT_PATTERN_IX) ? currentPatternEditingIx : recvMsg.pattern;
			if (copySourcePatternIx > -1 && copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
			{
				// Clear clipboard 
				clearClipboard();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("(clear clipboard) Copy Edit Pattern: %d.", pat);
#endif
			}
			else
			{
				//int c = (recvMsg.channel == CURRENT_EDIT_CHANNEL_IX) ? currentChannelEditingIx : recvMsg.channel;
				copy(pat, TROWA_SEQ_COPY_CHANNELIX_ALL);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				currentPasteColor = COPY_PATTERN_COLOR; //TSColors::COLOR_WHITE;
				lights[COPY_PATTERN_LIGHT].value = 1; // Light up Pattern Copy as Active clipboard
				lights[COPY_CHANNEL_LIGHT].value = 0;	// Inactivate Gate Copy light
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Copy Edit Pattern: %d.", pat);
#endif
			}
		}
		break;
		case TSExternalControlMessage::MessageType::CopyEditChannel:
		{
			int pat = (recvMsg.pattern == CURRENT_EDIT_PATTERN_IX) ? currentPatternEditingIx : recvMsg.pattern;
			int ch = (recvMsg.channel == CURRENT_EDIT_CHANNEL_IX) ? currentChannelEditingIx : recvMsg.channel;
			if (copySourcePatternIx > -1 && copySourceChannelIx > -1)
			{
				// Clear clipboard 
				clearClipboard();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("(clear clipboard) Copy Edit Channel: (P:%d, C:%d).", pat, ch);
#endif
			}
			else
			{
				copy(pat, ch);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				currentPasteColor = voiceColors[currentChannelEditingIx];
				//pasteLight->setColor(voiceColors[currentChannelEditingIx]);
				lights[COPY_CHANNEL_LIGHT].value = 1;		// Light up Channel Copy Light as Active clipboard
				//copyGateLight->setColor(voiceColors[currentChannelEditingIx]); // Match the color with our Channel color
				currentCopyChannelColor = voiceColors[currentChannelEditingIx];
				lights[COPY_PATTERN_LIGHT].value = 0; // Inactivate Pattern Copy Light
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Copy Edit Channel: (P:%d, C:%d).", pat, ch);
#endif
			}

		}
		break;
		case TSExternalControlMessage::MessageType::SetPlayRunningState:
		case TSExternalControlMessage::MessageType::TogglePlayRunningState:
			if (recvMsg.messageType == TSExternalControlMessage::MessageType::TogglePlayRunningState)
			{
				running = !running;
			}
			else
			{
				running = recvMsg.val > 0;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Set Running to: %d.", running);
#endif
			runningTrigger.state = (running) ? TriggerSignal::HIGH : TriggerSignal::LOW;
			params[RUN_PARAM].setValue(running);
			lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;
			break;
		case TSExternalControlMessage::MessageType::RandomizeEditStepValue:
		{
			RandomizeEvent evt;
			onRandomize(evt);
		}
			break;
		case TSExternalControlMessage::MessageType::InitializeEditModule:
			onReset();
			resetParamQuantities();	 // Reset the param quantities the parameters/knobs.
			// /// TODO: This should be moved to Widget for future headless modules.
			// for (int i = 0; i < KnobIx::NumKnobs; i++)
			// {
				// if (controlKnobs[i]) {
					// controlKnobs[i]->setValue(controlKnobs[i]->getDefaultValue());
					// controlKnobs[i]->setDirty(true);					
				// }
			// }
			// We also need to make sure our controls reset....
			//Module::onReset(); // Base method reset should do the knobs
			//_ParentWidget->reset();
			/// TODO: We should also send our new values to OSC if OSC is enabled. We would have to re-read the vals though,
			/// I think the values should trigger that they changed next step()... TODO: double check that this happens
			break;
		default:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("Ooops - didn't handle this control message type yet %d.", recvMsg.messageType);
#endif
			break;
		} // end switch
	} // end loop through message queue	


	//-- COPY / PASTE --
	bool pasteCompleted = false;
	if (pasteTrigger.process(params[PASTE_PARAM].getValue()) || doPaste)
	{
		pasteCompleted = paste(); // Paste whatever we have if we have anything		
	}
	else
	{
		// Check Copy
		if (copyPatternTrigger.process(params[COPY_PATTERN_PARAM].getValue()))
		{
			if (copySourcePatternIx > -1 && copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
			{
				// Clear clipboard 
				clearClipboard();
			}
			else
			{
				copy(currentPatternEditingIx, TROWA_SEQ_COPY_CHANNELIX_ALL);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				currentPasteColor = COPY_PATTERN_COLOR; //TSColors::COLOR_WHITE;
				lights[COPY_PATTERN_LIGHT].value = 1; // Light up Pattern Copy as Active clipboard
				lights[COPY_CHANNEL_LIGHT].value = 0;	// Inactivate Gate Copy light				
			}
		}
		if (copyGateTrigger.process(params[COPY_CHANNEL_PARAM].getValue()))
		{
			if (copySourcePatternIx > -1 && copySourceChannelIx > -1)
			{
				// Clear clipboard 
				clearClipboard();
			}
			else
			{
				copy(currentPatternEditingIx, currentChannelEditingIx);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				currentPasteColor = voiceColors[currentChannelEditingIx];
				lights[COPY_CHANNEL_LIGHT].value = 1;		// Light up Channel Copy Light as Active clipboard
				currentCopyChannelColor = voiceColors[currentChannelEditingIx];
				lights[COPY_PATTERN_LIGHT].value = 0; // Inactivate Pattern Copy Light				
			}
		} // end if copyGateTrigger()
	}

	// Check value mode change after we have processed incoming messages.
	*valueModeChanged = (lastOutputValueMode != selectedOutputValueMode);
	lastOutputValueMode = selectedOutputValueMode;
	
	if (playPatternSetFromExternalMsg && allowPatternSequencing)
	{
		// Turn off our internal pattern sequencing.
		patternSequencingOn = false;
	}

	// Reset
	// [03/30/2018] So, now j4s0n wants RESET to wait until the next step is played... 
	// So it's delayed reset. https://github.com/j4s0n-c/trowaSoft-VCV/issues/11
	if (resetTrigger.process(params[RESET_PARAM].getValue() + inputs[RESET_INPUT].getVoltage()) || resetMsg)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		DEBUG("Reset");
#endif
		resetPaused = !running;
		if (running)
		{
			// [03/30/2018] Delay reset until the next step. (https://github.com/j4s0n-c/trowaSoft-VCV/issues/11)
			// So it's more like JUMP TO step 0 (waits until the next step).
			resetQueued = true; // Flag that the reset has been queued.						
		}
	} // end check for reset

	if (resetQueued && nextStep) 
	{
		resetQueued = false; 
		realPhase = 0.0;
		swingAdjustedPhase = 0; // Reset swing		
		index = 999;
		nextStep = true;
		lights[RESET_LIGHT].value = 1.0;
		nextIndex = TROWA_INDEX_UNDEFINED; // Reset our jump to index
		oscMutex.lock();
		if (useOSC && oscInitialized)
		{
			osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
			oscStream << osc::BeginBundleImmediate
				<< osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayReset])
				<< "bang" << osc::EndMessage
				<< osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
#if TROWA_SEQ_USE_INTERNAL_DIVISOR
		idleCounter = -1;
#endif
		if (allowPatternSequencing)
		{
			// Reset Pattern
			patternPlayHeadIx = -1;
#if DEBUG_PATT_SEQ
			DEBUG("Pattern PlayHead Ix RESET to %d (out of %d).", patternPlayHeadIx, numPatternsInSequence);
#endif // DEBUG_PATT_SEQ
		}		
	} // end if resetQueued and it's time to reset
	

	// Next Step
	if (nextStep)
	{
		if (nextIndex == TROWA_INDEX_UNDEFINED)
		{
			index++; // Advance step
		}
		else
		{
			index = nextIndex; // Set to our 'jump to' value
			nextIndex = TROWA_INDEX_UNDEFINED; // Reset our jump to index
		}
		if (index >= currentNumberSteps || index < 0) 
		{
			index = 0; // Reset (artifical limit)
			if (allowPatternSequencing && patternSequencingOn)
			{
				// Go to the next pattern.
				patternPlayHeadIx++;	
#if DEBUG_PATT_SEQ
				DEBUG("Pattern PlayHead Ix Incremented to %d out of %d.", patternPlayHeadIx, numPatternsInSequence);
#endif // DEBUG_PATT_SEQ
			}			
		}
		
		// Show which step we are on:
		r = index / this->numCols;// TROWA_SEQ_STEP_NUM_COLS;
		c = index % this->numCols; //TROWA_SEQ_STEP_NUM_COLS;
		stepLights[r][c] = 1.0f;
		gatePulse.trigger(TROWA_PULSE_WIDTH);
		
		// INTERNAL PATTERN SEQUENCING ///////
		if (allowPatternSequencing && patternSequencingOn)
		{
			if (patternPlayHeadIx < 0 || patternPlayHeadIx >= numPatternsInSequence)
				patternPlayHeadIx = 0; // Loop around
			currentPatternPlayingIx = patternData[patternPlayHeadIx];
			if (lights[PATTERN_SEQ_LIGHT_START + patternPlayHeadIx].value < 0.3f)
				lights[PATTERN_SEQ_LIGHT_START + patternPlayHeadIx].value = 0.9f;
		} // end internal pattern sequencing		

		oscMutex.lock();
		if (useOSC && oscInitialized)
		{
			// [01/06/2018] Changed to one-based for OSC (send index+1 instead of index)
			osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
			oscStream << osc::BeginBundleImmediate
				<< osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayClock])
				<< index + 1 << osc::EndMessage
				<< osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end if next step

	// // If we were just unpaused and we were reset during the pause, make sure we fire the first step.
	// if (running && !lastRunning)
	// {
		// if (resetPaused)
		// {
			// gatePulse.trigger(TROWA_PULSE_WIDTH);
		// }
		// resetPaused = false;
	// } // end if


	if (allowPatternSequencing)
	{
		// INTERNAL PATTERN SEQUENCING Enabled and Active Light
		lights[LightIds::PATTERN_SEQ_ENABLED_LIGHT].value = (patternSequencingOn) ? 1.0 : 0.0;	
		// INTERNAL PATTERN SEQUENCING Configuration Showing
		lights[LightIds::PATTERN_SEQ_CONFIGURE_LIGHT].value = (showPatternSequencingConfig) ? 1.0 : 0.0;		
	}
	
	
	// Reset light
	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / lightLambda / args.sampleRate;
	// BPM Note Calc light:
	lights[SELECTED_BPM_MULT_IX_LIGHT].value -= lights[SELECTED_BPM_MULT_IX_LIGHT].value / lightLambda / args.sampleRate;
	*pulse =gatePulse.process(args.sampleTime);// (nextStep || realPhase < 0.5f);//  gatePulse.process(args.sampleTime);

	editChannelChanged = currentChannelEditingIx != lastChannelIx;
	editPatternChanged = currentPatternEditingIx != lastEditPatternIx;
	// See if we should reload our matrix
	*reloadMatrix = reloadEditMatrix || editChannelChanged || editPatternChanged || pasteCompleted || firstLoad || oscStarted;

	// Send messages if needed
	/// TODO: Make a message sender to do this crap
	oscMutex.lock();
	if (useOSC && oscInitialized)
	{
		bool bundleOpened = false;
		// If something has changed or we just started up osc, then send the status of our sequencer.
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (lastRunning != running || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayRunningState])
				<< (int)(running)
				<< osc::EndMessage;
			// Send another toggle message for touchOSC
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayToggleRun])
				<< (int)(running)
				<< osc::EndMessage;
		}
		if (lastPatternPlayingIx != currentPatternPlayingIx || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayPattern])
				<< (currentPatternPlayingIx + 1)
				<< osc::EndMessage;
		}
		if (playBPMChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayBPM])
				<< currentBPM << this->selectedBPMNoteIx
				<< osc::EndMessage;
		}
		if (lastNumberSteps != currentNumberSteps || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayLength])
				<< currentNumberSteps
				<< osc::EndMessage;
		} // end playLengthChanged
		if (*valueModeChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayOutputMode])
				<< (int)(this->selectedOutputValueMode)
				<< osc::EndMessage;
		} // end playOutputModeChanged
		if (editPatternChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPattern])
				<< (currentPatternEditingIx + 1)
				<< osc::EndMessage;
		} // end editPatternChanged
		if (editChannelChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannel])
				<< (currentChannelEditingIx + 1)
				<< osc::EndMessage;
		} // end editChannelChanged
		if (lastBPMNoteIx != this->selectedBPMNoteIx || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayBPMNote])
				<< selectedBPMNoteIx
				<< osc::EndMessage;
		} // end bpmNoteChanged

		if (storedPatternChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayPatternSav])
				<< ((storedPatternPlayingIx > -1) ? storedPatternPlayingIx + 1 : currentPatternPlayingIx + 1)
				<< osc::EndMessage;
		} // end storedPatternChanged

		if (storedLengthChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayLengthSav])
				<< ((storedNumberSteps > 0) ? storedNumberSteps : currentNumberSteps)
				<< osc::EndMessage;
		} // end storedPatternChanged

		if (storedBPMChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayBPMSav])
				<< ((storedBPM > 0) ? storedBPM : (int)(currentBPM))
				<< osc::EndMessage;
		}


		if (copySourcePatternIx != prevCopyPatternIx || copySourceChannelIx != prevCopyChannelIx || oscStarted)
		{
			// Clipboard has changed
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}

			if (copySourcePatternIx == TROWA_INDEX_UNDEFINED)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				DEBUG("Sending Clear Clipboard: %s %d.", oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr], 0);
#endif
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditClipboard])
					<< 0
					<< 0
					<< osc::EndMessage;

				// Clipboard was cleared
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr])
					<< 0
					<< osc::EndMessage;
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr])
					<< 0
					<< osc::EndMessage;
			}
			else
			{
				// Send clipboard message (pattern, channel)
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditClipboard])
					<< (copySourcePatternIx + 1)
					<< (copySourceChannelIx + 1)
					<< osc::EndMessage;
				if (copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
				{
					// Pattern copied (pattern)
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("Sending Copied Pattern: %s %d.", oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr], (copySourcePatternIx + 1));
#endif
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr])
						<< (copySourcePatternIx + 1)
						<< osc::EndMessage;
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr])
						<< 0
						<< osc::EndMessage;
				}
				else
				{
					// Channel copied (channel)
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("Sending Copied Channel: %s %d.", oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr], (copySourceChannelIx + 1));
#endif
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr])
						<< 0
						<< osc::EndMessage;
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr])
						<< (copySourceChannelIx + 1)
						<< osc::EndMessage;
				}
			}// end else
		} // end if clipboard change

#if OSC_UPDATE_CURRENT_STEP_LED
		if (lastStepIndex != index)
		{
			// Turn off last led, turn on this led
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			char addrBuff[TROWA_SEQ_BUFF_SIZE] = { 0 };
			// Prev step should turn off:
			sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], lastStepIndex + 1);
			oscStream << osc::BeginMessage(addrBuff)
				<< 0
				<< osc::EndMessage;
			// Current step should turn on:
			sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], index + 1);
			oscStream << osc::BeginMessage(addrBuff)
				<< 1
				<< osc::EndMessage;
		}
#endif

		//--- FINISH BUNDLE ---
		if (bundleOpened)
		{
			// Finish and send
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
	} // end send osc
	oscMutex.unlock();

	firstLoad = false;
	return;
} // end getStepInputs()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataToJson(void)
// Save our junk to json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *TSSequencerModuleBase::dataToJson() {
	json_t *rootJ = json_object();

	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));

	// running
	json_object_set_new(rootJ, "running", json_boolean(running));

	// Current Items:
	json_object_set_new(rootJ, "currentPatternEditIx", json_integer((int)currentPatternEditingIx));
	json_object_set_new(rootJ, "currentTriggerEditIx", json_integer((int)currentChannelEditingIx));
	// The current output / knob mode.
	json_object_set_new(rootJ, "selectedOutputValueMode", json_integer((int)selectedOutputValueMode));
	
	// Current BPM calculation note (i.e. 1/4, 1/8, 1/8T, 1/16)
	json_object_set_new(rootJ, "selectedBPMNoteIx", json_integer((int)selectedBPMNoteIx));
	
	json_t* channelValModesJ = json_array();
	for (int ch = 0; ch < TROWA_SEQ_NUM_CHNLS; ch++)
	{
		json_t* itemJ = json_integer((int)channelValueModes[ch]);
		json_array_append_new(channelValModesJ, itemJ);
	}
	json_object_set_new(rootJ, "chValModes", channelValModesJ);

	// triggers
	json_t *triggersJ = json_array();
	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int t = 0; t < TROWA_SEQ_NUM_CHNLS; t++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				json_t *gateJ = json_real((float)triggerState[p][t][s]);
				json_array_append_new(triggersJ, gateJ);
			} // end for (steps)
		} // end for (triggers)
	} // end for (patterns)
	json_object_set_new(rootJ, "triggers", triggersJ);

	// gateMode
	json_t *gateModeJ = json_integer((int)gateMode);
	json_object_set_new(rootJ, "gateMode", gateModeJ);

	// OSC Parameters
	json_t* oscJ = json_object();
	json_object_set_new(oscJ, "IpAddress", json_string(this->currentOSCSettings.oscTxIpAddress.c_str()));
	json_object_set_new(oscJ, "TxPort", json_integer(this->currentOSCSettings.oscTxPort));
	json_object_set_new(oscJ, "RxPort", json_integer(this->currentOSCSettings.oscRxPort));
	json_object_set_new(oscJ, "Client", json_integer(this->oscCurrentClient));
	json_object_set_new(oscJ, "AutoReconnectAtLoad", json_boolean(oscReconnectAtLoad)); // [v11, v0.6.3]
	json_object_set_new(oscJ, "Initialized", json_boolean(oscInitialized)); // [v11, v0.6.3] We know the settings are good at least at the time of save
	json_object_set_new(rootJ, "osc", oscJ);

	if (allowPatternSequencing)
	{
		// Pattern Sequencing:
		json_t* psJ = json_object();
		json_object_set_new(psJ, "AutoPatternSequence", json_boolean(patternSequencingOn)); 
		json_object_set_new(psJ, "PatternSequenceLength", json_integer(numPatternsInSequence)); 
		if (patternData != NULL)
		{
			json_t * pStepsJ = json_array();
			for (int p = 0; p < maxSteps; p++)
			{
				json_array_append_new(pStepsJ, json_integer(patternData[p]));
			} // end for (patterns)
			json_object_set_new(psJ, "Sequence", pStepsJ);			
		}
		json_object_set_new(rootJ, "patternSeq", psJ);
	} // end if pattern sequencing
	return rootJ;
} // end dataToJson()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// dataFromJson(void)
// Read in our junk from json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::dataFromJson(json_t *rootJ) 
{
	DEBUG("TSSequencerModuleBase(%d): Loading from json!", oscId);
	// running
	json_t *runningJ = json_object_get(rootJ, "running");
	if (runningJ)
		running = json_is_true(runningJ);

	// Current Items:
	json_t *currJ = NULL;
	saveVersion = 0;
	currJ = json_object_get(rootJ, "version");
	if (currJ)
	{
		saveVersion = (int)(json_integer_value(currJ));
	}
	currJ = json_object_get(rootJ, "currentPatternEditIx");
	if (currJ)
		currentPatternEditingIx = json_integer_value(currJ);
	currJ = json_object_get(rootJ, "currentTriggerEditIx");
	if (currJ)
		currentChannelEditingIx = json_integer_value(currJ);
	
	
	currJ = json_object_get(rootJ, "selectedOutputValueMode");	
	int remapValueMode = 0;
	if (currJ)
	{
		//18: 1.0.4 -- Sequencer change
		// Now ValueMode has all modes with unique values (so we are not overriding/sharing TRIG = VOLT, RTRG = NOTE, GATE = PATT).
		// Supported modes are in valueModesSupported array.
		selectedOutputValueMode = static_cast<ValueMode>(json_integer_value(currJ));		
		if (saveVersion < 18)
		{
			// If this is a voltSeq we need to remap it.
			bool mSupported = valueModeIsSupported(selectedOutputValueMode);
			if (!mSupported)
			{
				// Most likely voltSeq so +3
				remapValueMode = ValueMode::VALUE_VOLT;
				selectedOutputValueMode = (ValueMode)((int)(selectedOutputValueMode) + remapValueMode);
				mSupported = valueModeIsSupported(selectedOutputValueMode);
				if (!mSupported)
				{
					// I give up then, just set it to the first one
					selectedOutputValueMode = valueModesSupported[0];
					remapValueMode = -selectedOutputValueMode;
				}
			}
		}
		selectedOutputValueModeIx = getSupportedValueModeIndex(selectedOutputValueMode);
		modeString = modeStrings[selectedOutputValueMode];
	}	
		
	json_t* channelValModesJ = json_object_get(rootJ, "chValModes");
	if (channelValModesJ)
	{ 
		// v1.0.1 (16) or higher:
		for (int ch = 0; ch < TROWA_SEQ_NUM_CHNLS; ch++)
		{
			currJ = json_array_get(channelValModesJ, ch);
			if (currJ)
			{
				if (remapValueMode == 0)
				{
					channelValueModes[ch] = static_cast<ValueMode>(json_integer_value(currJ));					
				}
				else if (remapValueMode > 0)
				{
					channelValueModes[ch] = static_cast<ValueMode>(json_integer_value(currJ) + remapValueMode);					
				}
				else
				{
					channelValueModes[ch] = valueModesSupported[0];
				}	
			}
		}
		modeString = modeStrings[channelValueModes[currentChannelEditingIx]]; // Mode string will be the channel we are currently showing/editing
	}
	else 
	{
		// Set them all to selectedOutputValueMode (this is from an older save where there was only one output mode).
		for (int ch = 0; ch < TROWA_SEQ_NUM_CHNLS; ch++)
		{
			channelValueModes[ch] = selectedOutputValueMode;
		}
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
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			for (int t = 0; t < TROWA_SEQ_NUM_CHNLS; t++)
			{
				for (int s = 0; s < maxSteps; s++)
				{
					json_t *gateJ = json_array_get(triggersJ, i++);
					if (gateJ)
						triggerState[p][t][s] = (float)json_number_value(gateJ);
				} // end for (steps)
			} // end for (triggers)
		} // end for (patterns)			
	}
	// gateMode
	json_t *gateModeJ = json_object_get(rootJ, "gateMode");
	if (gateModeJ)
		gateMode = (GateMode)json_integer_value(gateModeJ);

	json_t* oscJ = json_object_get(rootJ, "osc");
	if (oscJ)
	{
		currJ = json_object_get(oscJ, "IpAddress");
		if (currJ)
			this->currentOSCSettings.oscTxIpAddress = json_string_value(currJ);
		currJ = json_object_get(oscJ, "TxPort");
		if (currJ)
			this->currentOSCSettings.oscTxPort = (uint16_t)(json_integer_value(currJ));
		currJ = json_object_get(oscJ, "RxPort");
		if (currJ)
			this->currentOSCSettings.oscRxPort = (uint16_t)(json_integer_value(currJ));
		currJ = json_object_get(oscJ, "Client");
		if (currJ)
			this->oscCurrentClient = static_cast<OSCClient>((uint8_t)(json_integer_value(currJ)));
		currJ = json_object_get(oscJ, "AutoReconnectAtLoad");
		if (currJ)
			oscReconnectAtLoad = json_boolean_value(currJ);
		if (oscReconnectAtLoad)
		{
			currJ = json_object_get(oscJ, "Initialized");
			if (currJ && json_boolean_value(currJ))
			{
				oscCurrentAction = OSCAction::Enable; // Will enable at next step
			}
		}
	} // end if osc
		
	// Pattern Sequencing:
	if (allowPatternSequencing)
	{
		DEBUG("(json) Retreiving Pattern Json");		
		json_t* psJ = json_object_get(rootJ, "patternSeq");
		if (psJ)
		{
			currJ = json_object_get(psJ, "AutoPatternSequence");
			if (currJ)
				patternSequencingOn = json_boolean_value(currJ);
			currJ = json_object_get(psJ, "PatternSequenceLength");
			if (currJ)
				numPatternsInSequence = (int)(json_integer_value(currJ));
			
			if (patternData != NULL)
			{
				json_t * pStepsJ = json_object_get(psJ, "Sequence");
				for (int p = 0; p < maxSteps; p++)
				{
					currJ = json_array_get(pStepsJ, p);
					patternData[p] = (short)(json_integer_value(currJ));
				} // end for (patterns)			
			}
			
			if (numPatternsInSequence < 1)
				numPatternsInSequence = 1;			
		} // end if patternSeq
	}
	firstLoad = true;
	return;
} // end dataFromJson()

// If a particular mode is supported by this module.
bool TSSequencerModuleBase::valueModeIsSupported(ValueMode mode)
{
	// If this is a voltSeq we need to remap it.
	return getSupportedValueModeIndex(mode) > -1;
} // end valueModeIsSupported()
// Gets the index into our supported value modes array of the given mode. Returns -1 if not found.
int TSSequencerModuleBase::getSupportedValueModeIndex(ValueMode mode)
{
	int ix = -1;
	if (valueModesSupported != NULL)
	{
		int k = 0;
		while (ix < 0 && k < numValueModesSupported)
		{
			if (valueModesSupported[k++] == mode)
			{
				ix = k - 1;
			}
		}		
	}
	return ix;
} // end getSupportedValueModeIndex(()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
// Reset param quantities (i.e. from knobs) when we get a reset from an external message source.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::resetParamQuantities()
{
	// List of parameters that need to be reset when we get an external message.
	const int numParamsToReset = 6;
	int paramIdsToReset[numParamsToReset] = 
	{ 
		ParamIds::BPM_PARAM, ParamIds::STEPS_PARAM, ParamIds::SELECTED_PATTERN_PLAY_PARAM, 
		ParamIds::SELECTED_PATTERN_EDIT_PARAM, ParamIds::SELECTED_CHANNEL_PARAM, ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM
	};
	for (int i = 0; i < numParamsToReset; i++)
	{
		paramQuantities[paramIdsToReset[i]]->setValue(paramQuantities[paramIdsToReset[i]]->getDefaultValue());
	}			
	return;
} // end resetParamQuantities()



//=======================================================================================================================================
//
// TS_ValueSequencerParamQuantity
//
//=======================================================================================================================================

// Gets the display string based on our value mode.
std::string TS_ValueSequencerParamQuantity::getDisplayValueString()
{
	std::string str;
	if (valueMode)
	{
		float val = valueMode->GetOutputValue(this->getValue());
		valueMode->GetDisplayString(val, buffer);
		str = std::string(buffer);
	}
	else 
	{
		str = ParamQuantity::getDisplayValueString();
	}
	return str;
}

void TS_ValueSequencerParamQuantity::setDisplayValueString(std::string s)
{
	float val = 0.0f;
	if (valueMode)
	{
		val = valueMode->GetKnobValueFromString(s);
		this->setDisplayValue(val);
	}
	else 
	{
		ParamQuantity::setDisplayValueString(s);
	}
	return;	
}
void TS_ValueSequencerParamQuantity::setValueMode(ValueSequencerMode* vMode)
{
	valueMode = vMode;
	minValue = valueMode->voltageMin;
	maxValue = valueMode->voltageMax;
	defaultValue = valueMode->zeroValue;
	//DEBUG("Setting valueMode = %s", valueMode->displayName);
	if (valueMode->unit && std::strlen(valueMode->unit) > 0)
	{
		unit = std::string(" ") + std::string(valueMode->unit);		
	}
	else
	{
		unit = std::string("");
	}
	//label = valueMode->displayName;
	return;
}


