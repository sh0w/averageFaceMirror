#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"
#include "ofxPS3EyeGrabber.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
    
    bool debugMode;
    void toggleDebugMode();
    
    bool rotateScreen;
    
    
    // note, this is probably not the best way to do this
    // because ofxPS3EyeGrabber actually extends ofBaseVideoGrabber
    bool usePS3;
    
    /*****  for PS3 cam: *****/
    ofxPS3EyeGrabber vidGrabber;
    ofTexture videoTexture;
    
    int camWidth;
    int camHeight;
    int camFrameRate;
    
    void setupPS3Eye();
    void updatePS3Eye();
    
    /*****  for internal webcam: *****/
	ofVideoGrabber cam;
    
	ofxFaceTracker tracker;
    
    ofPoint leftEyePos;
    ofPoint rightEyePos;
    ofPoint mouthPos;
    ofPoint nosePos;
    
    ofFbo allFaces;
    ofFbo currentFace;
    ofFbo currentFaceWithMouthAligned;
    
    void addCurrentFaceToAllFaces();
    int numFaces;
    
    int WIDTH;
    int HEIGHT;
    
    
    unsigned long* a;
    
    unsigned long long timestampLastFaceSaved;
    
    bool justDidExpression;
    
    float totalEyeMouthRatio;
    float scaleForEyeAlignment;
    float eyeDist;
    float mouthDist;
    
    ofTrueTypeFont font;
    
    float averageEyeMouthRatio;
};
