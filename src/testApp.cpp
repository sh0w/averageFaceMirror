#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
    ofBackground(0);
    
    WIDTH = 1280;
    HEIGHT = 720;
    
    usePS3 = false;
    debugMode = false;
    rotateScreen = true;
    
    if(usePS3) {
	    setupPS3Eye();
    } else {
		cam.initGrabber(WIDTH, HEIGHT);
    }
    tracker.setup();
    tracker.setHaarMinSize(HEIGHT / 4);
    classifier.classify(tracker);
    
    numFaces = 0;
    
    allFaces.allocate(WIDTH, HEIGHT, GL_RGB);
    currentFace.allocate(WIDTH, HEIGHT, GL_RGB);
    currentFaceWithMouthAligned.allocate(WIDTH, HEIGHT, GL_RGB);
    
    /*
    allFaces.begin();
    ofSetColor(0);
    ofRect(0,0,ofGetWidth(), ofGetHeight());
    allFaces.end();*/
    
    a = new unsigned long[WIDTH * HEIGHT * 3];
    totalEyeMouthRatio = 0;
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
    
    vidGrabber.setAutogain(false);
    vidGrabber.setAutoWhiteBalance(true);
    vidGrabber.setBrightness(50);
    vidGrabber.setSharpness(255);
    vidGrabber.setContrast(50);
    vidGrabber.setRedBalance(120);
    vidGrabber.setBlueBalance(133);
    vidGrabber.setHue(100);
    
    tracker.setRescale(1);
}

void testApp::update() {
    
    if(numFaces > 0) {
	    averageEyeMouthRatio = totalEyeMouthRatio/numFaces;
    }
    
    if(usePS3) {
        vidGrabber.update();
        
        if (vidGrabber.isFrameNew()) {
            videoTexture.loadData(vidGrabber.getPixelsRef());
            ofPixels pix = vidGrabber.getPixelsRef();
            pix.setImageType(OF_IMAGE_GRAYSCALE);
            tracker.update(toCv(pix));
            classifier.classify(tracker);
        }
        
    } else {
        // use internal webcam:
        cam.update();
        
        if(cam.isFrameNew()) {
            tracker.update(toCv(cam));
            classifier.classify(tracker);
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
            ofTranslate(WIDTH* 0.8, -HEIGHT/9);
            ofRotate(90);
            ofScale(0.8,0.8);
        }
        
        ofTranslate(WIDTH/3, HEIGHT/3);
        ofTranslate(-leftEyePos);
        
        ofPushMatrix();
        
        ofTranslate(leftEyePos);
        
        // calculate angle between left and right eye
        ofRotate(-asin((rightEyePos.y - leftEyePos.y)/
                       ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y)) * 180/PI);
        
        
        eyeDist = ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y);
        mouthDist = ofDist((leftEyePos.x + rightEyePos.x)/2, (leftEyePos.y + rightEyePos.y)/2, mouthPos.x, mouthPos.y);
        
        // scale everything down to make left + right eye always at same distance
        ofScale(270 / ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y),
                270 / ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y));
        
        ofTranslate(-leftEyePos);
        
        if(usePS3) {
            videoTexture.draw(0, 0);
        } else {
            cam.draw(0, 0);
        }
        
        ofPopMatrix();
        
        currentFace.end();
        
        currentFaceWithMouthAligned.begin();
        
        ofTranslate(WIDTH*0.67, HEIGHT/2);
        
        //TODO: if(rotateScreen) ...
        
        // scale face vertically for mouth alignment:
        ofScale(averageEyeMouthRatio/(mouthDist/eyeDist),1);
        
        currentFace.draw(-1.15*WIDTH*0.565,-HEIGHT/2);
        
        if(ofGetElapsedTimeMillis() > timestampLastFaceSaved + 1000) {
            ofImage temp;
            temp.grabScreen(0, 0, WIDTH, HEIGHT);
            temp.saveImage( ofToString(numFaces) + ".jpg");
            addCurrentFaceToAllFaces();
        }
        
        currentFaceWithMouthAligned.end();
    }
    
    
    if(debugMode) {
        
        ofPushMatrix();
        ofScale(0.5,0.5);
        
        currentFace.draw(0, HEIGHT, WIDTH, HEIGHT);
        allFaces.draw(WIDTH, HEIGHT, WIDTH, HEIGHT);
        currentFaceWithMouthAligned.draw(WIDTH, 0, WIDTH, HEIGHT);
        
        
        if(usePS3) {
            videoTexture.draw(0,0);
        } else {
            cam.draw(0,0);
        }
        
        ofSetLineWidth(2);
        tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).draw();
        tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).draw();
        tracker.getImageFeature(ofxFaceTracker::NOSE_BASE).draw();
        tracker.getImageFeature(ofxFaceTracker::INNER_MOUTH).draw();
        
        
        ofPopMatrix();
        
        ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
        ofDrawBitmapString(ofToString(numFaces), 10, 35);
        ofDrawBitmapString(ofToString((int)ofGetElapsedTimeMillis()/1000), 10, 50);
        ofDrawBitmapString(ofToString(averageEyeMouthRatio), 10, 65);
        
    } else {
        allFaces.draw(0, 0, ofGetWidth(), ofGetHeight());
        ofDrawBitmapString(ofToString(numFaces), 30, 30);
    }
    
    if(classifier.getProbability(5) > 0.99 && !justDidExpression) {
        toggleDebugMode();
        justDidExpression = true;
    }
    if(classifier.getProbability(5) < 0.9 && justDidExpression) {
        justDidExpression = false;
    }
}

