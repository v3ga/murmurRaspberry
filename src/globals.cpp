/*
 *  globals.cpp
 *  versatubes
 *
 *  Created by 2Roqs on 01/05/12.
 *  Copyright 2012 2Roqs. All rights reserved.
 *
 */

#include "globals.h"
#include "testApp.h"
#include "deviceEcho.h"

Globals* Globals::smp_instance = 0;

Globals::Globals()
{
	mp_app = 0;
    mp_oscReceiver = 0;
    mp_deviceEcho = 0;
}

Globals* Globals::instance(){
	if (smp_instance == 0){
		smp_instance = new Globals();
	}
	return smp_instance; 
}

//--------------------------------------------------------------
void Globals::logMessageOSC(const ofxOscMessage& message, bool isIn)
{
}


