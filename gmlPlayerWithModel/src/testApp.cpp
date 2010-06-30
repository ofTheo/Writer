#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	panel.setup("panel", 0, 0, 340, 700);
	panel.addPanel("gml lister", 4);
	
	lister.allowExt("gml");
	lister.listDir("gml/");
	panel.addFileLister("gml lister", &lister, 280, 200);
	panel.addSlider("drawSpeed", "drawSpeed", 0.001, 0.00001, 0.01, false);
	panel.addSlider("shiftX", "shiftX", 0, -400.0, 400.0, false);
	panel.addSlider("shiftY", "shiftY", 0, -400.0, 400.0, false);
	panel.addChartPlotter("drawPct", guiStatVarPointer("drawPct", &drawPct, GUI_VAR_FLOAT, true, 2), 200, 100, 200, -0.2, 2.0);

	panel.loadSettings("panel.xml");

	drawPct = 0.0;
	
	ofSetVerticalSync(true);
	
	loadGml(lister.getPath(0), true);
	
	testMan.setup();
	testMan.moveEye(0, 0, -6);
	
	shiftX = 0.0;
}

//--------------------------------------------------------------
void testApp::loadGml(string gmlFile, bool bSwapXY){
	
	ofxXmlSettings xml;
	
	if( xml.loadFile(gmlFile) ){
	
		int numStrokes = xml.getNumTags("GML:tag:drawing:stroke");
		
		printf("num strokes is %i\n", numStrokes);
		
		if( xml.pushTag("GML") ){
			if( xml.pushTag("tag") ){
				if( xml.pushTag("drawing") ){
		
					tag.strokes.assign(numStrokes, simpleStroke());
					for(int k = 0; k < numStrokes; k++){
						if( xml.pushTag("stroke", k) ){
							
							int numPts = xml.getNumTags("pt");
							
							printf("num pts is %i\n", numPts);
							
							tag.strokes[k].pts.assign(numPts, simplePoint());
							for(int j = 0; j < numPts; j++){
							
								if( bSwapXY ){
									tag.strokes[k].pts[j].y = 1.0-xml.getValue("pt:x", 0.0f, j);
									tag.strokes[k].pts[j].x = xml.getValue("pt:y", 0.0f, j);
								}else{
									tag.strokes[k].pts[j].x = xml.getValue("pt:x", 0.0f, j);
									tag.strokes[k].pts[j].y = xml.getValue("pt:y", 0.0f, j);								
								}
								tag.strokes[k].pts[j].t = xml.getValue("pt:t", 0.0f, j);
								printf("adding point!\n");
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

//--------------------------------------------------------------
void testApp::update(){
	panel.update();
	
	if( ofGetFrameNum() == 0 ){
		
		testMan.moveLeftHand(0, 0, true);
		
		for(int k = 0; k < 100; k++){
			testMan.moveLeftHand(-(float)k/2, -k*2);
		}
	}
	
	testMan.update();
	
	if( lister.selectedHasChanged() ){
		loadGml(lister.getSelectedPath(), true);
	}
	
	if( tag.totalNumPoints() ){
		tag.setPct( ofClamp(drawPct, 0, 1) );
		drawPct += panel.getValueF("drawSpeed");
		if( drawPct >= 1.5 ){
			drawPct = 0.0;
			shiftX  = 0.0;
		}
	}
	
	panel.clearAllChanged();
}


//--------------------------------------------------------------
void testApp::draw(){
	
	float tlX = 340.0 + panel.getValueF("shiftX");
	float tlY = 100.0 + panel.getValueF("shiftY");
	float w   = 640.0;
	float h   = 480.0;
	
	ofPushStyle();
	ofNoFill();
	tag.draw(tlX , tlY , w, h);
	ofPopStyle();
	
	ofCircle(tlX + tag.currentPt.x * w, tlY + tag.currentPt.y * h, 20);

	float relX = tag.currentPt.x * w;
	float relY = tag.currentPt.y * h;
	
	if( fabs(relX - shiftX) > 100.0 ){
		if( relX > shiftX ){
			shiftX = ofLerp(shiftX, relX-100, 0.2); 
		}else{
			shiftX = ofLerp(shiftX, relX+100, 0.2); 		
		}
	}
	
	relX -= shiftX;

	printf("shiftX is %f relX is %f\n", shiftX, relX);

	testMan.modelRelX = 8.0 * (float)(0.0 + shiftX)/ofGetWidth();

	testMan.moveLeftHand( relX , tag.currentPt.y * h, ofGetFrameNum() == 0 );		
	testMan.draw();
	
	ofSetupScreen();
	panel.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x, y, button);
	testMan.moveLeftHand( x, y );		

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}

