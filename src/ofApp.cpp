#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    loadOK = false;
    OSCPort = 4000;
    serialPort = "/dev/cu.usbmodemfa131";
    baud = 9600;
    messageFromArduino = "";
    arduinoBuff = "";
    errorMessage = "";
    on_SoundFile = "";
    off_SoundFile = "";
    onVolume = 0.0f;
    offVolume = 0.0f;
    
    this->textRenderer = new ofTrueTypeFont();
    this->textRendererSmall = new ofTrueTypeFont();
    this->textRenderer->loadFont("verdana.ttf", 12);
    this->textRendererSmall->loadFont("verdana.ttf", 8);
    this->logo = new ofImage();
    logo->loadImage("shamanLogoBox.png");
    
    //initializing app state
    currentState = 0;
    prevState = 0;
    
    //listing devices to console
    serial.listDevices();
    try{
        loadFromXML();
    }
    catch(std::exception const& e){
        errorMessage = e.what();
    }
    
    if(loadOK){
        oscSender.setup(OSCIP,OSCPort);
        serial.setup(serialPort, baud); //open the first device
        on_state.loadSound(on_SoundFile);
        on_state.setVolume(onVolume);
        on_state.setLoop(false);
        on_state.setMultiPlay(true);
        off_state.loadSound(off_SoundFile);
        off_state.setVolume(offVolume);
        off_state.setLoop(true);
        
        //initializing in off state:
        setState(currentState);
    }
    
    ofSetFrameRate(30);

}

void ofApp::setState(int currentState){
    if(currentState==0){
        off_state.setPosition(0.0f);
        off_state.play();
        //on_state.setPosition(0.0f);
        //on_state.stop();
    }
    else{
        off_state.setPosition(0.0f);
        off_state.stop();
        on_state.setPosition(0.0f);
        on_state.play();

    }
}

void ofApp::loadFromXML(){
    
    loadOK = true;
    
    if( XML.loadFile("appSettings.xml") ){
        
        int numMainSettingsTag = XML.getNumTags("MAIN_SETTINGS");
        
        if(numMainSettingsTag==1){
            XML.pushTag("MAIN_SETTINGS");
            
            int numInputTag = XML.getNumTags("OSC_OUTPUT");
            if(numInputTag>0){
                OSCIP = XML.getAttribute("OSC_OUTPUT","ip","127.0.0.1");
                OSCPort = ofToInt(XML.getAttribute("OSC_OUTPUT","port","30000"));
            }
            else{
                cout << "No osc output defined!" << endl;
                loadOK = false;
                throw std::runtime_error(std::string("No osc output defined!"));
            }
            
            int numOutputTag = XML.getNumTags("SERIAL_CONF");
            if(numOutputTag>0){
                baud = ofToInt(XML.getAttribute("SERIAL_CONF","bauds","9600"));
                serialPort = XML.getAttribute("SERIAL_CONF","name","default");
            }
            else{
                cout << "No Serial definition!" << endl;
                loadOK = false;
                throw std::runtime_error(std::string("No Serial definition!"));
            }
            
            
            int numSoundTag = XML.getNumTags("SOUND_CONF");
            if(numSoundTag>0){
                on_SoundFile = XML.getAttribute("SOUND_CONF","onSoundFile","on.mp3");
                onVolume = ofToFloat(XML.getAttribute("SOUND_CONF","onSoundVolume","0.75"));
                off_SoundFile = XML.getAttribute("SOUND_CONF","offSoundFile","on.mp3");
                offVolume = ofToFloat(XML.getAttribute("SOUND_CONF","offSoundVolume","0.75"));
            }
            else{
                cout << "No Sound definition!" << endl;
                loadOK = false;
                throw std::runtime_error(std::string("No Sound definition!"));
            }

            
            XML.popTag();
        }
        else{
            cout << "No main settings defined!" << endl;
            loadOK = false;
            throw std::runtime_error(std::string("No main settings defined!"));
            
        }
    }
    else{
        cout << "File could not be loaded!" << endl;
        loadOK = false;
        throw std::runtime_error(std::string("File could not be loaded!"));
    }
    
}



//--------------------------------------------------------------
void ofApp::update(){

    if(loadOK){
        readFromArduino();
        
        if(prevState!=currentState){
            //we are on la flanc, so we send the corresponding message
            setState(currentState);
        }

        sendOSCData();
        
        if(!serial.isInitialized()){
            serial.setup(serialPort,baud);
        }
        
        ofSoundUpdate();
    }
}


