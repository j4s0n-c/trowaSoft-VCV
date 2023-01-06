#include "TSOSCCV_RxConnector.hpp"
#include "Module_oscCV.hpp"


//--------------------------------------------------------------------------------------------------------------------------------------------
// ProcessMessage()
// @rxMsg : (IN) The received message from the OSC library.
// @remoteEndPoint: (IN) The remove end point (sender).
// Handler for receiving messages from the OSC library. Taken from their example listener.
// Parse incoming OSC messages and create TSOSCCVSimpleMessage objects and dump in the queues of cvOSCcv modules and any of their expansion
// modules (not really correct way to use expansion bypasses delay of 1 sample time).
//--------------------------------------------------------------------------------------------------------------------------------------------
void OscCVRxMsgRouter::ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint)
{
	(void)remoteEndpoint; // suppress unused parameter warning
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
//	DEBUG("[RECV] OSC Message: %s", rxMsg.AddressPattern());
#endif
	try 
	{
		std::string addr = rxMsg.AddressPattern();
		// Get the argument (not strongly typed.... We'll read in as float and cast as what we need in case > 1 channel is receiving this message)
		// (touchOSC will only ever send floats)
		//osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
		osc::int32 intArg = 0;
		float floatArg = 0.0;
		bool boolArg = false;
		osc::MidiMessage midiArg;
		//uint32_t uintArg = 0;
		// Polyphonic channels supported now, so we could have 16 values coming in.
		std::vector<float> fArgs; 
		std::vector<float> iArgs;
		std::vector<float> bArgs;
		int numArgs = 0;
		std::string debugMsg;
		bool debugMsgCreated = false;
		try
		{
			numArgs = rxMsg.ArgumentCount();
			if (numArgs > 0)
			{
				osc::ReceivedMessage::const_iterator arg = rxMsg.ArgumentsBegin();
				int k = 0;
				// Polyphonic cables are limited to 16 values anyway, so don't bother with any more (TROWA_OSCCV_VECTOR_MAX_SIZE [engine::PORT_MAX_CHANNEL] should be 16).
				while (arg != rxMsg.ArgumentsEnd() && k++ < TROWA_OSCCV_VECTOR_MAX_SIZE) 
				{
					bool add = false;
					switch (arg->TypeTag())
					{
						case osc::TypeTagValues::INT32_TYPE_TAG:
							intArg = arg->AsInt32();
							boolArg = intArg > 0;
							floatArg = static_cast<float>(intArg);
							add = true;
							break;
						case osc::TypeTagValues::TRUE_TYPE_TAG:
						case osc::TypeTagValues::FALSE_TYPE_TAG:
							boolArg = arg->AsBool();
							floatArg = static_cast<float>(boolArg);
							intArg = static_cast<int>(boolArg);
							add = true;							
							break;
						// case osc::TypeTagValues::MIDI_MESSAGE_TYPE_TAG: 
							// // Not full supported yet.
							// midiArg = arg.AsMidiMessage();
							// uintArg = midiArg.value;
							// break;
						case osc::TypeTagValues::FLOAT_TYPE_TAG:
						default:
							floatArg = arg->AsFloat();
							intArg = static_cast<int>(floatArg);
							boolArg = floatArg > 0;
							add = true;							
							break;						
					}
					if (add)
					{
						// cvOSCcv is limited one data type for the entire channel. In reality all CVs are floats,
						// so all these should be converted back to floats.			
						fArgs.push_back(floatArg);
						iArgs.push_back(static_cast<float>(intArg));
						bArgs.push_back(static_cast<float>(boolArg));
					}
					arg++;
				}					
				numArgs = static_cast<int>(fArgs.size());
			}
		}
		catch (osc::WrongArgumentTypeException &touchOSCEx)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			DEBUG("Wrong argument type: Error %s message: ", path, touchOSCEx.what());
			DEBUG("We received (float) %05.2f.", floatArg);
#endif
		}
		
		// Add message to each module's queue where this matches
		for (int i = 0; i < static_cast<int>(modules.size()); i++)
		{
			oscCV* oscModule = modules[i];
			try
			{
				if (oscModule)
				{				
					if (oscModule->debugOSCConsoleOn)
					{
						if (!debugMsgCreated)
						{
							debugMsgCreated = true;
							debugMsg = getDebugMessage(rxMsg);
						}
						oscModule->addDebugMessage(debugMsg);
					}
			
					bool recipientFound = false;
					std::string oscNamespace = oscModule->getOscNamespace();
					int len = 0;//(oscNamespace.empty()) ? 0 : oscNamespace.length();
					if (oscNamespace.empty())
						len = 0;
					else
					{
						oscNamespace = "/" + oscNamespace;
						len = oscNamespace.length();
					}
					const char* ns = oscNamespace.c_str();					
					// [2019-04-03] Allow empty namespaces
					if (!oscNamespace.empty() && (static_cast<int>(addr.length()) < len || std::strcmp(addr.substr(0, len).c_str(), ns) != 0)) // Message is not for us
					{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
						DEBUG("Message is not for module (oscId %d) namespace (%s). [Address: %s] Move along, nothing to see here...", oscModule->oscId, ns, addr.c_str());
#endif
						continue;
					}
					std::string subAddr = addr.substr(len);
					const char* path = subAddr.c_str();
					for (int c = 0; c < oscModule->numberChannels; c++)
					{
						if (strlen(path) == strlen(oscModule->outputChannels[c].path.c_str()) && std::strcmp(path, oscModule->outputChannels[c].path.c_str()) == 0)
						{
							switch (oscModule->outputChannels[c].dataType)
							{
								case TSOSCCVChannel::ArgDataType::OscBool:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
									DEBUG("OSC Recv Ch %d: Bool %d at %s.", c+1, boolArg, oscModule->outputChannels[c].path.c_str());
#endif
#if USE_MODULE_STATIC_RX
									// Re-use static buffer on module (this is a test):
									if (numArgs > 1)
										oscModule->addRxMsgToQueue(c + 1, bArgs);
									else
										oscModule->addRxMsgToQueue(c + 1, boolArg);
#else
									// Add *new* Message
									oscModule->rxMsgMutex.lock();	
									if (numArgs > 1)
										oscModule->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, bArgs));
									else
										oscModule->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, boolArg));
									oscModule->rxMsgMutex.unlock();									
#endif	
									#if OSC_CV_RECV_ONLY_ONE
									recipientFound = true;
									#endif
									break;
								case TSOSCCVChannel::ArgDataType::OscInt:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
									DEBUG("OSC Recv Ch %d: Int %d at %s.", c + 1, intArg, oscModule->outputChannels[c].path.c_str());
#endif
#if USE_MODULE_STATIC_RX
									// Re-use static buffer on module:
									if (numArgs > 1)
										oscModule->addRxMsgToQueue(c + 1, iArgs);
									else
										oscModule->addRxMsgToQueue(c + 1, intArg);
#else
									// Add *new* Message
									oscModule->rxMsgMutex.lock();
									if (numArgs > 1)
										oscModule->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, iArgs));
									else
										oscModule->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, intArg));
									oscModule->rxMsgMutex.unlock();
