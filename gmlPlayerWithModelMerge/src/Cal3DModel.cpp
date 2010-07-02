/*
 *  Cal3DModel.cpp
 *  ikChain
 *
 *  Created by damian on 22/06/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "Cal3DModel.h"
#include "ofMain.h"
#include "ofxVectorMath.h"

Cal3DModel::Cal3DModel( )
: model(0), instance(0)
{
}


bool Cal3DModel::setup( string name, string skeleton_file, string mesh_file )
{
	model = new CalCoreModel( name );
	
	if ( !model->loadCoreSkeleton( ofToDataPath(skeleton_file) ) )
	{
		printf("couldn't load skeleton %s\n", skeleton_file.c_str() );
		CalError::printLastError();
		return false;
	}
	mesh_id = model->loadCoreMesh( ofToDataPath(mesh_file) );
	if ( mesh_id == -1 )
	{
		printf("couldn't load mesh %s\n", mesh_file.c_str() );
		CalError::printLastError();
		return false;
	}
	
	num_bones = model->getCoreSkeleton()->getNumBones();
	
	return true;
}

bool Cal3DModel::createInstance()
{
	instance = new CalModel( model );
	if ( !instance->attachMesh( mesh_id ) )
	{
		printf("couldn't attach mesh %i to new model instance\n", mesh_id );
		CalError::printLastError();
		return false;
	}
	instance->update( 0.01f );
	updateMesh();

	return true;
	
}

void Cal3DModel::updateAnimation( float elapsed )
{
	prev_cycle_times.clear();
	// update previous animation times
	list<CalAnimationCycle *> cycles = instance->getMixer()->getAnimationCycle();
	for  ( list<CalAnimationCycle *> ::iterator it = cycles.begin();
           it != cycles.end();
           ++it )
    {
		string anim_name =  (*it)->getCoreAnimation()->getName();
		float time = instance->getMixer()->getAnimationTimeFor(*it);
		//printf("cycle %s: time %f\n", anim_name.c_str(), time );
		prev_cycle_times[anim_name].first = time;
		prev_cycle_times[anim_name].second = getRootBonePosition();
	}
	
	instance->getMixer()->updateAnimation( elapsed );
	instance->getMixer()->updateSkeleton();
	
}

void Cal3DModel::updateMesh( )
{
	// lock in the state
	instance->getSkeleton()->lockState();
	// update vertices of mesh
	instance->getPhysique()->update();
	
}

void Cal3DModel::dumpSkeleton()
{
	// walk through skeleton
	printf("dumping bones:\n");
	CalSkeleton* skeleton = instance->getSkeleton();
	vector<int> core_root_bones = skeleton->getCoreSkeleton()->getVectorRootCoreBoneId();
	for ( int i=0; i<core_root_bones.size(); i++ )
	{
		printf("core %i %i %s\n", i, core_root_bones[i], skeleton->getBone( core_root_bones[i] )->getCoreBone()->getName().c_str() );
		
		dumpSkeletonImp( skeleton->getBone( core_root_bones[i] )->getCoreBone(), " " );
	}
}

void Cal3DModel::dumpSkeletonImp( CalCoreBone* b, string prefix )
{
	printf("%s - %s\n", prefix.c_str(), b->getName().c_str() );
	list<int> children = b->getListChildId();
	for ( list<int>::iterator ch = children.begin(); ch != children.end(); ++ch )
	{
		dumpSkeletonImp( b->getCoreSkeleton()->getCoreBone( *ch ), prefix+"  " );
	}
}

void Cal3DModel::draw( bool wireframe, float scale )
{
	// get the renderer of the model
	CalRenderer *pCalRenderer;
	pCalRenderer = instance->getRenderer();
	
	// begin the rendering loop
	if(!pCalRenderer->beginRendering())
	{
		printf("couldn't render: error calling beginRendering()\n");
		CalError::printLastError();
		return;
	}

/*	glPushMatrix();
	glTranslatef( 0, 0, 10.0f );
	glScalef( scale, scale, scale );*/

	
	
	
	
	
	// get the number of meshes
	int meshCount;
	meshCount = pCalRenderer->getMeshCount();
	
	// loop through all meshes of the model
	int meshId;
	for(meshId = 0; meshId < meshCount; meshId++)
	{
		// get the number of submeshes
		int submeshCount;
		submeshCount = pCalRenderer->getSubmeshCount(meshId);
		
		// loop through all submeshes of the mesh
		int submeshId;
		for(submeshId = 0; submeshId < submeshCount; submeshId++)
		{
			// select mesh and submesh for further data access
			if(pCalRenderer->selectMeshSubmesh(meshId, submeshId))
			{
				// get the material colors
				unsigned char ambientColor[4], diffuseColor[4], specularColor[4];
				pCalRenderer->getAmbientColor(&ambientColor[0]);
				pCalRenderer->getDiffuseColor(&diffuseColor[0]);
				pCalRenderer->getSpecularColor(&specularColor[0]);
				
				// get the material shininess factor
				float shininess;
				shininess = pCalRenderer->getShininess();
				
				// get the transformed vertices of the submesh
				static float meshVertices[30000][3];
				assert( pCalRenderer->getVertexCount() <= 30000 );
				int vertexCount = pCalRenderer->getVertices(&meshVertices[0][0]);
				
				// get the transformed normals of the submesh
				static float meshNormals[30000][3];
				pCalRenderer->getNormals(&meshNormals[0][0]);
				
				// get the texture coordinates of the submesh
				// (only for the first map as example, others can be accessed in the same way though)
				static float meshTextureCoordinates[30000][2];
				int textureCoordinateCount;
				textureCoordinateCount = pCalRenderer->getTextureCoordinates(0, &meshTextureCoordinates[0][0]);
				
				// get the stored texture identifier
				// (only for the first map as example, others can be accessed in the same way though)
				Cal::UserData textureId;
				textureId = pCalRenderer->getMapUserData(0);
				
				// [ set the material, vertex, normal and texture states in the graphic-API here ]
				
				// get the faces of the submesh
				static int meshFaces[50000][3];
				int faceCount;
				faceCount = pCalRenderer->getFaces(&meshFaces[0][0]);
				
/*				glColor3f( 0,0,0 );
				glBegin( GL_POINTS );*/
				ofPushMatrix();
				ofSetColor( 0,0,0 );
/*				for ( int i=0; i<faceCount; i++ )
				{
//					glVertex3fv( meshVertices[i] );
					for ( int j=0; j<3; j++ )
					{
						ofLine(meshVertices[meshFaces[i][j]][0]*scale,		meshVertices[meshFaces[i][j]][2]*scale, 
							   meshVertices[meshFaces[i][(j+1)%3]][0]*scale,meshVertices[meshFaces[i][(j+1)%3]][2]*scale );
					}
					printf("%7.3f %7.3f %7.3f\n", meshVertices[i][0], meshVertices[i][1], meshVertices[i][2] );
				}*/
				ofEnableAlphaBlending();
				glColor4f( 0,0,0,0.2f );
				glPushAttrib( GL_DEPTH_BUFFER_BIT );
				glDepthMask( GL_TRUE );
				//glColor4f( 0,0,0,1 );
				//glEnable( GL_DEPTH_TEST );
				//glDepthMask( GL_TRUE );
				glBegin( GL_TRIANGLES );
				for ( int i=0; i<faceCount; i++ )
				{
					//					glVertex3fv( meshVertices[i] );
					for ( int j=0; j<3; j++ )
					{
						glNormal3f(meshNormals[meshFaces[i][j]][0],		meshNormals[meshFaces[i][j]][2],			meshNormals[meshFaces[i][j]][1]);
						glVertex3f(meshVertices[meshFaces[i][j]][0]*scale,		meshVertices[meshFaces[i][j]][2]*scale,			meshVertices[meshFaces[i][j]][1]*scale);
					}
				}
				glEnd();
				//glEnable( GL_DEPTH_TEST );
				if ( wireframe )
				{
					glEnable( GL_DEPTH_TEST );
					glBegin( GL_LINES );
					glColor4f( 1,1,1,0.8f );
					for ( int i=0; i<faceCount; i++ )
					{
						//					glVertex3fv( meshVertices[i] );
						for ( int j=0; j<3; j++ )
						{
							glVertex3f(meshVertices[meshFaces[i][j]][0]*scale,		meshVertices[meshFaces[i][j]][2]*scale,			meshVertices[meshFaces[i][j]][1]*scale);
							glVertex3f(meshVertices[meshFaces[i][(j+1)%3]][0]*scale,meshVertices[meshFaces[i][(j+1)%3]][2]*scale,	meshVertices[meshFaces[i][(j+1)%3]][1]*scale);
						}
					}
					glEnd();
				}
				glPopAttrib();
				
				ofPopMatrix();
//				glEnd();
				// [ render the faces with the graphic-API here ]
			}
		}
	}
	

