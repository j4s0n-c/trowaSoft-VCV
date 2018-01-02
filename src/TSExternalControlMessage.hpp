#ifndef TSEXTERNALCONTROLMESSAGE_HPP
#define TSEXTERNALCONTROLMESSAGE_HPP

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// External (from Rack) Control message.
// Currently this will be for OSC but may be from MIDI or anything else in the future.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSExternalControlMessage {
	// External source of a control message (i.e. OSC).
	// Currently only OSC
	enum MessageSource {
		// OSC: Open Sound Control
		OSC = 1
	};
	// Message Type / Action to do. Currently enumerated for our sequencers.
	enum MessageType {
		// Set Play State
		SetPlayRunningState,
		// Toggle Play/Pause
		TogglePlayRunningState,
		// Reset
		SetPlayReset,
		// Change Playing Pattern
		SetPlayPattern,
		// Set BPM
		SetPlayBPM,
		// Set BPM Note
		SetPlayBPMNote,
		// Add to the BPM Note Index (selection)
		AddPlayBPMNote,
		// Change Step Length
		SetPlayLength,
		// Set Ouput Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT)
		SetPlayOutputMode,
		// Change Edit Pattern
		SetEditPattern,
		// Change Edit Channel
		SetEditChannel,
		// Set the Step Value
		SetEditStepValue,
		// Toggle Edit Step
		ToggleEditStepValue,
		// Jump to Step Number (playing)
		SetPlayCurrentStep,
		// Set Mode (Edit, Performance)
		SetPlayMode,
		// Toggle Mode (Edit, Performance)
		TogglePlayMode,
		// Total # message types
		NUM_MESSAGE_TYPES
	};
	// The message type / action.
	MessageType messageType;
	// The message source (i.e. OSC).
	MessageSource messageSource;
	// Pattern number 0-63
	int pattern;
	// Channel 0-15
	int channel;
	// Step number 0-MaxSteps
	int step;
	// The mode.
	int mode;
	// The value / mode.
	float val;
};


#endif // !TSEXTERNALCONTROLMESSAGE_HPP

