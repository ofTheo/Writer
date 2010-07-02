#include "testApp.h"

float tagScale = 6.0;

//--------------------------------------------------------------
void testApp::setup(){
	panel.setup("panel", 0, 0, 340, 700);
	panel.addPanel("gml lister", 4);
	
	lister.allowExt("gml");
	lister.listDir("gml/");
	panel.addFileLister("gml lister", &lister, 280, 200);

	panel.addToggle("restart tag", "restart", false);
	panel.addSlider("drawSpeed", "drawSpeed", 0.001, 0.00001, 0.01, false);

	panel.addSlider("shiftX", "shiftX", 0, -400.0, 400.0, false);
	panel.addSlider("shiftY", "shiftY", 0, -400.0, 400.0, false);
	panel.addSlider("rotate 3D", "rotate", 0, -360.0, 360.0, false);
	panel.addChartPlotter("drawPct", guiStatVarPointer("drawPct", &drawPct, GUI_VAR_FLOAT, true, 2), 200, 100, 200, -0.2, 2.0);

	panel.loadSettings("panel.xml");

	drawPct = 0.0;
	
	ofSetVerticalSync(true);
	
	loadNewTag(lister.getPath(0));
	

	ofSetFrameRate( 60.0f ) ;

	ofxVec3f start_eye_pos( 0, 2.0, 15 );
	util_3d.setup( start_eye_pos );
	
	
	z = 0;
	
	tagger.setup( "man_good.xml" );
	//tagger.setOtherArmTarget( ofxVec3f(-1.0, -1.5, 2.0) );	
		
	shiftX = 0.0;
}

//--------------------------------------------------------------
void testApp::loadNewTag(string path){
	loadGml(path, tag, true);
	startOffset = ofPoint(0.18 + tag.getFirstPoint().x, -1.0+tag.getFirstPoint().y, 0);
	startOffset *= tagScale;
	drawPct = 0.0;
	tag.setPct(drawPct);
	panel.setValueB("restart", false);	
	printf("LOAD NEW TAG\n");
	ease_speed = 0;
	ease_target = 1;
}

//--------------------------------------------------------------
void testApp::update(){
	panel.update();
	
	if( panel.getValueB("restart") ){
		drawPct = 0.0;
		tag.setPct(0);		
		tagger.setRootPosition(ofxVec3f());
		panel.setValueB("restart", false);
	}

	// update the tagger's position
	tagger.update( ofGetLastFrameTime() );
	
	if( lister.selectedHasChanged() ){
		loadNewTag(lister.getSelectedPath());
		lister.clearChangedFlag();
	}
	
	// update the tag
	if( tag.totalNumPoints() ){
		// base on the comfort level of the tagger:
		// don't update if it's too uncomfortable
		if ( tagger.getHandTargetDiscomfort() > 1.2f )
		{
			ease_target = 0;
		}
		else
		{
			ease_target = 1;
		}
		// update ease speed
		static const float EASE_ADJUST_RATE = 2.0f;
		if ( ease_target > ease_speed )
			ease_speed += min(ease_target-ease_speed,EASE_ADJUST_RATE*float(ofGetLastFrameTime()));
		else  
			ease_speed -= min(ease_speed-ease_target,EASE_ADJUST_RATE*float(ofGetLastFrameTime()));
		
		tag.setPct( ofClamp(drawPct, 0, 1) );
		drawPct += panel.getValueF("drawSpeed")*ease_speed;
		if( drawPct >= 1.5 ){
			drawPct = 0.0;
		}
	}
	
	panel.clearAllChanged();
}


//--------------------------------------------------------------
void testApp::draw(){
	
	float relX = startOffset.x + tag.currentPt.x * tagScale;
	float relY = startOffset.y + tag.currentPt.y * tagScale;
	
	tagger.setTagArmTarget( ofxVec3f(relX, relY, 0) );
	
	bool draw_ground_plane = true;
	
	util_3d.begin3dDrawing( draw_ground_plane );
	
	ofRotate(panel.getValueF("rotate"), 0, 1, 0);

	ofPushStyle();
	ofSetLineWidth(2);
	ofNoFill();
	tag.draw(startOffset.x , startOffset.y, tagScale, tagScale);
	ofPopStyle();
		
	ofCircle(-0.5, 0.0, 0.1);

	// tagger responds to ofSetColor :-)
	ofSetColor( 128,128,128,255 );
	// tagger must be drawn at 0,0,0 otherwise the setTagArmTarget() offsets get all fucked up
	bool wireframe = false;
	tagger.draw( wireframe );
	
	util_3d.end3dDrawing();
	
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

