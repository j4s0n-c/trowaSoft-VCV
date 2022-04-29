#ifndef TROWASOFT_MODULE_TSSEQUENCERBASE_HPP
#define TROWASOFT_MODULE_TSSEQUENCERBASE_HPP

#include <rack.hpp>
using namespace rack;

/// TODO: Updates to sequencers:
/// 1. Allow output type per Channel (TRIG/RTRG/GATE or VOLT/NOTE/PATT).
/// Maybe/TODO:
/// 1. Each Channel has own length or Each Pattern-Channel has it's own length. Then each channel needs its own playhead and length.
/// 2. Song Mode: Auto go to the next Pattern --> The playhead from Channel i will stay the in the step??? This is funky...
/// --> Redo into Channel structure with Length, StepValues, OutputType, PlayHead.
/// --> 64 Patterns * 16 Channels...



#include <thread> // std::thread
#include <mutex>
#include <queue>
#include <vector>
#include <string.h>
#include <stdio.h>
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include <chrono>
#include "TSTempoBPM.hpp"
#include "TSExternalControlMessage.hpp"
#include "TSOSCCommon.hpp"
#include "TSOSCSequencerListener.hpp"
#include "TSOSCCommunicator.hpp"
#include "TSOSCSequencerOutputMessages.hpp"
//#include "TSSequencerWidgetBase.hpp"
#include "TSParamQuantity.hpp"

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

#define TROWA_SEQ_NUM_CHNLS		16	// Num of channels/triggers/voices
#define TROWA_SEQ_NUM_STEPS		16  // Num of steps per channel/gate/voice
#define TROWA_SEQ_MAX_NUM_STEPS	64  // Maximum number of steps

#define N64_NUM_STEPS	64
#define N64_NUM_ROWS	 8
#define N64_NUM_COLS	 (N64_NUM_STEPS/N64_NUM_ROWS)


// If we should update the current step pointer to OSC (turn off prev step, highlight current step).
// This gets slow though during testing.
#define OSC_UPDATE_CURRENT_STEP_LED		1

// We only show 4x4 grid of steps at time.
#define TROWA_SEQ_STEP_NUM_ROWS	4	// Num of rows for display of the Steps (single channel displayed at a time)
#define TROWA_SEQ_STEP_NUM_COLS	(TROWA_SEQ_NUM_STEPS/TROWA_SEQ_STEP_NUM_ROWS)

#define TROWA_SEQ_NUM_MODES		3
#define TROWA_SEQ_STEPS_MIN_V	TROWA_SEQ_PATTERN_MIN_V   // Min voltage input / output for controlling # steps
#define TROWA_SEQ_STEPS_MAX_V	TROWA_SEQ_PATTERN_MAX_V   // Max voltage input / output for controlling # steps
#define TROWA_SEQ_BPM_KNOB_MIN		-2	
#define TROWA_SEQ_BPM_KNOB_MAX		 6
#define TROWA_SEQ_SWING_ADJ_MIN			-0.5
#define TROWA_SEQ_SWING_ADJ_MAX		     0.5
#define TROWA_SEQ_SWING_STEPS		4
// 0 WILL BE NO SWING

// To copy all channels in the selected target Pattern
#define TROWA_SEQ_COPY_CHANNELIX_ALL		TROWA_INDEX_UNDEFINED 

#define TROWA_SEQ_NUM_RANDOM_PATTERNS			27
#define TROWA_SEQ_BOOLEAN_NUM_RANDOM_PATTERNS	 7	// Num of patterns that actually apply to a boolean sequencer. List should always be sorted by #Unique Values. Only #Unique vals 1-2 should be chosen.

#define TROWA_SEQ_BUFF_SIZE		100

////////////////////////
// Pattern Sequencing //
////////////////////////
#define DEBUG_PATT_SEQ						0 // Debug pattern sequencing.
#define TS_PATTERN_SEQ_STATUS_COLOR		    nvgRGB(0x5e, 0x5e, 0xB8) //nvgRGB(0xCD, 0xE2, 0xB8) /// TSColors::COLOR_MAGENTA
#define TS_PATTERN_SEQ_STEP_COLOR			TSColors::COLOR_WHITE
#define TS_PATTERN_SEQ_STEP_COLOR_INACTIVE	nvgRGB(0xDE, 0xDE, 0xDE)


