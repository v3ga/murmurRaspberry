#include "testApp.h"
#include "globals.h"
#include "ofxXmlSettings.h"
#include "ofAppLog.h"

//--------------------------------------------------------------
bool testApp::sm_reboot = false;
bool testApp::sm_turnoff = false;
float testApp::sm_timeTurnoff = 0.0f;

//--------------------------------------------------------------
void testApp::setup()
{
	OFAPPLOG->begin("testApp::setup()");
	m_enableOSC = 0;

    // Settings
    ofxXmlSettings settings;
    settings.loadFile("configuration.xml");
    
    int debugSinewave = settings.getValue("device:debug:sinewave", 0);
    
    string deviceId = settings.getValue("device:properties:id", "chambreEcho_###");
    int deviceNbLEDs = settings.getValue("device:properties:nbLEDs", 40);
    int deviceNbLEDsTotal = settings.getValue("device:properties:nbLEDsTotal", deviceNbLEDs);
    if (deviceNbLEDs > deviceNbLEDsTotal) deviceNbLEDs = deviceNbLEDsTotal; // safe
    float deviceLength = settings.getValue("device:properties:length", 4.0);
    int distLED = deviceLength / float(deviceNbLEDs-1);
    
 
	//m_enableOSC = settings.getValue("device:osc:enable", 0);
    m_enableOSC = 1;
    int port  = settings.getValue("device:osc:port", 1235);
    string serverIP  = settings.getValue("device:osc:serverIP", "127.0.0.1");
    int serverPort  = settings.getValue("device:osc:serverPort", 1234);
    
    int soundInputDeviceId = settings.getValue("device:soundInput:deviceId", 0);
    int soundInputNbChannels = settings.getValue("device:soundInput:nbChannels", 2);

	m_audioVolumeAccum = 0;

	OFAPPLOG->println("- deviceId="+deviceId);
	OFAPPLOG->println("- deviceNbLeds="+ofToString(deviceNbLEDs));
	OFAPPLOG->println("- deviceNbLedsTotal="+ofToString(deviceNbLEDsTotal));
	OFAPPLOG->println("");
	OFAPPLOG->println("- enableOSC="+ofToString(m_enableOSC));
	OFAPPLOG->println("- serverOSC="+serverIP+":"+ofToString(serverPort));
	OFAPPLOG->println("");
	OFAPPLOG->println("- soundInputDeviceId="+ofToString(soundInputDeviceId));
	OFAPPLOG->println("- soundInputNbChannels="+ofToString(soundInputNbChannels));
	OFAPPLOG->println("");
 
    // OSC
	//if (m_enableOSC > 0)
	    m_oscReceiver.setup(port);
    
    // Globals
    Globals::instance()->mp_app = this;
    Globals::instance()->mp_oscReceiver = &m_oscReceiver;
 
    // Device
    mp_deviceEcho = new DeviceEcho(deviceId, deviceNbLEDs, deviceNbLEDsTotal, distLED); // id, nbLEDs, distLEDs
	mp_deviceEcho->m_resetVolHistoryPing = false;
    mp_deviceEcho->setSendMessagesOSC(m_enableOSC > 0 ? true:false);
    mp_deviceEcho->setDebugSinewave( (debugSinewave == 1) );
	mp_deviceEcho->startSoundInput(soundInputDeviceId,soundInputNbChannels);
	if (m_enableOSC > 0)
	    mp_deviceEcho->setupSenderOSC(serverIP, serverPort);

	if (m_enableOSC == 0)
	{

		// Valeurs ÇmanuellesÈ pour le sound input / ping si jamais le fichier ne chargeait pas
		mp_deviceEcho->setSoundInputUseRawVolumeOSC( true );
		mp_deviceEcho->setSoundInputVolHistorySizeOSC( 200 );
		mp_deviceEcho->setSoundInputVolumeMaxOSC( 0.03 );
		mp_deviceEcho->setVolHistoryPingThOSC( 0.9 );

		// Si le deviceId est chambreEcho_002 (dŽfini dans configuration.xml)
		// Chargera alors le fichier chambreEcho_002.xml
	    mp_deviceEcho->loadXML("");

		// Positionne les valeurs du sound input aux valeurs chargŽes depuis le fichier xml
		// (si le chargement a ŽchouŽ, on prend les valeurs au dessus)
		mp_deviceEcho->setSoundInputUseRawVolumeOSC( mp_deviceEcho->m_soundInputUseRawVol );
		mp_deviceEcho->setSoundInputVolHistorySizeOSC( mp_deviceEcho->m_soundInputVolHistorySize );
		mp_deviceEcho->setSoundInputVolumeMaxOSC( mp_deviceEcho->m_soundInputVolEmpiricalMax );

		// Imprime la valeur du ping
		//OFAPPLOG->println("- ping timeline="+ofToString(mp_deviceEcho->m_volHistoryPingTh) );
		//OFAPPLOG->println("");

		// On imprime les valeurs sur la console
		// les valeurs prŽcŽdentes relayŽes ˆ l'entrŽe micro du device (instance mp_soundInput)
		if (mp_deviceEcho->mp_soundInput) // test de routine sur son existence
		{
			OFAPPLOG->println("- mp_soundInput->getUseRawVol()="+ofToString(mp_deviceEcho->mp_soundInput->getUseRawVol()) );
			OFAPPLOG->println("- mp_soundInput->getVolHistory().size()="+ofToString(mp_deviceEcho->mp_soundInput->getVolHistory().size()));
			OFAPPLOG->println("- mp_soundInput->getVolEmpiricalMax()="+ofToString(mp_deviceEcho->mp_soundInput->getVolEmpiricalMax()));
		}
	}

	// Pas de stand by
	// mp_deviceEcho->enableStandbyMode(false);

	// Globals for device
    Globals::instance()->mp_deviceEcho = mp_deviceEcho;


    // Timeline
/*
	m_timeRestartTimeline = 0.0f;
	m_canRestartTimeline = true;
	
    m_timeline.load("timeline.xml");
    m_timeline.setEventsCallback( (timelineSimpleEvent::timelineSimpleCb) sM_timelineSimpleEvent, this);
	m_isTimelineStarted = false;
*/

	// Timing / update
	m_timeUpdate = 0.0f;

    // LEDs
    m_isSetupLEDs = false;
    m_timeSetupLEDs = 2.0f;
    m_time = 0.0f;

    #if MURMUR_USE_LEDS
    //mp_deviceEcho->setupLEDs();
    //startThread(true, false);
    #endif
    
    #if MURMUR_HEADLESS
    #else
	    ofSetVerticalSync(true);
    #endif

	OFAPPLOG->end();
}

