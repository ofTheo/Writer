/*
 *  IKTagger.cpp
 *  ikChain
 *
 *  Created by damian on 01/07/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "IKTagger.h"

#include "Cal3DModel.h"

static const float SLIDE_SPEED = 2.0f;

/*
 
 at z=0, root = 0,0
 comfortable, top: -0.59, 2.93
 left: 0.319 1.158
 bottom: -0.716 -1.429
 right: -3.09 1.178
 
 defines a circle with centre at
 c: -1.019 0.959
 and radius
 
 .429  1.97	= 4.065	= 2.016    
 1.338 .199 = 1.829 = 1.352    1.965 avg
 .303  2.388= 5.794 = 2.407
 2.071 .219	= 4.337 = 2.083
 */

static const float COMFORT_CX = -1.519;
static const float COMFORT_CY = 1.259;
static const float COMFORT_R = 1.965;
static const CalVector COMFORT_CENTRE( COMFORT_CX, COMFORT_CY, 0 );

void IKTagger::setup()
{
	bool loaded_model = model.setup( "test", "man_good/man_good.xsf", "man_good/man_goodMan.xmf" );
	//	bool loaded_model = model.setup( "doc", "doc/doctmp.csf", "doc/docmale.cmf" );
	if ( !loaded_model )
	{
		printf("couldn't load model\n");
		assert(false);
	}
	bool loaded_anim = model.loadAnimation( "man_good/man_goodSkeleton.001.xaf", "walk" );
	if ( !loaded_anim )
	{
		printf("couldn't load anim\n");
	}
	model.createInstance();
	
	head = "Head";
	tag_arm = "Hand.l";
	other_arm = "Hand.r";
	string root = "Root";
	string neck = "Spine.1";
	
	character.setup( model.getSkeleton(), /* auto follow root */ false );
	character.enableTargetFor( head, root );
	character.enableTargetFor( tag_arm, neck );
	character.enableTargetFor( other_arm, neck );

	//character.disableTargetFor( other_arm );
	
	root_pos.set(0,0,0);
	move_speed = 0.0f;
}


void IKTagger::setTagArmTarget( ofxVec3f target )
{
	ofxVec3f target_relative = target-ofxVec3f( root_pos.x, root_pos.y, root_pos.z );
	character.setTarget( tag_arm, target_relative );
	// work out x distance
/*	float delta_x = target.x-root.x;
	if ( delta_x > */
	
	// comfortable?
/*	CalVector relative = CalVector(target_relative.x,target_relative.y,target_relative.z);
	relative.z = 0;
	relative.y *= relative.y;
	CalVector comfort_centre_delta = COMFORT_CENTRE - relative;
	float discomfort = comfort_centre_delta.length() / COMFORT_R;*/
	float relative_x = target_relative.x - COMFORT_CENTRE.x;
	float relative_y = target_relative.y - COMFORT_CENTRE.y;
	float x_discomfort = relative_x / COMFORT_R;
	float y_discomfort = min(1.0f,relative_y / COMFORT_R);
	float discomfort = fabsf(x_discomfort) + y_discomfort*y_discomfort;
	if ( discomfort > 1.0f )
	{
		// need to move feet
		moveRootRelativeX( relative_x );
		move_speed = (discomfort*discomfort)-1.0f;
	}
	
}

void IKTagger::setOtherArmTarget(ofxVec3f target)
{
	ofxVec3f target_relative = target-ofxVec3f( root_pos.x, root_pos.y, root_pos.z );
	character.setTarget( other_arm, target_relative );
	// work out x distance
/*	float delta_x = target.x-root.x;
	if ( delta_x > */
	
	
}

void IKTagger::moveRootRelativeX( float x )
{
	root_target_pos = root_pos;
	root_target_pos.x += x;
}




ofxVec3f IKTagger::getTagArmTarget()
{
	ofxVec3f target_relative = character.getTarget( tag_arm );
	ofxVec3f target = ofxVec3f( root_pos.x, root_pos.y, root_pos.z )+target_relative;
	return target;

}



void IKTagger::update( float elapsed )
{
	// deal with moving root
	CalVector target_delta = root_target_pos-root_pos;
	float target_delta_length = target_delta.length();
	if ( target_delta_length > 0.1f )
	{
		// need to move root
		ofxVec3f tag_arm_target = getTagArmTarget();
		CalVector direction = target_delta / target_delta_length;
		root_pos += direction*SLIDE_SPEED*move_speed*elapsed;
		setTagArmTarget( tag_arm_target );
	}
	
	model.updateAnimation( elapsed );
		/*
	CalVector loop_root_pos;
	if ( model.animationDidLoop( "walk", &loop_root_pos ) )
	{
		printf("-- loop\n");
		root_pos += (loop_root_pos)-model.getRootBonePosition();
	}*/
	character.pullFromModel();
	character.solve( 5 );
	character.pushToModel( true );
	model.updateMesh();
}


void IKTagger::draw()
{
	glPushMatrix();
	glTranslatef( root_pos.x, root_pos.y, root_pos.z );
	bool draw_extended = false;
	//character.draw( 1.0f, false );

	//glRotatef( 180, 0, 1, 0 );
	glRotatef( -90, 1, 0, 0 );
	// swap left handed to right handed
	glScalef( 1, 1, 1 );
	
	bool draw_wireframe = true;
	model.draw( true );
	glPopMatrix();
}

