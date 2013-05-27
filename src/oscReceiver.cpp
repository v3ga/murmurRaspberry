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
                    if (propName == "volMax")
                    {
                        Globals::instance()->mp_deviceEcho->setSoundInputVolumeMaxOSC( m_oscMessage.getArgAsFloat(indexArg) );
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
                    if (propName == "enableStandbyMode")
                    {
                        Globals::instance()->mp_deviceEcho->setEnableStandbyModeOSC( m_oscMessage.getArgAsInt32(indexArg)==1 ? true : false );
                    }
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
