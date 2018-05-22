/*
 *  device.cpp
 *  murmur
 *
 *  Created by Julien on 10/04/13.
 *  Copyright 2013 2Roqs. All rights reserved.
 *
 */

#include "device.h"
#include "soundInput.h"
#include "ofAppLog.h"
#include "globals.h"

//--------------------------------------------------------------
DevicePacket::DevicePacket()
{
    m_volume = 0.0f;
	m_pitch = 0.0f;
}

//--------------------------------------------------------------
void DevicePacket::copy(DevicePacket* pPacket)
{
    m_volume = pPacket->m_volume;
    m_pitch = pPacket->m_pitch;
	m_color = pPacket->m_color;
}

//--------------------------------------------------------------
void DevicePacket::computeColor(const ofColor& deviceColor, bool isColor, bool isInvert)
{
	float volume = isInvert ? 1.0f-m_volume : m_volume;

	if (isColor)
	{
		m_color.setHue(deviceColor.getHue());
		m_color.setSaturation(deviceColor.getSaturation());
		m_color.setBrightness(volume*255.0f);
	}
	else
	{
		m_color.set(volume*255.0f,volume*255.0f,volume*255.0f);
	}
}


//--------------------------------------------------------------
void DevicePacket::computeColor(const float* deviceColor,bool isColor, bool isInvert)
{
	float volume = isInvert ? 1.0f-m_volume : m_volume;

	if (isColor)
		m_color = ofColor::fromHsb(deviceColor[0],deviceColor[1],volume*255.0f);
	else
		m_color.set(volume*255.0f,volume*255.0f,volume*255.0f);
//	m_color.setHsb(deviceColor[0],deviceColor[1],m_volume*255.0f);
}

//--------------------------------------------------------------
void DevicePacket::computeColor2(const float* deviceColor, const float* deviceColor2, float t, bool isColor, bool isInvert)
{
	float volume = isInvert ? 1.0f-m_volume : m_volume;

	if (isColor)
	{
//		m_color = ofColor::fromHsb(ofLerp(deviceColor[0],deviceColor2[0],t),ofLerp(deviceColor[1],deviceColor2[1],t),volume*255.0f);
		m_color = ofColor::fromHsb(deviceColor[0],deviceColor[1], volume*255).lerp(  ofColor::fromHsb(deviceColor2[0],deviceColor2[1], volume*255) ,t);
	}
	else
		m_color.set(volume*255.0f,volume*255.0f,volume*255.0f);
}


//--------------------------------------------------------------
Device::Device(string id, int nbLEDs, float distLEDs)
{
    mp_soundInput = 0;

    m_indexPacketReceived = 0;
    setPointSurface(0.5f,0.5f);
    set(id,nbLEDs,distLEDs);
    m_isActive = false;
    m_isEnableStandbyMode = true;
    m_stateStandby = EStandby_active;
    m_stateStandbyDuration = 0.0f;
    m_timeStandby = 10.0f;
	mp_sampleStandBy = 0;
	m_sampleVolStandby = 0.35f;
	m_sampleNameStandby = "Sounds/StandBy/theme1-4.wav";
	m_soundInputVolEmpiricalMaxMin = 0.0;
	m_soundInputVolEmpiricalMaxMax = 0.04;
	m_soundInputPitchMin = 30.0f;
	m_soundInputPitchMax = 128.0f;
	m_bEnablePitch		 = true;


	m_bpmEnable			= false;
	m_bpm				= 120;
	
	m_bpmTime			= 0.0f;
	

	m_isEnableStandup	= false;
	m_standupTh			= 0.55f;
	
	
	m_color.setHsb(200, 255, 255); // Brightness not used as set by packets
	m_colorMode = colorMode_manual_hsb;
	m_colorSpeedOscillation = 10.0f;

	m_color2.setHsb(200, 255, 255);
	for (int i=0;i<3;i++){
		m_colorHsv[i]=0.0f;
		m_colorHsv2[i]=0.0f;
	}
	
	m_isUpdatingPacket = false;
	m_isSendMessagesOSC = true;
	m_isEnableColor = false;
	m_isInvertPacketsVolume = false;
	m_isReverseDirPackets = false;

	m_soundInputUseRawVol = true;
	m_soundInputMute = false;
	
	m_volHistoryPingTh = 0.5;
	
	m_bGenerative = true;
}

//--------------------------------------------------------------
Device::~Device()
{
	Sample::lock();
	delete mp_sampleStandBy;
	mp_sampleStandBy = 0;
	if (mp_soundInput) mp_soundInput->setSample(0);
	Sample::unlock();

    delete mp_soundInput;
}

//--------------------------------------------------------------
void Device::set(string id, int nbLEDs, float distLEDs)
{
    if (nbLEDs<2)
        nbLEDs = 2;
    
    m_id = id;
    m_nbLEDs = nbLEDs;
    m_distLEDs = distLEDs;
    m_length = m_distLEDs*(m_nbLEDs-1);
//    printf("m_length=%.3f\n", m_length);
    
    createPackets(m_nbLEDs);
}

//--------------------------------------------------------------
void Device::setPointSurface(float xNorm, float yNorm)
{
    m_pointSurface.set(xNorm,yNorm);
}

