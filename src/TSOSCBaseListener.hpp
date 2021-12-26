#ifndef TSOSCCV_BASELISTENER_HPP
#define TSOSCCV_BASELISTENER_HPP

#include "TSOSCCV_Common.hpp"
#include <mutex>
#include <map>
#include <vector>
#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"
#include "../lib/oscpack/osc/OscPrintReceivedElements.h"
#include <thread>
#include <rack.hpp>
using namespace rack;


#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Base Listener for OSC incoming messages.
// This listener should route the messages on one Rx port to N oscCV or trowaSoft sequencer modules. 
// Apparently in C++, your parent thread's death does not result in your own demise, so if whichever thread spawns the slave listener exits,
// the listener thread won't die... Yay!
// So type <T> should be oscCV or TSSequencerBase if we allow sequencers to share the same ports...
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
template<class T>
class TSOSCBaseMsgRouter : public osc::OscPacketListener {
public:
	// Modules that are registered for messages.
	typename std::vector<T*> modules;
	// The port.
	uint16_t port;	
	
	// Instantiate a listener.
	TSOSCBaseMsgRouter() : osc::OscPacketListener()
	{
		/// TODO: Make a damn base OSC interface class.
		static_assert(std::is_base_of<Module, T>::value, "Must be a Module.");		
		return;
	}
	// Instantiate a listener.
	TSOSCBaseMsgRouter(T* oscModule) : TSOSCBaseMsgRouter()
	{
		modules.push_back(oscModule);
		return;
	}
	// Add a module to the list.
	void addModule(T* oscModule)
	{
		std::lock_guard<std::mutex> lock(mutModule);
		if (std::find(modules.begin(), modules.end(), oscModule) == modules.end())
		{
			modules.push_back(oscModule);
		}
		return;
	}
	// Remove a module.
	void removeModule(T* oscModule)
	{
		std::lock_guard<std::mutex> lock(mutModule);
		typename std::vector<T*>::iterator it = std::find(modules.begin(), modules.end(), oscModule);
		if (it != modules.end())
		{
			// Remove reference but don't delete the reference.
			modules.erase(it);
		}
		return;
	}
protected:
	// Mutex for adjust modules
	std::mutex mutModule;
	// Get the message as a debug message.
	virtual std::string getDebugMessage(const osc::ReceivedMessage& rxMsg)
	{
		std::ostringstream stream;		
		using namespace osc;
		using namespace std::chrono;
		using clock = system_clock;    
		const auto current_time_point {clock::now()};
		const auto current_time {clock::to_time_t (current_time_point)};
		const auto current_localtime {*std::localtime (&current_time)};
		const auto current_time_since_epoch {current_time_point.time_since_epoch()};
		const auto current_milliseconds {duration_cast<milliseconds> (current_time_since_epoch).count() % 1000};
		
		stream << "[" << std::put_time(&current_localtime, "%H:%M:%S") << "." << std::setw (3) << std::setfill ('0') << current_milliseconds << "] :" << port << " ";
		stream << rxMsg;		
		return stream.str();		
	}

	// Derived class should implement ProcessMessage() ================
	
