/*
 *  globals.h
 *  versatubes
 *
 *  Created by 2Roqs on 01/05/12.
 *  Copyright 2012 2Roqs. All rights reserved.
 *
 */

#pragma once
#include "oscReceiver.h"

class testApp;
class oscReceiver;
class DeviceEcho;

#define GLOBALS Globals::instance()
#define LOG_MESSAGE_OSC(message,isIn) GLOBALS->logMessageOSC(message,isIn)

class Globals
{
	public:
		Globals					();

		static Globals*			instance();
		testApp*				mp_app;
        oscReceiver*            mp_oscReceiver;
        DeviceEcho*             mp_deviceEcho;

		void					logMessageOSC				(const ofxOscMessage&, bool isIn=true);

	private:
		static Globals*			smp_instance;
};
