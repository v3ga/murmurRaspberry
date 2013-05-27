#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

#if MURMUR_HEADLESS
#include "ofAppNoWindow.h"
#include "ofNoWindowRunner.h"
#endif


//========================================================================
int main()
{
    #if MURMUR_HEADLESS
        ofAppNoWindow window;
        window.setFrameRate(60);
        ofSetupNoWindow(&window, 1024, 768, OF_WINDOW);
    #else
        ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context
    #endif
	
    ofRunApp(new testApp());

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	// ofRunFensterApp(new testApp());
}