//	glPopMatrix();
	
	pCalRenderer->endRendering();
}

void Cal3DModel::drawBones( int bone_id, float scale )
{
	CalSkeleton* skeleton = instance->getSkeleton();
	CalBone* bone = skeleton->getBone( bone_id );
	int parent_id = bone->getCoreBone()->getParentId();
	if ( parent_id != -1 )
	{
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

		CalVector u( 0, scale, 0);
		u *= bone->getRotationAbsolute();
		CalVector r( scale, 0, 0 );
		r *= bone->getRotationAbsolute();
		CalVector f( 0, 0, scale );
		f *= bone->getRotationAbsolute();

		
		// right blue
		glPushMatrix();
		glTranslatef( c.x, c.y, c.z );
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
		
		
	}
	
	list<int> children = bone->getCoreBone()->getListChildId();
	for ( list<int>::iterator it = children.begin(); 
		 it != children.end();
		 ++it )
	{
		drawBones( *it, scale );
	}
}

void Cal3DModel::drawBones( float scale )
{
	vector<int> roots = instance->getSkeleton()->getCoreSkeleton()->getVectorRootCoreBoneId();
	for ( int i=0; i<roots.size(); i++ )
	{
		drawBones( roots[i], scale  );
	}
}

void Cal3DModel::rotateBoneX( int id, float amount )
{
	CalBone* bone = instance->getSkeleton()->getBone( id );
	ofxQuaternion rot( amount, ofxVec3f( 0, 0, 1 ) );
	//bone->setTranslation( CalVector(0,0,0 ) );
	bone->setRotation( bone->getRotation()*CalQuaternion(rot.x(),rot.y(),rot.z(),rot.w()) );
	bone->calculateState();
}


