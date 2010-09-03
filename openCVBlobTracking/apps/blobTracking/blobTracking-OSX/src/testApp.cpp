
#include "testApp.h"

// Common codes - defined in the blog page
#define USER_JOINED 0
#define USER_MOVED 1
#define USER_LEFT 2
#define USER_MOOD 3
#define USER_PTBLOB_TEST 4
#define SCENE_MOOD_USER 99


// Settings that can be adjusted from the settings file
/* The format is

SettingName1 SettingValue
SettingName2 SettingValue

And the filename is

exampleSettings

it should be located at the filesystem root, i.e. accesible as /exampleSettings from the terminal
*/

// The following are integer settings
#define DEVID "CaptureDeviceID"
#define S_TL_X "TopLeftX"
#define S_TL_Y "TopLeftY"
#define S_LR_X "LowRightX"
#define S_LR_Y "LowRightY"
#define S_BS_MIN "BlobSizeMin"
#define S_BS_MAX "BlobSizeMax"
#define S_THR "Threshold"
#define S_MAX_USERS "MaxUsers"
#define S_SVRPORT "ServerPort"
#define S_MOODFRAMES "NonMoodFrames"
#define S_UOFFSET_X "UserOffsetX"
#define S_UOFFSET_Y "UserOffsetY"

// These are limited to 0 means false and 1 means true
#define S_FLIP_HZ "FlipImageH"
#define S_FLIP_VR "FlipImageV"

// The following are float settings
#define S_MOODSPIKE "MoodSpike"

/// End of settings description


// Internal codes - not for use in processing or the other apps
#define B_DEAD 0
#define B_ALIVE_IN_AREA 1
#define B_ALIVE_OUT_AREA 2


// random defines
#define IMGTOP 10
#define IMGLEFT 10

#define MAX_LINE_SIZE 30





void testApp::setup() {
    // OpenFrameworks setup
	// Set default settings
	ofSetFrameRate( 60 );
    cwidth = 320;
    cheight = 240;
	threshold = 60;
	deviceID = 0;
	topLeftX = 1;
	topLeftY = 1;
	blobSizeMin = 100;
	blobSizeMax = 20000;
	maxUsers = 15;
	lowRightX = cwidth;
	lowRightY = cheight;
	userOffsetX = 0;
	userOffsetY = 0;
	flipImageH = 0;
	flipImageV = 0;
	serverPort = 9090;
	moodSpike = 0.25;
	nonMoodFrames = 3;
	currNonMoodFrame = 0;

	// Load our settings
	ifstream settings;
	settings.open("/exampleSettings.txt", ifstream::out);

	while (settings.good())
	{
		char sBuff[MAX_LINE_SIZE];
		settings.getline(sBuff, MAX_LINE_SIZE);
		GetSetting(DEVID, "int", sBuff, &deviceID);
		GetSetting(S_TL_X, "int", sBuff, &topLeftX);
		GetSetting(S_TL_Y, "int", sBuff, &topLeftY);
		GetSetting(S_LR_X, "int", sBuff, &lowRightX);
		GetSetting(S_LR_Y, "int", sBuff, &lowRightY);
		GetSetting(S_THR, "int", sBuff, &threshold);
		GetSetting(S_BS_MIN, "int", sBuff, &blobSizeMin);
		GetSetting(S_BS_MAX, "int", sBuff, &blobSizeMax);
		GetSetting(S_MAX_USERS, "int", sBuff, &maxUsers);
		GetSetting(S_FLIP_HZ, "int", sBuff, &flipImageH);
		GetSetting(S_FLIP_VR, "int", sBuff, &flipImageV);
		GetSetting(S_SVRPORT, "int", sBuff, &serverPort);
		GetSetting(S_UOFFSET_X, "int", sBuff, &userOffsetX);
		GetSetting(S_UOFFSET_Y, "int", sBuff, &userOffsetY);
		GetSetting(S_MOODSPIKE, "float", sBuff, &moodSpike);

	}
	settings.close();

	// Start the server in non-blocking mode
	tcps.setup(serverPort, false);

	bLearnBakground = true;
	vidGrabber.setDeviceID(deviceID);
	vidGrabber.initGrabber( cwidth, cheight );

	colorImg.allocate( cwidth, cheight );
	grayImg.allocate( cwidth, cheight );
	oldGrayImg.allocate(cwidth, cheight);
	bgImg.allocate( cwidth, cheight );

    blobTracker.setListener( this );

    // Blob output setup
    // Set the standard locale to avoid commas in floating point numbers.
    setenv("LANG", "en_US.UTF-8", 1);

    // Force unbuffered output
    setbuf(stdout, NULL);
	sprintf(selectCornerMsg, "d to define play area corners (currently [%d,%d,%d,%d])", topLeftX, topLeftY, lowRightX, lowRightY);

	csState = CO_SELNONE;
	// All blobs are marked as dead initially
	for (int i = 0; i < MAX_USERS_HARDLIMIT; i++)
	{
		usersTracking[i] = B_DEAD;
	}
	currAdjustment = AJ_THRESH;
}

