#include "manApp.h"
#include "ofxVectorMath.h"

//--------------------------------------------------------------
void manApp::setup(){
	ofSetFrameRate( 60.0f ) ;

	ofxVec3f start_eye_pos( 0, 3.5, -10 );
	util_3d.setup( start_eye_pos );
	
	z = 0;
	
	tagger.setup();
	
}

//--------------------------------------------------------------
void manApp::update()
{
	tagger.update( ofGetLastFrameTime() );
}

//--------------------------------------------------------------
void manApp::draw(){

	bool draw_ground_plane = true;
	

	util_3d.begin3dDrawing( draw_ground_plane );
		
	glColor3f( 0, 0, 0 );
	tagger.draw();

	util_3d.end3dDrawing();
	
	
	// now draw some debugging stuff
	ofSetColor( 0x000000 );
	ofDrawBitmapString( "Keys:\n"
					   "1-3 select targets    mouse drag target pos     z/Z move target z\n"
					   "; toggle mouse move eye rotation    ae,o.j (dvorak==qwerty adwsec) move eye pos\n"
					   "shift-W toggle walk cycle           p reset root pos to 0,0,0   \n"
					   "shift-S toggle adaptive ik solve (default enabled)      shift-E draw extended skeleton", 10, 23 );
	ofxVec3f tt = tagger.getTagArmTarget();
	char buf[256];
	sprintf(buf, "tag arm target now: %f %f %f\n", tt.x, tt.y, tt.z );
	ofDrawBitmapString( buf, 10, 101 );
	
	ofxVec3f e = util_3d.getEyePos();
	float heading = util_3d.getHeading();
	float pitch = util_3d.getPitch();
	sprintf(buf, "eye at %f %f %f heading %f pitch %f\n", e.x, e.y, e.z, heading, pitch );
	ofDrawBitmapString( buf, 10, ofGetHeight()-26 );
}

//--------------------------------------------------------------
void manApp::keyPressed(int key){
	
	util_3d.keyPressed( key );
	
	ofxVec3f pos;
	switch( key )
	{
		case 'P':
			tagger.setRootPosition( ofxVec3f(0,0,0) );
			break;
			
		case 'z':
			z += 0.1f;
			break;
		case 'Z':
			z -= 0.1f;
			break;
			
		default:
			break;
			
	}

}



//--------------------------------------------------------------
void manApp::keyReleased(int key){
}

//--------------------------------------------------------------
void manApp::mouseMoved(int x, int y ){
	last_moved_mx = x;
	last_moved_my = y;
	util_3d.mouseMoved( x, y );
}

//--------------------------------------------------------------
void manApp::mouseDragged(int x, int y, int button){
	
	ofxVec3f tt = tagger.getTagArmTarget();
	tt += ofxVec3f( 4*(float(x-last_mx)/ofGetWidth()), 4*(float(y-last_my)/ofGetHeight()), 0 );
	tt.z = z;
	tagger.setTagArmTarget( tt );
	last_mx = x;
	last_my = y;
	last_moved_mx = -1;
	
	util_3d.mouseDragged( x, y, button );
	
}

//--------------------------------------------------------------
void manApp::mousePressed(int x, int y, int button){
	
	last_mx = x;
	last_my = y;
	last_moved_mx = -1;
	util_3d.mousePressed( x, y, button );
	
}

//--------------------------------------------------------------
void manApp::mouseReleased(int x, int y, int button){

	last_mx = -1;
	last_moved_mx = x;
	last_moved_my = y;
	util_3d.mouseReleased( x, y, button );
}

//--------------------------------------------------------------
void manApp::windowResized(int w, int h){

}

