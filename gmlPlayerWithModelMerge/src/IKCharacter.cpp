/*
 *  IKCharacter.cpp
 *  ikChain
 *
 *  Created by damian on 25/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "IKCharacter.h"
#include <queue>
using namespace std;

int IKCharacter::debug_bone = -1;

#include "ofMain.h"

void IKCharacter::setup( CalSkeleton* cal_skel, bool _auto_root_follow )
{
	skeleton = cal_skel;
	auto_root_follow = _auto_root_follow;
	
	
	// find leaf bones
	deque<int> current;
	vector<int> roots = cal_skel->getCoreSkeleton()->getVectorRootCoreBoneId();
	current.insert( current.begin(), roots.begin(), roots.end() );
	while( !current.empty() )
	{
		CalCoreBone* check = skeleton->getCoreSkeleton()->getCoreBone( current.front() );
		list<int> children = check->getListChildId();
		if ( children.size()==0 )
		{
			// this is a leaf
			leaf_bones.push_back( current.front() );
			printf("found leaf: %s\n", skeleton->getCoreSkeleton()->getCoreBone( leaf_bones.back() )->getName().c_str() );
		}
		else
		{
			// not a leaf
			current.insert( current.end(), children.begin(), children.end() );
		}
		
		// store length
		// default non-zero
		bone_lengths[current.front()] = 0.0001f;
		if ( check->getParentId()!= -1 )
		{
			CalCoreBone* parent = skeleton->getCoreSkeleton()->getCoreBone( check->getParentId() );
			CalVector delta = check->getTranslationAbsolute() - parent->getTranslationAbsolute();
			bone_lengths[check->getParentId()] = delta.length();
		}
	
		// cached rotations
		debug_cached_rotations[current.front()] = CalQuaternion();
		
		current.pop_front();
	}

	// set all weights to default
	vector<CalCoreBone*> all_bones = skeleton->getCoreSkeleton()->getVectorCoreBone();
	for( int i=0; i<all_bones.size() ;i++ )
	{
		weight_centres[all_bones[i]->getId()] = 0.5f;
	}	
	
	// release all leaves
	for ( int i=0; i<leaf_bones.size() ;i++ )
	{
		weight_centres[leaf_bones[i]] = 0;
	}
	// pin all roots
	for ( int i=0; i<roots.size(); i++ )
	{
		weight_centres[roots[i]] = 1;
	}
	
	// fetch world positions from Cal3D model
	pullWorldPositions();
	
	// set targets
	/*
	for ( int i=0; i<leaf_bones.size(); i++ )
	{
		CalVector p = world_positions[leaf_bones[i]];
		setTarget( leaf_bones[i], ofxVec3f(p.x,p.y,p.z) );
	}*/
	
	// 
	setupMagicIgnoringRotationOffsets();
	
}


void IKCharacter::setTarget( int which_leaf, ofxVec3f pos )
{
	CalVector p(pos.x, pos.y, pos.z );
	leaf_targets[which_leaf].second = p;
}

void IKCharacter::pullWorldPositions( int root_id, int leaf_id )
{
	CalVector root_pos;
	if ( auto_root_follow )
	{
		// get root pos
		int skel_root_id = skeleton->getCoreSkeleton()->getVectorRootCoreBoneId()[0];
		root_pos = skeleton->getBone(skel_root_id)->getTranslationAbsolute();
	}
	
	// work from the leaves down to the root
	deque<int> queue;
	if ( leaf_id == -1 )
		queue.insert( queue.begin(), leaf_bones.begin(), leaf_bones.end() );
	else
		queue.push_back( leaf_id );
	while ( !queue.empty() )
	{
		int id = queue.front();
		queue.pop_front();
		CalBone* bone = skeleton->getBone(id);
		world_positions[id] = bone->getTranslationAbsolute() - root_pos;
		int parent_id = bone->getCoreBone()->getParentId();
		if ( parent_id != -1 && id != root_id )
			queue.push_back( parent_id );
	}
	
}