bool testApp::GetSetting(char * setting, char * settingType, char * line, void * variable)
{
	//printf("GetSetting %s at %s?\n", setting, line);
	if (strncasecmp(setting, line, strlen(setting))==0)
	{
		//printf("Setting found: %s (%s)\n", setting, &line[strlen(setting)+1]);

		if (strcmp(settingType, "int")==0)
		{
			sscanf(&line[strlen(setting)+1],"%d",(int *)variable);
			printf("%s set to %d\n", setting, *(int*)variable);
		} else if (strcmp(settingType, "float")==0)
		{
			sscanf(&line[strlen(setting)+1],"%f",(float *)variable);
		}
		return true;
	} else return false;
}


void testApp::update() {
	int curClient;
	ofBackground( 100, 100, 100 );
	vidGrabber.grabFrame();

	if( vidGrabber.isFrameNew() ) {
        colorImg = vidGrabber.getPixels();

        colorImg.mirror( flipImageV, !flipImageH ); // Image must be flipped vertically - screen coordinate system.
        grayImg = colorImg;



        if( bLearnBakground )
		{
            bgImg = grayImg;
            bLearnBakground = false;
			//maskingChanges = true;
        }

        //grayImg.absDiff( maskedBgImage );

		// oldGrayImg will be used to track the general "mood" or activity of the current frame.
		// It currently contains the last frame, unmodified
		// After absDiff instruction it will contain the difference between the last and the current frame.
		oldGrayImg.absDiff(grayImg);

		// Calculate the general "mood" of the scene now.
		mood = averageBnWVal(oldGrayImg.getCvImage());

		// update oldGrayImg
		oldGrayImg = grayImg;

		// Report scene mood.
		if ((mood > moodSpike) || (currNonMoodFrame > nonMoodFrames))
		{
			char updateMSG[100];
			sprintf(updateMSG,"%d|%d|%f|%f|%f|%f\n", USER_MOOD, SCENE_MOOD_USER, mood, 0.0,
					0.0, 0.0);
			printf(updateMSG);
			tcps.sendToAll(updateMSG);
			currNonMoodFrame = 0;

		} else
		{
			currNonMoodFrame++;
		}

		grayImg.absDiff(bgImg);
        grayImg.blur( 11 );
        grayImg.threshold( threshold );

        //findContures( img, minSize, maxSize, nMax, inner contours yes/no )
        contourFinder.findContours( grayImg, blobSizeMin, blobSizeMax, maxUsers, false );
        blobTracker.trackBlobs( contourFinder.blobs );
    }
	
	for (curClient = 0; curClient < tcps.getNumClients(); curClient++)
	{
		std::string request = tcps.receive(curClient);
		char data[100];
		if (request.    length())
		{
			if (strncmp(request.c_str(), "PtBlob?",7) == 0)
			{
				int curBlob;
				
				// PtBlob collision test
				// Get the coordinates
				float x, y;
				strncpy(data, &(request.c_str()[7]), 100);
				sscanf(data, "%f,%f", &x, &y);
				for (curBlob = 0; curBlob < blobTracker.blobs.size(); curBlob++)
				{
					ofxCvTrackedBlob blob = blobTracker.getById(curBlob);
					
				}

				
			}
			
		}
	}
}

// Average black and white value calcualtion (used for "mood detection")
float testApp::averageBnWVal(IplImage * img)
{
	int px = 0;
	int max = img->imageSize;
	float value = 0;

	for (px = 0; px < max; px++)
	{
		value += (float)img->imageData[px];
	}
	return value/(max*100.0);

}


