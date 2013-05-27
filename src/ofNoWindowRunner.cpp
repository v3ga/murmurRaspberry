//
//  ofNoWindowRunner.cpp

#include "ofNoWindowRunner.h"
#include "ofAppRunner.cpp"

void ofSetupNoWindow(ofAppBaseWindow * windowPtr, int w, int h, int screenMode){
    printf("1.ofSetupNoWindow()\n");
	ofSetupNoWindow(ofPtr<ofAppBaseWindow>(windowPtr,std::ptr_fun(noopDeleter)),w,h,screenMode);
}

void ofSetupNoWindow(ofPtr<ofAppBaseWindow> windowPtr, int w, int h, int screenMode){
    printf("2.ofSetupNoWindow()\n");
	window = windowPtr;
	window->setupOpenGL(w, h, screenMode);
}

void ofSetupNoWindow(int w, int h, int screenMode){
    printf("3.ofSetupNoWindow()\n");
/*
#ifdef TARGET_OF_IPHONE
    window = ofPtr<ofAppBaseWindow>(new ofAppiPhoneWindow());
#elif defined TARGET_ANDROID
    window = ofPtr<ofAppBaseWindow>(new ofAppAndroidWindow());
#else
    window = ofPtr<ofAppBaseWindow>(new ofAppGlutWindow());
#endif
    
	ofSetupNoWindow(window,w,h,screenMode);
*/
}