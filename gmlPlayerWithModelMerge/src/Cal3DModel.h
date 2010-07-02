/*
 *  Cal3DModel.h
 *  ikChain
 *
 *  Created by damian on 22/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

#include "cal3d/cal3d.h"
#include "cal3d/animcallback.h"
#include "ofxVectorMath.h"
#include <set>
using namespace std;

class Cal3DModel
{
public:
	Cal3DModel();
	~Cal3DModel();
	
	/// setup a new CalCoreModel with the given name, skeleton file and mesh file
	bool setup( string name, string skeleton_file, string mesh_file );
	/// load animation from anim_file and bind it to the name anim_name; return true on success
	bool loadAnimation( const string& anim_file, const string& anim_name );
	/// instantiate the loaded model. do _AFTER_ loading animations.
	bool createInstance();

	/// reset skeleton to rest position
	void resetToRest();

	/// call update on the underlying model
	void update( float elapsed ) { instance->update( elapsed ); }
	/// update the mesh based on the current skeleton state
	void updateMesh( );
	/// update the animation with the given timestep
	void updateAnimation( float elapsed );
	
	
	/// start the given animation looping
	void startCycle( string name, float weight=1.0f );
	/// stop the given animation 
	void stopCycle( string name );
	/// play the given animation once
	void doAction( string name, float weight=1.0f );
	/// true if the given animation has just looped; 
	/// if a loop has just happed put the previous root position in prev_root_pos if non-NULL
	bool animationDidLoop( string name, CalVector* prev_root_pos=NULL );
	/// true if the given animation has just finished
	bool actionDidFinish( string name, CalVector* final_root_pos=NULL );

	/// draw the model
	void draw( bool wireframe = false, float scale = 1.0f );	
	/// draw just the bones of the model (for debugging)
	void drawBones( float scale = 1.0f );
	
	/// return the location of the root bone in skeleton space
	CalVector getRootBonePosition();
	/// return the location of the requested bone in skeleton space
	CalVector getBonePosition( string bone_name );
	
	/// access to the skeleton
	/// if updating bone position/orientation, call updateMesh to push changes
	/// to the mesh.
	CalSkeleton* getSkeleton() { return instance->getSkeleton(); }
	/// dump the loaded skeleton structure to the console
	void dumpSkeleton();
	
	/// rotate the given bone about x by the given angle
	void rotateBoneX( int id, float amount );

	/// callback for animation completion
	void actionComplete( string name );
	
private:
	

	int num_bones;
	
	CalCoreModel* model;
	CalModel* instance;
	int mesh_id;

	void dumpSkeletonImp( CalCoreBone* root, string prefix );
	void drawBones(  int bone_id, float scale );
	
	// map of anim names to cycle times and root positions
	map<string,pair<float,CalVector> > prev_cycle_times;
	CalVector prev_root_bone_position;
	set<string> action_just_finished;
	
	// callback
	struct AnimCallback: CalAnimationCallback
	{
		AnimCallback( string _name, Cal3DModel* _model ): CalAnimationCallback() { name = _name; model = _model; }
		virtual void AnimationUpdate(float anim_time,CalModel *model, void * userData) {};
		virtual void AnimationComplete(CalModel *model, void * userData);
		string name;
		Cal3DModel* model;
	};
	vector<AnimCallback*> callbacks;

	
};