void ofApp::readFromArduino(){
    
    int newByte = serial.readByte();
    
    if(newByte!=OF_SERIAL_NO_DATA && newByte!=OF_SERIAL_ERROR){
        
        char newChar = (char) newByte;
        arduinoBuff += newChar;
        
        while(newByte!=OF_SERIAL_NO_DATA && newByte!=OF_SERIAL_ERROR){
            
            newByte = serial.readByte();
            
            if(newByte!=OF_SERIAL_NO_DATA && newByte!=OF_SERIAL_ERROR){
                newChar = (char) newByte;
                
                arduinoBuff += newChar;
                
                if(arduinoBuff.find("\n")!=string::npos){
                    messageFromArduino = arduinoBuff;
                    //verify state change
                    
                    if(messageFromArduino.compare("ON\r\n") == 0){
                        prevState = currentState;
                        currentState = 1; //turned on
                    }
                    
                    if(messageFromArduino.compare("OFF\r\n") == 0){
                        prevState = currentState;
                        currentState = 0; //turned off
                    }
                    
                    //end verification
                    arduinoBuff = "";
                }
            }
        }
    }
    
    if(newByte==OF_SERIAL_ERROR){
        serial.close();
        messageFromArduino = "";
        arduinoBuff = "";
    }
}

void ofApp::sendOSCData(){
    
    ofxOscMessage m;
    
    if(currentState == 0){
        m.setAddress("/littleServer/off");
    }
    else{
        m.setAddress("/littleServer/on");
    }
    
    m.addFloatArg(ofGetElapsedTimef());
    oscSender.sendMessage(m);

    
}


//--------------------------------------------------------------
void ofApp::draw(){
    float spacing = 100.0f;
    float initialYTitle = 50.0f;
    float initialY = 190.0f;
    float initialX = 50.0f;
    float lineSpacing = 20.0f;
    
    float bottomY = 360.0f;
    float bottomX = 50.0f;
    
    ofSetColor(0);
    
    this->textRenderer->drawString("/// Little State Server ///", initialX, initialYTitle);
    this->textRendererSmall->drawString("This application receives Serial messages (OFF and ON), controls the sound of the room,", initialX, initialYTitle + 2 * lineSpacing);
    this->textRendererSmall->drawString("and sends UDP messages to a destination on: " + ofToString(OSCIP) + ":" + ofToString(OSCPort), initialX, initialYTitle + 3 * lineSpacing );
    this->textRendererSmall->drawString("You can use keys 'i' and 'o' to switch on and off state respectively.", initialX, initialYTitle + 4 * lineSpacing );
    this->textRendererSmall->drawString("http://shaman.uy", bottomX, ofGetHeight() - 40);
    

    if (loadOK){
        
        if(serial.isInitialized()){
            ofSetColor(50,150,50);
            this->textRenderer->drawString("Connected to: " + this->serialPort, initialX, initialY);
        }
        else{
            ofSetColor(255,50,50);
            this->textRenderer->drawString("Not connected to: " + this->serialPort, initialX, initialY);
        }
        ofSetColor(0);
        this->textRenderer->drawString("Message from Arduino: " + this->messageFromArduino, initialX, lineSpacing + initialY);
        if(currentState == 1){
            this->textRenderer->drawString("Current State: ON", initialX, (3 * lineSpacing) + initialY);
        }
        else{
            this->textRenderer->drawString("Current State: OFF", initialX,  (3 * lineSpacing)  + initialY);
        }
        
    }
    else{
        this->textRenderer->drawString("Loading error. Please check XML and run again.", initialX, initialY);
        this->textRenderer->drawString(" :( ", initialX, initialY + 2 * lineSpacing);
        this->textRenderer->drawString(errorMessage, initialX, initialY + 4 * lineSpacing);
        
    }

    
    ofSetColor(255);
    this->logo->draw(ofGetWidth() - 80 - 50, ofGetHeight() - 40 - 118);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'i' || key == 'I'){
        prevState = 1;
        currentState = 1; //turned on
        setState(currentState);
    }
    
    if(key == 'o' || key == 'O'){
        prevState = 0;
        currentState = 0; //turned off
        setState(currentState);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::exit(){
    
}
