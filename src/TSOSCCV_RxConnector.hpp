#ifndef TSOSCCV_RX_CONNECTOR_HPP
#define TSOSCCV_RX_CONNECTOR_HPP

#include "Module_oscCV.hpp"
#include "TSOSCCV_Common.hpp"
#include "TSOSCBaseListener.hpp"
#include <mutex>
#include <map>
#include <vector>

#define OSC_CV_RECV_ONLY_ONE		1 // OSC standard states that any address that matches should get the message, but maybe turn it off for performance?
// 0: Multiple channels can receive the same message. 1: Only first channel that matches can receive it....


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Listener for OSC incoming messages.
// This listener should route the messages to N oscCV modules. 
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
class OscCVRxMsgRouter : public TSOSCBaseMsgRouter<oscCV> {
public:

protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessMessage()
	// @rxMsg : (IN) The received message from the OSC library.
	// @remoteEndPoint: (IN) The remove end point (sender).
	// Handler for receiving messages from the OSC library. Taken from their example listener.
	// Parse incoming OSC messages and create TSOSCCVSimpleMessage objects and dump in the queues of cvOSCcv modules and any of their expansion
	// modules (not really correct way to use expansion bypasses delay of 1 sample time).
	//--------------------------------------------------------------------------------------------------------------------------------------------
	virtual void ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) override;
	
	// Deliver message to the given queue.
	bool deliverMessage(const char* path, TSOSCCVChannel* channels, int nChannels, std::queue<TSOSCCVSimpleMessage>& targetQueue, std::vector<float>& bArgs, std::vector<float>& fArgs, std::vector<float>& iArgs);
	
};


// OSC Receiver Connector
class OscCVRxConnector : public TSOSCRxConnector<oscCV, OscCVRxMsgRouter>
{
public:
private:
};

#endif // !TSOSCCV_RX_CONNECTOR_HPP