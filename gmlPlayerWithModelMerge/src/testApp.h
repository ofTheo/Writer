#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxDirList.h"
#include "ofxVectorMath.h"
#include "ofxXmlSettings.h"

#include "ofxControlPanel.h"
#include "simpleStroke.h"
#include "appUtils.h"
#include "IKTagger.h"
#include "Util3d.h"


class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		
		void loadNewTag(string path);
		
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void resized(int w, int h);

		ofxControlPanel panel;	
		simpleFileLister lister;		
		float drawPct;
		float shiftX;
		simpleTag tag;
		
		Util3d util_3d;

		float last_mx, last_my;
		float last_moved_mx, last_moved_my;

		ofPoint startOffset;

		float z;	
		CalVector root_pos;
		IKTagger tagger;	
};

#endif