	//--------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessMessage()
	// @rxMsg : (IN) The received message from the OSC library.
	// @remoteEndPoint: (IN) The remove end point (sender).
	// Handler for receiving messages from the OSC library. Taken from their example listener.
	// Should create a generic TSExternalControlMessage for our trowaSoft sequencers/cvOSCcv and dump it in the module instance's queue.
	//--------------------------------------------------------------------------------------------------------------------------------------------
	//virtual void ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) override;	
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Information about the port, the socket, and the modules that are subscribed to receive messages.
// Type <T> should be oscCV or TSSequencerBase if we allow sequencers to share the same ports...
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
template<class T>
struct OscRxDetails
{
	// Receiving OSC socket.
	UdpListeningReceiveSocket* oscRxSocket = NULL;
	// The port.
	uint16_t port;
	// The message router.
	TSOSCBaseMsgRouter<T>* router = NULL;
	// The OSC listener thread
	std::thread oscListenerThread;	
	OscRxDetails(uint16_t port)
	{
		static_assert(std::is_base_of<Module, T>::value, "Must be a Module.");		
		this->port = port;
	}	
	~OscRxDetails()
	{
		cleanUp();
		return;
	}
	void cleanUp()
	{
		if (oscRxSocket != NULL) // No more modules
		{
			oscRxSocket->AsynchronousBreak();
			oscListenerThread.join(); // Wait for him to finish
			delete oscRxSocket;			
			oscRxSocket = NULL;
			
			delete router;
		}					
	}
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// OSC Rx Connector.
// Type <T> should be oscCV or TSSequencerBase if we allow sequencers to share the same ports...
// Type <R> should be derived from TOSCBaseMsgRouter.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
template<class T, class R>
class TSOSCRxConnector
{
private:
	TSOSCRxConnector()
	{
		static_assert(std::is_base_of<Module, T>::value, "Must be a Module.");
		static_assert(std::is_base_of<TSOSCBaseMsgRouter<T>, R>::value, "Must be a TSOSCBaseMsgRouter.");		
		return;
	}
	static TSOSCRxConnector<T, R>* _instance;
	// The ports that are receiving messages.
	std::map<uint16_t, OscRxDetails<T>*> _portMap;
	// Clean up thread and everything.
	//void cleanUpListener(OscRxDetails<T>* details);
	// Port mutex.
	std::mutex _mutex;
public:
	static TSOSCRxConnector<T, R>* Connector()
	{
		if (!TSOSCRxConnector<T, R>::_instance)
			TSOSCRxConnector<T, R>::_instance = new TSOSCRxConnector<T, R>();
		return _instance;	
	}
	bool startListener(uint16_t rxPort, T* module)
	{
		std::lock_guard<std::mutex> lock(_mutex);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("TSOSCRxConnector::startListener(port %d) - Starting for module id %d.", rxPort,  module->id);
#endif	
		bool success = false;
		OscRxDetails<T>* item = (_portMap.count(rxPort) < 1) ? NULL : _portMap[rxPort];
		if (item == NULL)
		{
			item = new OscRxDetails<T>(rxPort);
		}
		if (item->router == NULL)
		{
			item->router = new R();// TSOSCBaseMsgRouter<T>();
			item->port = rxPort;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("TSOSCRxConnector::startListener(port %d) - Create router/listener object.", rxPort);
#endif
		}
		item->router->addModule(module);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("TSOSCRxConnector::startListener(port %d) - Add module to router module list. Now it has %d items.", rxPort, item->router->modules.size());
#endif	
		if (item->oscRxSocket == NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			DEBUG("TSOSCRxConnector::startListener(port %d) - Creating Rx socket and starting listener thread.", rxPort);
#endif		
			item->oscRxSocket = new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, rxPort), item->router);
			// Start the thread
			item->oscListenerThread = std::thread(&UdpListeningReceiveSocket::Run, item->oscRxSocket);
		}
		_portMap[rxPort] = item;
		success = true;
		return success;		
	} // end startListener()
	bool stopListener(uint16_t rxPort, T* module)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("TSOSCRxConnector::stopListener(port %d, id=%d) - Stopping listener for module id %d.", rxPort, module->id, module->id);
#endif			
		std::lock_guard<std::mutex> lock(_mutex);	
		bool success = false;
		typename std::map<uint16_t, OscRxDetails<T>*>::iterator it = _portMap.find(rxPort);
		OscRxDetails<T>* item = (it == _portMap.end()) ? NULL : it->second;
		if (item != NULL)
		{
			if (item->router != NULL)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				DEBUG("TSOSCRxConnector::stopListener(port %d, id=%d) - Removing module from list.", rxPort, module->id);
#endif					
				item->router->removeModule(module); // Remove this module from the list.
			}		
			if (item->router == NULL || item->router->modules.size() < 1)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				DEBUG("TSOSCRxConnector::stopListener(port %d, id=%d) - NO MORE modules listening to this port. Deleting...", rxPort, module->id);
#endif		
				// No more modules are listening to this
				// No more registered, remove.
				item->cleanUp();
				delete it->second;			
				_portMap.erase(it);
			}
			success = true;
		}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		else
		{
			DEBUG("TSOSCRxConnector::stopListener(port %d, id=%d) - No item found for port!!!! Nothing to stop!", rxPort, module->id);		
		}
#endif			
		return success;			
	} // end stopListener()
	
	static bool StartListener(uint16_t rxPort, T* module)
	{
		return Connector()->startListener(rxPort, module);
	}
	static bool StopListener(uint16_t rxPort, T* module)
	{
		return Connector()->stopListener(rxPort, module);
	}	
};

template <class T, class R>
TSOSCRxConnector<T, R>* TSOSCRxConnector<T, R>::_instance = NULL;

#endif // !TSOSCCV_BASELISTENER_HPP