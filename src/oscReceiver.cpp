//
//  oscReceiver.cpp
//  murmur
//
//  Created by Julien on 14/04/13.
//
//

#include "oscReceiver.h"
#include "globals.h"
#include "deviceEcho.h"
#include "testApp.h"

//--------------------------------------------------------------
void oscReceiver::update()
{
    
    if (hasWaitingMessages())
    {
		while(getNextMessage(&m_oscMessage))
		{
            if (Globals::instance()->mp_deviceEcho)
            {
                int indexArg = 1; // Skip the id
                if (m_oscMessage.getAddress() == OSC_ADDRESS_SET_DEVICE_PROP)
                {
                    string propName = m_oscMessage.getArgAsString(indexArg++);
                    if (propName == "mute")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputMuteOSC( m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false );
                    }
					else
                    if (propName == "useRawVol")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputUseRawVolumeOSC( m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false );
                    }
					else
                    if (propName == "volMax")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputVolumeMaxOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "volMaxMax")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputVolumeMaxMaxOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "volHistorySize")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputVolHistorySizeOSC( m_oscMessage.getArgAsInt32(indexArg) );
                    }
                    else
                    if (propName == "volHistoryTh")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputVolHistoryThOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "volHistoryPingTh")
                    {
                        Globals::instance()->mp_deviceEcho->setVolHistoryPingThOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "enablePitch")
                    {
                        Globals::instance()->mp_deviceEcho->setEnablePitchOSC( m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false );
                    }
                    else
                    if (propName == "pitchMin")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputPitchMinOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "pitchMax")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputPitchMaxOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "enableStandbyMode")
                    {
                        Globals::instance()->mp_deviceEcho->setEnableStandbyModeOSC( m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false );
                    }
                    else
                    if (propName == "enableStandbyMode")
                    {
                        Globals::instance()->mp_deviceEcho->setEnableStandbyModeOSC( m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false );
                    }
                    else
                    if (propName == "timeStandby")
                    {
                        //printf("timeStandby = %.3f - ", m_oscMessage.getArgAsFloat(indexArg));
                        Globals::instance()->mp_deviceEcho->setTimeStandbyOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "sampleVolStandby")
					{
                        Globals::instance()->mp_deviceEcho->setSampleVolumeStandbyOSC( m_oscMessage.getArgAsFloat(indexArg) );
					}
                    else
                    if (propName == "enableColor")
					{
						int value = m_oscMessage.getArgAsInt32(indexArg++);
						Globals::instance()->mp_deviceEcho->enableColorOSC(value>0 ? true : false);
					}
                    else
                    if (propName == "color")
					{
						float hue = m_oscMessage.getArgAsFloat(indexArg++);
						float sat = m_oscMessage.getArgAsFloat(indexArg);
                        Globals::instance()->mp_deviceEcho->setColorHueSaturationOSC(hue,sat);
						
						//ofLog()<<"(hue,sat)="<<ofToString(hue)<<","<<ofToString(sat);
					}
                    else
                    if (propName == "color2")
                    {
                        float hue = m_oscMessage.getArgAsFloat(indexArg++);
                        float sat = m_oscMessage.getArgAsFloat(indexArg);
                        Globals::instance()->mp_deviceEcho->setColor2HueSaturationOSC(hue,sat);

                        // ofLog() << "(hue,sat)=" << hue << ',' << sat;
                    }
					else
					if (propName == "invertPacketsVolume")
					{
						int value = m_oscMessage.getArgAsInt32(indexArg++);
						Globals::instance()->mp_deviceEcho->invertPacketsVolumeOSC(value>0 ? true : false);
					}
					else
					if (propName == "reversePacketsDir")
					{
						int value = m_oscMessage.getArgAsInt32(indexArg++);
						Globals::instance()->mp_deviceEcho->reversePacketsDirOSC(value>0 ? true : false);
					}
                    else
                    if (propName == "bpmEnable")
					{
						int value = m_oscMessage.getArgAsInt32(indexArg++);
						Globals::instance()->mp_deviceEcho->setBPMEnableOSC(value>0 ? true : false);
					}
                    else
                    if (propName == "bpm")
					{
						int value = m_oscMessage.getArgAsInt32(indexArg++);
						Globals::instance()->mp_deviceEcho->setBPMOSC(value);
					}
                }
                else if (m_oscMessage.getAddress() == OSC_ADDRESS_TURN_OFF)
                {
                    bool bReboot = m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false;
                    testApp::turnoff( bReboot );
					// ofExit();
				}
			else
			if (m_oscMessage.getAddress() == OSC_ADDRESS_RESET_PING)
			{
				Globals::instance()->mp_deviceEcho->resetVolHistoryPingOSC();
			}
				
				
            }

/*
            if (m_oscMessage.getAddress() == OSC_ADDRESS_SET_DEVICE_PROP)
            {
                Device* pDevice = pDeviceManager->getDeviceById( m_oscMessage.getArgAsString(indexArg++) );
                if (pDevice){
                    // Property
                    string propName = m_oscMessage.getArgAsString(indexArg++);
                    if (propName == "volMax")
                    {
                        pDevice->setSoundInputVolumeMaxOSC( m_oscMessage.getArgAsFloat(indexArg) );
                    }
                    else
                    if (propName == "volHistorySize")
                    {
                        pDevice->setSoundInputVolHistorySizeOSC( m_oscMessage.getArgAsInt32(indexArg) );
                    }
                }
            }
           else
            if (m_oscMessage.getAddress() == OSC_ADDRESS_SEND_PACKETS)
            {
                if (pDeviceManager)
                {
                    Device* pDevice = pDeviceManager->getDeviceById( m_oscMessage.getArgAsString(indexArg++) );

                    // Not found, create the device now
                    if (pDevice == 0){
                        
                        // TODO : Create device
                        
                        // TODO : Send configuration parameters to it
                    
                    }
                    
                    // Decode new message
                    if (pDevice)
                    {
                        pDevice->onReceivePacketBegin();
                        
                        int nbPackets = m_oscMessage.getNumArgs() - indexArg; // TODO : Be careful with this
                        for (int i=0;i<nbPackets;i++)
                        {
                            m_packetTemp.m_volume = m_oscMessage.getArgAsFloat(indexArg++);
                            // TODO : Other arguments to decode here
                            
                            // Send to device
                            pDevice->onReceivePacket(&m_packetTemp);

                        }

                        pDevice->onReceivePacketEnd();

                        // Get the surface of device and update animation
                        //Surface* pSurface = Globals::instance()->mp_deviceInfoManager->getSurfaceForDevice(pDevice);
                        Surface* pSurface = Globals::instance()->mp_app->getSurfaceForDevice(pDevice);
                        
                        if (pSurface){
                            pSurface->onNewPacket( pDevice->getLastPacket(), pDevice->m_id );
                        }
                    
                    }
                }
        }
*/
            
            
            // New Device plugged
            // printf("osc %s - %d arguments\n", m_oscMessage.getAddress().c_str(), m_oscMessage.getNumArgs());
            
            // New Device event
        }
    }
}