/// calculate a rotation to bring the current direction vector between parent and bone
/// to the given new direction vector (non-normalized)
ofxQuaternion IKCharacter::getRotationForParentBone( int bone_id, CalVector new_parent_to_bone_direction )
{
	CalCoreSkeleton* core_skel = skeleton->getCoreSkeleton();
	CalBone* bone = skeleton->getBone( bone_id );
	int parent_id = core_skel->getCoreBone( bone_id )->getParentId();
	CalBone* parent_bone = skeleton->getBone( parent_id );
	int parent_parent_id = parent_bone->getCoreBone()->getParentId();
	// don't rotate the base state
	if ( parent_parent_id == -1 )
		return ofxQuaternion();


	// new_parent_to_bone_direction is currently in world-space
	// we need to bring it into the space of the parent bone
	CalQuaternion bone_space_rot = bone->getRotationBoneSpace();
	bone_space_rot.invert();
	new_parent_to_bone_direction *= bone_space_rot;

	CalVector old_dir = bone->getTranslation();
	CalVector new_dir = new_parent_to_bone_direction;
	old_dir.normalize();
	new_dir.normalize();
	
	
	// rotate from one to the other
	ofxQuaternion rot;
	ofxVec3f od( old_dir.x, old_dir.y, old_dir.z );
	ofxVec3f nd( new_dir.x, new_dir.y, new_dir.z );

	rot.makeRotate( od, nd );
	
	// return
	return rot.inverse();
}

void IKCharacter::setupMagicIgnoringRotationOffsets()
{
	pullWorldPositions();

	// start at the roots
	vector<int> roots = skeleton->getCoreSkeleton()->getVectorRootCoreBoneId();
	deque<int> parent_queue;
	parent_queue.insert( parent_queue.begin(), roots.begin(), roots.end() );
	while ( !parent_queue.empty() )
	{
		// get parent bone id
		int parent_id = parent_queue.front();
		parent_queue.pop_front();
		// get the bone
		CalBone* parent_bone = skeleton->getBone( parent_id );
		// get child list
		list<int> children = parent_bone->getCoreBone()->getListChildId();
		// no children?
		if ( children.size() == 0 )
			continue;
		
		// calculate a result for each child
		vector<ofxQuaternion> results;
		for ( list<int>::iterator it = children.begin();
			 it != children.end();
			 it++ )
		{
			// get new world positions
			CalVector b,p;
			int child_id = *it;
			b = world_positions[child_id];
			p = world_positions[parent_id];
			// get the rotation
			ofxQuaternion rot = getRotationForParentBone( child_id, (b-p) );
			
			// store as average
			results.push_back( rot );
		}
		// slerp
		vector<ofxQuaternion> other_results;
		while ( results.size() > 1 )
		{
			other_results.clear();
			for ( int i=0; i<results.size(); i+=2 )
			{
				other_results.push_back( ofxQuaternion() );
				other_results.back().slerp( 0.5f, results[0], results[1] );
			}
			results.swap( other_results );
		}
		ofxQuaternion rot( results[0] );
		CalQuaternion rot_cal( rot.x(), rot.y(), rot.z(), rot.w() );
		rot_cal.invert();
		magic_ignoring_rotation_offset[parent_id] = rot_cal;
		
		// add all children
		for ( list<int>::iterator it =children.begin() ;it != children.end(); ++it )
		{
			parent_queue.push_back( *it );
		}
		
	}
}


