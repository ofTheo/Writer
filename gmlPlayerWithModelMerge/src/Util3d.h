/*
 *  Util3d.h
 *  ikChain
 *
 *  Created by damian on 01/07/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */


#include "ofxVec3f.h"

class Util3d
{
public:
	
	void setup( ofxVec3f eye_pos=ofxVec3f(0,0,0), float heading=0, float pitch=0 );

	/// replace the current viewport and view matrix with a proper opengl 3d view
	void begin3dDrawing( bool draw_ground_plane = true );
	/// revert to previous viewport and view matrix
	void end3dDrawing();


	void keyPressed( int k );
	void mouseMoved( int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	
	

	ofxVec3f getEyePos() { return eye_pos; }
	float getHeading() { return heading; }
	float getPitch() { return pitch; }

private:


	float last_mx, last_my;
	float last_moved_mx, last_moved_my;
	
	void moveEye( float x, float y, float z);
	float heading, pitch, fov;
	ofxVec3f eye_pos;
	float move_speed, rotate_speed;
	bool rotate_eye;
	
	

};
