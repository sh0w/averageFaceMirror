#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
	cam.initGrabber(1280, 720);
	tracker.setup();
//    ofSetFrameRate(1);
    
    numFaces = 0;
    
    allFaces.allocate(1280, 720, GL_RGB);
    currentFace.allocate(1280, 720, GL_RGB);
    
    allFaces.begin();
    ofSetColor(255,255,255);
    ofRect(20,20,ofGetWidth(), ofGetHeight());
    allFaces.end();
    
    a = new unsigned long[1280 * 720 * 3];
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		tracker.update(toCv(cam));
	}
}

void testApp::draw() {
	ofSetLineWidth(2);
    
    cam.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
    tracker.draw();
    
    if(tracker.getFound()) {
        
        currentFace.begin();
        
        leftEyePos = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).getCentroid2D();
        rightEyePos = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).getCentroid2D();
        nosePos = tracker.getImageFeature(ofxFaceTracker::NOSE_BASE).getCentroid2D();
        mouthPos = tracker.getImageFeature(ofxFaceTracker::INNER_MOUTH).getCentroid2D();
        
        tracker.getImageFeature(ofxFaceTracker::LEFT_EYE).draw();
        tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE).draw();
        tracker.getImageFeature(ofxFaceTracker::NOSE_BASE).draw();
        tracker.getImageFeature(ofxFaceTracker::INNER_MOUTH).draw();
        
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
        
        cam.draw(0, 0);
        ofPopMatrix();
        
        if(ofGetFrameNum() % 100 == 0) {
            ofImage temp;
            temp.grabScreen(0, 0, 1280, 720);
            temp.saveImage( ofToString(numFaces) + ".jpg");
            addCurrentFaceToAllFaces();
        }
        
        currentFace.end();
    }
    
    currentFace.draw(ofGetWidth()/2,0,ofGetWidth()/2, ofGetHeight()/2);
    
    
//    ofScale(1,-1);
    
    allFaces.draw(ofGetWidth()/2, ofGetHeight()/2, ofGetWidth()/2, ofGetHeight()/2);
    
    
        
    //    ofEllipse(leftEyePos, 10, 10);
     //   ofEllipse(rightEyePos, 10, 10);
      //  ofEllipse(mouthPos, 10, 10);
    
    
    
    
    ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
    ofDrawBitmapString(ofToString(numFaces), 10, 35);
    ofDrawBitmapString(ofToString((int) ofDist(leftEyePos.x, leftEyePos.y, rightEyePos.x,  rightEyePos.y)), 10, 50);
}

void testApp::addCurrentFaceToAllFaces() {
    //ofReadPixels(allFaces.getTexture(), myPixels);
    
    numFaces++;
    
    ofPixels allPixels;
    ofPixels currentPixels;
    
    currentFace.readToPixels(currentPixels);
    allFaces.readToPixels(allPixels);
    
    
	// um yeah this is very inefficient:
    for (int i = numFaces-1; i < numFaces; i++) {
        ofImage temp;
        temp.loadImage(ofToString(i) + ".jpg");
        
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
}

void testApp::keyPressed(int key) {
	if(key == 'r') {
		tracker.reset();
	}
}