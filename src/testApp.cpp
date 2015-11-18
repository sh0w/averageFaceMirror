#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
    WIDTH = 1280;
    HEIGHT = 720;
    
    usePS3 = false;
    debugMode = false;
    rotateScreen = false;
    
    if(usePS3) {
	    setupPS3Eye();
    } else {
		cam.initGrabber(WIDTH, HEIGHT);
    }
	tracker.setup();
    
    numFaces = 0;
    
    allFaces.allocate(WIDTH, HEIGHT, GL_RGB);
    currentFace.allocate(WIDTH, HEIGHT, GL_RGB);
    
    allFaces.begin();
    ofSetColor(255,255,255);
    ofRect(20,20,ofGetWidth(), ofGetHeight());
    allFaces.end();
    
    a = new unsigned long[WIDTH * HEIGHT * 3];
}


void testApp::setupPS3Eye() {
    camWidth = WIDTH;
    camHeight = HEIGHT;
    camFrameRate = 120;
    
    //we can now get back a list of devices.
    std::vector<ofVideoDevice> devices = vidGrabber.listDevices();
    
    for(std::size_t i = 0; i < devices.size(); ++i)
    {
        std::stringstream ss;
        
        ss << devices[i].id << ": " << devices[i].deviceName;
        
        if(!devices[i].bAvailable)
        {
            ss << " - unavailable ";
        }
        
        ofLogNotice("ofApp::setup") << ss.str();
    }
    
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(camFrameRate);
    vidGrabber.initGrabber(camWidth, camHeight);
    
    vidGrabber.setAutogain(true);
    vidGrabber.setAutoWhiteBalance(true);
    
    tracker.setRescale(1);
}

void testApp::update() {
    
    if(usePS3) {
        vidGrabber.update();
        
        if (vidGrabber.isFrameNew())
        {
            videoTexture.loadData(vidGrabber.getPixelsRef());
            ofPixels pix = vidGrabber.getPixelsRef();
            pix.setImageType(OF_IMAGE_GRAYSCALE);
        	tracker.update(toCv(pix));
        }
        
    } else {
        // use internal webcam:
        cam.update();
        
        if(cam.isFrameNew()) {
            tracker.update(toCv(cam));
        }
    }
}

void testApp::draw() {
    ofPushMatrix();
    if(tracker.getFound()) {
        
        currentFace.begin();
        
        leftEyePos = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).getCentroid2D();
        rightEyePos = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).getCentroid2D();
        nosePos = tracker.getImageFeature(ofxFaceTracker::NOSE_BASE).getCentroid2D();
        mouthPos = tracker.getImageFeature(ofxFaceTracker::INNER_MOUTH).getCentroid2D();
        
        
        if(rotateScreen) {
            ofTranslate(ofGetWidth() * 0.8, -ofGetHeight()/9);
            ofRotate(90);
            ofScale(0.8,0.8);
        }
        
        ofTranslate(ofGetWidth()/3, ofGetHeight()/3);
        ofTranslate(-leftEyePos);
        
        ofPushMatrix();
        
        ofTranslate(leftEyePos);
        
        // calculate angle between left and right eye
        ofRotate(-asin((rightEyePos.y - leftEyePos.y)/
                       ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y)) * 180/PI);
        
        
        ofScale(270 / ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y),
                270 / ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y));
        
        ofTranslate(-leftEyePos);
        
        if(usePS3) {
            videoTexture.draw(0,0);
        } else {
            cam.draw(0, 0);
        }
        ofPopMatrix();
        
        if(ofGetElapsedTimeMillis() > timestampLastFaceSaved + 1000) {
            ofImage temp;
            temp.grabScreen(0, 0, WIDTH, HEIGHT);
            temp.saveImage( ofToString(numFaces % 1000) + ".jpg");
            addCurrentFaceToAllFaces();
        }
        
        currentFace.end();
    }
    
    
    if(debugMode) {
        
        ofSetLineWidth(2);
        
        currentFace.draw(ofGetWidth()/2,0,ofGetWidth()/2, ofGetHeight()/2);
        allFaces.draw(ofGetWidth()/2, ofGetHeight()/2, ofGetWidth()/2, ofGetHeight()/2);
        
        
        tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).draw();
        tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).draw();
        tracker.getImageFeature(ofxFaceTracker::NOSE_BASE).draw();
        tracker.getImageFeature(ofxFaceTracker::INNER_MOUTH).draw();
        
        if(usePS3) {
            videoTexture.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
        } else {
            cam.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
        }
        tracker.draw();
        
        ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
        ofDrawBitmapString(ofToString(numFaces), 10, 35);
        ofDrawBitmapString(ofToString((int)ofGetElapsedTimeMillis()/1000), 10, 50);
    } else {
        allFaces.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
}

void testApp::addCurrentFaceToAllFaces() {
    //ofReadPixels(allFaces.getTexture(), myPixels);
    
    timestampLastFaceSaved = ofGetElapsedTimeMillis();
    
    numFaces++;
    
    ofPixels allPixels;
    ofPixels currentPixels;
    
    currentFace.readToPixels(currentPixels);
    allFaces.readToPixels(allPixels);
    
    
    for (int i = numFaces-1; i < numFaces; i++) {
        ofImage temp;
        temp.loadImage(ofToString(i%1000) + ".jpg");
        
        unsigned char* tp = temp.getPixels();
        
        for( int i = 0; i < temp.getWidth() * temp.getHeight() * 3; i++) {
            //int c = (allPixels[i] * numFaces + currentPixels[i]) / (numFaces+1);
            a[i] += tp[i];
        }
    }
    
    for( int i = 0; i < currentPixels.size(); i++) {
        //int c = (allPixels[i] * numFaces + currentPixels[i]) / (numFaces+1);
        allPixels[i] = a[i] / numFaces;
    }
    
    ofTexture ot;
    ot.allocate(allPixels);
    ot.loadData(allPixels);
    allFaces.begin();
    ot.draw(0,0);
    allFaces.end();
    
    
//    delete [] a;
//    a = NULL;
    
    ofPopMatrix();
}

void testApp::keyPressed(int key) {
    if(key == 'x') {
        tracker.reset();
    }
    if(key == ' ') {
        debugMode = ! debugMode;
    }
    if(key == 'f') {
        ofToggleFullscreen();
    }
    if(key == 'r') {
        rotateScreen = !rotateScreen;
    }
}