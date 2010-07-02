/*
 *  simpleStroke.h
 *  allAddonsExample
 *
 *  Created by theo on 30/06/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
 
 #pragma once
 #include "ofMain.h"

class simplePoint{
	public:
		float x, y, t;
};

class simpleStroke{

	public:
		vector <simplePoint> pts;
};


class simpleTag{
	public:
	vector <simpleStroke> strokes;
	
	simpleTag(){
		numPts	= 0;
		drawPct = 0.0;
	}
	
	int totalNumPoints(){
		numPts = 0;
		for(int k = 0; k < strokes.size(); k++){
			numPts += strokes[k].pts.size();
		}
		return numPts;
	}
	
	void setPct(float pct){
		drawPct = pct;
	}
	
	ofPoint getFirstPoint(){
		if( strokes.size() ){
			for(int i = 0; i < strokes.size(); i++){
				if( strokes[i].pts.size() ){
					return ofPoint(strokes[i].pts.front().x, strokes[i].pts.front().y);
				}
			}
		}
		return ofPoint();
	}
	
	void draw(float x, float y, float w, float h){
			
		int drawToo		= drawPct * (float)numPts;
		int totalPts	= 0;
		bool bEnd		= false;
		
		for(int k = 0; k < strokes.size(); k++){
			simpleStroke & s = strokes[k];
			
			ofSetColor(0, 0, 0);
			ofBeginShape();
				for(int i = 0; i < s.pts.size(); i++){
					ofVertex(x + s.pts[i].x*w, y + s.pts[i].y*h);
					if( totalPts >= drawToo ){
						currentPt.set( s.pts[i].x, s.pts[i].y );
						bEnd = true;
						break;
					}
					totalPts++;
				}
			ofEndShape(false);
			
			if( bEnd ){
				break;
			}
		}
	}
	
	ofPoint currentPt;
	int numPts;
	float drawPct;
};