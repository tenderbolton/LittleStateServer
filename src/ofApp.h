#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofxNetwork.h"

#define MAX_BUFF_MESSAGES 25

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
        void exit();
        
        void sendOSCData();
        void sendToArduino();
        void readFromArduino();
        
        void loadFromXML();
    
    private:
        void setState(int currentState);
    
    
        ofSerial	serial;
        
        ofxXmlSettings XML;
        ofTrueTypeFont* textRenderer;
        ofTrueTypeFont* textRendererSmall;
        ofImage* logo;
        ofxOscSender oscSender;
    
        int OSCPort;
        string OSCIP;
    
        string serialPort;

        int baud;
        bool loadOK;
    
        string messageFromArduino;
        string arduinoBuff;
        char readByte;
    
        string errorMessage;
        
        vector<string> messageBuffer;
    
        int currentState;
        int prevState;
    
        //sound players
    
    
        string on_SoundFile;
        string off_SoundFile;
        float onVolume;
        float offVolume;
        ofSoundPlayer  on_state;
        ofSoundPlayer  off_state;



};
