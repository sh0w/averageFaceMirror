#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
    usePS3 = true;
    WIDTH = 640*2;
    HEIGHT = 480*2;
    
    if(usePS3) {
	    setupPS3Eye();
    } else {
		cam.initGrabber(WIDTH, HEIGHT);
    }
	tracker.setup();
//    ofSetFrameRate(1);

    
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
    
    vidGrabber.setAutogain(false);
    vidGrabber.setAutoWhiteBalance(false);
    
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
	ofSetLineWidth(2);
    
    if(usePS3) {
        videoTexture.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
    } else {
	    cam.draw(0,0,ofGetWidth()/2, ofGetHeight()/2);
    }
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
        
        if(usePS3) {
            videoTexture.draw(0,0);
        } else {
            cam.draw(0, 0);
        }
        ofPopMatrix();
        
        if(ofGetFrameNum() % 100 == 0) {
            ofImage temp;
            temp.grabScreen(0, 0, WIDTH, HEIGHT);
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