//--------------------------------------------------------------
void testApp::turnoff(bool bReboot)
{
	OFAPPLOG->begin("Raspberry testApp::turnoff("+ofToString(bReboot)+")");
	sm_reboot = bReboot;
	sm_turnoff = true;
	sm_timeTurnoff = 0.0;
	ofExit();
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void testApp::exit()
{
//	m_timeline.stopThread();
	OFAPPLOG->begin("Raspberry testApp::exit()");

    if (mp_deviceEcho)
    {
		OFAPPLOG->println(" - delete mp_deviceEcho");
		mp_deviceEcho->exit();
    	delete mp_deviceEcho;
	}

	OFAPPLOG->println(" - stopping thread");
    stopThread();

    if (sm_reboot)
    {
		OFAPPLOG->println(" - sudo reboot");
    	ofSystem("sudo reboot");
	}

	OFAPPLOG->end();
}

//--------------------------------------------------------------
void testApp::sM_timelineSimpleEvent(testApp* pThis, timelineSimpleEvent* pEvent)
{
	if (pEvent == 0) return;
	if (pThis->mp_deviceEcho == 0) return;

    int delta = pEvent->m_timeTrigger-pEvent->m_time;
    ofLog() << "TIME EVENT '"+pEvent->m_id+"' time="+ofToString(pEvent->m_time)+" millis, time trigger="+ofToString(pEvent->m_timeTrigger)+", delta="+ofToString(delta);

	if (pEvent->m_id == "normal")
	{
		pThis->mp_deviceEcho->reversePacketsDirOSC(false);
	}
	else if (pEvent->m_id == "backwards")
	{
		pThis->mp_deviceEcho->reversePacketsDirOSC(true);
	}
	else if (pEvent->m_id == "color_red")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::red.getHue(),ofColor::red.getSaturation());
	}
	else if (pEvent->m_id == "color_green")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::green.getHue(),ofColor::green.getSaturation());
	}
	else if (pEvent->m_id == "color_blue")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::blue.getHue(),ofColor::blue.getSaturation());
	}
	else if (pEvent->m_id == "color_yellow")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::yellow.getHue(),ofColor::yellow.getSaturation());
	}
	else if (pEvent->m_id == "color_cyan")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::cyan.getHue(),ofColor::cyan.getSaturation());
	}
	else if (pEvent->m_id == "color_magenta")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::magenta.getHue(),ofColor::magenta.getSaturation());
	}
	else if (pEvent->m_id == "color_white")
	{
		pThis->mp_deviceEcho->enableColorOSC(true);
		pThis->mp_deviceEcho->setColorHueSaturationOSC(ofColor::white.getHue(),ofColor::white.getSaturation());
	}
	

}

