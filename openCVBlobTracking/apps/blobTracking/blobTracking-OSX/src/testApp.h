
#include "ofMain.h"

#ifndef USE_OFX_006
//OF '05 code
#define OF_ADDON_USING_OFXNETWORK
#include "ofAddons.h"
#else
//OF '06 code
#include <ofxCvColorImage.h>
#include <ofxCvGrayscaleImage.h>
#include <ofxCvContourFinder.h>
#include "ofxNetwork.h"
#endif

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>


#include "ofxCvBlobTracker.h"
#include "ofxCvTrackedBlob.h"
#include "ofxCvConstants_Track.h"

enum cornerSelectStates{CO_SELNONE, CO_SELTOPLEFT, CO_SELLOWRIGHT};
#define AJ_THRESH 0
#define AJ_BLOBMIN 1
#define AJ_BLOBMAX 2
#define AJ_UOFFSETX 3
#define AJ_UOFFSETY 4

#define MAX_USERS_HARDLIMIT 25


class testApp : public ofSimpleApp, public ofxCvBlobListener {

  public:

    int cwidth;
    int cheight;
    ofVideoGrabber  vidGrabber;
    ofxCvColorImage  colorImg;
    ofxCvGrayscaleImage  grayImg;
	ofxCvGrayscaleImage  oldGrayImg;
    ofxCvGrayscaleImage  bgImg;

    ofxCvContourFinder  contourFinder;
    ofxCvBlobTracker  blobTracker;

	int threshold;
	bool bLearnBakground;


    void setup();
    void update();
    void draw();

    void keyPressed( int key );
    void mouseMoved( int x, int y );
    void mouseDragged( int x, int y, int button );
    void mousePressed( int x, int y, int button );
    void mouseReleased();

    void blobOn( int x, int y, int id, int order );
    void blobMoved( int x, int y, int id, int order );
    void blobOff( int x, int y, int id, int order );
	
	bool GetSetting(char * setting, char * settingType, char * line, void * variable);
	float averageBnWVal(IplImage * img);
	
	// settings
	int deviceID;
	// Select corner msg
	char  selectCornerMsg[100];
	cornerSelectStates csState;
	int topLeftX;
	int topLeftY;
	int lowRightX;
	int lowRightY;
	int userOffsetX;
	int userOffsetY;
	int blobSizeMin;
	int blobSizeMax;
	int maxUsers;
	int usersTracking[MAX_USERS_HARDLIMIT];
	int flipImageH;
	int flipImageV;
	int currAdjustment;
	int serverPort;
	
	float mood;
	float moodSpike;
	int currNonMoodFrame;
	int nonMoodFrames;
	
	// Network server
	ofxTCPServer tcps;

};