//--------------------------------------------------------------
void Device::setColorHueSaturation(float h, float s)
{
	m_colorHsv[0]=h;
	m_colorHsv[1]=s;

	m_color.setHue(h);
	m_color.setSaturation(s);

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("color");
	m.addFloatArg(m_colorHsv[0]);
	m.addFloatArg(m_colorHsv[1]);
    m_oscSender.sendMessage(m);
}

void Device::setColorHue(float h)
{
	OFAPPLOG->begin("Device::setColorHue("+ofToString(h)+")");

	m_color.setHue(h);
	m_colorHsv[0]=h;

	 OFAPPLOG->println("m_color.getHue()="+ofToString(m_color.getHue()));
	 OFAPPLOG->println("m_color.getSaturation()="+ofToString(m_color.getSaturation()));

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("color");
	m.addFloatArg(m_colorHsv[0]);
	m.addFloatArg(m_colorHsv[1]);
    m_oscSender.sendMessage(m);

	OFAPPLOG->end();
}

void Device::setColorSaturation(float s)
{
	OFAPPLOG->begin("Device::setColorSaturation("+ofToString(s)+")");


	m_color.setSaturation(s);
	m_colorHsv[1]=s;

	OFAPPLOG->println("m_color.getHue()="+ofToString(m_color.getHue()));
	OFAPPLOG->println("m_color.getSaturation()="+ofToString(m_color.getSaturation()));

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("color");
	m.addFloatArg(m_colorHsv[0]);
	m.addFloatArg(m_colorHsv[1]);
    m_oscSender.sendMessage(m);
	
	OFAPPLOG->end();
}

//--------------------------------------------------------------
void Device::enableColor(bool is)
{
	m_isEnableColor = is;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("enableColor");
	m.addIntArg( is ? 1 : 0);
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::enableColorOSC(bool is)
{
	m_isEnableColor = is;
}

//--------------------------------------------------------------
void Device::setColorHueSaturationOSC(float h, float s)
{
	m_colorHsv[0]=h;
	m_colorHsv[1]=s;

	m_color.setHue(h);
	m_color.setSaturation(h);
	
	
	// OFAPPLOG->println(ofToString(m_color));
}


//--------------------------------------------------------------
void Device::setColor2HueSaturation(float h, float s)
{
	m_colorHsv2[0]=h;
	m_colorHsv2[1]=s;

	m_color2.setHue(h);
	m_color2.setSaturation(s);

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("color2");
	m.addFloatArg(m_colorHsv2[0]);
	m.addFloatArg(m_colorHsv2[1]);
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::setColor2HueSaturationOSC(float h, float s)
{
	m_colorHsv2[0]=h;
	m_colorHsv2[1]=s;

	m_color2.setHue(h);
	m_color2.setSaturation(h);
}


//--------------------------------------------------------------
float Device::getWidthSoundInputVolume()
{
    if (mp_soundInput)
        return (float)2*mp_soundInput->getVolHistory().size();
    return 0.0f;
}

//--------------------------------------------------------------
float Device::getHeightSoundInputVolume()
{
    if (mp_soundInput)
        return (float)mp_soundInput->getHeightDraw();
    return 0.0f;
}


//--------------------------------------------------------------
void Device::setSoundInputVolumeMax(float v)
{
    m_soundInputVolEmpiricalMax = v;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volMax");
    m.addFloatArg(v);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);

}

//--------------------------------------------------------------
void Device::setSoundInputVolHistorySize(int nb)
{
    //printf("setSoundInputVolHistorySize, v=%d\n", nb);
    m_soundInputVolHistorySize = nb;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volHistorySize");
    m.addIntArg(nb);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);

}


//--------------------------------------------------------------
void Device::setSoundInputVolHistorySizeOSC(int nb)
{
    if (mp_soundInput){
        mp_soundInput->setVolHistorySize(nb);
    }
}


//--------------------------------------------------------------
void Device::setSoundInputVolumeMaxOSC(float v)
{
    //printf("setVolumeMaxOSC, v=%.3f\n", v);
    if (mp_soundInput){
        mp_soundInput->setVolEmpiricalMax(v);
    }
}

