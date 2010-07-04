/*
 *  Util3d.cpp
 *  ikChain
 *
 *  Created by damian on 01/07/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "Util3d.h"
#include "ofMain.h"

void Util3d::setup( ofxVec3f _eye_pos, float _heading, float _pitch )
{
	last_mx = -1;
	last_moved_mx = -1;
	
	heading = _heading;
	pitch = _pitch;
	eye_pos = _eye_pos;
	fov = 60.0f;
	move_speed = 0.15f;
	rotate_speed = 1.5f;
	
	rotate_eye = false;
	
}


void Util3d::begin3dDrawing( bool draw_ground_plane )
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	// load proper 3d world
	
	float w = 10;
	float h = 10;
	
	float halfFov, theTan, screenFov, aspect;
	screenFov 		= fov;
	
	float eyeX 		= /*(float)w / 2.0;*/eye_pos.x;
	float eyeY 		= /*(float)w / 2.0;*/eye_pos.y;
	halfFov 		= PI * screenFov / 360.0;
	theTan 			= tanf(halfFov);
	float dist 		= (h/2) / theTan;
	float nearDist 	= dist / 1000.0;	// near / far clip plane
	float farDist 	= dist * 10000.0;
	aspect 			= (float)w/(float)h;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(screenFov, aspect, nearDist, farDist);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// centre
	ofxVec3f d_centre(0, 0, dist);
	d_centre.rotate( heading, ofxVec3f(0, 1, 0) );
	d_centre.rotate( pitch, ofxVec3f(1, 0, 0) );
	ofxVec3f centre = eye_pos + d_centre;
	gluLookAt(eyeX, eyeY, eye_pos.z, centre.x, centre.y, centre.z, 0.0, 1.0, 0.0);
	
	//glScalef( -1, -1, 1 );
	
	if ( draw_ground_plane )
	{
		// draw ground plane
		glBegin( GL_LINES );
		glColor3f( 0.6f, 0.6f, 1.0f );
		for ( int i=0; i<20; i++ )
		{
			glVertex3f( -10, 0, i-10 );
			glVertex3f( 10, 0, i-10 );
		}
		for ( int i=0; i<20; i++ )
		{
			glVertex3f( i-10, 0, -10 );
			glVertex3f( i-10, 0, 10 );
		}
		glEnd();
	}	
	
	
}


void Util3d::end3dDrawing()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
}


void Util3d::keyPressed( int key )
{
	if ( key == ';' )
		rotate_eye = !rotate_eye;
	else if ( key == ':' )
		printf("eye at %f %f %f heading %f pitch %f\n", eye_pos.x, eye_pos.y, eye_pos.z, heading, pitch );
	else if ( key == ',' || key == OF_KEY_UP )
		moveEye( 0, 0, move_speed );
	else if ( key == 'o' || key == OF_KEY_DOWN )
		moveEye( 0, 0, -move_speed );
	else if ( key == 'a' || key == OF_KEY_LEFT )
		moveEye( move_speed, 0, 0 );
	else if ( key == 'e' || key == OF_KEY_RIGHT )
		moveEye( -move_speed, 0, 0 );
	else if ( key == '.' || key == OF_KEY_PAGE_UP )
		//moveEye( 0, move_speed, 0 );
		eye_pos.y += move_speed;
	else if ( key == 'j' || key == OF_KEY_PAGE_DOWN )
		eye_pos.y -= move_speed;
	//moveEye( 0, -move_speed, 0 );
	
}

void Util3d::moveEye( float x, float y, float z)
{
	ofxVec3f relative( x,y,z );
	relative.rotate( heading, ofxVec3f(0, 1, 0) );
	relative.rotate( pitch, ofxVec3f( 1, 0, 0 ) );
	eye_pos += relative;
}
					 

void Util3d::mouseMoved(int x, int y ){
	if ( last_moved_mx != -1 && rotate_eye )
	{
		heading += 180*rotate_speed*float(last_moved_mx-x)/ofGetWidth();
		pitch += 90*rotate_speed*float(last_moved_my-y)/ofGetHeight();
	}
	last_moved_mx = x;
	last_moved_my = y;
	
}

//--------------------------------------------------------------
void Util3d::mouseDragged(int x, int y, int button){
	
	last_mx = x;
	last_my = y;
	last_moved_mx = -1;
	
}

//--------------------------------------------------------------
void Util3d::mousePressed(int x, int y, int button){
	
	last_mx = x;
	last_my = y;
	last_moved_mx = -1;
	
}

//--------------------------------------------------------------
void Util3d::mouseReleased(int x, int y, int button){
	
	last_mx = -1;
	last_moved_mx = x;
	last_moved_my = y;
}



