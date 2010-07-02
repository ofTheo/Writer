/*
 *  IKchain.cpp
 *  emptyExample
 *
 *  Created by damian on 18/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "IKchain.h"
#include "ofMain.h"
#include <assert.h>

void IKChain::addBone( float length, float angle, float weight_centre )
{
	bones.push_back( IKBone( length, angle, weight_centre ) );
}



void IKChain::solve( int iterations )
{
	// solve using constraint relaxation
	// after http://www.ryanjuckett.com/programming/animation/22-constraint-relaxation-ik-in-2d
	
	// first put all bones to cartesian space
	vector<ofxVec2f> bone_positions = toCartesianSpace();
	
	
	for ( int i=0; i<iterations; i++ )
	{
		// push the last bone to the target position
		bone_positions[bones.size()] = target_pos;
		for ( int bone_num = bones.size()-1; bone_num >= 0; bone_num-- )
		{
			// child_pos
			ofxVec2f& b_c = bone_positions[bone_num+1];
			// parent pos
			ofxVec2f& b_p = bone_positions[bone_num];
			
			// now, the bone is the wrong length. correct its length to fulfil size constraint.
			ofxVec2f delta = b_c - b_p;
			float length = delta.length();
			length = max( 0.00001f, length );
			// pointing from parent to child
			ofxVec2f direction = delta/length;

			float desired_length = bones[bone_num].getLength();
			float delta_length = desired_length - length;
			
			// balance according to weight_centre
			float weight_centre;
			// for the last bone, we move only the start (parent) point
			if ( bone_num == bones.size()-1 )
				weight_centre = 0;
			// for other bones we move points based on weight centre
			else 
				weight_centre = bones[bone_num].getWeightCentre();
			
			// move
			b_c += weight_centre * delta_length * direction;
			b_p -= (1.0f-weight_centre) * delta_length * direction;
		}		

	}
	// convert back to angles
	fromCartesianSpace( bone_positions );
	
	
}


vector<ofxVec2f> IKChain::toCartesianSpace() const
{
	// put all the bones into 2d space
	vector<ofxVec2f> bone_positions;
	// start at the root
	bone_positions.push_back( root_pos );
	// start pointing right
	ofxVec2f dir( 1,0 );
	for ( int i=0; i<bones.size(); i++ )
	{
		// rotate our direction by the bone's angle
		dir.rotateRad( bones[i].getAngle() );
		// add on direction * length to get the end point of this bone
		ofxVec2f next_pos = bone_positions.back() + dir*bones[i].getLength();
		bone_positions.push_back( next_pos );
	}
	
	return bone_positions;
}

void IKChain::fromCartesianSpace( vector<ofxVec2f>& bone_positions )
{
	assert( bone_positions.size() == bones.size()+1 );
	
	root_pos.set( bone_positions[0] );
	ofxVec2f dir( 1, 0 );
	for ( int i=0; i<bones.size(); i++ )
	{
		// get bone parent->child delta
		ofxVec2f bone_delta = bone_positions[i+1]-bone_positions[i];
		// convert bone delta to a direction
		bone_delta.normalize();
		// -> angle
		float angle = bone_delta.angleRad( dir );
		bones[i].setAngle( -angle );
		// rotate our direction by the bone's angle (angles are all relative)
		dir.rotateRad( -angle );
	}
	
}


void IKChain::draw()
{
	vector<ofxVec2f> bone_positions = toCartesianSpace();
	
	ofSetColor( 128, 255, 128 );
	ofCircle( root_pos.x, root_pos.y, 5 );
	ofSetColor( 128, 128, 128 );
	ofBeginShape();
	ofNoFill();
	ofCurveVertex( bone_positions[0].x, bone_positions[0].y );
	for ( int i=0; i<bone_positions.size(); i++ )
	{
		if ( i < bone_positions.size()-1 )
		{
			ofLine( bone_positions[i].x, bone_positions[i].y, 
			   bone_positions[i+1].x, bone_positions[i+1].y );
		}
		ofCurveVertex( bone_positions[i].x, bone_positions[i].y );
	}
	ofCurveVertex( bone_positions[bone_positions.size()-1].x, bone_positions[bone_positions.size()-1].y );
	ofEndShape();
	ofSetColor( 255, 128, 128 );
	ofCircle(target_pos.x, 
			 target_pos.y, 
			 5 );
}