void IKCharacter::pushWorldPositions( bool re_solve, float weight )
{
//	skeleton->clearState();
	CalCoreSkeleton* core_skeleton = skeleton->getCoreSkeleton();
	
	// populate the parent queue
	deque<int> parent_queue;

	// get the (leaf, root) target pairs
	vector<TargetPair > target_pairs = getEnabledTargetPairs();
	// walk from leaves down to their roots, only marking where we want to have influence
	map<int,deque<int> > leaf_backtracks;
	for ( int i=0; i<target_pairs.size(); i++ )
	{
		int leaf = target_pairs[i].leaf;
		int root = target_pairs[i].root;
		int curr = leaf;
		// backtrack to root and store path
		//printf("leaf backtrack for leaf %2i-root %2i: ", leaf, root );
		while ( curr != -1 && curr != root )
		{
		//	printf(" %10s", core_skeleton->getCoreBone( curr )->getName().c_str() );
			leaf_backtracks[leaf].push_front(curr);
			curr = core_skeleton->getCoreBone( curr )->getParentId();
		}
		if ( curr == -1 )
		{
			printf("can't pushWorldPositions: invalid leaf/root target pair (leaf %s root %s)\n", core_skeleton->getCoreBone( leaf )->getName().c_str(),
				   core_skeleton->getCoreBone( root )->getName().c_str() );
			return;
		}
		//printf(" %10s", core_skeleton->getCoreBone( curr )->getName().c_str() );
		leaf_backtracks[leaf].push_front(curr);
		//printf("\n");
	}
	// now we have trails backtracking from leaves back to roots

	// two-pass algorithm
	map<int,CalQuaternion>   original_orientations;
	map<int,CalQuaternion> unweighted_rotations;
	
	// start forwardtracking from the roots
	vector<int> roots = skeleton->getCoreSkeleton()->getVectorRootCoreBoneId();
	parent_queue.insert( parent_queue.begin(), roots.begin(), roots.end() );
	while ( !parent_queue.empty() )
	{
		// get parent bone id
		int parent_id = parent_queue.front();
		parent_queue.pop_front();
		
		// determine if this bone is one we want to store
		bool should_store = false;
		for ( map<int,deque<int> >::iterator it = leaf_backtracks.begin();
			 it != leaf_backtracks.end();
			 ++it )
		{
			// nothing left in this leaf backtrack?
			if ( (*it).second.empty() )
				continue;
			// check if the current parent_id is part of htis leaf_backtrack
			if ( (*it).second.front()==parent_id )
			{
/*				printf("%10s is part of leaf backtrack for %2i -> should_store\n", 
					   core_skeleton->getCoreBone( parent_id )->getName().c_str(),
					   (*it).first );*/
				// it is -- then we should store
				should_store = true;
				// move to next point on leaf_backtrack
				(*it).second.pop_front();
			}
		}
		
		// get the bone
		CalBone* parent_bone = skeleton->getBone( parent_id );
		// get child list
		list<int> children = parent_bone->getCoreBone()->getListChildId();
		// no children?
		if ( children.size() == 0 )
			continue;

		if ( !should_store )
		{
			//CalBone* bone_to_set = parent_bone;
			//bone_to_set->clearState();
		}
		else
		{
			// calculate a result for each child
			vector<ofxQuaternion> results;
			for ( list<int>::iterator it = children.begin();
				 it != children.end();
				 it++ )
			{
				// get new world positions
				CalVector b,p;
				int child_id = *it;
				b = world_positions[child_id];
				p = world_positions[parent_id];
				// get the rotation
				ofxQuaternion rot = getRotationForParentBone( child_id, (b-p) );
				
				// store as average
				results.push_back( rot );
			}
			// slerp
			vector<ofxQuaternion> other_results;
			while ( results.size() > 1 )
			{
				other_results.clear();
				for ( int i=0; i<results.size(); i+=2 )
				{
					other_results.push_back( ofxQuaternion() );
					other_results.back().slerp( 0.5f, results[0], results[1] );
				}
				results.swap( other_results );
			}
			ofxQuaternion rot( results[0] );
			CalQuaternion rot_cal( rot.x(), rot.y(), rot.z(), rot.w() );
				
			// apply the rotation
			CalBone* bone_to_set = parent_bone;
			int bone_to_set_id = bone_to_set->getCoreBone()->getId();
			rot_cal *= magic_ignoring_rotation_offset[bone_to_set->getCoreBone()->getId()];
			//bone_to_set->blendState( weight, bone_to_set->getTranslation(),bone_to_set->getRotation()*rot_cal );
			// store original
			if ( original_orientations.find( bone_to_set_id ) == original_orientations.end() )
			{
				original_orientations[bone_to_set_id] = bone_to_set->getRotation();
				unweighted_rotations[bone_to_set_id] = CalQuaternion();
			}
			// store target
			unweighted_rotations[bone_to_set_id] *= rot_cal;
			// continue solving
			bone_to_set->setRotation( bone_to_set->getRotation() * rot_cal );

			// calculate absolute + children absolute
			bone_to_set->calculateState();
			debug_cached_rotations[bone_to_set->getCoreBone()->getId()] = rot_cal;
			
			// re-solve?
			if ( re_solve )
			{
				if ( children.size() > 0 )
				{
					// get a tree going from this node down to leaf, and re-solve for just that tree
					int root_id = children.front();
					
					int leaf_id = root_id;
					while( true )
					{
						list<int>& next_children = skeleton->getCoreSkeleton()->getCoreBone( leaf_id )->getListChildId();
						if ( next_children.size() > 1 )
						{
							leaf_id = -1;
							break;
						}
						else if ( next_children.size() == 0 )
							// found leaf
							break;
						else
							// exactly one child
							leaf_id = next_children.front();
					}
					// re-solve if we should
					if ( leaf_id != -1 )
					{
						pullWorldPositions( root_id, leaf_id );
						solve( 2, root_id, leaf_id );
					}
				}
			}
		}
		
		// add all children
		for ( list<int>::iterator it =children.begin() ;it != children.end(); ++it )
		{
			parent_queue.push_back( *it );
		}
	}

	
	// second pass 
	{
		// actually do the rotations
		for ( map<int,CalQuaternion>::iterator it = original_orientations.begin();
			 it != original_orientations.end();
			 ++it )
		{
			assert( unweighted_rotations.find( (*it).first ) != unweighted_rotations.end() );

			CalQuaternion original_orientation = (*it).second;
			CalQuaternion unweighted_rotation = unweighted_rotations[(*it).first];
			CalQuaternion weighted_rotation;
			weighted_rotation.blend( weight, unweighted_rotation );
			skeleton->getBone( (*it).first )->setRotation( original_orientation*weighted_rotation );
		}

		// update skeleton
		skeleton->calculateState();
	}				   
	
	//skeleton->lockState();

}




