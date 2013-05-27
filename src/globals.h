/*
 *  globals.h
 *  versatubes
 *
 *  Created by 2Roqs on 01/05/12.
 *  Copyright 2012 2Roqs. All rights reserved.
 *
 */

#pragma once

class testApp;
class oscReceiver;
class DeviceEcho;

class Globals
{
	public:
		Globals					();

		static Globals*			instance();
		testApp*				mp_app;
        oscReceiver*            mp_oscReceiver;
        DeviceEcho*             mp_deviceEcho;

	private:
		static Globals*			smp_instance;
};