#endif
									#if OSC_CV_RECV_ONLY_ONE
									recipientFound = true;
									#endif
									break;
								// case TSOSCCVChannel::ArgDataType::OscMidi:
									// // Actually, I don't think anything natively supports this, so this would be unused.
// #if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
									// DEBUG("OSC Recv Ch %d: MIDI %08x at %s.", c + 1, uintArg, oscModule->outputChannels[c].path.c_str());
// #endif
									// oscModule->rxMsgQueue.push(TSOSCCVSimpleMessage(c + 1, floatArg, uintArg));
									// #if OSC_CV_RECV_ONLY_ONE
									// recipientFound = true;
									// #endif
									// break;
								case TSOSCCVChannel::ArgDataType::OscFloat:
								default:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
									DEBUG("OSC Recv Ch %d: Float %7.4f at %s.", c + 1, floatArg, oscModule->outputChannels[c].path.c_str());
#endif
#if USE_MODULE_STATIC_RX
									// Re-use static buffer on module:
									if (numArgs > 1)
										oscModule->addRxMsgToQueue(c + 1, fArgs);
									else
										oscModule->addRxMsgToQueue(c + 1, floatArg);
#else
									oscModule->rxMsgMutex.lock();
									if (numArgs > 1)
									{
										oscModule->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, fArgs));										
									}
									else
									{
										oscModule->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, floatArg));
									}
									oscModule->rxMsgMutex.unlock();									
#endif 									
									#if OSC_CV_RECV_ONLY_ONE
									recipientFound = true; // OSC specs dictates that all recipients get the message, but we mant to turn off for performance.
									#endif
									break;
							} // end switch
						} // end if path matches
					} // end loop through channels		

					// ### Expansions ###
					if (!recipientFound)
					{
						try
						{
							// Let this thread handle sorting/delivery.
							Module::Expander* exp = &(oscModule->rightExpander);
							while (!recipientFound && exp != NULL && exp->module && CVOSCCV_IS_EXPANDER_OUTPUT_MODEL(exp->module->model)) // == modelOscCVExpanderOutput)
							{
								oscCVExpander* expMod = dynamic_cast<oscCVExpander*>(exp->module);
								//recipientFound = deliverMessage(path, expMod->outputChannels, expMod->numberChannels, expMod->rxMsgQueue, bArgs, fArgs, iArgs);// boolArg, floatArg, uintArg, intArg);
								// Less RAM usage on MAC if use a static buffer:
								recipientFound = deliverMessage(path, expMod, bArgs, fArgs, iArgs);
								exp = &(exp->module->rightExpander); // Go to next so we can see if that's another expander.
							} // end loop through RIGHT expanders
						}
						catch (const std::exception& expansionEx)
						{
							WARN("Error with Expansion Module: %s.", expansionEx.what());
						}			
					}					
				} // end if module				
			}
			catch (osc::Exception& moduleEx)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				DEBUG("Error for module %d (%s): %s", i, rxMsg.AddressPattern(), moduleEx.what());