Cal3DModel::~Cal3DModel() { 
	if ( instance )
		delete instance;
}

void Cal3DModel::resetToRest()
{
	CalSkeleton* skeleton = instance->getSkeleton();
	skeleton->clearState();
	vector<int> roots = skeleton->getCoreSkeleton()->getVectorRootCoreBoneId();
	for ( int i=0; i<roots.size(); i++ )
	{
		skeleton->getBone( roots[i] )->setCoreStateRecursive();
	}
	skeleton->calculateState();
}


bool Cal3DModel::loadAnimation( const string& anim_file, const string& anim_name )
{
	int anim_id = model->loadCoreAnimation( ofToDataPath(anim_file), anim_name );
	if ( anim_id == -1 )
	{
		printf("couldn't load anim from file '%s'\n", anim_file.c_str() );
		CalError::printLastError();
		return false;
	}
	
	printf("loaded anim '%s'\n", anim_name.c_str() );
	
	return true;
}

void Cal3DModel::startAnimation( string name, float delay, float weight )
{
	int id = instance->getCoreModel()->getCoreAnimationId( name );
	instance->getMixer()->blendCycle( id, weight, delay );
}

void Cal3DModel::stopAnimation( string name, float delay )
{
	int id = instance->getCoreModel()->getCoreAnimationId( name );
	instance->getMixer()->clearCycle( id, delay );
}

CalVector Cal3DModel::getRootBonePosition()
{
	int root_id = model->getCoreSkeleton()->getVectorRootCoreBoneId()[0];
	return instance->getSkeleton()->getBone( root_id )->getTranslationAbsolute();
}

bool Cal3DModel::animationDidLoop( string name, CalVector* root_pos )
{
	// no prev time for this anim?
	if ( prev_cycle_times.find( name ) == prev_cycle_times.end() )
		return false;
	
	// find in cycles
	list<CalAnimationCycle *> cycles = instance->getMixer()->getAnimationCycle();
	for  ( list<CalAnimationCycle *> ::iterator it = cycles.begin();
		  it != cycles.end();
		  ++it )
    {
		string anim_name =  (*it)->getCoreAnimation()->getName();
		if ( anim_name == name )
		{
			float time = instance->getMixer()->getAnimationTimeFor(*it);
			// time has rolled over?
			if ( ( time < prev_cycle_times[name].first && (*it)->getTimeFactor() > 0 ) ||
				 ( time > prev_cycle_times[name].first && (*it)->getTimeFactor() <= 0 ) )
			{
				if ( root_pos )
					*root_pos = prev_cycle_times[name].second;
				return true;
			}
		}
	}
	return false;
}