//--------------------------------------------------------------
void Device::setSoundInputVolumeMaxMin(float v)
{
	m_soundInputVolEmpiricalMaxMin = v;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volMaxMin");
    m.addFloatArg(v);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::setSoundInputVolumeMaxMax(float v)
{
	m_soundInputVolEmpiricalMaxMax = v;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volMaxMax");
    m.addFloatArg(v);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::setSoundInputVolumeMaxMinOSC(float v)
{
	m_soundInputVolEmpiricalMaxMin = v;
}

//--------------------------------------------------------------
void Device::setSoundInputVolumeMaxMaxOSC(float v)
{
	m_soundInputVolEmpiricalMaxMax = v;
}


//--------------------------------------------------------------
void Device::setSoundInputVolHistoryTh(float th)
{
    m_volHistoryTh = th;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volHistoryTh");
    m.addFloatArg(th);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::setSoundInputVolHistoryThOSC(float th)
{
    m_volHistoryTh = th;
}


//--------------------------------------------------------------
void Device::setEnablePitch(bool is)
{
	m_bEnablePitch = is;
	
	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("enablePitch");
    m.addIntArg( m_bEnablePitch ? 1 : 0 );
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::setEnablePitchOSC(bool is)
{
	m_bEnablePitch = is;
}


//--------------------------------------------------------------
void Device::setSoundInputPitchMin(float min)
{
	m_soundInputPitchMin = min;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("pitchMin");
    m.addFloatArg(min);
    m_oscSender.sendMessage(m);
	
}

//--------------------------------------------------------------
void Device::setSoundInputPitchMinOSC(float min)
{
	if (mp_soundInput)
		mp_soundInput->setPitchMin(min);
}

//--------------------------------------------------------------
void Device::setSoundInputPitchMax(float max)
{
	m_soundInputPitchMax = max;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("pitchMax");
    m.addFloatArg(max);
    m_oscSender.sendMessage(m);
	
}

//--------------------------------------------------------------
void Device::setSoundInputPitchMaxOSC(float max)
{
	if (mp_soundInput)
		mp_soundInput->setPitchMax(max);
}

//--------------------------------------------------------------
void Device::setGenerative(bool is)
{
	m_bGenerative = is;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
	m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("enableGenerative");
	m.addIntArg( is ? 1 : 0);
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::setGenerativeOSC(bool is)
{
	m_bGenerative = is;
}


//--------------------------------------------------------------
void Device::turnoff(bool bReboot)
{
    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_TURN_OFF );
    m.addStringArg(m_id);
	m.addIntArg( bReboot ? 1 : 0);
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::setEnableStandbyMode(bool is)
{
    enableStandbyMode(is);

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("enableStandbyMode");
    m.addIntArg(is ? 1 : 0);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::setEnableStandbyModeOSC(bool is)
{
    enableStandbyMode(is);
}

//--------------------------------------------------------------
float Device::getSoundInputVolumeMax()
{
//    if (mp_soundInput)
//        return mp_soundInput->getVolEmpiricalMax();
    return m_soundInputVolEmpiricalMax;
}

//--------------------------------------------------------------
float Device::getSoundInputVolumeMaxMin()
{
    return m_soundInputVolEmpiricalMaxMin;
}

//--------------------------------------------------------------
float Device::getSoundInputVolumeMaxMax()
{
    return m_soundInputVolEmpiricalMaxMax;
}

//--------------------------------------------------------------
float Device::getSoundInputVolHistorySize()
{
//    if (mp_soundInput)
//        return mp_soundInput->getVolHistory().size();
    return m_soundInputVolHistorySize;
}

//--------------------------------------------------------------
void Device::resetStandBy()
{
	if (mp_soundInput==0) return;

	Sample::lock();

	if (mp_sampleStandBy)
	{
		delete mp_sampleStandBy;
		mp_sampleStandBy = 0;
	}

	mp_sampleStandBy = new Sample();
	if ( mp_sampleStandBy->load(ofToDataPath(m_sampleNameStandby)) )
	{
		mp_soundInput->setSample(mp_sampleStandBy);
	}else
	{
		delete mp_sampleStandBy;
		mp_sampleStandBy = 0;
		mp_soundInput->setSample(0);
	}
	Sample::unlock();
}

//--------------------------------------------------------------
void Device::setTimeStandby(float v)
{
    m_timeStandby = v;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("timeStandby");
    m.addFloatArg(v);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
 
}

//--------------------------------------------------------------
void Device::setTimeStandbyOSC(float v)
{
    m_timeStandby = v;
}

//--------------------------------------------------------------
void Device::setSampleVolumeStandby(float v)
{
	m_sampleVolStandby = v;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("sampleVolStandby");
    m.addFloatArg(v);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
	
}

//--------------------------------------------------------------
void Device::setSampleVolumeStandbyOSC(float v)
{
    if (mp_soundInput){
        mp_soundInput->setSampleVolume(v);
    }
}

//--------------------------------------------------------------
void Device::setEnableStandup(bool is)
{
	m_isEnableStandup = is;

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("enableStandup");
    m.addIntArg(is ? 1 : 0);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
	
}

//--------------------------------------------------------------
void Device::setEnableStandupOSC(bool is)
{
	m_isEnableStandup = is;
}


//--------------------------------------------------------------
void Device::setStandupVol(float v)
{
	m_standupTh = v;

	if (m_isSendMessagesOSC == false) return;
    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volStandup");
    m.addFloatArg(v);
    m_oscSender.sendMessage(m);
	
	LOG_MESSAGE_OSC(m,false);
	
}

//--------------------------------------------------------------
void Device::setStandupVolOSC(float v)
{
	m_standupTh = v;
}

//--------------------------------------------------------------
void Device::setBPM(int v)
{
	OFAPPLOG->println("Device::setBPM("+ofToString(v)+") for device '"+this->m_id+"'");
	m_bpm = v;
	computeBPM();

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("bpm");
    m.addIntArg(m_bpm);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);

}

//--------------------------------------------------------------
void Device::setBPMOSC(int v)
{
	OFAPPLOG->println("Device::setBPMOSC("+ofToString(v)+") for device '"+this->m_id+"'");
	m_bpm = v;
	computeBPM();
}

//--------------------------------------------------------------
void Device::computeBPM()
{
	m_bpmPeriod = 60.0f/(float)m_bpm*0.5f;
	if (m_bpmEnable){
		m_bpmTime = 0.0f;
	}
}

//--------------------------------------------------------------
void Device::setBPMEnable(bool is)
{
	m_bpmEnable = is;
	computeBPM();

	if (m_isSendMessagesOSC == false) return;

    ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("bpmEnable");
    m.addIntArg(is ? 1 : 0);
    m_oscSender.sendMessage(m);

	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::setBPMEnableOSC(bool is)
{
	m_bpmEnable = is;
	computeBPM();
}




//--------------------------------------------------------------
void Device::drawSoundInputVolume(float x, float y)
{
    if (mp_soundInput){
        mp_soundInput->drawVolume(x,y);
        ofSetColor(255);

        if (m_stateStandby == EStandby_active)
            m_stateStandbyStr = "active";
        else if (m_stateStandby == EStandby_pre_standby)
            m_stateStandbyStr = "standby (pre)";
        else if (m_stateStandby == EStandby_standby)
            m_stateStandbyStr = "standby";
        else if (m_stateStandby == EStandby_standup){
            m_stateStandbyStr = "standup";
        }
     
        ofDrawBitmapString(m_stateStandbyStr, x-getWidthSoundInputVolume()/2,y+12);
        ofSetColor(0,255,0);
        float yTh = y+(1.0-m_volHistoryTh)*getHeightSoundInputVolume();
        ofLine(x-getWidthSoundInputVolume()/2,yTh,x,yTh);

        ofSetColor(0,0,255);
		yTh = y+(1.0-m_standupTh)*getHeightSoundInputVolume();
        ofLine(x-getWidthSoundInputVolume()/2,yTh,x,yTh);

        ofSetColor(255,255,0);
		yTh = y+(1.0-m_volHistoryPingTh)*getHeightSoundInputVolume();
        ofLine(x-getWidthSoundInputVolume()/2,yTh,x,yTh);

    }
}


//--------------------------------------------------------------
void Device::drawSoundInputPitch(float x, float y)
{
	if (mp_soundInput)
	{
		mp_soundInput->drawPitch(x,y);
	}
}
//--------------------------------------------------------------
void Device::startSoundInput(int deviceId, int nbChannels)
{
    if (mp_soundInput==0)
    {
        // Create sound input
        mp_soundInput = new SoundInput();
        mp_soundInput->setup(deviceId,nbChannels);
    }
}

//--------------------------------------------------------------
void Device::startSoundInput(int nbChannels)
{
    if (mp_soundInput==0)
    {
        // Create sound input
        mp_soundInput = new SoundInput();
        mp_soundInput->setup(nbChannels); // do not create an audio stream
    }
}

//--------------------------------------------------------------
void Device::setSoundInputUseRawVolume(bool is)
{
	m_soundInputUseRawVol = is;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("useRawVol");
    m.addIntArg(m_soundInputUseRawVol ? 1 : 0);
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::setSoundInputUseRawVolumeOSC(bool is)
{
	if (mp_soundInput)
		mp_soundInput->useRawVolume(is);
}

//--------------------------------------------------------------
void Device::setSoundInputMute(bool is)
{
	m_soundInputMute = is;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("mute");
    m.addIntArg(m_soundInputMute ? 1 : 0);
    m_oscSender.sendMessage(m);

}

//--------------------------------------------------------------
void Device::setSoundInputMuteOSC(bool is)
{
	if (mp_soundInput)
		mp_soundInput->mute(is);
}

//--------------------------------------------------------------
void Device::audioIn(float * input, int bufferSize, int nChannels)
{
    if (mp_soundInput){
		mp_soundInput->audioIn(input, bufferSize, nChannels);
	}
}

//--------------------------------------------------------------
void Device::createPackets(int nb)
{
    deletePackets();
    for (int i=0;i<nb;i++){
        m_listPackets.push_back( new DevicePacket() );
    }
}

//--------------------------------------------------------------
void Device::deletePackets()
{
    vector<DevicePacket*>::iterator it;
    for(it = m_listPackets.begin(); it != m_listPackets.end();)
    {
        delete *it;
        it = m_listPackets.erase(it);
    }
}

//--------------------------------------------------------------
void Device::sampleSoundInput()
{
    if (!mp_soundInput) return;
    if (m_listPackets.size()<=2) return;
    
    int nbPackets = m_listPackets.size();
    int nbVolHistorySize = mp_soundInput->getVolHistory().size();
    float stepSample = float( mp_soundInput->getVolHistory().size() )/ float(nbPackets-1) ;
    
    float indexSample = 0.0f;
	float pitch = 0.0f;
	int indexPacket = 0;
    for (int i=0;i<nbPackets;i++)
    {
        if (indexSample>nbVolHistorySize-1)
            indexSample = nbVolHistorySize-1;

		indexPacket = i;
		pitch = mp_soundInput->getPitchHistory()[(int)indexSample];
	 
		m_listPackets[indexPacket]->m_volume = mp_soundInput->getVolHistory()[(int)indexSample]; // Nearest sampling
		m_listPackets[indexPacket]->m_pitch = pitch;

		if (m_bEnablePitch)
			m_listPackets[indexPacket]->computeColor2(m_colorHsv, m_colorHsv2, pitch, m_isEnableColor, m_isInvertPacketsVolume);
		else
			m_listPackets[indexPacket]->computeColor(m_colorHsv, m_isEnableColor, m_isInvertPacketsVolume);


        indexSample += stepSample;
    }
}

//--------------------------------------------------------------
void Device::invertPacketsVolume(bool is)
{
	m_isInvertPacketsVolume = is;

	if (m_isSendMessagesOSC == false) return;
	ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("invertPacketsVolume");
    m.addIntArg(m_isInvertPacketsVolume ? 1 : 0);
    m_oscSender.sendMessage(m);
	
	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::invertPacketsVolumeOSC(bool is)
{
	m_isInvertPacketsVolume = is;
}

//--------------------------------------------------------------
void Device::reversePacketsDir(bool is)
{
	m_isReverseDirPackets = is;

	if (m_isSendMessagesOSC == false) return;
	ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("reversePacketsDir");
    m.addIntArg(m_isReverseDirPackets ? 1 : 0);
    m_oscSender.sendMessage(m);
	
	LOG_MESSAGE_OSC(m,false);
}

//--------------------------------------------------------------
void Device::reversePacketsDirOSC(bool is)
{
	m_isReverseDirPackets = is;
}

//--------------------------------------------------------------
void Device::setVolHistoryPingTh(float v)
{
	m_volHistoryPingTh = v;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_SET_DEVICE_PROP );
    m.addStringArg(m_id);
    m.addStringArg("volHistoryPingTh");
    m.addFloatArg(m_volHistoryPingTh);
    m_oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void Device::setVolHistoryPingThOSC(float v)
{
	m_volHistoryPingTh = v;
}

//--------------------------------------------------------------
void Device::resetVolHistoryPing()
{
	m_resetVolHistoryPing = true;

	if (m_isSendMessagesOSC == false) return;

	ofxOscMessage m;
    m.setAddress( OSC_ADDRESS_RESET_PING );
    m.addStringArg(m_id);
    m_oscSender.sendMessage(m);

}

//--------------------------------------------------------------
void Device::resetVolHistoryPingOSC()
{
	m_resetVolHistoryPing = true;
}

//--------------------------------------------------------------
void Device::sendPacketsOSC()
{
	if (m_isSendMessagesOSC == false) return;

	int nbLEDsUpdate = 80;
	int nbOSCMessages = m_nbLEDs / nbLEDsUpdate;
	int nbLEDsRest = m_nbLEDs % nbLEDsUpdate;
	if (nbLEDsRest > 0){
		nbOSCMessages += 1;
	}


	// BEGIN SENDING PACKETS
    ofxOscMessage m_begin;
    m_begin.setAddress( OSC_ADDRESS_SEND_PACKETS_BEGIN );
    m_begin.addStringArg(m_id);
	m_begin.addIntArg(m_nbLEDs);
    m_oscSender.sendMessage(m_begin);

	
	// SENDING PACKETS
	int offsetPackets = 0;
	int offset = 0;
	for (int k=0;k<nbOSCMessages;k++)
	{

	    ofxOscMessage m;
    	m.setAddress( OSC_ADDRESS_SEND_PACKETS );
    	m.addStringArg(m_id);

    	int nbPackets = (nbLEDsRest && k == nbOSCMessages-1) ?  nbLEDsRest : nbLEDsUpdate;
    	for (int i=0;i<nbPackets;i++)
    	{
			offset = offsetPackets+i;
	
			m.addFloatArg(m_listPackets[offset]->m_volume);
			m.addFloatArg(m_listPackets[offset]->m_pitch);

			m.addFloatArg(m_listPackets[offset]->m_color[0]);
			m.addFloatArg(m_listPackets[offset]->m_color[1]);
			m.addFloatArg(m_listPackets[offset]->m_color[2]);
		}
    	m_oscSender.sendMessage(m);

		offsetPackets += nbLEDsUpdate;
	}

	// END SENDING PACKETS
    ofxOscMessage m_end;
    m_end.setAddress( OSC_ADDRESS_SEND_PACKETS_END );
    m_end.addStringArg(m_id);
    m_oscSender.sendMessage(m_end);
}

//--------------------------------------------------------------
void Device::onReceivePacketBegin()
{
    m_indexPacketReceived = 0;
	m_isUpdatingPacket = true;
}


//--------------------------------------------------------------
void Device::onReceivePacket(DevicePacket* pPacket)
{
    if (m_isUpdatingPacket && m_indexPacketReceived<m_listPackets.size())
    {
        m_listPackets[m_indexPacketReceived]->copy(pPacket);
        m_indexPacketReceived++;
    }
}

//--------------------------------------------------------------
void Device::onReceivePacketEnd()
{
	m_isUpdatingPacket = false;
}

//--------------------------------------------------------------
void Device::enableStandbyMode(bool is)
{
    m_isEnableStandbyMode = is;
    if (is){
        m_stateStandbyDuration=0.0f;
    }
}

//--------------------------------------------------------------
void Device::checkForActivity(float dt)
{
	if (mp_soundInput == 0) return;


    if (m_isEnableStandbyMode)
    {
        m_stateStandbyDuration += dt;
        
        if (m_stateStandby == EStandby_active)
        {
		   if (mp_soundInput->getVolHistoryMeanFiltered() <= m_volHistoryTh)
		   {
			   m_stateStandby = EStandby_pre_standby;
			   m_stateStandbyDuration=0.0f;
		   }
		   else if (mp_soundInput->getVolHistoryMeanFiltered() >= m_standupTh)
		   {
			   if (m_isEnableStandup)
				   m_stateStandby = EStandby_standup;
		   }
        }
        else
        if (m_stateStandby == EStandby_standup)
        {
		   if (mp_soundInput->getVolHistoryMeanFiltered() < m_standupTh)
		   {
			   m_stateStandby = EStandby_active;
		   }
		}
        else
        if (m_stateStandby == EStandby_pre_standby)
        {
            //printf("%.2f - ",m_stateStandbyDuration);
            if (mp_soundInput && (mp_soundInput->getVolHistoryMeanFiltered() > m_volHistoryTh))
            {
                m_stateStandby = EStandby_active;
                m_stateStandbyDuration=0.0f;
            }
			else
			{
	            if (m_stateStandbyDuration>=m_timeStandby)
    	        {
        	        m_stateStandby = EStandby_standby;
            	    m_stateStandbyDuration=0.0f;
					resetStandBy();
                // ofLog() << "> Entering standby\n";
            	}
        	}
		}
        else
        if (m_stateStandby == EStandby_standby)
        {
			Sample::lock();
			bool isSampleFinished = false;
			if (mp_sampleStandBy && !mp_sampleStandBy->getIsPlaying())
				isSampleFinished = true;
		
            if (isSampleFinished || mp_soundInput->getVolHistoryMeanFiltered() > m_volHistoryTh)
            {
                m_stateStandby = EStandby_pre_standby;
                m_stateStandbyDuration=0.0f;
				
				mp_soundInput->setSample(0);

            }
			Sample::unlock();
        }
    }
    else
	{
		if (m_stateStandby == EStandby_active)
		{
			if (mp_soundInput->getVolHistoryMeanFiltered() >= this->m_standupTh)
			{
				if (m_isEnableStandup)
		        	m_stateStandby = EStandby_standup;
			}
		}
		else if (m_stateStandby == EStandby_standup)
		{
			if (mp_soundInput->getVolHistoryMeanFiltered() < this->m_standupTh)
			{
		        m_stateStandby = EStandby_active;
			}
		}

	
	}
}

//--------------------------------------------------------------
void Device::updateBPM(float dt)
{
	if (m_bpmEnable)
	{
		m_bpmTime += dt;
		if (m_bpmTime>=m_bpmPeriod)
		{
			m_bpmTime = 0.0f;
			m_bpmSoundValue = (m_bpmSoundValue == 1.0f) ? 0.0f : 1.0f;
			if (mp_soundInput)
				mp_soundInput->setInputVolumeModulate( m_bpmSoundValue );
				//ofLog() << "HERE";
		}
	}
	else
	{
		if (mp_soundInput)
			mp_soundInput->setInputVolumeModulate( 1.0f );
	}

}

//--------------------------------------------------------------
void Device::update(float dt)
{

    checkForActivity(dt);
	updateBPM(dt);
 
    if (mp_soundInput)
    {
        // Update sound data
        mp_soundInput->update();

		// Sample them and writes into packet
        sampleSoundInput();

        // Send packets / values to network
        sendPacketsOSC();
    }

}


//--------------------------------------------------------------
void Device::loadXMLSurface(ofxXmlSettings& settings)
{
	string surfaceId = settings.getAttribute("device:surface", "id", "main");
	float xNorm = settings.getValue("device:surface:xNorm", 0.5f);
	float yNorm = settings.getValue("device:surface:yNorm", 0.5f);

    OFAPPLOG->println(" - surface="+surfaceId+" (xNorm="+ofToString(xNorm)+",yNorm="+ofToString(yNorm)+")");
	setPointSurface(xNorm, yNorm);
}


//--------------------------------------------------------------
void Device::loadXMLSoundInput(ofxXmlSettings& settings)
{
  int	 	useRawVol 			= settings.getValue("device:soundInput:useRawVol",0.05f);
  int		mute				= settings.getValue("device:soundInput:mute",0);
  float 	volMax 				= settings.getValue("device:soundInput:volMax",0.05f);
  float 	volMaxMin 			= settings.getValue("device:soundInput:volMaxMin",0.0f);
  float 	volMaxMax 			= settings.getValue("device:soundInput:volMaxMax",0.05f);
  int 		volHistoryNb 		= settings.getValue("device:soundInput:volHistoryNb", 400);
  float 	volHistoryTh		= settings.getValue("device:soundInput:volHistoryTh",0.1f);
  float 	pitchMin			= settings.getValue("device:soundInput:pitchMin",30.0f);
  float 	pitchMax			= settings.getValue("device:soundInput:pitchMax",100.0f);
  int		enableStandby		= settings.getValue("device:enableStandby",1);
  float 	timeStandby			= settings.getValue("device:timeStandby",10.0f);
  float 	sampleVolStandby	= settings.getValue("device:sampleVolStandby",0.35f);
  int		enableStandup		= settings.getValue("device:enableStandup",1);
 


   setSoundInputUseRawVolume(useRawVol==1 ? true : false );
   setSoundInputMute(mute==1 ? true : false );
   setSoundInputVolumeMax( volMax );
   setSoundInputVolumeMaxMin( volMaxMin );
   setSoundInputVolumeMaxMax( volMaxMax );
   setSoundInputVolHistorySize( volHistoryNb );
   setSoundInputVolHistoryTh( volHistoryTh );
   setSoundInputPitchMin(pitchMin);
   setSoundInputPitchMax(pitchMax);
   setEnableStandbyMode( enableStandby == 1 ? true : false );
   setTimeStandby( timeStandby );
   setSampleVolumeStandby( sampleVolStandby );
   // setEnableStandup( enableStandup );
   // setStandupVol( volStandup );


   OFAPPLOG->println(" - useRawVol="+ofToString(useRawVol));
   OFAPPLOG->println(" - mute="+ofToString(mute));
   OFAPPLOG->println(" - volMax="+ofToString(volMax));
   OFAPPLOG->println(" - volMaxMin="+ofToString(volMaxMax));
   OFAPPLOG->println(" - volMaxMax="+ofToString(volMaxMax));
   OFAPPLOG->println(" - volHistoryNb="+ofToString(volHistoryNb));
   OFAPPLOG->println(" - volHistoryTh="+ofToString(volHistoryTh));
   OFAPPLOG->println(" - pitchMin="+ofToString(pitchMin));
   OFAPPLOG->println(" - pitchMax="+ofToString(pitchMax));
   OFAPPLOG->println(" - enableStandby="+ofToString(timeStandby));
   OFAPPLOG->println(" - timeStandby="+ofToString(enableStandby));
   OFAPPLOG->println(" - sampleVolStandby="+ofToString(sampleVolStandby));
   OFAPPLOG->println(" - enableStandup="+ofToString(enableStandup));
//   OFAPPLOG->println(" - volStandup="+ofToString(volStandup));
}




//--------------------------------------------------------------
void Device::loadXMLSoundOutput(ofxXmlSettings& settings)
{
   settings.pushTag("device");
   settings.pushTag("soundOutput");
   int nbSpeakers = settings.getNumTags("speaker");
   clearListSpeakers();
   for (int i=0;i<nbSpeakers;i++){
	   int speakerId = settings.getValue("speaker",0,i);
	   addSpeakerId( speakerId );
	   OFAPPLOG->println(" - speaker "+ofToString(speakerId)+" added");
   }
   settings.popTag();
   settings.popTag();
}

//--------------------------------------------------------------
void Device::loadXMLColor(ofxXmlSettings& settings)
{
  float	colorManualHsb[2];
  colorManualHsb[0]			= settings.getValue("device:color:colorMode_manual_hsb:hue", 			127.0f);
  colorManualHsb[1]			= settings.getValue("device:color:colorMode_manual_hsb:saturation", 	127.0f);
  setColorHueSaturation(colorManualHsb[0],colorManualHsb[1]);
  OFAPPLOG->println(" - colorManualHsb, (hue="+ofToString(colorManualHsb[0])+", saturation="+ofToString(colorManualHsb[1])+")");

  int enable = settings.getValue("device:color:enable", 			0);
  enableColor(enable>0 ? true : false);
}

//--------------------------------------------------------------
void Device::loadXMLPackets(ofxXmlSettings& settings)
{
   int invert 	= settings.getValue("device:packets:invert", 			0);
   int reverse 	= settings.getValue("device:packets:reverseDir", 		0);
   invertPacketsVolume(invert>0?true:false);
   reversePacketsDir(reverse>0?true:false);
}

//--------------------------------------------------------------
void Device::loadXMLPing(ofxXmlSettings& settings)
{
	m_volHistoryPingTh = settings.getValue("device:ping:value", 			0.5f);
}

//--------------------------------------------------------------
void Device::loadXMLGenerative(ofxXmlSettings& settings)
{
	int generative = settings.getValue("device:enableGenerative", 			1);
	setGenerative(generative>0?true:false);
}

//--------------------------------------------------------------
void Device::loadXMLBPM(ofxXmlSettings& settings)
{
  bool		bpmEnable		= settings.getValue("device:bpm:enable",0) > 0 ? true : false;
  int	 	bpm 			= settings.getValue("device:bpm:value",90);
  if (bpm<=0) bpm = 100;

  setBPMEnable(bpmEnable);
  setBPM(bpm);
}

//--------------------------------------------------------------
void Device::loadXMLData(ofxXmlSettings& settings)
{
	loadXMLSoundInput(settings);
	loadXMLSurface(settings);
	loadXMLColor(settings);
	loadXMLSoundOutput(settings);
	loadXMLPackets(settings);
	loadXMLPing(settings);
	loadXMLGenerative(settings);
	loadXMLBPM(settings);
}

//--------------------------------------------------------------
void Device::loadXML(string dir)
{
	OFAPPLOG->begin("Device::loadXML()");

    ofxXmlSettings settings;
    string pathFile = getPathXML(dir);
    if ( settings.loadFile(pathFile) )
    {
		OFAPPLOG->println("Device, loaded "+pathFile);
		loadXMLData(settings);
    }
    else{
        OFAPPLOG->println("Device, error loading "+pathFile);
    }


	OFAPPLOG->end();
}


//--------------------------------------------------------------
void Device::saveXML(string dir)
{
    ofxXmlSettings settings;
    settings.addTag("device");
    settings.setAttribute("device", "id", m_id,0);
    settings.pushTag("device");

        settings.addTag("soundInput");
        settings.pushTag("soundInput");

            settings.addValue("useRawVol", 			getSoundInputUseRawVolume());
            settings.addValue("mute", 				getSoundInputMute());
            settings.addValue("volMax", 			getSoundInputVolumeMax());
            settings.addValue("volMaxMin", 			getSoundInputVolumeMaxMin());
            settings.addValue("volMaxMax", 			getSoundInputVolumeMaxMax());
            settings.addValue("volHistoryNb", 		getSoundInputVolHistorySize());
            settings.addValue("volHistoryTh", 		getSoundInputVolHistoryTh());
            settings.addValue("pitchMin", 			getSoundInputPitchMin());
            settings.addValue("pitchMax", 			getSoundInputPitchMax());


		settings.popTag();

        settings.addTag("soundOutput");
        settings.pushTag("soundOutput");
		for (int i=0; i<m_listSpeakerIds.size(); i++)
            settings.addValue("speaker", m_listSpeakerIds[i]);
        settings.popTag();

        settings.addTag("color");
        settings.pushTag("color");
			settings.addValue("enable", 			m_isEnableColor ? 1:0);
			settings.addTag("colorMode_manual_hsb");
			settings.pushTag("colorMode_manual_hsb");
				settings.addValue("hue", 			m_colorHsv[0]);
				settings.addValue("saturation", 	m_colorHsv[1]);
			settings.popTag();
        settings.popTag();


        settings.addTag("packets");
        settings.pushTag("packets");
		settings.addValue("invert", m_isInvertPacketsVolume ? 1 : 0);
        settings.popTag();

        settings.addTag("ping");
        settings.pushTag("ping");
		settings.addValue("value", m_volHistoryPingTh);
        settings.popTag();


    settings.addValue("enableGenerative", 	isGenerative() ? 1 : 0);

	settings.addTag("bpm");
	settings.pushTag("bpm");
	settings.addValue("enable", m_bpmEnable ? 1 : 0);
	settings.addValue("value", m_bpm);
	settings.popTag();



    settings.addValue("enableStandby", 		getEnableStandbyMode() ? 1 : 0);
    settings.addValue("timeStandby", 		m_timeStandby);
    settings.addValue("sampleVolStandby", 	getSampleVolStandby());
    settings.addValue("enableStandby", 		getEnableStandup() ? 1 : 0);
/*
    settings.addValue("enableStandup", 		getEnableStandup() ? 1 : 0);
    settings.addValue("volStandup", 		getStandupVol());
*/
    settings.addTag("surface");
    settings.setAttribute("surface", "id", "main", 0);
    settings.pushTag("surface");
        settings.addValue("xNorm", 			m_pointSurface.x);
        settings.addValue("yNorm", 			m_pointSurface.y);
    settings.popTag();
   
 
    settings.popTag();
    settings.saveFile( getPathXML(dir) );
    
}

//--------------------------------------------------------------
string Device::getPathXML(string dir)
{
    string path = dir + m_id + ".xml";
    return path;
}


//--------------------------------------------------------------
void Device::clearListSpeakers()
{
	m_listSpeakerIds.clear();
}

//--------------------------------------------------------------
void Device::addSpeakerId(int id)
{
	m_listSpeakerIds.push_back(id);
}


//--------------------------------------------------------------
DeviceManager::DeviceManager()
{
    mp_deviceCurrent = 0;
}

//--------------------------------------------------------------
DeviceManager::~DeviceManager()
{
    deleteDevices();
}

//--------------------------------------------------------------
void DeviceManager::deleteDevices()
{
	for (itDevices = m_listDevices.begin(); itDevices != m_listDevices.end();)
    {
        delete *itDevices;
        itDevices = m_listDevices.erase(itDevices);
    }
}

//--------------------------------------------------------------
void DeviceManager::addDevice(Device* pDevice)
{
    m_listDevices.push_back( pDevice );
}


//--------------------------------------------------------------
Device* DeviceManager::getDeviceById(string id)
{
    Device* pDevice=0;
	for (itDevices = m_listDevices.begin(); itDevices != m_listDevices.end(); ++itDevices){
        if ( (*itDevices)->m_id == id ){
            pDevice = *itDevices;
            break;
        }
    }
    return pDevice;
}

//--------------------------------------------------------------
void DeviceManager::getDevicesListId(vector<string>& listIds)
{
	for (itDevices = m_listDevices.begin(); itDevices != m_listDevices.end(); ++itDevices){
        listIds.push_back( (*itDevices)->m_id );
    }
}

//--------------------------------------------------------------
void DeviceManager::setDeviceCurrent(Device* pDevice)
{
    mp_deviceCurrent = pDevice;
}

//--------------------------------------------------------------
Device* DeviceManager::setDeviceCurrent(string deviceId)
{
    Device* pDevice = getDeviceById(deviceId);
    setDeviceCurrent( pDevice );
    return pDevice;
}

//--------------------------------------------------------------
void DeviceManager::saveDevicesXML(string dir)
{
	for (itDevices = m_listDevices.begin(); itDevices != m_listDevices.end(); ++itDevices)
        (*itDevices)->saveXML(dir);
}

//--------------------------------------------------------------
void DeviceManager::turnoffDevices(bool bReboot)
{
	for (itDevices = m_listDevices.begin(); itDevices != m_listDevices.end(); ++itDevices)
        (*itDevices)->turnoff(bReboot);
}

//--------------------------------------------------------------
int DeviceManager::indexOf(Device* pDevice)
{
	int index=0;
	for (itDevices = m_listDevices.begin(); itDevices != m_listDevices.end(); ++itDevices)
	{
		if (*itDevices == pDevice)
			return index;
		index++;
	}
	return -1;
}







