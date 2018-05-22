//
//  deviceEcho.cpp
//  murmur
//
//  Created by Julien on 15/04/13.
//
//

#include "deviceEcho.h"
#include "ofAppLog.h"

//--------------------------------------------------------------
DeviceEcho::DeviceEcho(string id, int nbLEDs, int nbLEDsTotal, float distLEDs) : Device(id,nbLEDs,distLEDs)
{
    m_nbLEDsTotal = nbLEDsTotal;
    m_connectSPI = false;
    m_isDebugSinewave = false;
    m_isDebugSinewaveAngle = 0.0f;
    m_bExit = false;
}

//--------------------------------------------------------------
void DeviceEcho::exit()
{
    m_bExit = true;
    clearLEDs();
}

//--------------------------------------------------------------
void DeviceEcho::loadXMLData(ofxXmlSettings& settings)
{
	loadXMLSoundInput(settings);
	loadXMLColor(settings);
}

//--------------------------------------------------------------
void DeviceEcho::setupSenderOSC(string ip, int port)
{
    m_oscSender.setup(ip, port);
}

//--------------------------------------------------------------
void DeviceEcho::setupLEDs()
{
    bool connectSPI = false;
	if(	m_spi.connect())
	{
		ofLogNotice()<<"connected to SPI";
		m_connectSPI = true;
	}

    for (int i=0;i<m_nbLEDsTotal*3;i++)
    {
        m_spiData.push_back(0);
    }

    // +1 for latch
    m_spiData.push_back(0);


    m_spiColors.assign(m_nbLEDsTotal,ofColor());
 
//     clearLEDs();
}

//--------------------------------------------------------------
void DeviceEcho::clearLEDs()
{
    OFAPPLOG->begin("DeviceEcho::clearLEDs()");
    if (m_connectSPI)
    {
        OFAPPLOG->println(" - nbLeds = "+ofToString(m_nbLEDs));
        OFAPPLOG->println(" - data size = "+ofToString(m_spiData.size()));

        for (int i=0; i < m_spiData.size(); i++)
        {
             m_spiData[i]=m_spi.toGamma(0);   
        }
        m_spiColors.assign(m_nbLEDsTotal,ofColor());

        // Send data
        m_spi.send(m_spiData);
    }    

    OFAPPLOG->end();
}


//--------------------------------------------------------------
void DeviceEcho::update(float dt)
{
    // Parent
    //Device::update(dt);
    checkForActivity(dt);
    updateBPM(dt);


    bool isSample = false;
    if (mp_soundInput)
        {
            mp_soundInput->update();
            isSample = true;
        }

    if(isSample)
    {
        // Sample them and writes into packet
        sampleSoundInput();
        
        // Send packets / values to network
        sendPacketsOSC();
    }
}

//--------------------------------------------------------------
void DeviceEcho::updateLEDs()
{
/*    if (m_bExit) 
    {
        OFAPPLOG->println("DeviceEcho::updateLEDs(), m_bExit=1, not sending data to spi");
        return;   
    }
*/

    // LEDs
    int nbPackets = m_listPackets.size();
    int indexPacket = 0;
    int indexSPI = 0;
    if (m_connectSPI && nbPackets == m_nbLEDs) // Sanity check
    {
        // Tranforms packets -> colors
        DevicePacket* pDevicePacket=0;
        for (int i=0; i<nbPackets; i++)
        {
            indexPacket = i;
            indexSPI = m_isReverseDirPackets ? m_nbLEDsTotal-i-1 : i;
            pDevicePacket = m_listPackets[indexPacket];

            m_spiColors[indexSPI][0] = pDevicePacket->m_color[0];
            m_spiColors[indexSPI][1] = pDevicePacket->m_color[1];
            m_spiColors[indexSPI][2] = pDevicePacket->m_color[2];
        }

        if (m_nbLEDsTotal > nbPackets)
        {
            for (int i=nbPackets; i<m_nbLEDsTotal ; i++)
            {
                indexSPI = m_isReverseDirPackets ? m_nbLEDsTotal-i-1 : i;
                m_spiColors[indexSPI][0] = 0;
                m_spiColors[indexSPI][1] = 0;
                m_spiColors[indexSPI][2] = 0;
            }
        }
        
        // Tranforms colors -> uint_8 buffer
        if (!m_bExit)
        {
            int offsetData = 0;
            int indexLED = 0;
            for (int i=0;i<m_spiColors.size();i++)
            {
                m_spiData[offsetData] = m_spi.toGamma( (u_int8_t) m_spiColors[i][1] );
                m_spiData[offsetData+1] = m_spi.toGamma( (u_int8_t) m_spiColors[i][0] );
                m_spiData[offsetData+2] = m_spi.toGamma( (u_int8_t) m_spiColors[i][2] );
                offsetData+=3;
            }
        }
        
        // Send data
        m_spi.send(m_spiData);
    }
}


