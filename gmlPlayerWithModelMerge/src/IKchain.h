/*
 *  IKChain.h
 *  emptyExample
 *
 *  Created by damian on 18/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

#include "ofxVectorMath.h"
#include "IKBone.h"
#include <assert.h>

class IKChain
{
public:
	IKChain() : root_pos( 0, 0 ), target_pos( 1, 1 ) {};
	
	// weight_centre determines which end of the bone moves:
	// 0   = the start (==parent link) only 
	// 1   = the end   (==child link) only
	// 0.5 = start and end both move the same amount
	void addBone( float length, ofxQuaternion angle, float weight_centre=0.5f );
	
	// solve using relaxation method from current pose to new pose
	void solve( int iterations = 10);

	// set root/target positions
	void setRootPos( ofxVec2f new_root ) { root_pos = new_root; }
	void setTargetPos( ofxVec2f new_target ) { target_pos = new_target; }
	
	// draw
	void draw();
	
	// how many bones?
	int getNumBones() { return bones.size(); }
	IKBone& getBone( int i ) { assert( i <bones.size() ); return bones.at(i); }
	
private:
	vector<IKBone> bones;

	/// return a list of cartesian space coordinates for the chain
	/// returns a vector of (bones.size()+1) 2d coordinates;
	/// result[0] is the root pos
	/// result[bones.size()] is the last bone's endpoint
	vector<ofxVec2f> toCartesianSpace() const;
	
	/// update the bones[] vector from the given bone positions
	/// doesn't update or check lengths
	void fromCartesianSpace( vector<ofxVec2f>& bone_positions );
	
	
	//ofxVec3f root_pos;
	//ofxVec3f target_pos, 
	ofxVec2f root_pos, target_pos;

};


