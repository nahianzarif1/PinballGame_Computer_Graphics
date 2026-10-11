#include "GameHub.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(){try{
    GameHub room;
    require(!room.pinballCam,"Start in reference lounge view");
    for(int i=0;i<120;++i)room.update(1.0f/60);
    require(room.fanAngularSpeed>200 && room.fanAngle>0,"Ceiling fan rotates when enabled");
    float previousAngle=room.fanAngle;
    room.toggleLights();room.update(.1f);
    require(!room.lightsOn && room.fanOn && room.fanAngle!=previousAngle,"Light switch does not stop fan");
    require(room.interactSwitches({4.8f,3,2.15f},{0,1,0}),"Ray can activate visible light rocker");
    require(room.lightsOn && room.fanOn,"Light rocker changes only room lighting");
    require(room.interactSwitches({5.35f,3,2.15f},{0,1,0}),"Ray can activate visible fan rocker");
    require(!room.fanOn && room.lightsOn,"Fan rocker changes only fan power");
    for(int i=0;i<150;++i)room.update(1.0f/60);
    require(room.fanAngularSpeed==0,"Disabled fan coasts to a complete stop");
    previousAngle=room.fanAngle;room.update(.1f);
    require(room.fanAngle==previousAngle,"Stopped fan remains stationary");
    require(!room.interactSwitches({4.8f,3,2.15f},{0,-1,0}),"Missed switch ray has no effect");
    room.pinballCam=true;
    require(!room.interactSwitches({4.8f,3,2.15f},{0,1,0}),"Table controls do not activate room switches");
    room.machine.launchFromLane(18);
    for(int i=0;i<240;++i)room.update(1.0f/240);
    require(room.machine.ballState==BallState::IN_PLAY,"Room placement preserves pinball physics");
    room.toggleCamera();room.hubCamera.position={100,100,100};room.update(0);
    require(room.hubCamera.position.x<6 && room.hubCamera.position.y<7 && room.hubCamera.position.z<5.6f,"Room camera stays within walls");
    room.resetRoomCamera();require(room.hubCamera.position.y<0,"H restores reference view");
    require(!room.doorOpen && room.doorAngle==0,"Fridge starts closed");
    room.toggleDoor();room.update(.25f);
    require(room.doorOpen && room.doorAngle>0 && room.doorAngle<100,"Door opens with animation");
    room.toggleDoor();room.update(.25f);
    require(!room.doorOpen && room.doorAngle==0,"Door can reverse and close during animation");
    require(room.interactSwitches({-4.66f,3,1.6f},{0,1,0}),"Scene picking opens fridge door");
    for(int i=0;i<120;++i)room.update(1.0f/60);
    require(room.doorAngle==100,"Door reaches open stop");
    room.toggleDoor();for(int i=0;i<120;++i)room.update(1.0f/60);
    require(room.doorAngle==0,"Door reaches closed stop");
    room.toggleRayTracing();require(room.rayTracing,"Ray mode toggle");
    room.setShadingMode(ShadingMode::GOURAUD);
    require(!room.rayTracing && room.machine.getShadingMode()==ShadingMode::GOURAUD,"Gouraud selection restores raster mode");
    for(auto size : {glm::ivec2(1280,720),glm::ivec2(2560,1440),glm::ivec2(900,600)}){
        float scale=std::min(size.x*.27f/320,size.y/720.0f);
        float ox=size.x-size.x*.27f+(size.x*.27f-320*scale)*.5f,oy=(size.y-720*scale)*.5f;
        require(room.hud.roomSwitchAt(ox+150*scale,oy+500*scale,size.x,size.y)==1,"Light console switch click at window scaling");
        require(room.hud.roomSwitchAt(ox+150*scale,oy+390*scale,size.x,size.y)==2,"Fan console switch click at window scaling");
        require(room.hud.roomSwitchAt(0,0,size.x,size.y)==0,"Non-control console click is ignored");
        require(room.hud.roomSwitchAt(ox+150*scale,oy+188*scale,size.x,size.y)==6,"Ray tracing button scales correctly");
        require(room.hud.roomSwitchAt(ox+150*scale,oy+151*scale,size.x,size.y)==7,"Open/close button scales correctly");
    }
    std::cout<<"PASS: room switch picking, independent controls, fan motion and stopping, camera bounds, console scaling, pinball compatibility\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<"\n";return 1;}}
