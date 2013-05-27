#include "testApp.h"
#include "globals.h"
#include "ofxXmlSettings.h"

//--------------------------------------------------------------
void testApp::setup()
{
    // Settings
    ofxXmlSettings settings;
    settings.loadFile("configuration.xml");
    
    int debugSinewave = settings.getValue("device:debug:sinewave", 0);
    
    string deviceId = settings.getValue("device:properties:id", "chambreEcho_###");
    int deviceNbLEDs = settings.getValue("device:properties:nbLEDs", 40);
    float deviceLength = settings.getValue("device:properties:length", 4.0);
    int distLED = deviceLength / float(deviceNbLEDs-1);
    
    
    int port  = settings.getValue("device:osc:port", 1235);
    string serverIP  = settings.getValue("device:osc:serverIP", "127.0.0.1");
    int serverPort  = settings.getValue("device:osc:serverPort", 1234);
    
    int soundInputDeviceId = settings.getValue("device:soundInput:deviceId", 0);
    int soundInputNbChannels = settings.getValue("device:soundInput:nbChannels", 2);

    printf("Settings\n----------------\n");
    printf("> sinewave=%s\n", deviceId.c_str());
    printf("-\n");
    printf("> deviceId=%s\n", deviceId.c_str());
    printf("> deviceNbLeds=%d\n", deviceNbLEDs);
    printf("> deviceLength=%.2f\n", deviceLength);
    printf("-\n");
    printf("> port=%d\n", port);
    printf("> serverIP=%s\n", serverIP.c_str());
    printf("> serverPort=%d\n", serverPort);
    printf("-\n");
    printf("> soundInputDeviceId=%d\n", soundInputDeviceId);
    printf("> soundInputNbChannels=%d\n", soundInputNbChannels);
    printf("----------------\n");
    
    
    // OSC
    m_oscReceiver.setup(port);
    
    // Globals
    Globals::instance()->mp_app = this;
    Globals::instance()->mp_oscReceiver = &m_oscReceiver;
    
    // Device
    mp_deviceEcho = new DeviceEcho(deviceId, deviceNbLEDs, distLED); // id, nbLEDs, distLEDs
    mp_deviceEcho->setDebugSinewave( (debugSinewave == 1) );
    mp_deviceEcho->startSoundInput(soundInputDeviceId,soundInputNbChannels);
    mp_deviceEcho->setupSenderOSC(serverIP, serverPort);

    Globals::instance()->mp_deviceEcho = mp_deviceEcho;

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
}

//--------------------------------------------------------------
void testApp::exit()
{
    stopThread();
    delete mp_deviceEcho;
}

//--------------------------------------------------------------
void testApp::update()
{
	// dt
	float dt = (float) ofGetLastFrameTime();
    m_time+=dt;
    if (!m_isSetupLEDs && m_time>=m_timeSetupLEDs){

        #if MURMUR_USE_LEDS
        if (mp_deviceEcho){
            printf(">>> starting LEDs\n");
            mp_deviceEcho->setupLEDs();
            startThread(true, false);
        }
        #endif
        m_isSetupLEDs = true;
    }

    // OSC
    m_oscReceiver.update();

    // Device
    lock();
    if (mp_deviceEcho)
        mp_deviceEcho->update(dt);
    unlock();
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofBackground(0);
    ofEnableAlphaBlending();
    if (mp_deviceEcho)
        mp_deviceEcho->drawSoundInputVolume( 0.5f*(ofGetWidth()-mp_deviceEcho->getWidthSoundInputVolume()) ,0.5*(ofGetHeight()-mp_deviceEcho->getHeightSoundInputVolume()) );
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
	while( isThreadRunning() != 0 ){
		if( lock() ){
            
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