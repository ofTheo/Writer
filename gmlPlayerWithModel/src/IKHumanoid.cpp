/*
 *  IKHumanoid.cpp
 *  emptyExample
 *
 *  Created by damian on 20/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "IKHumanoid.h"
#include "ofMain.h"
#include <assert.h>
#include "Cal3d.h"

// 1 = don't invert, -1 = invert
#define QINV 1
#define VINV 1

#define UP ofxVec3f(0,0,1)


static IKHumanoid::Component COMPONENT_LIST[5] = { 
	IKHumanoid::C_SPINE, 
	IKHumanoid::C_ARM_L, 
	IKHumanoid::C_ARM_R, 
	IKHumanoid::C_LEG_L, 
	IKHumanoid::C_LEG_R 
};

void IKHumanoid::setup( float scale, int num_spine_bones )
{
	for ( int i=0; i<5; i++ )
	{
		setBaseOffsetFor( COMPONENT_LIST[i], ofxVec3f(0,0,0) );
	}
	ofxQuaternion up;
	
	// spine
	float spine_joint_length = (1.4f)/float(num_spine_bones);
	for( int i=0; i<num_spine_bones; i++ )
	{
		spine.push_back( IKBone( spine_joint_length*scale, up, 0.5f ) );
	}
	// head
	spine.push_back( IKBone( 0.9f*scale, up, 0.5f ) );

	// convert spine to cartesian coordinates
	vector<ofxVec3f> spine_pos = toCartesianSpace( C_SPINE );
	// set head target pos
	setTargetPos( C_SPINE, spine_pos.back() );
	
	// store branch locations
	spine_leg_branch = 0;
	spine_arm_branch = num_spine_bones;

	// arms
	arms[0].resize( 4 );
	arms[1].resize( 4 );
	for ( int i= 0; i<2;i ++ )
	{
		float dir = ((i==0)?(-1.0f):(1.0f));
		vector<ofxVec3f> pos;
		// neck joint
		pos.push_back( spine_pos[spine_arm_branch] );
		// shoulder
		ofxVec3f delta = ofxVec3f( dir*1.0f, -0.1f, 0 )*scale;
		arms[i][0].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// upper arm
		delta = ofxVec3f( dir*1.0f, 0.0f, 0)*scale;
		arms[i][1].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// lower arm
		delta = ofxVec3f( dir*1.5f, 0.0f, 0 )*scale;
		arms[i][2].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// hand
		delta = ofxVec3f( dir*0.001f, 0, 0 )*scale;
		arms[i][3].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// store
		Component which = (i==0?C_ARM_L:C_ARM_R);
		fromCartesianSpace( which, pos );
		setTargetPos( which, pos.back() );
/*		printf("target pos for arms is: [%5.1f %5.1f] (%5.1f %5.1f) from (%5.1f %5.1f)\n", 
			   arm_target_pos[i].x, arm_target_pos[i].y, 
			   getTargetPosFor( which ).x, getTargetPos( which ).y, 
			   pos.back().x, pos.back().y );*/
	}
	
	// legs
	legs[0].resize( 3 );
	legs[1].resize( 3 );
	for ( int i=0; i<2; i++ )
	{
		float dir = ((i==0)?(-1.0f):(1.0f));
		vector<ofxVec3f> pos;
		// hip joint
		pos.push_back( spine_pos[spine_leg_branch] );
		// thigh
		ofxVec3f delta = ofxVec3f( dir*0.5f, -0.4f, 0 )*scale;
		legs[i][0].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// knee
		delta = ofxVec3f( dir*-0.05f, -1.6f, 0 )*scale;
		legs[i][1].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// foot
		delta = ofxVec3f( dir*-0.05f, -2.2f, 0 )*scale;
		legs[i][2].setLength( delta.length() );
		pos.push_back( pos.back()+delta );
		// store
		Component which = (i==0?C_LEG_L:C_LEG_R);
		fromCartesianSpace( which, pos );
		setTargetPos( which, pos.back() );
	}
	
	// set rest angles
	for ( int i=0; i<5; i++ )
	{
		Component which = COMPONENT_LIST[i];
		vector<IKBone>& bones = getBonesFor( which );
		for ( int j=0; j<bones.size() ;j++ )
		{
			bones[j].setRestAngle( bones[j].getAngle() );
		}
	}
	
	// set angle constraints for particular joints
	// shoulders only move a very small distance
	arms[0][0].setAngleLimits( -PI/16, PI/16 );
	arms[0][1].setAngleLimits( -PI/8, PI/2 );
	arms[0][2].setAngleLimits( -PI/16, 3*PI/4 );
	arms[1][0].setAngleLimits( -PI/16, PI/16 );
	arms[1][1].setAngleLimits( -PI/2, PI/8 );
	arms[1][2].setAngleLimits( -3*PI/4, PI/16 );
	// waist to thigh doesn't move
	legs[0][0].setAngleLimits( 0, 0 );
	legs[1][0].setAngleLimits( 0, 0 );
	// waist to spine doesn't move
	spine[0].setAngleLimits( 0, 0 );

	// anchor the waist
	getBone( C_SPINE, 0 ).setWeightCentre( 1 );
	
	dump();
}