void testApp::addCurrentFaceToAllFaces() {
    totalEyeMouthRatio += mouthDist/eyeDist;
    
    timestampLastFaceSaved = ofGetElapsedTimeMillis();
    
    numFaces++;
    
    ofPixels allPixels;
    ofPixels currentPixels;
    
    currentFaceWithMouthAligned.readToPixels(currentPixels);
    allFaces.readToPixels(allPixels);
    
    
    for (int i = numFaces-1; i < numFaces; i++) {
        ofImage temp;
        temp.loadImage(ofToString(i) + ".jpg");
        
        unsigned char* tp = temp.getPixels();
        
        for( int i = 0; i < temp.getWidth() * temp.getHeight() * 3; i++) {
            a[i] += tp[i];
        }
    }
    
    for( int i = 0; i < currentPixels.size(); i++) {
        allPixels[i] = a[i] / numFaces;
    }
    
    ofTexture ot;
    ot.allocate(allPixels);
    ot.loadData(allPixels);
    allFaces.begin();
    ot.draw(0,0);
    allFaces.end();
    
    ofPopMatrix();
    
    
}

void testApp::toggleDebugMode() {
    debugMode = ! debugMode;
}

void testApp::keyPressed(int key) {
    if(key == 'x') {
        tracker.reset();
        classifier.reset();
    }
    if(key == ' ') {
        toggleDebugMode();
    }
    if(key == 'f') {
        ofToggleFullscreen();
    }
    if(key == 'r') {
        rotateScreen = !rotateScreen;
    }
    
    
    if(key == 'e') {
        classifier.addExpression();
    }
    if(key == 'a') {
        classifier.addSample(tracker);
    }
    if(key == 's') {
        classifier.save("expressions");
    }
    if(key == 'l') {
        classifier.load("expressions");
    }
    
    
    if(key == 's') {
        allFaces.begin();
        ofImage temp;
        temp.grabScreen(0, 0, WIDTH, HEIGHT);
        temp.saveImage( "results/"+ ofToString(numFaces) + "faces_" + ofToString((int)ofGetElapsedTimef()/60) + "_min.jpg");
        allFaces.end();
    }
}