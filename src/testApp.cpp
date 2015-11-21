#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
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
    
    output.allocate(WIDTH, HEIGHT, GL_RGB);
    allFaces.allocate(WIDTH, HEIGHT, GL_RGB);
    currentFace.allocate(WIDTH, HEIGHT, GL_RGB);
    
    allFaces.begin();
    ofSetColor(255,255,255);
    ofRect(20,20,ofGetWidth(), ofGetHeight());
    allFaces.end();
    
    a = new unsigned long[WIDTH * HEIGHT * 3];
    
    
//    classifier.load("expressions");
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
    
    
    tracker.setRescale(1);
}

void testApp::update() {
    
    vidGrabber.setHue(100);
    if(usePS3) {
        vidGrabber.update();
        
        if (vidGrabber.isFrameNew())
        {
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
    output.begin();
    
    
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
        
        /*
        ofLogNotice("eye-dist");
        ofLogNotice(ofToString(ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y)));
        ofLogNotice("mouth-eye-dist");
        ofLogNotice(ofToString(ofDist((leftEyePos.x + rightEyePos.x)/2, (leftEyePos.y + rightEyePos.y)/2, mouthPos.x, mouthPos.y)));
        
        ofLogNotice("righteye x, y");
        ofLogNotice(ofToString(rightEyePos.x));
        ofLogNotice(ofToString(rightEyePos.y));
        ofLogNotice("lefteye x, y");
        ofLogNotice(ofToString(leftEyePos.x));
        ofLogNotice(ofToString(leftEyePos.y));
        
        ofLogNotice("between left and right");
        ofLogNotice(ofToString(leftEyePos.x + ));
        ofLogNotice(ofToString(leftEyePos.y));
        
        ofLogNotice("mouth x, y");
        ofLogNotice(ofToString(mouthPos.x));
        ofLogNotice(ofToString(mouthPos.y));*/
        
        //ofPushMatrix();
        
        // scale everything down to make left + right eye always at same distance
        ofScale(270 / ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y),
                270 / ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y));
        
        
        
        /*float scalefactorForMouth = 136 / ofDist(leftEyePos.x, leftEyePos.y, mouthPos.x,  mouthPos.y);
        
        ofLogNotice("scalefactorForMouth:");
        ofLogNotice(ofToString(scalefactorForMouth));
        
        ofScale(1, scalefactorForMouth);
         */
        
        //ofPopMatrix();
         
         
        ofTranslate(-leftEyePos.x, -leftEyePos.y);
         
         
        
        //ofTranslate(-leftEyePos);
        
        // scale everything down to mouth at same position too!
        //ofLogNotice("i d like to scale y:");
        //ofLogNotice(ofToString(323 / ofDist((leftEyePos.x + rightEyePos.x)/2, (leftEyePos.y + rightEyePos.y)/2, mouthPos.x, mouthPos.y)));
        //ofScale(1, 323 / ofDist((leftEyePos.x + rightEyePos.x)/2, (leftEyePos.y + rightEyePos.y)/2, mouthPos.x, mouthPos.y));
        
        
        // scale everything down to mouth at same position too!
        //ofScale(1, 323 / ofDist((leftEyePos.x + rightEyePos.x)/2, (leftEyePos.y + rightEyePos.y)/2, mouthPos.x, mouthPos.y));
        
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
        //tracker.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
        
        /*
        ofPushStyle();
        ofSetColor(0,255,0);
        ofEllipse(tracker.getImageFeature(ofxFaceTracker::INNER_MOUTH).getCentroid2D(), 10,10);
        ofSetColor(255,0,0);
        ofEllipse(tracker.getImageFeature(ofxFaceTracker::OUTER_MOUTH).getCentroid2D(), 10,10);
        ofPopStyle();
         */
        ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
        ofDrawBitmapString(ofToString(numFaces), 10, 35);
        ofDrawBitmapString(ofToString((int)ofGetElapsedTimeMillis()/1000), 10, 50);
    } else {
        allFaces.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    
    if(classifier.getProbability(5) > 0.99 && !justDidExpression) {
        toggleDebugMode();
        justDidExpression = true;
    }
    if(classifier.getProbability(5) < 0.9 && justDidExpression) {
        justDidExpression = false;
    }

    
    output.end();
    
    ofBackground(0);
    ofSetBackgroundColor(0);
    
    ofPushMatrix();
    
    ofTranslate(WIDTH/2, HEIGHT/2);
    //ofRotateX(-mouseX/5);
    //ofLogNotice(ofToString(-mouseY/5));
    ofRotateY(-mouseX/5);
    
    ofTranslate(-WIDTH/2, -HEIGHT/2);
    
    ofTranslate(100, 0, -100);
    
    output.draw(0,0);
    
    ofDrawBitmapString(ofToString(numFaces), 10, 35);
    ofPopMatrix();
    
    ofDrawBitmapString(ofToString(numFaces), 10, 35);
    //output.draw(0,0);
    
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
    
    
//    delete [] a;
//    a = NULL;
    
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
}