#endif				
			}
		} // end loop through modules
	}
	catch (osc::Exception& e) {
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		DEBUG("Error parsing OSC message %s: %s", rxMsg.AddressPattern(), e.what());
#endif
	} // end catch
	return;
} // end ProcessMessage()

// Deliver the message to expander.
bool OscCVRxMsgRouter::deliverMessage(const char* path, oscCVExpander* expander, std::vector<float>& bArgs, std::vector<float>& fArgs, std::vector<float>& iArgs)
{
	bool rFound = false;
	// Make a message for each channel that may be listening to this address.
	for (int c = 0; c < expander->numberChannels; c++)
	{
		if (strlen(path) == strlen(expander->outputChannels[c].path.c_str()) && std::strcmp(path, expander->outputChannels[c].path.c_str()) == 0)
		{
			switch (expander->outputChannels[c].dataType)
			{
				case TSOSCCVChannel::ArgDataType::OscBool:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("OSC Recv Ch %d: Bool %d at %s.", c+1, boolArg, channels[c].path.c_str());
#endif
#if USE_MODULE_STATIC_RX
					// Re-use static buffer on module (this is a test):
					expander->addRxMsgToQueue(c + 1, bArgs);
#else
					// Add *new* Message
					queueMutex->lock();	
					expander->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, bArgs));
					queueMutex->unlock();									
#endif	

					#if OSC_CV_RECV_ONLY_ONE
					rFound = true;
					#endif
					break;
				case TSOSCCVChannel::ArgDataType::OscInt:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("OSC Recv Ch %d: Int %d at %s.", c + 1, intArg, channels[c].path.c_str());
#endif
#if USE_MODULE_STATIC_RX
					// Re-use static buffer on module (this is a test):
					expander->addRxMsgToQueue(c + 1, iArgs);
#else
					// Add *new* Message
					queueMutex->lock();	
					expander->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, iArgs));
					queueMutex->unlock();									
#endif	
					#if OSC_CV_RECV_ONLY_ONE
					rFound = true;
					#endif
					break;
				case TSOSCCVChannel::ArgDataType::OscFloat:
				default:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("OSC Recv Ch %d: Float %7.4f at %s.", c + 1, floatArg, channels[c].path.c_str());
#endif
#if USE_MODULE_STATIC_RX
					// Re-use static buffer on module (this is a test):
					expander->addRxMsgToQueue(c + 1, fArgs);
#else
					// Add *new* Message
					queueMutex->lock();	
					expander->rxMsgQueue.push(new TSOSCCVSimpleMessage(c + 1, fArgs));
					queueMutex->unlock();									
#endif	
					#if OSC_CV_RECV_ONLY_ONE
					rFound = true;
					#endif
					break;
			} // end switch
		} // end if path matches
	} // end loop through channels
	return rFound;
}


// Deliver a message to the target queue.
bool OscCVRxMsgRouter::deliverMessage(const char* path, TSOSCCVChannel* channels, int nChannels, std::queue<TSOSCCVSimpleMessage*>& targetQueue, 
	std::vector<float>& bArgs, std::vector<float>& fArgs, std::vector<float>& iArgs)
{
	bool rFound = false;
	// Make a message for each channel that may be listening to this address.
	for (int c = 0; c < nChannels; c++)
	{
		if (strlen(path) == strlen(channels[c].path.c_str()) && std::strcmp(path, channels[c].path.c_str()) == 0)
		{
			switch (channels[c].dataType)
			{
				case TSOSCCVChannel::ArgDataType::OscBool:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("OSC Recv Ch %d: Bool %d at %s.", c+1, boolArg, channels[c].path.c_str());
#endif
					targetQueue.push(new TSOSCCVSimpleMessage(c + 1, bArgs));
					#if OSC_CV_RECV_ONLY_ONE
					rFound = true;
					#endif
					break;
				case TSOSCCVChannel::ArgDataType::OscInt:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("OSC Recv Ch %d: Int %d at %s.", c + 1, intArg, channels[c].path.c_str());
#endif
					targetQueue.push(new TSOSCCVSimpleMessage(c + 1, iArgs));
					#if OSC_CV_RECV_ONLY_ONE
					rFound = true;
					#endif
					break;
				case TSOSCCVChannel::ArgDataType::OscFloat:
				default:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					DEBUG("OSC Recv Ch %d: Float %7.4f at %s.", c + 1, floatArg, channels[c].path.c_str());
#endif
					targetQueue.push(new TSOSCCVSimpleMessage(c + 1, fArgs));
					#if OSC_CV_RECV_ONLY_ONE
					rFound = true;
					#endif
					break;
			} // end switch
		} // end if path matches
	} // end loop through channels
	return rFound;
}