//--------------------------------------------------------------
void testApp::update()
{
	// dt
	float dt = (float) ofGetLastFrameTime();
    m_time+=dt;
    // ofLog() << ofToString(dt);

/*    if (sm_turnoff)
    {

    	sm_timeTurnoff += dt;

    	// force LEDs to 0
    	if (mp_deviceEcho)
    		mp_deviceEcho->exit();

    	if (sm_timeTurnoff >= 1.0f)
    	{
			OFAPPLOG->println(" - calling ofExit()");
			ofExit();
    	}
    	//return;
    }
*/
    
	// TIMELINE
/*
	m_timeRestartTimeline += dt;
	if (m_timeRestartTimeline>=1.0 && m_canRestartTimeline == false)
	{
		m_canRestartTimeline = true;
	}
*/

	// LEDS
    if (!m_isSetupLEDs && m_time>=m_timeSetupLEDs){

        #if MURMUR_USE_LEDS
        if (mp_deviceEcho)
		{
            OFAPPLOG->println("STARTING LEDs !");
            mp_deviceEcho->setupLEDs();
            startThread(true, false);
        }
        #endif
        m_isSetupLEDs = true;
    }

    // OSC
 	//if (m_enableOSC>0)

    // Device
    bool bUpdate = false;
    m_timeUpdate += dt;
    if (m_timeUpdate >= 1.0f/60.0f)
    {
    	m_timeUpdate = 0.0f;
		bUpdate = true;
    }

    lock();
    if (mp_deviceEcho && m_isSetupLEDs && bUpdate)
	{
		m_oscReceiver.update();
        mp_deviceEcho->update(1.0f/60.0f);
	}
    unlock();
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofBackground(0);
    ofEnableAlphaBlending();
    if (mp_deviceEcho)
	{
        mp_deviceEcho->drawSoundInputVolume( 0.5f*(ofGetWidth()-mp_deviceEcho->getWidthSoundInputVolume()) ,0.5*(ofGetHeight()-mp_deviceEcho->getHeightSoundInputVolume()) );
	}
}


//--------------------------------------------------------------
float testApp::computeAudioVolume(float * input, int bufferSize, int nChannels)
{
	float curVol = 0.0f;
	if (nChannels == 1)
	{
		if (m_mono.size()==0 && mp_deviceEcho && mp_deviceEcho->mp_soundInput)
		{
	        m_mono.assign(mp_deviceEcho->mp_soundInput->getBufferSize(), 0.0f);
		}

		if (m_mono.size()>0)
		{
			int numCounted = 0;
			for (int i = 0; i < bufferSize; i++){
				m_mono[i] = input[i];
				curVol += m_mono[i]*m_mono[i];
				numCounted++;
			}
	        curVol /= (float)numCounted;
	        curVol = sqrt( curVol );
		}
	}
	return curVol;
}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels)
{
    if (mp_deviceEcho)
        mp_deviceEcho->audioIn(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void testApp::threadedFunction()
{
	while( isThreadRunning() != 0 )
	{
		if( lock() )
		{
            
            if (mp_deviceEcho)
                mp_deviceEcho->updateLEDs();
            
			unlock();
			usleep(10000); // micro-seconds
		}
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}