void IKCharacter::solve( int iterations, int root_id, int leaf_id )
{
	while ( iterations>0 )
	{
		// start at leaf nodes
		deque<int> queue;
		
		// push leaf bones to target positions
		for ( int i=0; i<leaf_bones.size(); i++ )
		{
			// if we have a leaf id to use, skip all other leaves
			if ( leaf_id != -1 && leaf_id != leaf_bones[i] )
				continue;
			
			// if we have a target for this one
			if ( leaf_targets.find(leaf_bones[i]) != leaf_targets.end() )
			{
				// set it
				world_positions[leaf_bones[i]] = leaf_targets[leaf_bones[i]].second;
			}
			int parent_id = skeleton->getCoreSkeleton()->getCoreBone( leaf_bones[i] )->getParentId();
			if ( parent_id != -1 )
				queue.push_back( parent_id );
		}
		
		// queue to handle branching
		deque< int > branch_queue;
		while ( !queue.empty() || !branch_queue.empty() )
		{
			// if main queue is empty then we are ready to deal with branches
			if ( queue.empty() )
			{
				queue.push_back( branch_queue.front() );
				branch_queue.pop_front();
				continue;
			}
			
			int next_id = queue.front();
			queue.pop_front();
			// bail out if we should
			if ( root_id != -1 && next_id == root_id )
				continue;
			
			CalBone* bone = skeleton->getBone( next_id );
			list<int>& children = bone->getCoreBone()->getListChildId();
			// is this a branch?
			if ( children.size()>1 )
			{
				// still other children to process -- push to branch queue
				if ( !queue.empty() )
				{
					branch_queue.push_back( next_id );
					continue;
				}
				
				// otherwise, process branch here
				// if we're here, then all the children of this branch have been processed already
				int parent_id = bone->getCoreBone()->getParentId();
				if ( parent_id != -1 )
				{
					//printf("averaging %lu positions for %s wc %f: ", children.size(), bone->getCoreBone()->getName().c_str(), getWeightCentre( next_id ) );
					// fetch all child positions
					vector<CalVector> results;
					results.insert( results.begin(), children.size(), world_positions[next_id] );
					int count=0;
					for ( list<int>::iterator it = children.begin(); it != children.end(); ++it,++count ) 
					{
						
						// child_pos
						CalVector& b_c = world_positions[*it];
						// current pos
						CalVector& b_p = results[count];

						// now, the bone is the wrong length. correct its length to fulfil size constraint.
						CalVector delta = b_c - b_p;
						float length = delta.length();
						length = max( 0.00001f, length );
						// pointing from parent to child
						CalVector direction = delta/length;
						
						CalCoreBone* child_bone = skeleton->getCoreSkeleton()->getCoreBone( *it );
						CalVector rest_delta = bone->getCoreBone()->getTranslationAbsolute() - child_bone->getTranslationAbsolute();
						float desired_length = rest_delta.length();
						float delta_length = desired_length - length;
						
						// balance according to weight_centre
						float weight_centre = getWeightCentre(next_id);
						
						// move
						b_c += weight_centre * delta_length * direction;
						b_p -= (1.0f-weight_centre) * delta_length * direction;
						
						//printf("%s %f (%f %f %f), ", child_bone->getName().c_str(), delta_length, b_p.x, b_p.y, b_p.z );
						
					}
					//printf("\n");
					
					// now average
					CalVector average;
					for ( int i=0; i<results.size(); i++ )
					{
						average += results[i];
					}
					average /= results.size();

					// store
					world_positions[next_id] = average;
					
					// add parent
					queue.push_back( parent_id );
				}				
				
			}
			else
			{
				// children.size() is exactly 1
				assert( children.size()==1 );
				int child_id = children.front();
				
				// child_pos
				CalVector& b_c = world_positions[child_id];
				// current pos
				CalVector& b_p = world_positions[next_id];
				
				// now, the bone is the wrong length. correct its length to fulfil size constraint.
				float desired_length = getBoneLength(next_id);
				CalVector delta = b_c - b_p;
				float length = delta.length();
				length = max( 0.00001f, length );
				length = min( desired_length*1.5f, length );
				// pointing from parent to child
				CalVector direction = delta/length;
				
				float delta_length = desired_length - length;
				
				// balance according to weight_centre
				float weight_centre = getWeightCentre(next_id);
				
				// move
				b_c += weight_centre * delta_length * direction;
				b_p -= (1.0f-weight_centre) * delta_length * direction;
				
				
				// add parent
				int parent_id = bone->getCoreBone()->getParentId();
				if ( parent_id != -1 )
					queue.push_back( parent_id );
			}
		}	
		
		iterations--;
	}
	
	
}





