#include "TSOSCCommunicator.hpp"
#include "trowaSoftUtilities.hpp"

#include <thread> // std::thread
#include <string.h>
#include <stdio.h>
#include <map>
#include <mutex>

#define MIN_PORT	1000
#define MAX_PORT	0xFFFF

// The connector
TSOSCConnector* TSOSCConnector::_instance = NULL;

TSOSCConnector::TSOSCConnector()
{
	_lastId = 0;
	return;
}

TSOSCConnector* TSOSCConnector::Connector()
{
	if (!_instance)
		_instance = new TSOSCConnector();
	return _instance;
}
// Get an id for the module instance.
int TSOSCConnector::getId()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return ++_lastId;
}
// Register the usage of these ports.
bool TSOSCConnector::registerPorts(int id, uint16_t txPort, uint16_t rxPort, bool txSharedAllowed, bool rxSharedAllowed)
{
	std::lock_guard<std::mutex> lock(_mutex);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	DEBUG("OSC Connector - Try to register ports Tx %u and Rx %u. Sharing allowed = %d, %d.", txPort, rxPort, txSharedAllowed, rxSharedAllowed);	
#endif
	int tx = -1;
	int rx = -1;
	TSOSCPortInfo* rxItem = NULL;
	TSOSCPortInfo* txItem = NULL;
	const int ActionNone = 0;
	const int ActionAddNew = 1;
	const int ActionAddId = 2;
	int rxAction = ActionNone; // 0 none, 1 add, 2 add id.
	int txAction = ActionNone; // 0 none, 1 add, 2 add id.
	// Rx Port ==========
	rxItem = (_portMap.count(rxPort) < 1) ? NULL : _portMap[rxPort];
	TSOSCPortInfo::PortUse rxPortUse = (rxSharedAllowed) ? TSOSCPortInfo::PortUse::Multiple : TSOSCPortInfo::PortUse::Singular;	
	if (rxItem == NULL)
	{
		// Doesn't exist, so it's allowed. Add the item.
		rx = rxPort;
		rxItem = new TSOSCPortInfo(rxPortUse, TSOSCPortInfo::PortType::Rx, id);
		rxAction = ActionAddNew; // Add
	}
	else 
	{
		// Port entry exists
		if (std::find(rxItem->ids.begin(), rxItem->ids.end(), id) != rxItem->ids.end())
		{
			// This id is using this port
			rx = rxPort;
		}
		else if (rxSharedAllowed && rxItem->portUse == TSOSCPortInfo::PortUse::Multiple)
		{
			// Sharing is allowed.
			rx = rxPort;
			rxAction = ActionAddId; // Add id			
		}
	}
	
	// Tx Port ============
	txItem = (_portMap.count(txPort) < 1) ? NULL : _portMap[txPort];
	TSOSCPortInfo::PortUse txPortUse = (txSharedAllowed) ? TSOSCPortInfo::PortUse::Multiple : TSOSCPortInfo::PortUse::Singular;
	if (txItem == NULL)//_portMap[txPort] == id)
	{
		// Doesn't exist, so it's allowed. Add the item.
		tx = txPort;
		txItem = new TSOSCPortInfo(txPortUse, TSOSCPortInfo::PortType::Tx, id);
		txAction = ActionAddNew; // Add
	}
	else 
	{
		
		// Item is the has this id has already registered
		if (std::find(txItem->ids.begin(), txItem->ids.end(), id) != txItem->ids.end()) // Id is in list
		{
			// This id already has this Tx port registered.
			tx = txPort;
		}
		else if (txSharedAllowed && txItem->portUse == TSOSCPortInfo::PortUse::Multiple)
		{
			// Sharing is allowed.
			tx = txPort;
			txAction = ActionAddId; // Add id
		}
	}	
	if (tx > -1 && rx > -1)
	{
		// Both ports are OK
		switch (rxAction)
		{
			case ActionAddNew:
				// Add the item
				_portMap.insert(std::pair<uint16_t, TSOSCPortInfo*>(rxPort, rxItem));			
				break;
			case ActionAddId:
				// Add the id to the list
				rxItem->ids.push_back(id);			
				break;
			case ActionNone:
			default:
				// Do nothing.
				break;
		} // end switch
		
		switch (txAction)
		{
			case ActionAddNew:
				// Add the item
				_portMap.insert(std::pair<uint16_t, TSOSCPortInfo*>(txPort, txItem));			
				break;
			case ActionAddId:
				// Add the id to the list
				txItem->ids.push_back(id);			
				break;
			case ActionNone:
			default:
				// Do nothing.
				break;			
		}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW		
		DEBUG("Registered ports Tx %u and Rx %u", tx, rx);
#endif
		return true;
	}
	else
	{
		return false; // Do nothing
	}
} // end registerPorts()

