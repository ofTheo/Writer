#pragma once

#include "ofMain.h"
#include "IKTagger.h"
#include "Util3d.h"

class manApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

	
//private:
	
	Util3d util_3d;
	
	float last_mx, last_my;
	float last_moved_mx, last_moved_my;

	float z;	
	CalVector root_pos;
	IKTagger tagger;
};