void testApp::draw() {
	char adjMessage[100];
	ofSetColor( 0xffffff );

    colorImg.draw( IMGLEFT, IMGTOP );
    grayImg.draw( IMGLEFT + cwidth + 10 , IMGTOP );

    blobTracker.draw( IMGLEFT,IMGTOP );

	// It sucks to do this with a CASE, but it's quick.
	switch (currAdjustment) {
		case AJ_THRESH:
			sprintf(adjMessage, "[+]/[-] currently adjusting %s (current value: %d)", S_THR, threshold);
			break;
		case AJ_BLOBMAX:
			sprintf(adjMessage, "[+]/[-] currently adjusting %s (current value: %d)", S_BS_MAX, blobSizeMax);
			break;
		case AJ_BLOBMIN:
			sprintf(adjMessage, "[+]/[-] currently adjusting %s (current value: %d)", S_BS_MIN, blobSizeMin);
			break;
		case AJ_UOFFSETX:
			sprintf(adjMessage, "[+]/[-] currently adjusting %s (current value: %d)", S_UOFFSET_X, userOffsetX);
			break;
		case AJ_UOFFSETY:
			sprintf(adjMessage, "[+]/[-] currently adjusting %s (current value: %d)", S_UOFFSET_Y, userOffsetY);
			break;
		default:
			sprintf(adjMessage, "[+]/[-] to adjust ?? (current value: ??)");
			break;
	}


    ofDrawBitmapString( "[space] to learn background\t [t] to adjust Threshold;\n"
					    "[n] to adjust blob miN size\t [m] blob Max size\t [x,y] user offset",
                        10,20+cheight );
	ofDrawBitmapString(adjMessage, 10, 55 + cheight);
	ofDrawBitmapString(selectCornerMsg, 10, 78+cheight);
	ofNoFill();
	ofRect(IMGLEFT+topLeftX, IMGTOP+topLeftY, lowRightX-topLeftX, lowRightY-topLeftY);

}





void testApp::keyPressed( int key ) {
	int * value;
	int maxValue;
	int minValue;
	int step;
	// It sucks to do this with a CASE, but it's quick.
	switch (currAdjustment) {
		case AJ_THRESH:
			value = &threshold;
			step = 1;
			maxValue = 255;
			minValue = 0;
			break;
		case AJ_BLOBMAX:
			value = &blobSizeMax;
			maxValue = 20000;
			minValue = 0;
			step = 100;
			break;
		case AJ_BLOBMIN:
			value = &blobSizeMin;
			maxValue = 20000;
			minValue = 0;
			step = 10;
			break;
		case AJ_UOFFSETX:
			value = &userOffsetX;
			maxValue = 100;
			minValue = -100;
			step = 1;
			break;
		case AJ_UOFFSETY:
			value = &userOffsetY;
			maxValue = 100;
			minValue = -100;
			step = 1;
			break;
		default:
			break;
	}
    if( key == ' ' ) {
        bLearnBakground = true;
    } else if( key == '-' ) {
        * value = MAX( minValue, *value-step );
    } else if( key == '+' || key == '=' ) {
        * value = MIN( maxValue, *value+step );
    } else if (key == 'd'){
		if (csState == CO_SELNONE)
		{
			sprintf(selectCornerMsg, "%s", "click color img to select top left corner");
			csState = CO_SELTOPLEFT;
		}
	} else if (key == 'n')
	{
		currAdjustment = AJ_BLOBMIN;
	} else if (key =='m')
	{
		currAdjustment = AJ_BLOBMAX;
	} else if (key =='t')
	{
		currAdjustment = AJ_THRESH;
	} else if (key =='x')
	{
		currAdjustment = AJ_UOFFSETX;
	} else if (key =='y')
	{
		currAdjustment = AJ_UOFFSETY;
	}

}
void testApp::mouseMoved( int x, int y ) {}
void testApp::mouseDragged( int x, int y, int button ) {}

void testApp::mousePressed( int x, int y, int button )
{
	if (csState == CO_SELTOPLEFT)
	{
		// Define top left corner
		if (((x > IMGLEFT) && (x < (IMGLEFT + cwidth))) &&
			((y > IMGTOP) && (y < (IMGTOP + cheight))))
		{
			sprintf(selectCornerMsg, "%s", "click color img to select low right corner");
			topLeftX = x-IMGLEFT;
			topLeftY = y-IMGTOP;
			csState = CO_SELLOWRIGHT;

		}
	} else if (csState == CO_SELLOWRIGHT)
	{
		// Define top left corner
		if (((x > IMGLEFT) && (x < (IMGLEFT + cwidth))) &&
			((y > IMGTOP) && (y < (IMGTOP + cheight))))
		{
			lowRightX = x-IMGLEFT;
			lowRightY = y-IMGTOP;
			csState = CO_SELNONE;
			sprintf(selectCornerMsg, "d to define play area corners (currently [%d,%d,%d,%d])", topLeftX, topLeftY, lowRightX, lowRightY);
		}
	}
}

