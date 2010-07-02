/*
 *  appUtils.h
 *  ikChainWithGML
 *
 *  Created by theo on 02/07/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
 
 #include "ofxXmlSettings.h"
 #include "ofMain.h"
 #include "simpleStroke.h"

//--------------------------------------------------------------
static void loadGml(string gmlFile, simpleTag & tag, bool bSwapXY){
	
	ofxXmlSettings xml;
	
	if( xml.loadFile(gmlFile) ){
	
		
		if( xml.pushTag("GML") ){
			if( xml.pushTag("tag") ){
				if( xml.pushTag("drawing") ){
				
					int numStrokes = xml.getNumTags("stroke");
					printf("num strokes is %i\n", numStrokes);
		
					tag.strokes.assign(numStrokes, simpleStroke());
					for(int k = 0; k < numStrokes; k++){
						if( xml.pushTag("stroke", k) ){
							
							int numPts = xml.getNumTags("pt");
							
							printf("num pts is %i\n", numPts);
							
							tag.strokes[k].pts.assign(numPts, simplePoint());
							for(int j = 0; j < numPts; j++){
							
								if( bSwapXY ){
									tag.strokes[k].pts[j].y = xml.getValue("pt:x", 0.0f, j);
									tag.strokes[k].pts[j].x = xml.getValue("pt:y", 0.0f, j);
								}else{
									tag.strokes[k].pts[j].x = xml.getValue("pt:x", 0.0f, j);
									tag.strokes[k].pts[j].y = xml.getValue("pt:y", 0.0f, j);								
								}
								tag.strokes[k].pts[j].t = xml.getValue("pt:t", 0.0f, j);
							}
							xml.popTag();
						}
					}

					xml.popTag();
				}

				xml.popTag();
			}
			
			xml.popTag();
		}

	}else{
		printf("unable to load %s\n", gmlFile.c_str());
	}
	
}