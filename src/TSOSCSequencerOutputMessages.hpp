#ifndef  TSOSCSEQEUNCEROUTPUTMESSAGES_HPP
#define TSOSCSEQEUNCEROUTPUTMESSAGES_HPP


// Output messages over OSC for trowaSoft Sequencers.
enum SeqOSCOutputMsg {
	// Send Play State (Playing/Paused)
	PlayRunningState,
	// Send Internal step clock tick or External clock pulse
	PlayClock,
	// Sequencer RESET button click
	PlayReset,
	// Send Current Playing Pattern
	PlayPattern,
	// Send Current BPM
	PlayBPM,
	// Send BPM Divisor
	PlayBPMNote,
	// Send Step Value
	EditStep,
	// Send Current Length
	PlayLength,
	// Send Current Output Mode
	PlayOutputMode,
	// Send Current Edit Pattern
	EditPattern,
	// Send Current Edit Channel
	EditChannel,
	NUM_OSC_OUTPUT_MSGS
};


// Send Play State (Playing/Paused) (format string).
#define OSC_SEND_PLAY_RUNNINGSTATE_FS	"%s/play/state"
// Send Internal step clock tick or External clock pulse (format string).
#define OSC_SEND_PLAY_CLOCK_FS	"%s/clock"
// Sequencer RESET button click (format string).
#define OSC_SEND_PLAY_RESET_FS	"%s/reset"
// Send Current Playing Pattern (format string).
#define OSC_SEND_PLAY_PATTERN_FS	"%s/play/pat"
// Send Current BPM (format string).
#define OSC_SEND_PLAY_BPM_FS	"%s/bpm"
// Send BPM Divisor (format string).
#define OSC_SEND_PLAY_BPMNOTE_FS	"%s/bpmnote"
// Send Step Value (format string).
#define OSC_SEND_EDIT_STEP_FS	"%s/step"
// Send Current Length (format string).
#define OSC_SEND_PLAY_LENGTH_FS	"%s/play/len"
// Send Current Output Mode (format string).
#define OSC_SEND_PLAY_OUTPUTMODE_FS	"%s/play/omode"
// Send Current Edit Pattern (format string).
#define OSC_SEND_EDIT_PATTERN_FS	"%s/edit/pat"
// Send Current Edit Channel (format string).
#define OSC_SEND_EDIT_CHANNEL_FS	"%s/edit/ch"


// Format strings for our output OSC messages for our sequencers.
const char* const TSSeqOSCOutputFormats[] = {  
	OSC_SEND_PLAY_RUNNINGSTATE_FS,
	OSC_SEND_PLAY_CLOCK_FS,
	OSC_SEND_PLAY_RESET_FS,
	OSC_SEND_PLAY_PATTERN_FS,
	OSC_SEND_PLAY_BPM_FS,
	OSC_SEND_PLAY_BPMNOTE_FS,
	OSC_SEND_EDIT_STEP_FS,
	OSC_SEND_PLAY_LENGTH_FS,
	OSC_SEND_PLAY_OUTPUTMODE_FS,
	OSC_SEND_EDIT_PATTERN_FS,
	OSC_SEND_EDIT_CHANNEL_FS
};


#endif //  TSOSCSEQEUNCEROUTPUTMESSAGES_HPP