void testApp::mouseReleased() {}


void testApp::blobOn( int x, int y, int id, int order )
{
	char updateMSG[100];

	int insideArea = 1;
    //cout << "blobOn() - id:" << id << " order:" << order << endl;
    float scaledX = (float) ((userOffsetX+x)-topLeftX) / (float)(lowRightX-topLeftX);
    float scaledY = (float) ((userOffsetY+y)-topLeftY) / (float)(lowRightY-topLeftY);


	if (scaledX < 0){ scaledX = 0; insideArea = 0;}
	if (scaledY < 0){ scaledY = 0; insideArea = 0;}
	if (scaledX > 1){ scaledX = 1; insideArea = 0;}
	if (scaledY > 1){ scaledY = 1; insideArea = 0;}
	if (insideArea)
	{
		// A new user has appeared inside the area, therefore he's joined.
		sprintf(updateMSG, "%d|%d|%f|%f|%f|%f\n", USER_JOINED, order, scaledX, scaledY,
							      0.0, 0.0);
		printf(updateMSG);
		tcps.sendToAll(updateMSG);

		usersTracking[order] = B_ALIVE_IN_AREA;
	} else
	{
		// A new user has appeared outside the play area. S/He's not reported as joining, but
		// taken into account when s/he walks inside the play area.
		usersTracking[order] = B_ALIVE_OUT_AREA;
	}

}
void testApp::blobMoved( int x, int y, int id, int order)
{
	char updateMSG[100];
	int insideArea = 1;
    //cout << "blobMoved() - id:" << id << " order:" << order << endl;

    // full access to blob object ( get a reference)
    //ofxCvTrackedBlob blob = blobTracker.getById( id );
    //cout << "area: " << blob.area << endl;
	float scaledX = (float) ((userOffsetX+x)-topLeftX) / (float)(lowRightX-topLeftX);
    float scaledY = (float) ((userOffsetY+y)-topLeftY) / (float)(lowRightY-topLeftY);



	if (scaledX < 0){ scaledX = 0; insideArea = 0;}
	if (scaledY < 0){ scaledY = 0; insideArea = 0;}
	if (scaledX > 1){ scaledX = 1; insideArea = 0;}
	if (scaledY > 1){ scaledY = 1; insideArea = 0;}

	if (insideArea)
	{
		if (usersTracking[order] == B_ALIVE_IN_AREA)
		{
			// The user is moving inside the area
			sprintf(updateMSG, "%d|%d|%f|%f|%f|%f\n", USER_MOVED, order, scaledX, scaledY,
									0.0, 0.0);
			printf(updateMSG);
			tcps.sendToAll(updateMSG);
		} else {
			// The user was outside the play area, but now came inside and therefore joined.
			usersTracking[order] = B_ALIVE_IN_AREA;
			sprintf(updateMSG, "%d|%d|%f|%f|%f|%f\n", USER_JOINED, order, scaledX, scaledY,
				   0.0, 0.0);
			printf(updateMSG);
			tcps.sendToAll(updateMSG);

		}

	} else {
		if (usersTracking[order] == B_ALIVE_IN_AREA)
		{
			// The user was inside the play area but has left
			sprintf(updateMSG,"%d|%d|%f|%f|%f|%f\n", USER_LEFT, order, scaledX, scaledY,
				   0.0, 0.0);
			printf(updateMSG);
			tcps.sendToAll(updateMSG);
		}
		usersTracking[order] = B_ALIVE_OUT_AREA;
	}
}
void testApp::blobOff( int x, int y, int id, int order )
{
	char updateMSG[100];
    //cout << "blobOff() - id:" << id << " order:" << order << endl;
    float scaledX = (float) ((userOffsetX+x)-topLeftX) / (float)(lowRightX-topLeftX);
    float scaledY = (float) ((userOffsetY+y)-topLeftY) / (float)(lowRightY-topLeftY);

	// Is this blob alive and inside the play area?
	// Then send the notification. Do not send notification otherwise..
	if (usersTracking[order] == B_ALIVE_IN_AREA)
	{
		if (scaledX < 0) scaledX = 0;
		if (scaledY < 0) scaledY = 0;
		if (scaledX > 1) scaledX = 1;
		if (scaledY > 1) scaledY = 1;
		sprintf(updateMSG, "%d|%d|%f|%f|%f|%f\n", USER_LEFT, order, scaledX, scaledY,
				0.0, 0.0);
		printf(updateMSG);
		tcps.sendToAll(updateMSG);
	}
	// mark blob as dead
	usersTracking[order] = B_DEAD;
}
