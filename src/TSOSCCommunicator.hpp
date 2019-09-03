#ifndef TSOSCCOMMUNICATOR_HPP
#define TSOSCCOMMUNICATOR_HPP

#include <thread> // std::thread
#include <mutex>
#include <string.h>
#include <map>
#include <vector>

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

#include "TSOSCSequencerListener.hpp"

// OSC connection information
typedef struct TSOSCInfo {	
	// OSC output IP address.
	std::string oscTxIpAddress;
	// OSC output port number.
	uint16_t oscTxPort;
	// OSC input port number.
	uint16_t oscRxPort;
} TSOSCConnectionInfo;

// Osc port information
struct TSOSCPortInfo {
	enum PortUse
	{
		// Normal behavior for sequencers Rx and Tx. Only one module can register the port.
		Singular,
		// Tx and Rx will be allowed for multiple cvOSCcv's, but shouldn't be allowed for sequencers (it doesn't make sense 
		// for > 1 sequencer to talk to the same endpoint and possibly fight each other, especially since we have feedback to the endpoint...).
		// For now (for debugging), allow cvOSCcv's to talk to each other. Perhaps this may cause problems down the line though...
		Multiple
	};
	PortUse portUse = PortUse::Singular;
	
	enum PortType
	{
		Rx,
		Tx
	};	
	PortType portType = PortType::Rx;
	// List of ids using this port.
	std::vector<int> ids;	
	
	TSOSCPortInfo(PortUse pUse, PortType pType, int id)
	{
		portUse = pUse;
		portType = pType;
		ids.push_back(id);
		return;
	}
};


// OSC Connection / track ports used to try to auto-increment and probably eventually allow multiple guys to talk on the same
// ports just with namespace or id routing.
class TSOSCConnector
{
public:
	static TSOSCConnector* Connector();
	// Get an id for the module instance.
	int getId();
	// Register the usage of these ports.
	bool registerPorts(int id, uint16_t txPort, uint16_t rxPort, bool txSharedAllowed = false, bool rxSharedAllowed = false);
	// Clear the usage of these ports.
	bool clearPorts(int id, uint16_t txPort, uint16_t rxPort);
	// Register the usage of these ports.
	bool registerPort(int id, TSOSCPortInfo::PortType portType, uint16_t port, bool sharingAllowed = false);
	// Clear the usage of these ports.
	bool clearPort(int id, uint16_t port);
	// Get an available port.
	uint16_t getAvailablePort(int id, TSOSCPortInfo::PortType portType, uint16_t desiredPort, bool sharingAllowed = false);
	// See if the port is in use (returns the id of the module using it or 0 if it is free).
	int portInUse(uint16_t port);

	// Get an id for the module instance.
	static int GetId() { return Connector()->getId(); }
	// Get an available port.
	static uint16_t GetAvailablePortTrans(int id, uint16_t desiredPort, bool sharingAllowed = false) { return Connector()->getAvailablePort(id, TSOSCPortInfo::PortType::Tx, desiredPort, sharingAllowed); }
	static uint16_t GetAvailablePortRecv(int id, uint16_t desiredPort, bool sharingAllowed = false) { return Connector()->getAvailablePort(id, TSOSCPortInfo::PortType::Rx, desiredPort, sharingAllowed); }	
	// Register the usage of these ports.
	static bool RegisterPorts(int id, uint16_t txPort, uint16_t rxPort, bool txSharedAllowed = false, bool rxSharedAllowed = false) { return Connector()->registerPorts(id, txPort, rxPort, txSharedAllowed, rxSharedAllowed); }
	// Clear the usage of these ports.
	static bool ClearPorts(int id, uint16_t txPort, uint16_t rxPort) { return Connector()->clearPorts(id, txPort, rxPort);}
	// Register the usage of these ports.
	static bool RegisterPortTrans(int id, uint16_t port, bool sharingAllowed = false) { return Connector()->registerPort(id, TSOSCPortInfo::PortType::Tx, port, sharingAllowed); }
	// Register the usage of these ports.
	static bool RegisterPortRecv(int id, uint16_t port, bool sharingAllowed = false) { return Connector()->registerPort(id, TSOSCPortInfo::PortType::Rx, port, sharingAllowed); }
	
	// Clear the usage of these ports.
	static bool ClearPort(int id, uint16_t port) { return Connector()->clearPort(id, port); }

	// See if the port is in use (returns the id of the module using it or 0 if it is free).
	static int PortInUse(uint16_t port) { return Connector()->portInUse(port); }
private:
	TSOSCConnector();
	//TSOSCConnector(TSOSCConnector const&) {};             // copy constructor is private
	//TSOSCConnector& operator=(TSOSCConnector const&) { return this; };  // assignment operator is private

	static TSOSCConnector* _instance;
	// The last id we gave out.
	int _lastId;
	// The ports that are in use by which id.
	//std::map<uint16_t, int> _portMap;	
	std::map<uint16_t, TSOSCPortInfo*> _portMap;

	// Port mutex.
	std::mutex _mutex;
};

#endif // !TSOSCCOMMUNICATOR_HPP
