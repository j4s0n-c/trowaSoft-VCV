#ifndef TS_OSC_SEQEUNCER_LISTENER_HPP
#define TS_OSC_SEQEUNCER_LISTENER_HPP

#include <string.h>
#include <stdio.h>
#include "util.hpp"
#include "rack.hpp"
using namespace rack;
#include "TSExternalControlMessage.hpp"
#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

struct TSSequencerModuleBase;


#define CURRENT_EDIT_PATTERN_IX		-1 // Flags that we should use the currentEditingPatternIx
#define CURRENT_EDIT_CHANNEL_IX		-1 // Flags that we should use the currentEditingTriggerIx

// Set Play State
// Parameters: int value
#define OSC_SET_PLAY_RUNNINGSTATE	"/play/state/set"
// Toggle Play/Pause
// Parameters: -NONE-
#define OSC_TOGGLE_PLAY_RUNNINGSTATE	"/play/state/tog"
// Reset
// Parameters: -NONE-
#define OSC_SET_PLAY_RESET	"/reset/set"
// Change Playing Pattern
// Parameters: int pattern
#define OSC_SET_PLAY_PATTERN	"/play/pat/set"
// Set BPM
// Parameters: float tempo
#define OSC_SET_PLAY_BPM	"/play/bpm/set"
// Set BPM Note
// Parameters: int divisorId
#define OSC_SET_PLAY_BPMNOTE	"/play/bpmnote/set"
// Add to the BPM Note Index (selection)
// Parameters: int addIx
#define OSC_ADD_PLAY_BPMNOTE	"/play/bpmnote/add"
// Change Step Length
// Parameters: int step
#define OSC_SET_PLAY_LENGTH	"/play/len/set"
// Set Ouput Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT)
// Parameters: int modeId
#define OSC_SET_PLAY_OUTPUTMODE	"/play/omode/set"
// Change Edit Pattern
// Parameters: int pattern
#define OSC_SET_EDIT_PATTERN	"/edit/pat/set"
// Change Edit Channel
// Parameters: int channel
#define OSC_SET_EDIT_CHANNEL	"/edit/ch/set"
// Set the Step Value
// Parameters: int stepNumber, float value, (opt) int pattern, (opt) int channel
#define OSC_SET_EDIT_STEPVALUE	"/edit/step/set"
// Toggle Edit Step
// Parameters: int stepNumber, (opt) float value, (opt) int pattern, (opt) int channel
#define OSC_TOGGLE_EDIT_STEPVALUE	"/edit/step/tog"
// Jump to Step Number (playing)
// Parameters: int stepNumber, float value, (opt) int pattern, (opt) int channel
#define OSC_SET_PLAY_CURRENTSTEP	"/play/step/set"
// Set Mode (Edit, Performance)
// Parameters: int mode
#define OSC_SET_PLAY_MODE	"/mode/set"
// Toggle Mode (Edit, Performance)
// Parameters: -NONE-
#define OSC_TOGGLE_PLAY_MODE	"/mode/tog"





//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Listener for OSC incoming messages.
// Currently each module must have its own listener object & slave thread since I'm not 100% sure about the threading in Rack (if we could keep
// one thread alive throughout the deaths of other modules). This way, its easy to clean up (when module dies, it kills its slave listener thread)
// instead of tracking how many modules are still alive and using OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
class TSOSCSequencerListener : public osc::OscPacketListener {
public:
	// Pointer to our sequencer module (so we can dump messages in its queue).
	TSSequencerModuleBase* sequencerModule;
	// OSC namespace to use. Currently, if message doesn't have this namespace, we will ignore it. In future, maybe one listener can feed multiple modules with different namespaces?
	std::string oscNamespace;
	// Instantiate a listener.
	TSOSCSequencerListener();
protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessMessage()
	// @rxMsg : (IN) The received message from the OSC library.
	// @remoteEndPoint: (IN) The remove end point (sender).
	// Handler for receiving messages from the OSC library. Taken from their example listener.
	// Should create a generic TSExternalControlMessage for our trowaSoft sequencers and dump it in the module instance's queue.
	//--------------------------------------------------------------------------------------------------------------------------------------------
	virtual void ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) override;
};
#endif