void IKCharacter::draw( int bone_id, float scale, bool additional_drawing )
{
	CalBone* bone = skeleton->getBone( bone_id );
	int parent_id = bone->getCoreBone()->getParentId();
	if ( parent_id != -1 )
	{
		// current
		CalBone* parent = skeleton->getBone( parent_id );
		glBegin( GL_LINES );
		CalVector v = parent->getTranslationAbsolute();
		v*=scale;
		CalVector c = v;
		glVertex3f( v.x, v.y, v.z );
		v = bone->getTranslationAbsolute();
		v*=scale;
		c += v;
		c /= 2.0f;
		glVertex3f( v.x, v.y, v.z );
		glEnd();

		// world
		glPushAttrib( GL_CURRENT_BIT );
		glColor3f( 0.1f, 0.8f, 0.1f );
		glBegin( GL_LINES );
		v = world_positions[parent_id];
		v*=scale;
		glVertex3f( v.x, v.y, v.z );
		v = world_positions[bone_id];
		v*=scale;
		glVertex3f( v.x, v.y, v.z );
		glEnd();
		glPopAttrib();


		if ( additional_drawing )
		{
			glPushAttrib( GL_CURRENT_BIT );
			glBegin( GL_LINES );
			// core
			glColor3f( (parent_id==debug_bone)?1.0f:0.3f, 0.3f, 0.5f );
			v = parent->getCoreBone()->getTranslationAbsolute();
			v*=scale;
			glVertex3f( v.x, v.y, v.z );
			v = bone->getCoreBone()->getTranslationAbsolute();
			v*=scale;
			glVertex3f( v.x, v.y, v.z );
			glEnd();
			

			// draw rotation
			glPushMatrix();
			CalVector root = bone->getCoreBone()->getTranslationAbsolute();
			glTranslatef( root.x, root.y, root.z );
			CalVector rot;
			
			rot.set( 0, 0.1f*scale, 0 );
			rot *= bone->getCoreBone()->getRotationAbsolute();
			ofEnableAlphaBlending();
			glColor4f( 0.2f, 0.2f, 0.8f, 0.2f );
			glBegin( GL_TRIANGLES );
			//glVertex3f( 0,0,0 );
			glVertex3f( rot.x, rot.y, rot.z );
			rot *= debug_cached_rotations[bone_id];
			glVertex3f( rot.x, rot.y, rot.z );
			glEnd();
			
			glColor4f( 0.2f, 0.2f, 0.8f, 0.8f );
			rot.set( 0, 0.1f*scale, 0 );
			rot *= bone->getCoreBone()->getRotationAbsolute();
			glBegin( GL_LINES );
			glVertex3f( 0,0,0 );
			glVertex3f( rot.x, rot.y, rot.z );
			rot *= debug_cached_rotations[bone_id];
			glVertex3f( 0,0,0 );
			glVertex3f( rot.x, rot.y, rot.z );
			glEnd();
			
			ofDisableAlphaBlending();
			
			glPopMatrix();
			
			CalVector u( 0, 0.1f*scale, 0);
			u *= bone->getRotationAbsolute();
			CalVector r( 0.1f*scale, 0, 0 );
			r *= bone->getRotationAbsolute();
			CalVector f( 0, 0, 0.1f*scale );
			f *= bone->getRotationAbsolute();
			
			
			// right blue
			glPushMatrix();
			root = bone->getTranslationAbsolute();
			glTranslatef( root.x, root.y, root.z );
			glBegin( GL_LINES );
			glColor3f( 0, 0, 1 );
			glVertex3f( 0,0,0 );
			glVertex3f( r.x, r.y, r.z );
			// up red
			glColor3f( 1, 0, 0 );
			glVertex3f( 0,0,0 );
			glVertex3f( u.x, u.y, u.z );
			// forward green
			glColor3f( 0, 1, 0 );
			glVertex3f( 0,0,0 );
			glVertex3f( f.x, f.y, f.z );
			glEnd();
			glPopMatrix();

			// right blue
			glPushMatrix();
			root = world_positions[bone_id];
			glTranslatef( root.x, root.y, root.z );
			glBegin( GL_LINES );
			glColor3f( 0, 0, 1 );
			glVertex3f( 0,0,0 );
			glVertex3f( r.x, r.y, r.z );
			// up red
			glColor3f( 1, 0, 0 );
			glVertex3f( 0,0,0 );
			glVertex3f( u.x, u.y, u.z );
			// forward green
			glColor3f( 0, 1, 0 );
			glVertex3f( 0,0,0 );
			glVertex3f( f.x, f.y, f.z );
			glEnd();
			glPopMatrix();
			
			glPopAttrib();
		}
		
	}
	
	
	list<int> children = bone->getCoreBone()->getListChildId();
	for ( list<int>::iterator it = children.begin(); 
		 it != children.end();
		 ++it )
	{
		draw( *it, scale, additional_drawing );
	}
	
	
}

