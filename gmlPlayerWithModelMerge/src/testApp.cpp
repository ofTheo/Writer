#include "testApp.h"

float tagScale = 6.0;

//--------------------------------------------------------------
void testApp::setup(){
	panel.setup("panel", 0, 0, 340, 700);
	panel.addPanel("gml lister", 4);
	
	lister.allowExt("gml");
	lister.listDir("gml/");
	panel.addFileLister("gml lister", &lister, 280, 200);
	panel.addSlider("drawSpeed", "drawSpeed", 0.001, 0.00001, 0.01, false);
	panel.addSlider("shiftX", "shiftX", 0, -400.0, 400.0, false);
	panel.addSlider("shiftY", "shiftY", 0, -400.0, 400.0, false);
	panel.addChartPlotter("drawPct", guiStatVarPointer("drawPct", &drawPct, GUI_VAR_FLOAT, true, 2), 200, 100, 200, -0.2, 2.0);

	panel.loadSettings("panel.xml");

	drawPct = 0.0;
	
	ofSetVerticalSync(true);
	
	loadNewTag(lister.getPath(0));
	

	ofSetFrameRate( 60.0f ) ;

	ofxVec3f start_eye_pos( 0, 2.0, 15 );
	util_3d.setup( start_eye_pos );
	
	
	z = 0;
	
	tagger.setup();
	tagger.setOtherArmTarget( ofxVec3f(-1.0, -1.5) );	
		
	shiftX = 0.0;
}

//--------------------------------------------------------------
void testApp::loadNewTag(string path){
	loadGml(path, tag, true);
	startOffset = ofPoint(0.18 + tag.getFirstPoint().x, -1.0+tag.getFirstPoint().y, 0);
	startOffset *= tagScale;
}

//--------------------------------------------------------------
void testApp::update(){
	panel.update();

	tagger.update( ofGetLastFrameTime() );
	
	if( lister.selectedHasChanged() ){
		loadNewTag(lister.getSelectedPath());
	}
	
	if( tag.totalNumPoints() ){
		tag.setPct( ofClamp(drawPct, 0, 1) );
		drawPct += panel.getValueF("drawSpeed");
		if( drawPct >= 1.5 ){
			drawPct = 0.0;
			shiftX  = 0.0;
		}
	}
	
	
	panel.clearAllChanged();
}


//--------------------------------------------------------------
void testApp::draw(){
	
	//ofCircle(tlX + tag.currentPt.x * w, tlY + tag.currentPt.y * h, 20);
//
//	float relX = tag.currentPt.x * w;
//	float relY = tag.currentPt.y * h;
//	
//	if( fabs(relX - shiftX) > 100.0 ){
//		if( relX > shiftX ){
//			shiftX = ofLerp(shiftX, relX-100, 0.2); 
//		}else{
//			shiftX = ofLerp(shiftX, relX+100, 0.2); 		
//		}
//	}
//	
//	relX -= shiftX;
//	
//	float relXPct = relX/640.0;
//	float relYPct = 0.8 -tag.currentPt.y;
//
//	printf("shiftX is %f relX is %f\n", shiftX, relX);
//
//	tagger.setRootPosition(ofxVec3f(-10.0 * (float)(0.0 + shiftX)/ofGetWidth(), 0.0));
//	tagger.setTagArmTarget(ofxVec3f(-relXPct, relYPct) * 10.0);
//
//	tagger.setOtherArmTarget(ofxVec3f(relXPct*5.0, -1.5) );
//	

	float relX = tag.currentPt.x * tagScale;
	float relY = (tag.currentPt.y-0.4) * tagScale*1.8;
	
	float needToMoveDist = 0.3 * tagScale;
	
	if( fabs(relX - shiftX) > needToMoveDist ){
	
		float posToMove;
		posToMove = relX-needToMoveDist;

		shiftX = ofLerp(shiftX, posToMove, 0.01); 		
		//tagger.setRootPosition(ofxVec3f(posToMove, 0.0));

		//relX -= shiftX;			
	}
	
	tagger.setTagArmTarget( ofxVec3f(relX, relY, -0.9) );
	
	bool draw_ground_plane = true;
	
	util_3d.begin3dDrawing( draw_ground_plane );

	ofPushStyle();
	ofSetLineWidth(2);
	ofNoFill();
	tag.draw(startOffset.x , startOffset.y, tagScale, tagScale);
	ofPopStyle();
	
	ofCircle(-0.5, 0.0, 0.1);
		
	glColor4f( 0, 0, 0, 1);
	tagger.draw();

	
	util_3d.end3dDrawing();
	
//	
//	// now draw some debugging stuff
//	ofSetColor( 0x000000 );
//	ofDrawBitmapString( "Keys:\n"
//					   "1-3 select targets    mouse drag target pos     z/Z move target z\n"
//					   "; toggle mouse move eye rotation    ae,o.j (dvorak==qwerty adwsec) move eye pos\n"
//					   "shift-W toggle walk cycle           p reset root pos to 0,0,0   \n"
//					   "shift-S toggle adaptive ik solve (default enabled)      shift-E draw extended skeleton", 10, 23 );
//	ofxVec3f tt = tagger.getTagArmTarget();
//	char buf[256];
//	sprintf(buf, "tag arm target now: %f %f %f\n", tt.x, tt.y, tt.z );
//	ofDrawBitmapString( buf, 10, 101 );
//	
//	ofxVec3f e = util_3d.getEyePos();
//	float heading = util_3d.getHeading();
//	float pitch = util_3d.getPitch();
//	sprintf(buf, "eye at %f %f %f heading %f pitch %f\n", e.x, e.y, e.z, heading, pitch );
//	ofDrawBitmapString( buf, 10, ofGetHeight()-26 );	
//	
	
	ofSetupScreen();
	panel.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x, y, button);

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}

