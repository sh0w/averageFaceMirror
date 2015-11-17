#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	ofVideoGrabber cam;
	ofxFaceTracker tracker;
    
    ofPoint leftEyePos;
    ofPoint rightEyePos;
    ofPoint mouthPos;
    ofPoint nosePos;
    
    ofFbo allFaces;
    ofFbo currentFace;
    
    void addCurrentFaceToAllFaces();
    int numFaces;
    
    
    unsigned long* a;
};
