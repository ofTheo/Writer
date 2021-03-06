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
	// NO: use the Util3d interface // panel.addSlider("rotate 3D", "rotate", 0, -360.0, 360.0, false);
	panel.addChartPlotter("drawPct", guiStatVarPointer("drawPct", &drawPct, GUI_VAR_FLOAT, true, 2), 200, 100, 200, -0.2, 2.0);

	panel.loadSettings("panel.xml");

	drawPct = 0.0;
	
	ofSetVerticalSync(true);
	
	
	tagger.setup( "man_good.xml" );
	//tagger.setOtherArmTarget( ofxVec3f(-1.0, -1.5, 2.0) );	
		
	shiftX = 0.0;

	// loadNewTag must happen after tagger is setup
	loadNewTag(lister.getPath(0));
	
	
	ofSetFrameRate( 60.0f ) ;
	
	
	// setup 3d world
	// THEO: DON'T TOUCH UNTIL YOU'VE PLAYED WITH THE UTIL_3D INTERFACE
	ofxVec3f start_eye_pos( 0, 2.0, -15 );
	float heading = 0;
	float pitch = 0;
	util_3d.setup( start_eye_pos, heading, pitch );
	
	
	z = 0;
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
	tagger.startWalkon( 0 );
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

	// update the tagger's position and pose
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

		if ( drawPct > 1.0f )
			// walk off
			tagger.startWalkoff();
		tag.setPct( ofClamp(drawPct, 0, 1) );
		drawPct += panel.getValueF("drawSpeed")*ease_speed;
		
		if ( tagger.isFinished() )
		{
			drawPct = 0.0;
			tagger.startWalkon( 0 );
		}
	}
	
	panel.clearAllChanged();
}


//--------------------------------------------------------------
void testApp::draw(){
	
	bool flip_tag_x = true;
	float tagScaleX = tagScale*(flip_tag_x?-1:1);
	float tagScaleY = tagScale*1;
	float relX = startOffset.x*(flip_tag_x?-1:1) + tag.currentPt.x * tagScaleX;
	float relY = startOffset.y + tag.currentPt.y * tagScaleY;
	
	tagger.setTagArmTarget( ofxVec3f(relX, relY, 0) );
	
	bool draw_ground_plane = true;
	
	util_3d.begin3dDrawing( draw_ground_plane );
	
	// turn on the z-buffer
	glEnable( GL_DEPTH_TEST );
	
	ofPushStyle();
	ofSetLineWidth(2);
	ofNoFill();
	tag.draw(startOffset.x*(flip_tag_x?-1:1), startOffset.y, tagScaleX, tagScaleY);
	ofPopStyle();
		
	ofCircle(-0.5, 0.0, 0.1);

	// tagger responds to ofSetColor :-)
	ofSetColor( 64,64,64,255 );
	// tagger must be drawn at 0,0,0 otherwise the setTagArmTarget() offsets get all fucked up
	bool wireframe = false;
	tagger.draw( wireframe );
	
	// turn off the z-buffer
	glDisable( GL_DEPTH_TEST );

	// go back to oF coordinate system
	util_3d.end3dDrawing();

	
	panel.draw();
	
	// draw info
	bool draw_info = true;
	if ( draw_info )
	{
		// now draw some debugging stuff
		ofSetColor( 0x000000 );
		ofDrawBitmapString( "UI :\n"
						   "; toggle mouse move eye rotation    up/down/left/right/pgup/pgdn move eye pos\n"
						   , 400, 23 );
		ofxVec3f tt = tagger.getTagArmTarget();
		char buf[256];
		sprintf(buf, "tag arm target now: %6.3f %6.3f %6.3f    discomfort %5.3f\n", tt.x, tt.y, tt.z, tagger.getHandTargetDiscomfort() );
		ofDrawBitmapString( buf, 400, 23+2*13+13 );
		
		ofxVec3f e = util_3d.getEyePos();
		float heading = util_3d.getHeading();
		float pitch = util_3d.getPitch();
		sprintf(buf, "eye at %f %f %f heading %f pitch %f\n", e.x, e.y, e.z, heading, pitch );
		ofDrawBitmapString( buf, 400, ofGetHeight()-26 );
	}
	
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	util_3d.keyPressed( key );
	
	if ( key=='P' )
		tagger.reset();
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	util_3d.mouseMoved( x, y );

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x, y, button);
	util_3d.mouseDragged( x, y, button );
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x, y, button);
	util_3d.mouseReleased( x, y, button );
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
	util_3d.mouseReleased( x, y, button );
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}


