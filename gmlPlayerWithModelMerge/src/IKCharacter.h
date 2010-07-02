/*
 *  IKCharacter.h
 *  ikChain
 *
 *  Created by damian on 25/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#pragma once

#include "cal3d.h"
#include <vector>
using namespace std;
#include "ofxVectorMath.h"

class IKCharacter
{
public:
	void setup( CalSkeleton* cal_skel, bool auto_root_follow = true );

	/// if additional_drawing is true, draw lots of extra markers
	void draw( float scale=1.0f, bool additional_drawing=false );

	/// enable ik target from the given leaf to the given root
	bool enableTargetFor( string leaf_name, string root );
	/// disable ik target from the given leaf
	void disableTargetFor( string leaf_name );
	/// get/set ik target handles
	void setTarget( string leaf_name, ofxVec3f target ) { setTarget( getTargetId( leaf_name ), target ); }
	void setTarget( int which_leaf_bone_id, ofxVec3f target );
	ofxVec3f getTarget( string leaf_name ) const { return getTarget( getTargetId( leaf_name ) ); }
	ofxVec3f getTarget( int which_leaf_bone_id ) const { CalVector t = (*leaf_targets.find(which_leaf_bone_id)).second.second; return ofxVec3f( t.x, t.y, t.z ); }

	/// get the bone id for this leaf
	int getLeafId( int index ) { return leaf_bones.at(index); }
	int getTargetId( string name ) const;	
	
	

	/// start_id is the root id to try to solve down to; if -1, solve everything
	/// leaf_id is the leaf bone_id to solve from; if -1, use all leaves
	void solve( int iterations, int start_id = -1, int leaf_bone_id = -1 );
	
	/// fetch the current model pose as a start point for IK
	void pullFromModel() { setupMagicIgnoringRotationOffsets(); /* does a pullWorldPositions internally */ }
	/// push the found IK solution pose to the model
	void pushToModel( bool do_re_solve=false ) { pushWorldPositions( do_re_solve ); }
	
	static int debug_bone;

private:
	/// get world positions from Cal3D skeleton
	void pullWorldPositions() { pullWorldPositions( -1, -1 ); }
	/// push world positions to Cal3D skeleton
	/// if re_solve is true, try to re-solve the ik where possible
	void pushWorldPositions( bool re_solve=false );
	
	// pull world positions starting at leaf_bone_id and working up to root_id
	// if leaf_bone_id == -1, use all leaves
	// if root_id == -1, go to the actual root
	void pullWorldPositions( int root_id, int leaf_bone_id );
	
	float getWeightCentre( int id ) { return (*weight_centres.find(id)).second; }
	float getBoneLength( int id ) { return (*bone_lengths.find(id)).second; }
	
	/// return a rotation that will rotate the parent of bone_id in such a way as 
	/// to bring it to the same orientation as described by the direction vector from new_parent_pos_world
	/// to new_bone_pos_world
	ofxQuaternion getRotationForParentBone( int bone_id, CalVector new_parent_to_bone_direction );
	
	/// draw from the given bone id down
	void draw( int bone_id, float scale, bool additional_drawing );
	
	// return enabled targets and their root termination points (or -1)
	struct TargetPair{ int leaf; int root; TargetPair(int l,int r):leaf(l),root(r) {} };
	vector< TargetPair > getEnabledTargetPairs();

	vector<int> leaf_bones;
	typedef map<int,pair<int,CalVector> > LeafTargets;
	LeafTargets leaf_targets; // map from leaf bone ids to root termination bones and target points
	map<int,float> bone_lengths;    // map from bone ids to lengths
	map<int,float> weight_centres;  // map from bone ids to weight centres; 0 means pin to parent

	bool auto_root_follow;
	
	CalSkeleton* skeleton;
	
	void swapWorldPositionHandedness();
	map<int,CalVector> world_positions;
	
	map<int,CalQuaternion> debug_cached_rotations;

	void setupMagicIgnoringRotationOffsets();
	map<int,CalQuaternion> magic_ignoring_rotation_offset;
	
};
