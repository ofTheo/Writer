/*
 *  IKBone.h
 *  emptyExample
 *
 *  Created by damian on 18/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

#include "ofxVectorMath.h"
#include "ofMain.h"

class IKBone
{
public:
	IKBone() { length = 1; weight_centre = 0.5f; setRestAngle(ofxQuaternion()); };

	// weight_centre determines which end of the bone moves:
	// 0   = the start (==parent link) only 
	// 1   = the end   (==child link) only
	// 0.5 = start and end both move the same amount
	IKBone( float _length, ofxQuaternion _angle, float _weight_centre )
		: length(_length), angle(_angle), weight_centre(_weight_centre)
		{};
	/// set rest angle and extension in either direction. 
	void setRestAngle( ofxQuaternion a /*, float min = -PI/2, float max = PI/2*/ ) 
		{ rest_angle = a; /*min_angle = a+min; max_angle=a+max;*/ }
	void setAngleLimits( float min = -PI/2, float max = PI/2 )
		{ /*min_angle = rest_angle+min; max_angle = rest_angle+max;*/ }

	ofxQuaternion getAngle() const { return angle; }
	ofxQuaternion getRestAngle() const { return rest_angle; }
	float getLength() const { return length; }
	float getWeightCentre() const { return weight_centre; }
	void setAngle( ofxQuaternion a ) { angle = a; }
	void setLength( float l ) { length = l; }
	void setWeightCentre( float wc ) { weight_centre = wc; }
	
	void constrainAngle();
	
	// set current pose as rest pose
	void setCurrentAsRest() { rest_angle = angle; }
	
private:
	
	
	float length, weight_centre;
	ofxQuaternion angle;
	ofxQuaternion rest_angle;
	float min_angle;
	float max_angle;

};
