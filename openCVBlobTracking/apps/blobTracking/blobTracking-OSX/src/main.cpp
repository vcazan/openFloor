
#include "ofMain.h"
#include "testApp.h"

int main() {
	ofSetupOpenGL( 680,350, OF_WINDOW );
	testApp APP;
	ofRunApp( &APP );
}