void IKHumanoid::dump()
{
	// dump
	/*
	vector<ofxVec2f> pos = toCartesianSpace( C_SPINE, 0 );
	printf("Spine: \n");
	for ( int i=0; i<pos.size(); i++ )
	{
		printf(" (%5.1f %5.1f)", pos[i].x, pos[i].y );
	}
	pos = toCartesianSpace( C_ARM_L );
	printf("\nArm_L: \n");
	for ( int i=0; i<pos.size(); i++ )
	{
		printf(" (%5.1f %5.1f)", pos[i].x, pos[i].y );
	}
	pos = toCartesianSpace( C_ARM_R );
	printf("\nArm_R: \n");
	for ( int i=0; i<pos.size(); i++ )
	{
		printf(" (%5.1f %5.1f)", pos[i].x, pos[i].y );
	}
	pos = toCartesianSpace( C_LEG_L );
	printf("\nLeg_L: \n");
	for ( int i=0; i<pos.size(); i++ )
	{
		printf(" (%5.1f %5.1f)", pos[i].x, pos[i].y );
	}
	pos = toCartesianSpace( C_LEG_R );
	printf("\nLeg_R: \n");
	for ( int i=0; i<pos.size(); i++ )
	{
		printf(" (%5.1f %5.1f)", pos[i].x, pos[i].y );
	}
*/
}

