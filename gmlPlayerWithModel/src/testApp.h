#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxDirList.h"
#include "ofxVectorMath.h"
#include "ofxXmlSettings.h"

#include "ofxControlPanel.h"
#include "simpleStroke.h"

#include "manApp.h"

class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		
		void loadGml(string gmlFile, bool bSwap);

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void resized(int w, int h);

		manApp testMan;

		ofxControlPanel panel;	
		simpleFileLister lister;		
		float drawPct;
		float shiftX;
		simpleTag tag;
};

#endif