void IKCharacter::draw( float scale,  bool additional_drawing )
{
	vector<int> roots = skeleton->getCoreSkeleton()->getVectorRootCoreBoneId();
	for ( int i=0; i<roots.size(); i++ )
	{
		draw( roots[i], scale, additional_drawing );
	}

	glPushMatrix();
	glScalef( scale, scale, scale );
	glPushAttrib( GL_CURRENT_BIT );
	glColor3f( 0.8f, 0.1f, 0.1f );
	glBegin( GL_TRIANGLES );
	for ( map<int,pair<int,CalVector> >::iterator it = leaf_targets.begin();
		 it != leaf_targets.end(); 
		 ++it )
	{
		CalVector target_pos = (*it).second.second;
		glVertex3f( target_pos.x-0.1, target_pos.y-0.05, target_pos.z );
		glVertex3f( target_pos.x+0.1, target_pos.y-0.05, target_pos.z );
		glVertex3f( target_pos.x, target_pos.y+0.05, target_pos.z );
	}
	glEnd();
	glPopAttrib();
	glPopMatrix();
	
}




bool IKCharacter::enableTargetFor( string bone, string root )
{
	int bone_id = skeleton->getCoreSkeleton()->getCoreBoneId( bone );
	if ( bone_id == -1 )
	{
		printf("couldn't find bone_id for bone name %s\n", bone.c_str() );
		return false;
	}
	int root_id = skeleton->getCoreSkeleton()->getCoreBoneId( root );
	if ( root_id == -1 )
	{
		printf("couldn't find bone_id for bone name %s\n", root.c_str() );
		return false;
	}
	
	leaf_targets[bone_id]=make_pair(root_id,skeleton->getCoreSkeleton()->getCoreBone( bone_id )->getTranslationAbsolute());
	return true;
}


void IKCharacter::disableTargetFor( string bone )
{
	int bone_id = skeleton->getCoreSkeleton()->getCoreBoneId( bone );
	LeafTargets::iterator it = leaf_targets.find( bone_id );
	if ( it != leaf_targets.end() )
		leaf_targets.erase( it );
}

int IKCharacter::getTargetId( string name ) const
{
	int id = skeleton->getCoreSkeleton()->getCoreBoneId( name );
	if ( leaf_targets.find( id ) != leaf_targets.end() )
		return id;
	else
		return -1;
}


vector< IKCharacter::TargetPair > IKCharacter::getEnabledTargetPairs()
{
	vector<TargetPair > pairs;
	for ( map<int,pair<int,CalVector> >::iterator it = leaf_targets.begin();
		 it != leaf_targets.end();
		 ++it )
	{
		pairs.push_back( TargetPair( (*it).first, (*it).second.first ) );
	}
	return pairs;
}