// Register the usage of these ports.
bool TSOSCConnector::registerPort(int id, TSOSCPortInfo::PortType portType, uint16_t port, bool sharingAllowed)
{
	std::lock_guard<std::mutex> lock(_mutex);
	int portAssigned = -1;
	TSOSCPortInfo* item = NULL;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW	
	DEBUG("OSC Connector - Try to register %s port %u Sharing allowed = %d.", (portType == TSOSCPortInfo::PortType::Tx) ? "Tx" : "Rx", port, sharingAllowed);
#endif
	item = (_portMap.count(port) < 1) ? NULL : _portMap[port];
	TSOSCPortInfo::PortUse portUse = (sharingAllowed) ? TSOSCPortInfo::PortUse::Multiple : TSOSCPortInfo::PortUse::Singular;
	if (item == NULL)
	{
		// Doesn't exist, so it's allowed. Add the item.
		item = new TSOSCPortInfo(portUse, portType, id);
		_portMap.insert(std::pair<uint16_t, TSOSCPortInfo*>(port, item));
		portAssigned = port;
	}
	else 
	{
		// Port exists
		// Item is the same port type
		if (std::find(item->ids.begin(), item->ids.end(), id) != item->ids.end()) // Id is in list
		{
			// This id already has this port registered.
			portAssigned = port;
		}
		else if (sharingAllowed && item->portUse == TSOSCPortInfo::PortUse::Multiple)
		{
			// This Tx port can be shared.
			portAssigned = port;
			item->ids.push_back(id); // Add this id to the list
		}
	}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	DEBUG("OSC Connector - Assigned %s port is %d.", (portType == TSOSCPortInfo::PortType::Tx) ? "Tx" : "Rx", portAssigned);
#endif
	return portAssigned > -1;
} // end registerPort()

// Clear the usage of these ports.
bool TSOSCConnector::clearPorts(int id, uint16_t txPort, uint16_t rxPort)
{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW	
	DEBUG("OSC Connector - Clearing ports for id %d: %u and %u.", id, txPort, rxPort);
#endif
	std::lock_guard<std::mutex> lock(_mutex);
	std::map<uint16_t, TSOSCPortInfo*>::iterator it;
	int nErased = 0;
	
	uint16_t port = txPort;
	it = _portMap.find(port);
	if (it != _portMap.end())  // _portMap[port] == id)
	{
		std::vector<int>::iterator position = std::find(_portMap[port]->ids.begin(), _portMap[port]->ids.end(), id);
		if (position != _portMap[port]->ids.end())
		{
			// This id was registered with this port
			_portMap[port]->ids.erase(position);			
		}
		if (_portMap[port]->ids.size() < 1)
		{
			// No more registered, remove.
			delete it->second; // Delete port info
			_portMap.erase(it);			
		}
		nErased++;
	}	
	
	port = rxPort;
	it = _portMap.find(port);
	if (it != _portMap.end())  // _portMap[port] == id)
	{
		std::vector<int>::iterator position = std::find(_portMap[port]->ids.begin(), _portMap[port]->ids.end(), id);
		if (position != _portMap[port]->ids.end())
		{
			// This id was registered with this port
			_portMap[port]->ids.erase(position);			
		}
		if (_portMap[port]->ids.size() < 1)
		{
			// No more registered, remove.
			delete it->second; // Delete port info
			_portMap.erase(it);			
		}
		nErased++;
	}	
	return nErased == 2;
} // end clearPorts()

// Clear the usage of these ports.
bool TSOSCConnector::clearPort(int id, uint16_t port)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::map<uint16_t, TSOSCPortInfo*>::iterator it;
	it = _portMap.find(port);
	if (it != _portMap.end())  // _portMap[port] == id)
	{
		std::vector<int>::iterator position = std::find(_portMap[port]->ids.begin(), _portMap[port]->ids.end(), id);
		if (position != _portMap[port]->ids.end())
		{
			// This id was registered with this port
			_portMap[port]->ids.erase(position);			
		}
		if (_portMap[port]->ids.size() < 1)
		{
			// No more registered, remove.
			delete it->second; // Delete port info
			_portMap.erase(it);			
		}
		return true;
	}
	return false;
} // end clearPort()

// See if the port is in use (returns the id of the module using it or 0 if it is free).
int TSOSCConnector::portInUse(uint16_t port)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::map<uint16_t, TSOSCPortInfo*>::iterator it;
	int id = 0;
	it = _portMap.find(port);
	if (it != _portMap.end() && _portMap[port]->ids.size() > 0)
	{
		id = _portMap[port]->ids[0];
	}
	return id;
}

// Get an available port.
uint16_t TSOSCConnector::getAvailablePort(int id, TSOSCPortInfo::PortType portType, uint16_t desiredPort, bool sharingAllowed)
{
	// Currently we are just using this function to look for free ports, so don't use @portType or @sharingAllowed.
	std::lock_guard<std::mutex> lock(_mutex);
	bool portFound = false;
	uint16_t port = desiredPort;
	uint16_t portA, portB;
	if (_portMap.count(port) < 1 || std::find(_portMap[port]->ids.begin(), _portMap[port]->ids.end(), id) != _portMap[port]->ids.end())
	{
		portFound = true;
	}
	else
	{
		portA = port;
		portB = port;
		while (!portFound && (portA < MAX_PORT || portB > MIN_PORT))
		{
			if (portA < MAX_PORT)
			{
				portA += 2;
				portFound = _portMap.count(portA) < 1 || std::find(_portMap[portA]->ids.begin(), _portMap[portA]->ids.end(), id) != _portMap[portA]->ids.end();   // _portMap[portA] == id;
				if (portFound)
					port = portA;
			}
			if (!portFound && portB > MIN_PORT)
			{
				portB -= 2;
				portFound = _portMap.count(portB) < 1 || std::find(_portMap[portB]->ids.begin(), _portMap[portB]->ids.end(), id) != _portMap[portB]->ids.end();//_portMap[portB] == id;
				if (portFound)
					port = portB;
			}
		} // end while
	}
	return (portFound) ? port : 0;
} // end getAvailablePort()