void IKHumanoid::solveSimpleChain(const vector<IKBone>& bones, 
								  vector<ofxVec3f>& bone_positions, 
								  const ofxVec3f& target_pos,
								  bool set_target )
{
	// solve using constraint relaxation
	// after http://www.ryanjuckett.com/programming/animation/22-constraint-relaxation-ik-in-2d

	// push the last bone to the target position
	//if ( true )
		bone_positions[bones.size()] = target_pos;
	/*else
	{
		// just try to point the last bone in the right direction
		ofxVec3f delta = bone_positions[bones.size()]-bone_positions[bones.size()-1];
		ofxVec3f target_delta = target_pos-bone_positions[bones.size()-1];
		bone_positions[bones.size()] = bone_positions[bones.size()-1] +
			target_delta.normalized()*bones[bones.size()-1].getLength();
	}*/

	
	for ( int bone_num = bones.size()-1; bone_num >= 0; bone_num-- )
	{
		// child_pos
		ofxVec3f& b_c = bone_positions[bone_num+1];
		// parent pos
		ofxVec3f& b_p = bone_positions[bone_num];
		
		// now, the bone is the wrong length. correct its length to fulfil size constraint.
		ofxVec3f delta = b_c - b_p;
		float length = delta.length();
		length = max( 0.00001f, length );
		// pointing from parent to child
		ofxVec3f direction = delta/length;
		
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


void IKHumanoid::solve( int iterations )
{
	// solve using constraint relaxation
	// after http://www.ryanjuckett.com/programming/animation/22-constraint-relaxation-ik-in-2d
	
	
	for ( int i=0; i<iterations; i++ )
	{
		// first put spine bones to cartesian space
		vector<ofxVec3f> spine_bone_positions = toCartesianSpace( C_SPINE );
		// now solve for the spine
		solveSimpleChain( spine, spine_bone_positions, head_target_pos, i==0 );

		// put the rest of the bones to cartesian space
		vector<ofxVec3f> arm_bone_positions[2];
		arm_bone_positions[0] = toCartesianSpace( C_ARM_L );
		arm_bone_positions[1] = toCartesianSpace( C_ARM_R );
		vector<ofxVec3f> leg_bone_positions[2];
		leg_bone_positions[0] = toCartesianSpace( C_LEG_L );
		leg_bone_positions[1] = toCartesianSpace( C_LEG_R );

		
		// next, solve the two arms, setting the neck position to be a weighted 
		// average of spine neck, arm_l neck and arm_r neck calculated positions
		solveSimpleChain( arms[0], arm_bone_positions[0], arm_target_pos[0], i==0 );
		solveSimpleChain( arms[1], arm_bone_positions[1], arm_target_pos[1], i==0 );
		/*ofxVec3f neck_pos = spine_bone_positions[spine_arm_branch]*0.8f;
		neck_pos += arm_bone_positions[0][0]*0.1f;
		neck_pos += arm_bone_positions[1][0]*0.1f;
		spine_bone_positions[spine_arm_branch] = neck_pos;
		arm_bone_positions[0][0] = neck_pos;
		arm_bone_positions[1][0] = neck_pos;*/
		
		// last, solve the two legs, setting the hip position to be a weighted 
		// average of spine hip, leg_l hip and leg_r hip
		solveSimpleChain( legs[0], leg_bone_positions[0], leg_target_pos[0], i==0 );
		solveSimpleChain( legs[1], leg_bone_positions[1], leg_target_pos[1], i==0 );
/*		ofxVec3f hip_pos = spine_bone_positions[spine_leg_branch]*0.5f;
		hip_pos += leg_bone_positions[0][0]*0.25f;
		hip_pos += leg_bone_positions[1][0]*0.25f;
		spine_bone_positions[spine_leg_branch] = hip_pos;
		leg_bone_positions[0][0] = hip_pos;
		leg_bone_positions[1][0] = hip_pos;*/

		// convert back to angles
		fromCartesianSpace( C_SPINE, spine_bone_positions );
		fromCartesianSpace( C_ARM_L, arm_bone_positions[0] );
		fromCartesianSpace( C_ARM_R, arm_bone_positions[1] );
		/*
		fromCartesianSpace( C_LEG_L, leg_bone_positions[0] );
		fromCartesianSpace( C_LEG_R, leg_bone_positions[1] );*/
		
		// constrain angles
		for ( int component = 0; component<5; component++ )
		{
			vector<IKBone>& bones = getBonesFor( COMPONENT_LIST[component] );
			for ( int j=0; j<bones.size(); j++ )
			{
				bones[j].constrainAngle();
			}
		}		
	}
}



ofxQuaternion orientationFromDirection(ofxVec3f vDirection)
{
	// Step 1. Setup basis vectors describing the rotation given the input vector and assuming an initial up direction of (0, 1, 0)
	ofxVec3f vUp(0, 1.0f, 0.0f);           // Y Up vector
	ofxVec3f vRight = vUp.cross(vDirection);    // The perpendicular vector to Up and Direction
	vUp = vDirection.cross(vRight);            // The actual up vector given the direction and the right vector
	
	// Step 2. Put the three vectors into the matrix to bulid a basis rotation matrix
	// This step isnt necessary, but im adding it because often you would want to convert from matricies to quaternions instead of vectors to quaternions
	// If you want to skip this step, you can use the vector values directly in the quaternion setup below
	ofxMatrix4x4 mBasis(vRight.x, vRight.y, vRight.z, 0.0f,
							   vUp.x, vUp.y, vUp.z, 0.0f,
							   vDirection.x, vDirection.y, vDirection.z, 0.0f,
							   0.0f, 0.0f, 0.0f, 1.0f);
	
	// Step 3. Build a quaternion from the matrix
	float w = sqrtf(1.0f + mBasis._mat[1][1] + mBasis._mat[2][2] + mBasis._mat[3][3]) /2.0f;
	float dfWScale = w * 4.0;
	float x = (float)((mBasis._mat[3][2] - mBasis._mat[2][3]) / dfWScale);
	float y = (float)((mBasis._mat[1][3] - mBasis._mat[3][1]) / dfWScale);
	float z = (float)((mBasis._mat[2][1] - mBasis._mat[1][2]) / dfWScale);
	
	ofxQuaternion qrot( x, y, z, w );
	return qrot;
}



vector<ofxVec3f> IKHumanoid::toCartesianSpace( Component which )
{
	// put all the bones into 2d space
	vector<ofxVec3f> bone_positions;

	/*// start at the root
	ofxVec3f dir = UP*getStartAngleFor(which);
	bone_positions.push_back( getRootPosFor( which )+getBaseOffsetFor(which)*getStartAngleFor(which) );
	
	
	
	vector<IKBone>& bones = getBonesFor( which );
	for ( int i=0; i<bones.size(); i++ )
	{
		// rotate our direction by the bone's angle
		dir = dir*bones[i].getAngle();
		// add on direction * length to get the end point of this bone
		ofxVec3f next_pos = bone_positions.back() + dir*bones[i].getLength();
		bone_positions.push_back( next_pos );
	}*/
	
	ofxQuaternion parent_rotation_space = getStartAngleFor( which );

	// first position is (root pos + base_offset*
	bone_positions.push_back( getRootPosFor( which ) + getBaseOffsetFor(which) );
	
	vector<IKBone>& bones = getBonesFor( which );
	for ( int i=0; i<bones.size(); i++ )
	{
		// add on direction * length to get the end point of this bone
		ofxVec3f next_pos = bone_positions.back() + UP*bones[i].getLength()*parent_rotation_space*bones[i].getAngle();
		bone_positions.push_back( next_pos );
		parent_rotation_space *= bones[i].getAngle();
	}		
	
	return bone_positions;
}


void IKHumanoid::fromCartesianSpace( Component which, vector<ofxVec3f>& bone_positions )
{
	saved_cartesian_import[which] = bone_positions;
	
	vector<IKBone>& bones = getBonesFor( which );
	//assert( bone_positions.size() == bones.size()+1 );
	
	ofxQuaternion parent_rotation_space = getStartAngleFor( which );
	
	for ( int i=0; i<bones.size(); i++ )
	{
		// get bone parent->child delta
		ofxVec3f bone_delta = bone_positions[i+1]-bone_positions[i];
		// convert bone delta to a direction
		ofxVec3f curr_dir = bone_delta.normalized();

		// now bring this direction into the parent bone's space
		curr_dir = curr_dir*parent_rotation_space.inverse();
		
		// now we have the delta relative to the parent
		
		// make a direction vector for rest,
		// and calculate a quaternion to bring rest_dir to bone_dir
		ofxVec3f rest_dir = UP*bones[i].getRestAngle();
		ofxQuaternion rot;
		rot.makeRotate( rest_dir, curr_dir );
		
		// store
		bones[i].setAngle( bones[i].getRestAngle()*rot );
		
		// apply for next time
		parent_rotation_space *= rot;
	
	}		
		/*
		
		 
		 // starting direction is root->base
		 // which is expressed as a direction in root bone space
		 ofxVec3f dir = UP*getStartAngleFor(which);
		
		 for ( int i=0; i<bones.size(); i++ )
		 {
		 // get bone parent->child delta
		 ofxVec3f bone_delta = bone_positions[i+1]-bone_positions[i];
		 // convert bone delta to a direction
		 bone_delta.normalize();
		 // -> angle
		ofxQuaternion angle;
		
		angle.makeRotate( dir, bone_delta );
		//float angle = bone_delta.angleRad( dir );
		bones[i].setAngle( angle );
		// rotate our direction by the bone's angle (angles are all relative)
		dir = dir*angle;
		//dir.rotateRad( -angle );*/
		
	
	
	
}




void IKHumanoid::draw( int x, int y )
{
	ofPushMatrix();
	ofTranslate( x, y, 0 );
	for ( int i=0; i<5; i++ )
	{
		Component which;
		switch(i)
		{
			case 0:
				which = C_SPINE;
				break;
			case 1:
				which = C_ARM_L;
				break;
			case 2:
				which = C_ARM_R;
				break;
			case 3:
				which = C_LEG_L;
				break;
			case 4:
				which = C_LEG_R;
				break;
			default:
				which = C_SPINE;
				break;
		}
		
		vector<ofxVec3f> bone_positions = toCartesianSpace( which );
	
		ofSetColor( 128, 128, 128 );
		for ( int i=0; i<bone_positions.size(); i++ )
		{
			if ( i < bone_positions.size()-1 )
			{
				glBegin( GL_LINES );
				glVertex3f( bone_positions[i].x, bone_positions[i].y, bone_positions[i].z );
				glVertex3f( bone_positions[i+1].x, bone_positions[i+1].y, bone_positions[i+1].z );
/*				ofLine( bone_positions[i].x, bone_positions[i].y, 
					   bone_positions[i+1].x, bone_positions[i+1].y );*/
				glEnd();

				if ( model != NULL )
				{
					// draw an orientation vector for this bone
					int bone_id = mapping.getBoneId( which, i );
					ofxVec3f dir( 0, 1, 0 );
					CalQuaternion r = model->getSkeleton()->getBone( bone_id )->getRotation();
					dir = dir*ofxQuaternion( QINV*r.x, QINV*r.y, QINV*r.z, r.w );
					dir *= 0.2f;
					
					glPushMatrix();
					glTranslatef( bone_positions[i].x, bone_positions[i].y, bone_positions[i].z );
					glBegin( GL_LINES );
					glVertex3f( 0, 0, 0 );
					glVertex3f( dir.x, dir.y, dir.z );
					glEnd();
					glPopMatrix();
				}
			}
			
			
			
		}
		ofSetColor( 255, 128, 128 );
		ofxVec3f target_pos = getTargetPosFor( which );
		ofPushMatrix();
		ofNoFill();
		glBegin( GL_TRIANGLES );
		glVertex3f( target_pos.x-0.1, target_pos.y-0.05, target_pos.z );
		glVertex3f( target_pos.x+0.1, target_pos.y-0.05, target_pos.z );
		glVertex3f( target_pos.x, target_pos.y+0.05, target_pos.z );
		glEnd();
		ofPopMatrix();
	}
	

	for ( int c=0; c<5; c++ )
	{
		Component which = COMPONENT_LIST[c];
		glColor3f( 0.8, 0.2, 0.2f );
		glBegin( GL_LINE_STRIP );
		for ( int i=0;i<saved_cartesian_import[which].size(); i++) 
		{
			ofxVec3f p = saved_cartesian_import[which][i];
			glVertex3f( p.x, p.y, p.z );
		}
		glEnd();
	}	
	ofPopMatrix();
}


vector<IKBone>& IKHumanoid::getBonesFor( Component which )
{
	switch( which ) 
	{
		case C_SPINE:
			return spine;
		case C_ARM_L:
			return arms[0];
		case C_ARM_R:
			return arms[1];
		case C_LEG_L:
			return legs[0];
		case C_LEG_R:
			return legs[1];
		default:
			return spine;
	}
}

ofxVec3f& IKHumanoid::getTargetPosFor( Component which )
{
	switch( which ) 
	{
		case C_SPINE:
			return head_target_pos;
		case C_ARM_L:
			return arm_target_pos[0];
		case C_ARM_R:
			return arm_target_pos[1];
		case C_LEG_L:
			return leg_target_pos[0];
		case C_LEG_R:
			return leg_target_pos[1];
		default:
			return head_target_pos;
	}
}

ofxVec3f IKHumanoid::getRootPosFor( Component which )
{
	if ( which == C_SPINE )
		return ofxVec3f(0,0,0);
	else
	{
		vector<ofxVec3f> pos = toCartesianSpace( C_SPINE );
		switch (which)
		{
			case C_ARM_R:
			case C_ARM_L:
				return pos[spine_arm_branch];
			case C_LEG_L:
			case C_LEG_R:
				return ofxVec3f(0,0,0);
			default:
				return ofxVec3f(0,0,0);
		}
	}
}

void IKHumanoid::resetToRest()
{
	for ( int i=0; i<5; i++ )
	{
		vector<IKBone>& bones = getBonesFor( COMPONENT_LIST[i] );
		for ( int j=0; j<bones.size(); j++ )
		{
			bones[j].setAngle( bones[j].getRestAngle() );
		}
	}
}

ofxQuaternion IKHumanoid::getStartAngleFor( Component which )
{
	ofxQuaternion angle = root_angle;
	
	switch(which)
	{
		case C_SPINE:
			return angle;
		case C_ARM_R:
		case C_ARM_L:
			for ( int i=0; i<=spine_arm_branch; i++ )
				angle *= spine[i].getAngle();
			return angle;
		case C_LEG_R:
		case C_LEG_L:
			return angle;
		default:
			return angle;
	}
}


IKHumanoid::Cal3DModelMapping::Cal3DModelMapping()
{}

bool IKHumanoid::Cal3DModelMapping::setup( CalCoreSkeleton* skeleton, map< IKHumanoid::Component, pair<string,string> > bone_names,
									 string root_name, string spine_arm_attach_name )
{
	spine_arm_attach_id = skeleton->getCoreBoneId( spine_arm_attach_name );
	if ( spine_arm_attach_id == -1 )
	{
		printf("couldn't map from spine arm attach '%s' to id\n", spine_arm_attach_name.c_str() );
		return false;
	}
	root_id = skeleton->getCoreBoneId( root_name );
	if ( root_id == -1 )
	{
		printf("couldn't map from root '%s' to id\n", spine_arm_attach_name.c_str() );
		return false;
	}
	
	// go through the components
	for ( int c=0; c<5; c++ )
	{
		Component which = COMPONENT_LIST[c];
		string base_name = bone_names[which].first;
		string tip_name = bone_names[which].second;
		
		// start at the tip then work backward to the base
		int base_id = skeleton->getCoreBoneId( base_name );
		int tip_id = skeleton->getCoreBoneId( tip_name );
		int curr = tip_id;
		while( curr != -1 && curr != base_id )
		{
			bones[which].insert( bones[which].begin(), curr );
			curr = skeleton->getCoreBone( curr )->getParentId();
		}
		bones[which].insert( bones[which].begin(), curr );
		if ( curr == -1 )
		{
			printf("problem occurred finding cal3d boneId chain from '%s' (%i) to '%s' (%i)\n", base_name.c_str(), base_id,
				   tip_name.c_str(), tip_id );
			printf("chain is currently: [");
			for ( int i=0; i<bones[which].size(); i++ )
			{
				printf(" %2i", bones[which][i]);
			}
			printf(" ]\n");
			return false;
		}
		printf("chain from %s to %s : [", base_name.c_str(), tip_name.c_str() );
		for ( int i=0; i<bones[which].size(); i++ )
		{
			printf(" %2i:%s ", bones[which][i], skeleton->getCoreBone( bones[which][i] )->getName().c_str() );
		}
		printf(" ]\n");
		
	}		

	return true;
	// done
}


//ofxVec3f& operator=(ofxVec3f& a, const CalVector& b) { a.set( b.x, b.y, b.z ); return *a; }

void IKHumanoid::updateCal3DModel()
{
	CalSkeleton* skeleton = model->getSkeleton();
	
	// update root
	skeleton->getBone( mapping.getRootBoneId() )->setRotation( CalQuaternion( QINV*root_angle.x(), QINV*root_angle.y(), QINV*root_angle.z(), root_angle.w() ) );
	//skeleton->getBone( mapping.getRootBoneId() )->setTranslation( CalVector( VINV*root_pos.x, root_pos.y, root_pos.z ) );
	
	// run through all our bones + push angles to the cal3d model
	for ( int c=0; c<5; c++ )
	{
		
		Component which = COMPONENT_LIST[c];
		vector< IKBone>& bones = getBonesFor( which );
		
		for ( int i=0; i<bones.size(); i++ )
		{
			int cal3d_bone_id = mapping.getBoneId( which, i );
			CalBone* cal_bone = skeleton->getBone( cal3d_bone_id );
			ofxQuaternion rot = bones[i].getAngle();
			cal_bone->setRotation( CalQuaternion( QINV*rot.x(), QINV*rot.y(), QINV*rot.z(), rot.w() ) );
		}
		
	}
	
}

void IKHumanoid::setupFromCal3DModel( Cal3DModel* m, const Cal3DModelMapping& _mapping )
{
	setup();
	model = m;
	mapping = _mapping;

	CalSkeleton* skeleton = m->getSkeleton();

	// set root pos and angle
	CalCoreBone* root_root_bone = skeleton->getCoreSkeleton()->getCoreBone( mapping.getRootBoneId() );
	CalVector root_pos_cal = root_root_bone->getTranslationAbsolute();
	CalQuaternion root_angle_cal = root_root_bone->getRotationAbsolute();
	//root_pos.set( VINV*root_pos_cal.x, root_pos_cal.y, root_pos_cal.z );
	root_angle.set( QINV*root_angle_cal.x, QINV*root_angle_cal.y, QINV*root_angle_cal.z, root_angle_cal.w );
	
	 

	// loop through all components
	for ( int c=0; c<5; c++ )
	{
		// we want to fetch all the bones in this component, read off their absolute translation and rotation
		// and set on our own model.
		Component which = COMPONENT_LIST[c];
		vector<IKBone>& bones = getBonesFor( which );
		
		// set to right size
		bones.resize( mapping.getNumBones( which ) );
		vector<ofxVec3f> positions;
		positions.resize( mapping.getNumBones( which )+1 );
		
		
		for ( int i=0; i<mapping.getNumBones( which ); i++ )
		{
			// get bone
			CalBone* bone = skeleton->getBone( mapping.getBoneId( which, i ) );
			// read off angle
			CalQuaternion angle = bone->getRotation();
			// convert to oF
			ofxQuaternion angle_of( QINV*angle.x, QINV*angle.y, QINV*angle.z, angle.w );
			// store
			bones[i].setRestAngle( angle_of );
		}

		// set root offset
		int base_bone_id = mapping.getBoneIdBase( which );
		// get base bone position
		CalCoreBone* base_bone = skeleton->getCoreSkeleton()->getCoreBone( mapping.getBoneIdBase( which ) );
		int parent_id = base_bone->getParentId();
		CalCoreBone* root_bone;
		if ( parent_id == -1 )
			root_bone = base_bone;
		else
			root_bone = skeleton->getCoreSkeleton()->getCoreBone( parent_id );
		CalVector base_pos_cal = base_bone->getTranslationAbsolute();
		CalVector root_pos_cal = root_bone->getTranslationAbsolute();
		ofxVec3f base_pos( VINV*base_pos_cal.x, base_pos_cal.y, base_pos_cal.z );
		ofxVec3f root_pos( VINV*root_pos_cal.x,root_pos_cal.y,root_pos_cal.z);
		setBaseOffsetFor( which, base_pos-root_pos );
		
		positions[0] = base_pos;
		// go through bones
		for ( int i=1; i<mapping.getNumBones( which ); i++ )
		{
			// get bone 
			CalBone* bone = skeleton->getBone( mapping.getBoneId( which, i ) );
			// read off translation
			CalVector p = bone->getTranslationAbsolute();
			positions[i].set( VINV*p.x, p.y, p.z );
			printf(" ( %7.3f %7.3f %7.3f ) %s\n", VINV*p.x, p.y, p.z, bone->getCoreBone()->getName().c_str() );
			// read off length
			bones[i-1].setLength( (positions[i]-positions[i-1]).length() );
		}
		// set final bone to 0 length
		bones[mapping.getNumBones(which)-1].setLength( bones[mapping.getNumBones(which)-2].getLength()*0.001f );
		
		// pin to the start
		bones[0].setWeightCentre( 0.0f );
		
		// set
		//fromCartesianSpace( which, positions );
	}	

	// set current positions as default
	setCurrentAsRest();

}

		   
	
void IKHumanoid::setCurrentAsRest()
{
	for (int i=0; i<5; i++ )
	{
		Component which = COMPONENT_LIST[i];
		vector<IKBone>& bones = getBonesFor( which );
		for ( int j=0; j<bones.size(); j++ )
		{
			bones[j].setCurrentAsRest();
		}
		vector<ofxVec3f> pos = toCartesianSpace( which );
		setTargetPos( which, pos.back() );
	}
}


