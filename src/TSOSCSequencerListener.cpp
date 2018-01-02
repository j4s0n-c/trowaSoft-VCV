#include <string.h>
#include <stdio.h>
#include "util.hpp"
#include "TSOSCSequencerListener.hpp"
#include "TSSequencerModuleBase.hpp"
#include "trowaSoftUtilities.hpp" // For debug

#define OSC_MSG_SRC		TSExternalControlMessage::MessageSource::OSC

inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	return msg;
}
inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType, int mode)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	msg.mode = mode;
	return msg;
}
inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType, int pattern, int channel, int step, float val)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	msg.pattern = pattern;
	msg.channel = channel;
	msg.step = step;
	msg.val = val;
	return msg;
}
inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType, int pattern, int channel, int step, float val, int mode)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	msg.pattern = pattern;
	msg.channel = channel;
	msg.step = step;
	msg.val = val;
	msg.mode = mode;
	return msg;
}
TSOSCSequencerListener::TSOSCSequencerListener()
{
	return;
}
//--------------------------------------------------------------------------------------------------------------------------------------------
// ProcessMessage()
// @rxMsg : (IN) The received message from the OSC library.
// @remoteEndPoint: (IN) The remove end point (sender).
// Handler for receiving messages from the OSC library. Taken from their example listener.
// Should create a generic TSExternalControlMessage for our trowaSoft sequencers and dump it in the module instance's queue.
//--------------------------------------------------------------------------------------------------------------------------------------------
void TSOSCSequencerListener::ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) 
{
	(void)remoteEndpoint; // suppress unused parameter warning
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	debug("[RECV] OSC Message: %s", rxMsg.AddressPattern());
#endif
	osc::int32 step = -1;
	float stepVal = 0.0;
	osc::int32 pattern = CURRENT_EDIT_PATTERN_IX;
	osc::int32 channel = CURRENT_EDIT_CHANNEL_IX;
	osc::int32 intVal = -1;
	try {
		const char* ns = this->oscNamespace.c_str();
		std::string addr = rxMsg.AddressPattern();
		int len = strlen(ns);
		if (std::strcmp(addr.substr(0, len).c_str(), ns) != 0) // Message is not for us
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("Message is not for our namespace (%s).", ns);
#endif
			return;
		}
		std::string subAddr = addr.substr(len);
		const char* path = subAddr.c_str();

		/// TODO: Try to order in order of frequency/commonality of the messages
		/// TODO: Do better/more efficient parsing (tree)
		if (std::strcmp(path, OSC_TOGGLE_EDIT_STEPVALUE) == 0 || std::strcmp(path, OSC_SET_EDIT_STEPVALUE) == 0)
		{
			// Set Step Value ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int stepNumber, float value, int pattern, int channel
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			stepVal = 1.0;
			try
			{
				// We should always get step and stepVal, but we should allow pattern & channel to be optional
				args >> step >> stepVal >> pattern >> channel >> osc::EndMessage;
			}
			catch (osc::MissingArgumentException& ex)
			{
				pattern = CURRENT_EDIT_PATTERN_IX;
				channel = CURRENT_EDIT_CHANNEL_IX;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step %d, val %f (Pattern %d, Channel %d).", path, step, stepVal, pattern, channel);
#endif
			if (step > -1)
			{
				// If we at least have a step.
				step = clampi(step, 0, sequencerModule->maxSteps - 1);
				TSExternalControlMessage::MessageType messageType = TSExternalControlMessage::MessageType::SetEditStepValue;
				if (std::strcmp(path, OSC_TOGGLE_EDIT_STEPVALUE) == 0)
				{
					messageType = TSExternalControlMessage::MessageType::ToggleEditStepValue;
				}
				// Queue up this message.
				sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(messageType, pattern, channel, step, stepVal));
			}
		}
		else if (std::strcmp(path, OSC_SET_PLAY_PATTERN) == 0)
		{
			// Set Playing Pattern :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int pattern
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> pattern >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d.", path, pattern);
#endif
			pattern = clampi(pattern, 0, TROWA_SEQ_NUM_PATTERNS - 1);
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayPattern, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_CURRENTSTEP) == 0)
		{
			// Set Playing Step/Jump :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int step
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step %d.", path, step);
#endif
			step = clampi(step, 0, sequencerModule->maxSteps - 1);
			/// TODO: Should we purge the queue so this guaranteed to happen immediately?
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayCurrentStep, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_EDIT_PATTERN) == 0)
		{
			// Set Editing Pattern :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int pattern
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> pattern >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d.", path, pattern);
#endif
			pattern = clampi(pattern, 0, TROWA_SEQ_NUM_PATTERNS - 1);
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetEditPattern, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_EDIT_CHANNEL) == 0)
		{
			// Set Editing Channel :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int channel
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> channel >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Channel %d.", path, channel);
#endif
			channel = clampi(channel, 0, TROWA_SEQ_NUM_CHNLS -1);
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetEditChannel, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_OUTPUTMODE) == 0)
		{
			// Set Output Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT) :::::::::::::::::::::::::::::::::::::::::::::::::::::
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> intVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Output Mode %d.", path, intVal);
#endif
			intVal = clampi(intVal, TSSequencerModuleBase::ValueMode::MIN_VALUE_MODE, TSSequencerModuleBase::ValueMode::MAX_VALUE_MODE);
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayOutputMode, /*mode*/ intVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_RESET) == 0)
		{
			// Reset :::::::::::::::::::::::::::::::::::::::::::::::::::::
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message", path);
#endif
			stepVal = 1;
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayReset, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_LENGTH) == 0)
		{
			// Set Play Length :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int step
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step Length %d.", path, step);
#endif
			step = clampi(step, 1, sequencerModule->maxSteps); // Must be 1 to 64 (not 0 to 63)
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayLength, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_TOGGLE_PLAY_RUNNINGSTATE) == 0 || std::strcmp(path, OSC_SET_PLAY_RUNNINGSTATE) == 0)
		{
			// Set Playing State ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			try
			{
				// We may or not may not always get a value (we should really check on SET, but oh well).
				args >> intVal >> osc::EndMessage;
			}
			catch (osc::MissingArgumentException& ex)
			{
				intVal = 1;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Play Val %d.", path, intVal);
#endif
			if (intVal > -1)
			{
				TSExternalControlMessage::MessageType messageType = TSExternalControlMessage::MessageType::SetPlayRunningState;
				if (std::strcmp(path, OSC_TOGGLE_PLAY_RUNNINGSTATE) == 0)
					messageType = TSExternalControlMessage::MessageType::TogglePlayRunningState;
				// Queue up this message.
				sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(messageType, pattern, channel, step, intVal));
			}
		}
		else if (std::strcmp(path, OSC_TOGGLE_PLAY_MODE) == 0 || std::strcmp(path, OSC_SET_PLAY_MODE) == 0)
		{
			// Set Control Mode ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			try
			{
				// We may or not may not always get a value (we should really check on SET, but oh well).
				args >> intVal >> osc::EndMessage;
			}
			catch (osc::MissingArgumentException& ex)
			{
				intVal = 1;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Control Mode Val %d.", rxMsg.AddressPattern(), intVal);
#endif
			if (intVal > -1)
			{
				TSExternalControlMessage::MessageType messageType = TSExternalControlMessage::MessageType::SetPlayMode;
				if (std::strcmp(rxMsg.AddressPattern(), OSC_TOGGLE_PLAY_MODE) == 0)
					messageType = TSExternalControlMessage::MessageType::TogglePlayMode;
				// Queue up this message.
				sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(messageType, pattern, channel, step, intVal, intVal));
			}
		}
		else if (std::strcmp(path, OSC_SET_PLAY_BPM) == 0)
		{
			// Set BPM/tempo :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//float tempo [0-1.0]
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> stepVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Tempo %.2f.", path, stepVal);
#endif
			stepVal = clampf(stepVal, 0, 1.0); // Must be 0 to 1
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayBPM, pattern, channel, step, stepVal));
		} // end if BPM
		else if (std::strcmp(path, OSC_ADD_PLAY_BPMNOTE) == 0)
		{
			// Set BPM Note Index :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpmIx
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Add to BPM Note Ix %d.", path, step);
#endif
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::AddPlayBPMNote, pattern, channel, step, stepVal));
		} // end if BPMIncr
		else if (std::strcmp(path, OSC_SET_PLAY_BPMNOTE) == 0)
		{
			// Set BPM Note Index :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpmIx
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - BPM Note Ix %d.", path, step);
#endif
			step = clampi(step, 0, TROWA_TEMP_BPM_NUM_OPTIONS - 1); // Must be 0 to TROWA_TEMP_BPM_NUM_OPTIONS - 1
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayBPMNote, pattern, channel, step, stepVal));
		} // end if BPMNote
		else
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("Unknown OSC message: %s received.", rxMsg.AddressPattern());
#endif
		}
	} // end try
	catch (osc::Exception& e) {
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("Error parsing OSC message %s: %s", rxMsg.AddressPattern(), e.what());
#endif
	} // end catch
	return;
} // end ProcessMessage()
