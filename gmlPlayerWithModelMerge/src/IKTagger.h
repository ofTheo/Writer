/*
 *  IKTagger.h
 *  ikChain
 *
 *  Created by damian on 01/07/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

#include "Cal3DModel.h";
#include "IKCharacter.h"

class IKTagger
{
public:
	void setup( );
	void update( float elapsed );
	void draw();
	
	void setOtherArmTarget(ofxVec3f target);
	
	
	/// set the tag arm target to the given position
	void setTagArmTarget( ofxVec3f p );
	ofxVec3f getTagArmTarget();
	
	/// set the root position to the given position
	void setRootPosition( ofxVec3f p ) { root_pos.set(p.x,p.y,p.z); }
	
//private:
	
	// move the root by the given amount relative to the current root pos
	void moveRootRelativeX( float x );
	
	Cal3DModel model;
	IKCharacter character;
	
	string head, tag_arm, other_arm;
	
	CalVector root_pos;
	CalVector root_target_pos;
	
	float move_speed;

	
	
};