//////////////////////////////////////////////////////////////////////////
// Internal Sampling Divisor
// For performance, do not run every evaluation every step (process())
// https://github.com/j4s0n-c/trowaSoft-VCV/issues/51
//////////////////////////////////////////////////////////////////////////
#define TROWA_SEQ_USE_INTERNAL_DIVISOR		1
#define DEUBG_TROWA_SEQ_SAMPLE_DIVISOR		0


//////////////////////////////////////////////////////////////////////////
// Menu strings
//////////////////////////////////////////////////////////////////////////
// j4s0n wants 'ALL Patterns' to be 'ALL Patterns & Channels'
#define STR_ALL_PATTERNS		"ALL Patterns & Channels"
#define STR_CURR_EDIT_CHANNEL	"Current Edit Channel"
#define STR_CURR_EDIT_PATTERN	"Current Edit Pattern"
#define STR_SONG_MODE			"Song Mode"

// Random Structure
// From feature request: https ://github.com/j4s0n-c/trowaSoft-VCV/issues/10
struct RandStructure {
	uint8_t numDiffVals;
	std::vector<uint8_t> pattern;
};

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerModuleBase
// Sequencer Base Class
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct TSSequencerModuleBase : Module 
{
	// Some amount to add to Volts to Pattern since it doesn't seem to reverse back correctly when we are reading the input.
	const float volts2PatternAdj = 0.05f;
	
	bool debugFirstRealStep = false;
	bool debugFirstExecutedStep = false;
	
#if DEBUG_PATT_SEQ
	float debugLastPatternInputVoltage = -1000.0f;
	float debugLastPatternIxFloat = -1000.0f;
	int debugLastPatternIx = -1;
#endif
	

	enum ParamIds {
		// BPM Knob
		BPM_PARAM,
		// Run toggle		
		RUN_PARAM,
		// Reset Trigger (Momentary)		
		RESET_PARAM,
		// Step length
		STEPS_PARAM,
		SELECTED_PATTERN_PLAY_PARAM,  // What pattern we are playing
		SELECTED_PATTERN_EDIT_PARAM,  // What pattern we are editing
		SELECTED_CHANNEL_PARAM,	 // Which channel is selected for editing		
		SELECTED_OUTPUT_VALUE_MODE_PARAM,     // Which value mode we are doing	
		SWING_ADJ_PARAM, // Amount of swing adjustment (-0.1 to 0.1)
		COPY_PATTERN_PARAM, // Copy the current editing Pattern
		COPY_CHANNEL_PARAM, // Copy the current Channel/gate/trigger in the current Pattern only.
		PASTE_PARAM, // Paste what is on our clip board to the now current editing.
		SELECTED_BPM_MULT_IX_PARAM, // Selected index into our BPM calculation multipliers (for 1/4, 1/8, 1/8T, 1/16 note calcs)
		OSC_SAVE_CONF_PARAM, // ENABLE and Save the configuration for OSC
		OSC_AUTO_RECONNECT_PARAM,   // Auto-reconnect OSC on load from save file.
		OSC_SHOW_CONF_PARAM, // Configure OSC toggle
		PATTERN_SEQ_SHOW_CONFIG_PARAM, // Show Pattern Sequencing Configuration
		PATTERN_SEQ_ON_PARAM, // Pattern Sequencing On
		PATTERN_SEQ_LENGTH_PARAM, // How many patterns in the sequence.
		CHANNEL_PARAM, // Edit Channel/Step Buttons/Knobs
		NUM_PARAMS = CHANNEL_PARAM // Add the number of steps separately...
	};
	enum InputIds {
		// BPM Input
		BPM_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		SELECTED_PATTERN_PLAY_INPUT,  // What pattern we are playing		
		SELECTED_PATTERN_EDIT_INPUT,  // What pattern we are editing
		UNUSED_INPUT,
		NUM_INPUTS
	};
	// Each of the 16 voices need a gate output
	enum OutputIds {
		CHANNELS_OUTPUT, // Output Channel ports
		NUM_OUTPUTS = CHANNELS_OUTPUT + TROWA_SEQ_NUM_CHNLS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		COPY_PATTERN_LIGHT, // Copy pattern
		COPY_CHANNEL_LIGHT, // Copy channel
		PASTE_LIGHT,	// Paste light
		SELECTED_BPM_MULT_IX_LIGHT,	// BPM multiplier/note index
		OSC_CONFIGURE_LIGHT, // The light for configuring OSC.
		OSC_ENABLED_LIGHT, // Light for OSC enabled and currently running/active.
		PATTERN_SEQ_CONFIGURE_LIGHT, // Light for configurating Internal Pattern Sequencer
		PATTERN_SEQ_ENABLED_LIGHT,   // Light for Internal Pattern Sequencer currently on/active.
		CHANNEL_LIGHTS, // Channel output lights.		
		PAD_LIGHTS = CHANNEL_LIGHTS + TROWA_SEQ_NUM_CHNLS, // Lights for the steps/pads for the currently editing Channel
		// Not the number of lights yet, add the # of steps (maxSteps)
		NUM_LIGHTS = PAD_LIGHTS // Add the number of steps separately...
	};



	// The random structures/patterns
	static RandStructure RandomPatterns[TROWA_SEQ_NUM_RANDOM_PATTERNS];

	// If the module has been fully initialized or not.
	bool initialized = false;
	// If reset is pressed while paused, when we play, we should fire step 0.
	bool resetPaused = false;
	// [03/30/2015] A  reset has been queued for the next step. (https://github.com/j4s0n-c/trowaSoft-VCV/issues/11)
	// So now reset is not immediate, but will wait for the next step.
	bool resetQueued = false;
	// If this module is running.
	bool running = true;
	dsp::SchmittTrigger clockTrigger; 		// for external clock
	dsp::SchmittTrigger runningTrigger;		// Detect running btn press
	dsp::SchmittTrigger resetTrigger;		// Detect reset btn press
	float realPhase = 0.0;
	// Index into the sequence (step)
	int index = 0;
	// Last index we played (for OSC)
	int prevIndex = TROWA_INDEX_UNDEFINED;
	// Next index in to jump to in the sequence (step) if any. (for external controls)
	int nextIndex = TROWA_INDEX_UNDEFINED;
	// Flag if values are being changed outside of step().
	bool valuesChanging = false;
	// The current step being edited by knobs. (For the top display to show the value).
	int currentStepBeingEditedIx = -1;
	int currentStepBeingEditedParamId = -1;
		

	enum GateMode : short {
		TRIGGER = 0,
		RETRIGGER = 1,
		CONTINUOUS = 2,
	};
	// Gate mode from the knob.
	GateMode gateMode = TRIGGER;
	dsp::PulseGenerator gatePulse;

	// enum ValueMode : short 
	// {
		// VALUE_TRIGGER = 0,
		// VALUE_RETRIGGER = 1,
		// VALUE_CONTINUOUS = 2,
		// VALUE_VOLT = 0,
		// VALUE_MIDINOTE = 1,
		// VALUE_PATTERN = 2,
		// MIN_VALUE_MODE = 0,
		// MAX_VALUE_MODE = 2,
		// NUM_VALUE_MODES = MAX_VALUE_MODE + 1
	// };	

	/////////////////////////////////
	// Mode /////////////////////////
	/////////////////////////////////
	// [v1.0.4] Redo so they all have unique values.
	enum ValueMode : short 
	{
		VALUE_TRIGGER = 0,		// Trigger
		VALUE_RETRIGGER = 1, 	// Retrigger
		VALUE_CONTINUOUS = 2, 	// Gate
		// v1.0.4 - Modes need unique values since new sequencer can do them all....
		VALUE_VOLT = 3, 	// Raw voltage
		VALUE_MIDINOTE = 4, // Force to MIDI Note voltages
		VALUE_PATTERN = 5, 	// Force to Pattern voltages
		MIN_VALUE_MODE = 0,
		MAX_VALUE_MODE = 5,
		NUM_VALUE_MODES = MAX_VALUE_MODE + 1
	};	
	// The mode string.
	const char* modeString;
	// Mode strings (ENUMERATE ALL)
	const char* modeStrings[ValueMode::NUM_VALUE_MODES] = { "TRIG", "RTRG", "GATE", "VOLT", "NOTE", "PATT" };		
	// Array of ValueModes the sequencer supports. Derived class should set this. 
	ValueMode* valueModesSupported = NULL;
	// Number of ValueModes the sequencer supports. Derived class should set this. 
	int numValueModesSupported = 3;
	// Default value mode for this type of sequencer. Derived class should set this.
	ValueMode defaultChannelValueMode = ValueMode::VALUE_TRIGGER;
	// For child class to call after setting valueModesSupported.
	virtual void reconfigureValueModeParamQty()
	{
		// Value Mode Knob is now index into our array:
		TS_ParamQuantityEnum* pQty = dynamic_cast<TS_ParamQuantityEnum*>(this->paramQuantities[TSSequencerModuleBase::ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM]);
		pQty->minValue = 0; 
		pQty->maxValue = numValueModesSupported - 1;
		pQty->defaultValue = selectedOutputValueModeIx; // Index into our array.
		for (int i = 0; i < numValueModesSupported; i++)
		{
			int k = valueModesSupported[i];
			pQty->addToEnumMap(i, modeStrings[k]);
		}
		return;
	}

	// Selected output value mode.
	ValueMode selectedOutputValueMode = VALUE_TRIGGER;
	// Index into our valueModesSupported array of the selected output value mode. This is now what the Knob value will point to (index not the actual enum mode).
	int selectedOutputValueModeIx = 0;
	ValueMode lastOutputValueMode = VALUE_TRIGGER;
	// [v1.0.1] Each channel will now have its own mode.
	ValueMode channelValueModes[TROWA_SEQ_NUM_CHNLS];	


	///////////////////////////////////////////////////
	// Steps
	///////////////////////////////////////////////////	
	// Maximum number of steps for this sequencer.
	int maxSteps = 16;
	// The number of rows for steps (for layout).
	int numRows = 4;
	// The number of columns for steps (for layout).
	int numCols = 4;
	// Step data for each pattern and channel.
	float * triggerState[TROWA_SEQ_NUM_PATTERNS][TROWA_SEQ_NUM_CHNLS];
	dsp::SchmittTrigger* gateTriggers;

	// Knob indices for top control knobs.
	enum KnobIx {
		// Playing pattern knob ix
		PlayPatternKnob = 0,
		// Playing BPM knob ix
		BPMKnob,
		// Playing Step Length ix
		StepLengthKnob,
		// Output mode knob ix
		OutputModeKnob,
		// Edit pattern knob ix
		EditPatternKnob,
		// Edic channel knob ix
		EditChannelKnob,
		// Number of Control Knobs
		NumKnobs
	};
	// References to input knobs (top row of knobs)
	/// TODO: Get of Knob references & manipluation (separate GUI elements from module)
	//TS_RoundBlackKnob* controlKnobs[NumKnobs];

	// Another flag to reload the matrix.
	bool reloadEditMatrix = false;
	// Keep track of the pattern that was playing last step (for OSC)
	int lastPatternPlayingIx = -1;
	// Index of which pattern we are playing
	int currentPatternEditingIx = 0;
	// Index of which pattern we are editing 
	int currentPatternPlayingIx = 0;
	// Index of which channel (trigger/gate/voice) is currently displayed/edited.
	int currentChannelEditingIx = 0;
	/// TODO: Perhaps change this to setting for each pattern or each pattern-channel.
	// The current number of steps to play
	int currentNumberSteps = TROWA_SEQ_NUM_STEPS;
	// Calculated current BPM
	float currentBPM = 0.0f;
	// If the last step was the external clock
	bool lastStepWasExternalClock = false;
	// Currently stored pattern (for external control like OSC clients that can not store values themselves, the controls can set a 'stored' value
	// and then have some button click fire off the SetPlayPattern message with -1 as argument and we'll use this.
	int storedPatternPlayingIx = 0;
	// Currently stored length (for external control like OSC clients that can not store values themselves, the controls can set a 'stored' value
	// and then have some button click fire off the SetPlayLength message with -1 as argument and we'll use this.
	int storedNumberSteps = TROWA_SEQ_NUM_STEPS;
	// Currently stored BPM (for external control like OSC clients that can not store values themselves, the controls can set a 'stored' value
	// and then have some button click fire off the SetPlayBPM message with -1 as argument and we'll use this.
	int storedBPM = 120;
	//// Last time of the external step
	//std::chrono::high_resolution_clock::time_point lastExternalStepTime;

	/// TODO: Get of Light references & manipluation (separate GUI elements from module)
	// Pad/Knob lights - Step On
	float** stepLights; /// TODO: Just make linear
	float** gateLights; /// TODO: Just make linear

	// Default values for our pads/knobs:
	float defaultStateValue = 0.0;

	/// TODO: Get of Light references & manipluation (separate GUI elements from module)
	// References to our pad lights
	ColorValueLight*** padLightPtrs; /// TODO: Just make linear
	
	NVGcolor currentStepMatrixColor = TSColors::COLOR_TS_RED;
	NVGcolor currentCopyChannelColor;	
	NVGcolor currentPasteColor;	

	// Output lights (for triggers/gate jacks)
	float gateLightsOut[TROWA_SEQ_NUM_CHNLS];
	// Colors for each channel
	NVGcolor voiceColors[TROWA_SEQ_NUM_CHNLS] = {
		TSColors::COLOR_TS_RED, TSColors::COLOR_DARK_ORANGE, TSColors::COLOR_YELLOW, TSColors::COLOR_TS_GREEN,
		TSColors::COLOR_CYAN, TSColors::COLOR_TS_BLUE, TSColors::COLOR_PURPLE, TSColors::COLOR_PINK,
		TSColors::COLOR_TS_RED, TSColors::COLOR_DARK_ORANGE, TSColors::COLOR_YELLOW, TSColors::COLOR_TS_GREEN,
		TSColors::COLOR_CYAN, TSColors::COLOR_TS_BLUE, TSColors::COLOR_PURPLE, TSColors::COLOR_PINK
	};

	// Swing ////////////////////////////////
	float swingAdjustment = 0.0; // Amount of swing adjustment (i.e. -0.1 to 0.1)
	const int swingResetSteps = TROWA_SEQ_SWING_STEPS; // These many steps need to be adjusted.
	float swingAdjustedPhase = 0.0;
	int swingRealSteps = 0;

	// Copy & Paste /////////////////////////
	// Source pattern to copy
	int copySourcePatternIx = -1;
	// Source channel to copy (or TROWA_SEQ_COPY_CHANNELIX_ALL for all).
	int copySourceChannelIx = TROWA_SEQ_COPY_CHANNELIX_ALL;
	// Copy buffer (steps)
	float* copyBuffer[TROWA_SEQ_NUM_CHNLS];
	dsp::SchmittTrigger copyPatternTrigger;
	dsp::SchmittTrigger copyGateTrigger;
	dsp::SchmittTrigger pasteTrigger;
	// Color for Copy Pattern
	static const NVGcolor COPY_PATTERN_COLOR; 
	// Color for Running Light 
	static const NVGcolor RUNNING_COLOR;
	// Color for Reset Light 
	static const NVGcolor RESET_COLOR;

	// BPM Calculation //////////////
	// Index into the array BPMOptions
	int selectedBPMNoteIx = 1; // 1/8th
	dsp::SchmittTrigger selectedBPMNoteTrigger;

	// External Messages ///////////////////////////////////////////////
	// Message queue for external (to Rack) control messages
	std::queue<TSExternalControlMessage> ctlMsgQueue;

	enum ExternalControllerMode {
		// Edit Mode : Send to control what we are editing.
		EditMode = 0,
		// Performance/Play mode : Send to controller what we are playing
		// SetStepValue messages should be interupted as SetPlayingStep (Jump)
		PerformanceMode = 1
	};
	// The current control mode (i.e. Edit Mode or Play / Performance Mode)
	ExternalControllerMode currentCtlMode = ExternalControllerMode::EditMode;

	// OSC Messaging ////////////////
	// If we allow osc or not.
	bool allowOSC = true;
	// Flag if we should use OSC or not.
	bool useOSC = true;
	// An OSC id.
	int oscId = 0;
	// Mutex for osc messaging.
	std::mutex oscMutex;
	// Current OSC IP address and port settings.
	TSOSCConnectionInfo currentOSCSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Configure trigger
	dsp::SchmittTrigger oscConfigTrigger;
	dsp::SchmittTrigger oscConnectTrigger;
	dsp::SchmittTrigger oscDisconnectTrigger;
	// Show the OSC configuration screen or not.
	bool oscShowConfigurationScreen = false;
	// Flag to reconnect at load. IFF true and oscInitialized is also true.
	bool oscReconnectAtLoad = false;
	// Flag if OSC objects have been initialized
	bool oscInitialized = false;
	// If there is an osc error.
	bool oscError = false;
	// OSC output buffer.
	char* oscBuffer = NULL;
	// OSC namespace to use
	std::string oscNamespace = OSC_DEFAULT_NS;
	// Sending OSC socket
	UdpTransmitSocket* oscTxSocket = NULL;
	// OSC message listener
	TSOSCSequencerListener* oscListener = NULL;
	// Receiving OSC socket
	UdpListeningReceiveSocket* oscRxSocket = NULL;
	// The OSC listener thread
	std::thread oscListenerThread;
	// Osc address buffer. 
	char oscAddrBuffer[SeqOSCOutputMsg::NUM_OSC_OUTPUT_MSGS][OSC_ADDRESS_BUFFER_SIZE];
	// Prev step that was last turned off (when going to a new step).
	int oscLastPrevStepUpdated = TROWA_INDEX_UNDEFINED;
	// Settings for new OSC.
	TSOSCInfo oscNewSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Mode action (i.e. Enable, Disable)
	enum OSCAction {
		None,
		Disable,
		Enable
	};
	// Flag for our module to either enable or disable osc.
	OSCAction oscCurrentAction = OSCAction::None;
	// The current osc client. Clients such as touchOSC and Lemur are limited and need special treatment.
	OSCClient oscCurrentClient = OSCClient::GenericClient;


	// If it is the first load this session
	bool firstLoad = true;
	// If this was loaded from a save, what version
	int saveVersion = -1;
	
	// Base light change
	const float baseLightLambda = 0.05f;
	// Current light change
	float lightLambda = baseLightLambda;
	
#if TROWA_SEQ_USE_INTERNAL_DIVISOR	
	//--* Skipping Evaluations *--//
	// Issue #51 https://github.com/j4s0n-c/trowaSoft-VCV/issues/51
	// Skip evaluations of changes from UI
	int idleCounter = -1;
	// Either allow user to define ? or based off the current engine sampling rate
	// (my dev laptop doesn't show high usage unless I crank up the sample rate to over 96 k).
	// User Stubs42 used 32 as his internal sampling divisor but was also using the internal clock
	// For triggers, they must be 1 ms, so let's adjust the threshold/internal sampling divisor such that
	// we run at least every 0.5 ms based on the engine sampling rate.
	const int MAX_ILDETHRESHOLD = 32;	
	int IDLETHRESHOLD = MAX_ILDETHRESHOLD;	
#endif 	
	
	// The number of structured random patterns to actually use. Should be <= TROWA_SEQ_NUM_RANDOM_PATTERNS.
	int numStructuredRandomPatterns = TROWA_SEQ_BOOLEAN_NUM_RANDOM_PATTERNS;
	
	//-----------------------------///////	
	// INTERNAL PATTERN SEQUENCING ///////
	//-----------------------------///////
	// If this module has internal pattern sequencing (no to most, only multiSeq should have it for now).
	bool allowPatternSequencing = false;	
	// Which pattern to play (for pattern sequencing).
	int patternPlayHeadIx = -1;
	// Flag if pattern sequencing is on.
	bool patternSequencingOn = false;
	// Last step's pattern sequencing on/off value.
	bool lastPatternSequencingOn = false;
	// The patterns to play.
	short* patternData = NULL;
	// Show the pattern seqeuncing configuration.
	bool showPatternSequencingConfig = false;
	// Last value for showing the pattern sequencing configuration.
	bool lastShowPatternSequencingConfig = false;
	// Show pattern sequecing configuration trigger.
	dsp::SchmittTrigger patternSeqConfigTrigger;
	// Number of pattern steps in the sequence.
	int numPatternsInSequence = 0;
	// The parameter id where our pattern #'s start for sequencing.
	int PATTERN_SEQ_PARAM_START = -1;
	// The light id where our pattern #'s start for sequencing.
	int PATTERN_SEQ_LIGHT_START = -1;
	// The current pattern index being edited (index into patternData).
	int currentPatternDataBeingEditedIx = -1;
	// The current pattern param id being edited.
	int currentPatternDataBeingEditedParamId = -1;	
	// The control source. (Where a control command came from).
	enum ControlSource : uint8_t
	{
		// Highest Priority Source (0) - External message (OSC or in future direct MIDI integration)
		ExternalMsgSrc = 0,
		// Control Voltage Input - Priority (1)
		CVInputSrc = 1,
		// For like Auto-Pattern-Sequencing - Priority (2)
		AutoSrc = 2,
		// User Parameter (UI) Control - Priority (3 = Last)
		UserParameterSrc = 3
	};
	// What is controlling the current pattern playing.
	ControlSource patternPlayingControlSource = ControlSource::UserParameterSrc;
	

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSequencerModuleBase()
	// Instantiate the abstract base class.
	// @numSteps: (IN) Maximum number of steps
	// @numRows: (IN) The number of rows (for layout).
	// @numCols: (IN) The number of columns (for layout).
	// @numRows * @numCols = @numSteps.
	// @defStateVal : (IN) The default state value (i.e. 0/false for a boolean step sequencer or whatever float value you want).
	// @defChannelValMode : (IN) The default value mode for channels. (v1.0.4 now that VALUE_TRIGGER and VALUE_VOLT are unique).
	// @enablePatternSequencing: (IN) (Opt'l) Enable pattern sequencing for this type of module. Default is false.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSequencerModuleBase(/*in*/ int numSteps, /*in*/ int numRows, /*in*/ int numCols, /*in*/ float defStateVal, /*in*/ TSSequencerModuleBase::ValueMode defChannelValMode, bool enablePatternSequencing = false);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Delete our goodies.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	~TSSequencerModuleBase();
	
	// If a particular mode is supported by this module.
	bool valueModeIsSupported(ValueMode mode);	
	// Gets the index into our supported value modes array of the given mode. Returns -1 if not found.
	int getSupportedValueModeIndex(ValueMode mode);
	
	// Get the inputs for this step.
	void getStepInputs(const ProcessArgs &args, bool* pulse, bool* reloadMatrix, bool* valueModeChanged, bool nextStep, float clockTime);
	// Paste the clipboard pattern and/or specific gate to current selected pattern and/or gate.
	bool paste();
	// Copy the contents:
	void copy(int patternIx, int channelIx);
	// Set a single step value
	virtual void setStepValue(int step, float val, int channel, int pattern);
	// Get the toggle step value
	virtual float getToggleStepValue(int step, float val, int channel, int pattern) = 0;
	// Calculate a representation of all channels for this step
	virtual float getPlayingStepValue(int step, int pattern) = 0;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	// Reset param quantities (i.e. from knobs) when we get a reset from an external message source.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void resetParamQuantities();
	

	// Initialize OSC on the given ip and ports.
	void initOSC(const char* ipAddress, int outputPort, int inputPort);
	// Clean up OSC.
	void cleanupOSC();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Set the OSC namespace.
	// @oscNs: (IN) The namespace for OSC.
	// Sets the command address strings too.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setOSCNamespace(const char* oscNs);
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getValueSeqChannelModes()
	// Gets the array of ValueSequencerModes if any.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	virtual ValueSequencerMode** getValueSeqChannelModes() 
	{
		return NULL;
	}


	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset(void)
	// Reset ALL step values to default.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void onReset() override;	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset()
	// Only the given pattern and channel.
	// @patternIx: (IN) Pattern. TROWA_INDEX_UNDEFINED is all.
	// @channelIx: (IN) Channel. TROWA_INDEX_UNDEFINED is all.
	// @resetChannelMode: (IN)(Opt'l) Reset the channel mode also (to default like TRIG or VOLT). 
	//                    Default is true.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	virtual void reset(int patternIx, int channelIx, bool resetChannelMode = true);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// resetPatternSequence()
	// Reset the pattern sequence / songmode steps only.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void resetPatternSequence();
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize()
	// Only randomize the current gate/trigger steps by default.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void onRandomize(const RandomizeEvent& e) override
	{		
		randomize(currentPatternEditingIx, currentChannelEditingIx, false);
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize()
	// @patternIx : (IN) The index into our pattern matrix (0-63). Or TROWA_INDEX_UNDEFINED for all patterns.
	// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
	// @useStructured: (IN) Create a random sequence/pattern of random values.
	// Random all from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/8
	// Structured from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/10
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void randomize(int patternIx, int channelIx, bool useStructured);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomizePatternSequence()
	// Randomize just the pattern sequence/song mode.
	// @useStructured: (IN) Create a random sequence/pattern of random values.			
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void randomizePatternSequence(bool useStructured);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRandomValue()
	// Get a random value for a step in this sequencer.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	virtual float getRandomValue() {
		// Default are boolean sequencers
		return random::uniform() > 0.5;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRandomValue()
	// Get a random value for a step in this sequencer based on the channel.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	virtual float getRandomValue(int channelIx) {
		// Default are boolean sequencers
		return getRandomValue();
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// onShownStepChange()
	// If we changed a step that is shown on the matrix, then do something.
	// For voltSeq to adjust the knobs so we dont' read the old knob values again.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	virtual void onShownStepChange(int step, float val) {
		// DO nothing
		return;
	}
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// isNewStep()
	// We advance to new step or not.
	// @sampleRate : (IN) Current sample rate.
	// @clockTime : (OUT) The calculated internal clock time.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	bool isNewStep(float sampleRate, float* clockTime);

#if TROWA_SEQ_USE_INTERNAL_DIVISOR
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	// onSampleRateChange()
	// User changes the engine sampling rate. Adjust our internal sampling rate 
	// such that we run still at least every 0.5 ms (triggers should be 1 ms).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	virtual void onSampleRateChange() override;
#endif 
	

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// clearClipboard(void)
	// Shallow clear of clipboard and reset the Copy/Paste lights
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	void clearClipboard()
	{
		copySourcePatternIx = -1;
		copySourceChannelIx = TROWA_SEQ_COPY_CHANNELIX_ALL; // Which trigger we are copying, -1 for all		
		lights[COPY_CHANNEL_LIGHT].value = 0;
		currentPasteColor = TSColors::COLOR_WHITE; // Return the paste light to white
		//pasteLight->setColor(TSColors::COLOR_WHITE); // Return the paste light to white
		lights[COPY_PATTERN_LIGHT].value = 0;
		lights[PASTE_LIGHT].value = 0;
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataToJson(void)
	// Save our junk to json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual json_t *dataToJson() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	// Read in our junk from json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void dataFromJson(json_t *rootJ) override;
}; // end struct TSSequencerModuleBase


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ParamQuantity for our value modes.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TS_ValueSequencerParamQuantity : TS_ParamQuantity 
{
	// Most of this functionality was already done in ValueSequencerMode back in v0.5 or whatever, so use it.
	ValueSequencerMode* valueMode;	
	char buffer[50];
	TS_ValueSequencerParamQuantity() : TS_ParamQuantity()
	{
		return;
	}
	void setValueMode(ValueSequencerMode* vMode);
	// Returns a string representation of the display value 
	std::string getDisplayValueString() override;
	// Given the string make it the float voltage.
	void setDisplayValueString(std::string s) override;
};

#endif