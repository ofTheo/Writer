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
static const float COMFORT_DISTANCE_THRESH = 1.0f;

const static string WALK_ANIM = "walk";
const static string IDLE_ANIM = "idle";
const static string SIDESTEP_L_ANIM = "sidestep_l";
const static string SIDESTEP_R_ANIM = "sidestep_r";

void IKTagger::setup()
{
	bool loaded_model = model.setup( "test", "man_good/man_good.xsf", "man_good/man_goodMan.xmf" );
	//	bool loaded_model = model.setup( "doc", "doc/doctmp.csf", "doc/docmale.cmf" );
	if ( !loaded_model )
	{
		printf("couldn't load model\n");
		assert(false);
	}
	
	model.loadAnimation( "man_good/man_goodWalk.xaf", WALK_ANIM );
	model.loadAnimation( "man_good/man_goodIdle_baked.xaf", IDLE_ANIM );
	model.loadAnimation( "man_good/man_goodSidestep_baked_l_2.xaf", SIDESTEP_L_ANIM );
	model.loadAnimation( "man_good/man_goodSidestep_baked_r_2.xaf", SIDESTEP_R_ANIM );

	model.createInstance();
	
	model.startCycle( IDLE_ANIM, 0.2f );
	
	head = "Head";
	tag_arm = "Hand.r";
	other_arm = "Hand.l";
	string root = "Root";
	string neck = "Spine.1";
	
	character.setup( model.getSkeleton(), /* auto follow root */ false );
	//character.enableTargetFor( head, root );
	character.enableTargetFor( tag_arm, neck );
	//character.disableTargetFor( other_arm );
		
	target_offset.set( 0, 0, 3.5 );
	setRootPosition( CalVector(0,0,0) );
	setTagArmTarget( ofxVec3f( 0,0,0 ) );
	move_speed = 0.0f;
	store_sidestep_start_root_pos = false;
	sidestep_running = false;
}


void IKTagger::setTagArmTarget( ofxVec3f target )
{
	ofxVec3f target_relative = target-ofxVec3f( root_pos.x, root_pos.y, root_pos.z );
	character.setTarget( tag_arm, target_relative+target_offset );
	// work out x distance
/*	float delta_x = target.x-root.x;
	if ( delta_x > */
	
	// comfortable?
/*	float relative_x = target_relative.x - COMFORT_CENTRE.x;
	float relative_y = target_relative.y - COMFORT_CENTRE.y;
	float x_discomfort = relative_x / COMFORT_R;
	float y_discomfort = min(1.0f,relative_y / COMFORT_R);
	float discomfort = fabsf(x_discomfort) + y_discomfort*y_discomfort;*/
	
	
}


void IKTagger::moveRootRelativeX( float x )
{
	root_target_pos = root_pos;
	root_target_pos.x += x;
}




ofxVec3f IKTagger::getTagArmTarget()
{
	ofxVec3f target_relative = character.getTarget( tag_arm );
	ofxVec3f target = ofxVec3f( root_pos.x, root_pos.y, root_pos.z )+target_relative-target_offset;
	return target;

}


void IKTagger::setRootPosition( CalVector new_root_pos )
{
	ofxVec3f tag_arm_target = getTagArmTarget();
	root_pos = new_root_pos;
	setTagArmTarget( tag_arm_target );
}	

void IKTagger::update( float elapsed )
{
	// deal with moving root
	CalVector target_delta = root_target_pos-root_pos;
	float target_delta_length = target_delta.length();
	if ( target_delta_length > 0.1f )
	{
		// need to move root
		CalVector direction = target_delta / target_delta_length;
		setRootPosition( root_pos+direction*SLIDE_SPEED*move_speed*elapsed );
	}
	
	model.updateAnimation( elapsed );
	CalVector finish_root_pos;
	if ( sidestep_running && model.actionDidFinish( SIDESTEP_L_ANIM, &finish_root_pos ) ||
		model.actionDidFinish( SIDESTEP_R_ANIM, &finish_root_pos )  )
	{
		sidestep_running = false;
	/*	CalVector& ss = sidestep_start_root_pos;
		CalVector& f = finish_root_pos;
		printf("sidestep_start_root_pos %f %f %f, finish_root_pos %f %f %f\n", ss.x, ss.y, ss.z, f.x, f.y, f.z );*/
		setRootPosition( root_pos+(finish_root_pos-sidestep_start_root_pos) );
	}
	if ( store_sidestep_start_root_pos )
	{
		sidestep_start_root_pos = model.getRootBonePosition();
		store_sidestep_start_root_pos = false;
	}
	
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

	

	CalVector arm_actual_position = model.getBonePosition( tag_arm );
	ofxVec3f arm_target = character.getTarget(tag_arm);
	CalVector bone_target_delta = CalVector(arm_target.x,arm_target.y,arm_target.z)-arm_actual_position;
	float distance = bone_target_delta.length();
	float discomfort = distance/COMFORT_DISTANCE_THRESH;
	if ( discomfort > 1.0f && !sidestep_running )
	{
		// need to move feet
		//moveRootRelativeX( arm_target.x );
		//move_speed = (discomfort*discomfort)-1.0f;
		store_sidestep_start_root_pos = true;
		model.doAction( bone_target_delta.x<0?SIDESTEP_L_ANIM:SIDESTEP_R_ANIM, 1.0f );
		sidestep_running = true;
	}
	last_discomfort = discomfort;
	
}


void IKTagger::draw()
{
	glPushMatrix();
	glTranslatef( root_pos.x, root_pos.y, root_pos.z );
	glTranslatef( -target_offset.x, -target_offset.y, -target_offset.z );
	bool draw_extended = false;
	character.draw( 1.0f, draw_extended );

	glRotatef( 180, 0, 1, 0 );
	glRotatef( -90, 1, 0, 0 );
	// swap left handed to right handed
	glScalef( -1, 1, 1 );
	bool draw_wireframe = true;
	model.draw( draw_wireframe );
	glPopMatrix();
}

