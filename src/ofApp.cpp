#include "ofApp.h"

/*
 Build Phases の Run Scriptを以下に書き換える
 
 cp -f ../../../libs/fmodex/lib/osx/libfmodex.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libfmodex.dylib"; install_name_tool -change ./libfmodex.dylib @executable_path/libfmodex.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";
 
 cp -f ../../../addons/ofxLeapMotion2/libs/lib/osx/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libLeap.dylib"; install_name_tool -change ./libLeap.dylib @executable_path/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";
 
 mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/"
 cp -f "$ICON_FILE" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/"

*/

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_VERBOSE);
    mode=false;
    leap.open();
    
    cam.setOrientation(ofPoint(-20, 0, 0));
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    font.loadFont("SS_Adec2.0_main.otf", 150);
    
    ofHideCursor();
    ofToggleFullscreen();
}

//--------------------------------------------------------------
void ofApp::update(){
    fingersFound.clear();
    simpleHands = leap.getSimpleHands();//simpleHandsはベクター
    if( leap.isFrameNew() && simpleHands.size() ){//LeapMotionのフレームが更新されるかつ、ひとつ以上の手が検出されたとき
        
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
        
        for(int i = 0; i < simpleHands.size(); i++){
            for (int f=0; f<5; f++) {
                int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
                ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
                ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
                ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
                ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
                fingersFound.push_back(id);
            }
        }
    }
    leap.markFrameAsOld();
    
    
    if(simpleHands.size()==2&&preHandNum){//0or1→2本を認識したとき
        startCount=true;
    }else{
        startCount=false;
    }
    
    if(simpleHands.size()!=2){//手が2本でないときpreHandNumをtrueに
        preHandNum=true;
        mode=false;
    }else{
        preHandNum=false;
    }
    
    if(startCount==true){////0or1→2本を認識したときのスイッチ
        mode=true;
        timeStart=ofGetElapsedTimef();
        drop=false;//丸が落ちたままの状態
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackgroundGradient(ofColor(255), ofColor(200),  OF_GRADIENT_BAR);
    
    ofSetColor(200);
    ofDrawBitmapString("ofxLeapMotion - Example App\nLeap Connected? " + ofToString(leap.isConnected()), 20, 20);
//    light.enable();
    cam.begin();
    
//    ofPushMatrix();
//    ofRotate(90, 0, 0, 1);
//    ofSetColor(20);
//    ofDrawGridPlane(800, 20, false);
//    ofPopMatrix();
    
    fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    
    for(int i = 0; i < simpleHands.size(); i++){//手の数だけfor文を回す
        bool isLeft        = simpleHands[i].isLeft;
        ofPoint handPos    = simpleHands[i].handPos;
        hand[i]=handPos;
        ofPoint handNormal = simpleHands[i].handNormal;
        
        ofSetColor(0,100);
        ofDrawSphere(handPos.x, handPos.y, handPos.z, 20);//手の中心の座標に円を描画
        
        //手の法線ベクトルを描画
//        ofSetColor(255);
//        ofDrawArrow(handPos, handPos + 100*handNormal);
        
        for (int f=0; f<5; f++) {//手の関節を描画するためのfor文
            ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip;  // fingertip
            
            //手の関節を描画
            ofSetColor(120, 100);//手の関節の色
            ofDrawSphere(mcp.x, mcp.y, mcp.z, 15);
            ofDrawSphere(pip.x, pip.y, pip.z, 15);
            ofDrawSphere(dip.x, dip.y, dip.z, 15);
            ofDrawSphere(tip.x, tip.y, tip.z, 15);
            
            //手の骨を描画
            ofSetColor(150,100);//手の骨の色
            ofSetLineWidth(10);
            ofLine(mcp.x, mcp.y, mcp.z, pip.x, pip.y, pip.z);
            ofLine(pip.x, pip.y, pip.z, dip.x, dip.y, dip.z);
            ofLine(dip.x, dip.y, dip.z, tip.x, tip.y, tip.z);
        }//手の関節を描画するためのfor文
    }//手の数だけfor文を回す
    
    if(simpleHands.size()==2&&mode==true){//手を認識するまでの五秒間
        gravity=0;
        ofSetColor(20,50);
        rad = ofDist(hand[0].x,hand[1].x, hand[0].y,hand[1].y);
        rad+=sqrt(pow( hand[1].z,2)+pow( hand[0].z,2));
        ofDrawSphere((hand[0].x+hand[1].x)/2, (hand[0].y+hand[1].y)/2, (hand[0].z+hand[1].z)/2,rad/2-20);
        
    }else if(drop==true){//落ちた状態
        gravity+=5;
        if(hand[3].y+gravity>ofGetHeight()-400){
            gravity-=5;
        }
        ofSetColor(20,50);
        ofDrawSphere(hand[3].x,hand[3].y-gravity,hand[3].z,rad2/2);
    }
    
    
    
    cam.end();
//    light.enable();
    
    
    if(mode==true){//落ちるまでの五秒間
        float time = ofGetElapsedTimef()-timeStart;//残り時間をカウント
        if(time>=6){
            mode=false;
            hand[3].x=(hand[0].x+hand[1].x)/2;
            hand[3].y=(hand[0].y+hand[1].y)/2;
            hand[3].z=(hand[0].z+hand[1].z)/2;
            rad2=rad;
            drop=true;
        }
        ofSetColor(0);
        font.drawString(ofToString((int)(6-time))+"",150,250);
    }
}




void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
