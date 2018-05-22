#pragma once

#include "ofMain.h"
#include "oscReceiver.h"
#include "deviceEcho.h"
#include "testAppDefines.h"
#include "timelineSimple.h"
#include "ofxAubio.h"

class testApp : public ofBaseApp, public ofThread
{
	public:
		void setup();
		void exit();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void audioIn(float * input, int bufferSize, int nChannels);

        static void turnoff(bool bReboot);
 
         ofxAubioPitch pitch;

 		// Turnoff / reboot
 		static bool		sm_reboot;
 		static bool		sm_turnoff;
 		static float	sm_timeTurnoff;


        // Thread function for LEDs
        void 			threadedFunction();
        float			m_timeSetupLEDs;
        float			m_time;
        bool			m_isSetupLEDs;
    
        // OSC
		int				m_enableOSC;
        oscReceiver     m_oscReceiver;

        // Device
        DeviceEcho*     mp_deviceEcho;

        // Timing / update
        float			m_timeUpdate;
 
		// Timeline
		timelineSimple	m_timeline;
		bool			m_isTimelineStarted;
		static void 	sM_timelineSimpleEvent	(testApp* pThis, timelineSimpleEvent* pEvent);
 		bool			m_canRestartTimeline;
		float			m_timeRestartTimeline;

		// Ping
		float			m_audioPingValue;
		float			m_audioVolumeAccum;
		vector<float>	m_mono;

		float			computeAudioVolume		(float * input, int bufferSize, int nChannels);
};
