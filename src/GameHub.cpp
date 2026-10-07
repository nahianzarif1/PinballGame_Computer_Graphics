#include "GameHub.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace {
constexpr float HALF_W=6.0f, HALF_L=7.0f, HEIGHT=5.6f;
constexpr float MACHINE_SCALE=.28f;
const glm::vec3 MACHINE_ORIGIN(-3.5f,-3.8f,.45f);
const glm::vec3 FAN_CENTER(2.3f,4.1f,5.07f);
const glm::vec3 LIGHT_SWITCH(4.8f,6.54f,2.15f), FAN_SWITCH(5.35f,6.54f,2.15f);
const glm::vec3 DOOR_HINGE(-5.155f,5.50f,1.6f);
const glm::vec3 WHITE(.93f,.95f,.98f), NAVY(.018f,.045f,.13f), BLUE(.025f,.3f,1.0f);

void makeBox(GameObject& object, glm::vec3 position, glm::vec3 size, glm::vec3 color) {
    object.mesh=createCube(size.x,size.y,size.z);object.transform.position=position;object.color=color;
}
void uploadLight(unsigned shader, int index, const PointLight& light) {
    std::string p="pointLights["+std::to_string(index)+"].";
    glUniform3fv(glGetUniformLocation(shader,(p+"position").c_str()),1,&light.position[0]);
    glUniform3fv(glGetUniformLocation(shader,(p+"ambient").c_str()),1,&light.ambient[0]);
    glUniform3fv(glGetUniformLocation(shader,(p+"diffuse").c_str()),1,&light.diffuse[0]);
    glUniform3fv(glGetUniformLocation(shader,(p+"specular").c_str()),1,&light.specular[0]);
    glUniform1f(glGetUniformLocation(shader,(p+"intensity").c_str()),light.intensity);
    glUniform1f(glGetUniformLocation(shader,(p+"kc").c_str()),light.constant);
    glUniform1f(glGetUniformLocation(shader,(p+"kl").c_str()),light.linear);
    glUniform1f(glGetUniformLocation(shader,(p+"kq").c_str()),light.quadratic);
    glUniform1i(glGetUniformLocation(shader,(p+"enabled").c_str()),light.enabled);
}
Mesh beanbagMesh() {
    Mesh mesh;
    constexpr int latitudes=24,longitudes=40;
    for(int lat=0;lat<=latitudes;++lat) for(int lon=0;lon<=longitudes;++lon) {
        float a=3.14159265f*lat/latitudes,b=6.2831853f*lon/longitudes;
        float x=std::sin(a)*std::cos(b),y=std::sin(a)*std::sin(b),z=std::cos(a);
        float fold=1+.035f*std::sin(b*11+z*7)*std::pow(std::sin(a),2);
        glm::vec3 pos(x*.94f*fold,y*1.12f*fold,std::max(-.4f,z*.68f));
        if(z>0) {
            float seat=std::exp(-((x*x)/.45f+(y+.25f)*(y+.25f)/.25f));
            pos.z-=.34f*seat;
            pos.y+=z*.25f;
        }
        mesh.vertices.push_back({pos,{0,0,0},{1,1,1},{float(lon)/longitudes,float(lat)/latitudes}});
    }
    for(int lat=0;lat<latitudes;++lat) for(int lon=0;lon<longitudes;++lon) {
        unsigned a=lat*(longitudes+1)+lon,b=a+longitudes+1;
        for(unsigned i : {a,a+1,b,a+1,b+1,b})mesh.indices.push_back(i);
    }
    for(size_t i=0;i<mesh.indices.size();i+=3) {
        auto& a=mesh.vertices[mesh.indices[i]];auto& b=mesh.vertices[mesh.indices[i+1]];auto& c=mesh.vertices[mesh.indices[i+2]];
        auto n=-glm::cross(b.position-a.position,c.position-a.position);
        a.normal+=n;b.normal+=n;c.normal+=n;
    }
    for(auto& vertex : mesh.vertices)vertex.normal=glm::length(vertex.normal)>1e-6f ? glm::normalize(vertex.normal) : glm::vec3(0,0,1);
    mesh.setupMesh();return mesh;
}
float rayBox(const glm::vec3& origin,const glm::vec3& direction,const glm::vec3& center,glm::vec3 halfSize) {
    float nearDistance=0,farDistance=30;
    for(int axis=0;axis<3;++axis) {
        float low=center[axis]-halfSize[axis],high=center[axis]+halfSize[axis];
        if(std::abs(direction[axis])<1e-6f) {if(origin[axis]<low || origin[axis]>high)return -1;continue;}
        float a=(low-origin[axis])/direction[axis],b=(high-origin[axis])/direction[axis];
        if(a>b)std::swap(a,b);
        nearDistance=std::max(nearDistance,a);farDistance=std::min(farDistance,b);
        if(nearDistance>farDistance)return -1;
    }
    return nearDistance;
}
}

GameHub::GameHub() {initialize();}
GameHub::~GameHub() {hud.shutdown();}

GameObject& GameHub::addBox(const char* name,const glm::vec3& pos,const glm::vec3& size,
                           const glm::vec3& color,float emission,bool switched) {
    scenery.emplace_back(name);auto& object=scenery.back();
    makeBox(object,pos,size,color);object.emissiveStrength=emission;
    if(switched)lightSurfaces.push_back({scenery.size()-1,emission});
    return object;
}
void GameHub::addBeam(const char* name,const glm::vec3& a,const glm::vec3& b,float width,
                     const glm::vec3& color,float emission,bool switched) {
    glm::vec3 delta=b-a;
    auto& object=addBox(name,(a+b)*.5f,{glm::length(delta),width,width},color,emission,switched);
    object.transform.rotation={0,-glm::degrees(std::atan2(delta.z,glm::length(glm::vec2(delta)))),glm::degrees(std::atan2(delta.y,delta.x))};
}
void GameHub::addText(const char* text,const glm::vec3& position,float height,const glm::vec3& color) {
    scenery.emplace_back("Wall lettering");auto& object=scenery.back();object.mesh=createTextMesh(text,height);
    object.transform.position=position;object.color=color;object.emissiveStrength=.35f;
}
void GameHub::resetRoomCamera() {
    hubCamera=Camera({-2.0f,-2.5f,2.7f},{0,0,1},74.0f,-3.0f);
    hubCamera.zoom=56;hubCamera.movementSpeed=3.4f;
}
void GameHub::initialize() {
    scenery.clear();transparent.clear();lightSurfaces.clear();roomLights.clear();
    lightsOn=true;fanOn=true;dayMode=false;pinballCam=false;
    doorOpen=false;doorAngle=0;rayTracing=false;doorParts.clear();
    resetRoomCamera();
    machinePlacement=glm::translate(glm::mat4(1),MACHINE_ORIGIN);
    machinePlacement=glm::scale(machinePlacement,glm::vec3(MACHINE_SCALE));
    machine.camera.position=MACHINE_ORIGIN+machine.camera.position*MACHINE_SCALE;
    machine.camera.movementSpeed=1.6f;
    buildRoom();buildFeatureWall();buildLounge();buildCabinets();buildLighting();buildFan();buildSwitches();
    tableSpot.position=MACHINE_ORIGIN+glm::vec3(0,0,3.1f);
    tableSpot.direction={0,0,-1};tableSpot.cutOff=30;tableSpot.outerCutOff=45;
    tableSpot.intensity=2.0f;tableSpot.linear=.05f;tableSpot.quadratic=.02f;
    hud.initialize();update(0);
}

void GameHub::buildRoom() {
    scenery.emplace_back("White porcelain floor");auto& floor=scenery.back();
    floor.mesh=createPlane(12,14,3.5f);floor.color=WHITE;floor.mesh.loadTexture("white_tile");floor.shininess=120;
    floor.reflectivity=.18f;
    addBox("White ceiling",{0,0,HEIGHT+.1f},{12.2f,14.2f,.2f},WHITE);
    addBox("Left wall",{-6,0,2.8f},{.2f,14,HEIGHT},{.67f,.74f,.79f});
    addBox("Entrance wall",{0,-7,2.8f},{12,.2f,HEIGHT},{.74f,.8f,.85f});
    addBox("TV wall backing",{0,6.9f,2.8f},{12,.2f,HEIGHT},WHITE);
    addBox("Right window sill",{5.95f,0,.22f},{.16f,14,.44f},WHITE);
    addBox("Right window header",{5.95f,0,5.36f},{.16f,14,.48f},WHITE);
    for(float y : {-6.7f,-3.3f,.1f,3.5f,6.75f})
        addBox("Window frame",{5.9f,y,2.8f},{.15f,.09f,4.8f},WHITE);
    for(float z : {.5f,5.1f})addBox("Window rail",{5.9f,0,z},{.15f,13.6f,.075f},WHITE);
    transparent.emplace_back("Glass window wall");auto& glass=transparent.back();
    makeBox(glass,{5.94f,0,2.8f},{.025f,13.4f,4.6f},{.36f,.7f,.9f});glass.alpha=.12f;glass.shininess=128;
    addBox("Window exterior",{9,0,2.8f},{.1f,25,10},{.07f,.14f,.23f},.25f);
    for(float y : {-5.0f,0.0f,5.0f}) {
        addBox("Exterior blue column",{8.7f,y,2.5f},{.18f,.25f,7},{.08f,.14f,.25f});
        addBox("Exterior light",{8.55f,y,1.8f},{.03f,.35f,2.4f},{.04f,.33f,.6f},.5f);
    }
    addBox("Air conditioner",{4.9f,6.5f,5.1f},{1.1f,.5f,.55f},WHITE);
    for(int i=0;i<6;++i)addBox("AC vent",{4.9f,6.225f,4.92f+i*.035f},{.85f,.015f,.012f},{.14f,.19f,.23f});
    for(float x : {-4.9f,-2.5f,.1f,3.9f}) {
        scenery.emplace_back("Ceiling downlight");auto& object=scenery.back();object.mesh=createCylinder(.09f,.025f,18);
        object.transform.position={x,-2.0f,5.58f};object.color={.05f,.06f,.08f};
    }
    for(float y : {-5.5f,-2.0f,2.0f})addBox("Wall socket",{-5.86f,y,.32f},{.03f,.2f,.16f},WHITE);
}

void GameHub::buildFeatureWall() {
    addBox("Navy feature wall",{2.5f,6.72f,2.8f},{6.8f,.16f,5.5f},NAVY);
    auto& acoustic=addBox("Textured TV panel",{2.5f,6.61f,2.8f},{6.8f,.08f,2.95f},{.15f,.2f,.24f});
    acoustic.mesh.loadTexture("acoustic");acoustic.shininess=12;
    const glm::vec2 top[]={{-.9f,4.25f},{-.05f,4.15f},{.45f,4.52f},{1.2f,4.52f},{1.85f,5.02f},{2.6f,5.12f},{3.6f,4.02f},{5.88f,4.5f}};
    const glm::vec2 bottom[]={{-.9f,2.0f},{.25f,2.0f},{.8f,1.4f},{2.5f,1.45f},{3.45f,.58f},{5.88f,.58f}};
    auto contour=[&](const glm::vec2* points,size_t count) {
        for(size_t i=1;i<count;++i) {
            glm::vec3 a(points[i-1].x,6.48f,points[i-1].y),b(points[i].x,6.48f,points[i].y);
            addBeam("Blue LED surround",a+glm::vec3(0,.012f,0),b+glm::vec3(0,.012f,0),.09f,{.01f,.08f,.5f},1.0f,true);
            addBeam("Blue LED core",a-glm::vec3(0,.06f,0),b-glm::vec3(0,.06f,0),.035f,{.13f,.48f,1},6.0f,true);
        }
    };
    contour(top,std::size(top));contour(bottom,std::size(bottom));
    addText("TECHNOLOGY",{1.6f,6.43f,3.6f},.34f,{.62f,.78f,.84f});
    addBox("Television chassis",{1.65f,6.32f,2.55f},{3.15f,.2f,1.95f},{.025f,.026f,.04f}).shininess=80;
    auto& display=addBox("Television screen",{1.65f,6.205f,2.55f},{2.98f,.016f,1.77f},{.9f,.84f,.96f},1.3f);
    display.mesh=createScreenMesh(2.98f,1.77f);
    display.mesh.loadTexture("arena_screen");display.shininess=64;
    addText("GAME CENTER",{1.1f,6.186f,3.12f},.12f,{1,.78f,.45f});
    const char* lines[]={"01  NEON CIRCUIT","02  ARCADE CLASSICS","03  MULTIPLAYER","04  LEADERBOARD"};
    for(int i=0;i<4;++i)addText(lines[i],{.95f,6.185f,2.82f-i*.23f},.075f,{.83f,.83f,.96f});
    addBox("TV power LED",{3.05f,6.19f,1.57f},{.045f,.015f,.012f},{.15f,.8f,1},1);
    addBox("Blue lower wall panel",{2.48f,6.5f,.52f},{6.82f,.15f,.7f},{.03f,.15f,.44f});
    addBox("Violet corner lamp housing",{5.42f,6.28f,2.3f},{.1f,.13f,2.65f},{.035f,.028f,.08f});
    addBox("Violet corner lamp",{5.41f,6.20f,2.3f},{.028f,.03f,2.6f},{.32f,.035f,1},2.8f,true);
    addBox("Baseboard",{2.48f,6.47f,.11f},{6.82f,.16f,.15f},WHITE);
}

void GameHub::buildLounge() {
    scenery.emplace_back("Electric blue rug");auto& rug=scenery.back();rug.mesh=createPlane(6.8f,5.3f,1);
    rug.transform.position={1.25f,1.3f,.018f};rug.color={.86f,.92f,1};rug.mesh.loadTexture("electric_rug");rug.shininess=4;
    for(int i=0;i<2;++i) {
        scenery.emplace_back("Soft beanbag chair");auto& chair=scenery.back();chair.mesh=beanbagMesh();
        chair.transform.position={.10f+i*2.45f,.75f-i*.65f,.43f};chair.transform.rotation.z=i==0 ? -12 : 9;
        chair.transform.scale=i==0 ? glm::vec3(1.0f,1.06f,1) : glm::vec3(1.08f,1.0f,.96f);
        chair.color=i==0 ? glm::vec3(.24f,.31f,.36f) : glm::vec3(.29f,.34f,.38f);
        chair.mesh.loadTexture("upholstery");chair.shininess=5;
    }
    const glm::vec3 black(.035f,.043f,.055f);
    addBox("Black console table top",{1.65f,5.4f,.96f},{3.0f,1.07f,.13f},black).shininess=70;
    addBox("Console shelf",{1.65f,5.4f,.26f},{2.8f,.95f,.09f},black);
    for(float x : {.27f,3.03f})for(float y : {4.94f,5.85f})
        addBox("Console table leg",{x,y,.52f},{.1f,.1f,1.03f},black);
    addBox("Game console",{2.1f,5.32f,1.08f},{.62f,.38f,.11f},{.12f,.14f,.17f}).shininess=64;
    addBox("Console indicator",{2.1f,5.12f,1.10f},{.5f,.01f,.014f},BLUE,1.4f);
    for(float x : {.7f,1.25f}) {
        scenery.emplace_back("Controller");auto& pad=scenery.back();pad.mesh=createSphere(1,10,16);
        pad.transform.position={x,5.1f,1.10f};pad.transform.scale={.24f,.14f,.065f};pad.color={.1f,.12f,.16f};pad.shininess=48;
        for(float dx : {-.1f,.1f})addBox("Controller stick",{x+dx,5.08f,1.17f},{.055f,.055f,.025f},{.03f,.04f,.055f});
    }
    // Small decorative mascot beside the console, as in the photograph.
    auto orb=[&](const char* name,glm::vec3 pos,glm::vec3 size,glm::vec3 color) {
        scenery.emplace_back(name);auto& object=scenery.back();object.mesh=createSphere(1,14,20);
        object.transform.position=pos;object.transform.scale=size;object.color=color;object.shininess=8;
    };
    orb("Mascot body",{4.4f,5.45f,.71f},{.3f,.23f,.42f},{.035f,.03f,.035f});
    orb("Mascot head",{4.4f,5.43f,1.27f},{.32f,.25f,.31f},{.67f,.33f,.14f});
    orb("Mascot cap",{4.4f,5.45f,1.52f},{.34f,.28f,.11f},{.04f,.04f,.05f});
    for(float dx : {-.13f,.13f}) {
        scenery.emplace_back("Mascot leg");auto& leg=scenery.back();leg.mesh=createCylinder(.075f,.25f,16);
        leg.transform.position={4.4f+dx,5.38f,.3f};leg.color={.035f,.03f,.035f};
        orb("Mascot foot",{4.4f+dx,5.25f,.14f},{.12f,.23f,.11f},{.69f,.36f,.17f});
        orb("Mascot eye",{4.4f+dx,5.19f,1.31f},{.04f,.015f,.045f},{.03f,.025f,.025f});
    }
    addText("GAME HUB",{4.4f,5.20f,.81f},.055f,{.8f,.63f,.3f});
}

void GameHub::buildCabinets() {
    const glm::vec3 pale(.88f,.92f,.94f);
    addBox("Beverage refrigerator",{-4.66f,6.1f,1.5f},{1.14f,.9f,3.0f},{.035f,.055f,.075f}).shininess=80;
    addBox("Fridge header",{-4.66f,5.61f,2.91f},{1.03f,.04f,.17f},{.1f,.31f,.6f},1.0f);
    addBox("Fridge interior",{-4.66f,5.69f,1.58f},{.95f,.04f,2.25f},{.31f,.4f,.5f},.55f);
    for(int row=0;row<4;++row) {
        float z=.6f+row*.55f;
        addBox("Fridge shelf",{-4.66f,5.57f,z-.09f},{.93f,.32f,.027f},{.64f,.68f,.73f});
        for(int col=0;col<4;++col) {
            scenery.emplace_back("Beverage bottle");auto& bottle=scenery.back();bottle.mesh=createCylinder(.062f,.3f,12);
            bottle.transform.position={-5.0f+col*.22f,5.56f,z+.10f};
            bottle.color=col%2 ? glm::vec3(.06f,.37f,.24f) : glm::vec3(.62f,.14f,.11f);bottle.shininess=90;
            addBox("Bottle label",{-5.0f+col*.22f,5.49f,z+.09f},{.085f,.012f,.1f},pale);
        }
    }
    fridgeDoorIndex=transparent.size();
    transparent.emplace_back("Fridge glass door");auto& door=transparent.back();
    makeBox(door,{-4.66f,5.50f,1.6f},{.99f,.018f,2.4f},{.3f,.48f,.63f});door.alpha=.14f;door.shininess=100;
    auto part=[&](const char* name,glm::vec3 pos,glm::vec3 size) {
        auto& object=addBox(name,pos,size,{.63f,.67f,.72f});object.shininess=100;object.reflectivity=.45f;
        doorParts.push_back({scenery.size()-1,pos-DOOR_HINGE});
    };
    part("Fridge handle",{-4.18f,5.46f,1.52f},{.035f,.04f,.63f});
    part("Door left frame",{-5.155f,5.5f,1.6f},{.025f,.025f,2.43f});
    part("Door right frame",{-4.165f,5.5f,1.6f},{.025f,.025f,2.43f});
    part("Door top frame",{-4.66f,5.5f,2.8f},{1.015f,.025f,.025f});
    part("Door bottom frame",{-4.66f,5.5f,.4f},{1.015f,.025f,.025f});
    addBox("White cabinet",{-2.59f,6.20f,.73f},{2.78f,.95f,1.46f},pale).shininess=80;
    addBox("Countertop",{-2.59f,6.15f,1.49f},{2.9f,1.02f,.09f},WHITE).shininess=120;
    for(float x : {-3.95f,-2.6f,-1.25f})addBox("Cabinet seam",{x,5.70f,.78f},{.018f,.01f,1.35f},{.52f,.59f,.62f});
    for(float x : {-3.3f,-1.9f})addBox("Cabinet handle",{x,5.675f,1.27f},{.45f,.045f,.023f},{.6f,.65f,.7f});
    addBox("Kettle base",{-1.74f,6.06f,1.6f},{.43f,.37f,.12f},{.1f,.13f,.16f});
    scenery.emplace_back("Kettle");auto& kettle=scenery.back();kettle.mesh=createCylinder(.18f,.36f,24);
    kettle.transform.position={-1.74f,6.06f,1.82f};kettle.color={.6f,.65f,.7f};kettle.shininess=100;
    kettle.reflectivity=.5f;
    addBeam("Kettle handle",{-1.46f,6.07f,1.66f},{-1.46f,6.07f,1.99f},.05f,{.08f,.10f,.12f});
    for(int i=0;i<5;++i) {
        scenery.emplace_back("Counter drinks");auto& bottle=scenery.back();bottle.mesh=createCylinder(.065f,.31f,12);
        bottle.transform.position={-3.56f+i*.23f,6.12f,1.69f};bottle.color=i%2 ? glm::vec3(.13f,.46f,.22f) : glm::vec3(.82f,.64f,.08f);
    }
    addBox("Left wall tall cabinet",{-5.52f,-2.8f,2.2f},{.75f,1.85f,4.4f},{.40f,.5f,.56f});
    for(float z : {1.1f,2.8f})addBox("Cabinet steel handle",{-5.11f,-2.8f,z},{.025f,.35f,.035f},{.7f,.75f,.78f});
}

void GameHub::buildLighting() {
    auto light=[&](glm::vec3 position,glm::vec3 color,float intensity) {
        PointLight point;point.position=position;point.setColor(color);point.ambient=color*.045f;
        point.intensity=intensity;point.linear=.045f;point.quadratic=.025f;roomLights.push_back(point);
    };
    light({1.0f,2.8f,4.7f},{.79f,.9f,1},3.2f);
    light({-3.2f,5.2f,4.75f},{.88f,.95f,1},2.5f);
    light({2.0f,5.9f,3.9f},{.11f,.36f,1},1.5f);
    light({4.9f,5.95f,1.1f},{.24f,.09f,1},1.15f);
    light({-4.66f,5.25f,1.9f},{.5f,.74f,1},.45f);
    light(MACHINE_ORIGIN+glm::vec3(0,0,1.4f),{.8f,.88f,1},1.1f);
    light({5.2f,2.5f,3.9f},{.52f,.7f,1},.25f);
    // Suspended Y fixture from the reference, with a separate rotating fan.
    const glm::vec3 center(1.0f,2.75f,4.8f);
    for(int arm=0;arm<3;++arm) {
        float angle=glm::radians(arm*120.0f+10.0f);
        glm::vec3 end=center+glm::vec3(std::cos(angle),std::sin(angle),0)*1.13f;
        addBeam("Y pendant black frame",center,end,.21f,{.035f,.045f,.065f});
        addBeam("Y pendant LED",center-glm::vec3(0,0,.115f),end-glm::vec3(0,0,.115f),.115f,{.81f,.94f,1},3.5f,true);
        addBeam("Pendant suspension cable",end+glm::vec3(0,0,.11f),{end.x,end.y,5.6f},.012f,{.2f,.24f,.3f});
    }
    // Two square ring pendants on the cabinet side.
    for(glm::vec3 center : {glm::vec3(-3.15f,5.1f,4.87f),glm::vec3(-3.55f,1.2f,4.87f)}) {
        glm::vec3 corners[4]={{center.x-.76f,center.y-.55f,center.z},{center.x+.76f,center.y-.55f,center.z},
            {center.x+.76f,center.y+.55f,center.z},{center.x-.76f,center.y+.55f,center.z}};
        for(int i=0;i<4;++i) {
            addBeam("Square pendant frame",corners[i],corners[(i+1)%4],.15f,{.06f,.07f,.09f});
            addBeam("Square pendant LED",corners[i]-glm::vec3(0,0,.08f),corners[(i+1)%4]-glm::vec3(0,0,.08f),.055f,{.86f,.92f,1},3.0f,true);
        }
        for(int i : {0,2})addBeam("Pendant cable",corners[i],{corners[i].x,corners[i].y,5.6f},.01f,{.19f,.23f,.28f});
    }
}

void GameHub::buildFan() {
    fanRod.mesh=createCylinder(.04f,.35f,16);fanRod.transform.position=FAN_CENTER+glm::vec3(0,0,.22f);fanRod.color={.25f,.29f,.33f};
    fanHub.mesh=createCylinder(.17f,.16f,24);fanHub.transform.position=FAN_CENTER;fanHub.color={.13f,.17f,.21f};fanHub.shininess=70;
    for(auto& blade : fanBlade) {
        blade.mesh=createCube(1.28f,.22f,.035f);blade.color={.12f,.17f,.22f};blade.shininess=55;
    }
}
void GameHub::buildSwitches() {
    addBox("Light switch plate",LIGHT_SWITCH+glm::vec3(0,.035f,0),{.42f,.045f,.57f},WHITE).shininess=80;
    addBox("Fan switch plate",FAN_SWITCH+glm::vec3(0,.035f,0),{.42f,.045f,.57f},WHITE).shininess=80;
    makeBox(switchLight,LIGHT_SWITCH,{.21f,.075f,.3f},{.7f,.77f,.85f});switchLight.shininess=70;
    makeBox(switchFan,FAN_SWITCH,{.21f,.075f,.3f},{.7f,.77f,.85f});switchFan.shininess=70;
    addText("LIGHTS",LIGHT_SWITCH+glm::vec3(0,-.01f,.4f),.07f,{.65f,.82f,1});
    addText("FAN",FAN_SWITCH+glm::vec3(0,-.01f,.4f),.07f,{.65f,.82f,1});
}
void GameHub::toggleLights() {lightsOn=!lightsOn;}
void GameHub::toggleFan() {fanOn=!fanOn;}
void GameHub::toggleDoor() {doorOpen=!doorOpen;}
void GameHub::toggleRayTracing() {rayTracing=!rayTracing;}
void GameHub::setShadingMode(ShadingMode mode) {machine.setShadingMode(mode);rayTracing=false;}
void GameHub::setDayMode(bool day) {dayMode=day;}
void GameHub::toggleCamera() {pinballCam=!pinballCam;}
void GameHub::resetAll() {machine.resetGame();}
Camera& GameHub::activeCamera() {return pinballCam ? machine.camera : hubCamera;}
const Camera& GameHub::activeCamera() const {return pinballCam ? machine.camera : hubCamera;}
bool GameHub::interactSwitches(const glm::vec3& origin,const glm::vec3& direction) {
    if(pinballCam || glm::length(direction)<1e-6f)return false;
    glm::vec3 ray=glm::normalize(direction);
    const float lightDistance=rayBox(origin,ray,LIGHT_SWITCH,{.23f,.13f,.32f});
    const float fanDistance=rayBox(origin,ray,FAN_SWITCH,{.23f,.13f,.32f});
    // Pick the hinged door in its rotating local coordinate system.
    const auto& door=transparent[fridgeDoorIndex];
    glm::mat4 inverse=glm::inverse(door.transform.getModelMatrix());
    float doorDistance=rayBox(glm::vec3(inverse*glm::vec4(origin,1)),
                             glm::vec3(inverse*glm::vec4(ray,0)),{0,0,0},{.50f,.035f,1.21f});
    if(origin.y<6.6f && ray.y>0) {
        if(lightDistance>=0 && (fanDistance<0 || lightDistance<fanDistance)) {toggleLights();return true;}
        if(fanDistance>=0) {toggleFan();return true;}
    }
    if(doorDistance>=0 && origin.y<5.5f) {toggleDoor();return true;}
    return false;
}
void GameHub::updateFan(float dt) {
    float target=fanOn ? 240.0f : 0.0f;
    float change=(fanOn ? 120.0f : 160.0f)*dt;
    fanAngularSpeed+=std::clamp(target-fanAngularSpeed,-change,change);
    fanAngle=std::fmod(fanAngle+fanAngularSpeed*dt,360.0f);
    for(int i=0;i<4;++i) {
        float angle=fanAngle+i*90.0f,rad=glm::radians(angle);
        fanBlade[i].transform.position=FAN_CENTER+glm::vec3(std::cos(rad),std::sin(rad),0)*.77f;
        fanBlade[i].transform.rotation={0,0,angle};
    }
}
void GameHub::updateDoor(float dt) {
    float target=doorOpen ? 100.0f : 0.0f;
    doorAngle+=std::clamp(target-doorAngle,-100*dt,100*dt);
    glm::mat4 rotation=glm::rotate(glm::mat4(1),glm::radians(-doorAngle),glm::vec3(0,0,1));
    auto& door=transparent[fridgeDoorIndex];
    door.transform.position=DOOR_HINGE+glm::vec3(rotation*glm::vec4(.495f,0,0,0));
    door.transform.rotation.z=-doorAngle;
    for(const auto& part : doorParts) {
        auto& object=scenery[part.index];
        object.transform.position=DOOR_HINGE+glm::vec3(rotation*glm::vec4(part.offset,0));
        object.transform.rotation.z=-doorAngle;
    }
}
void GameHub::update(float dt) {
    if(!std::isfinite(dt) || dt<0)return;
    dt=std::min(dt,.25f);
    machine.update(dt);updateFan(dt);updateDoor(dt);constrainCameras();
    switchLight.transform.rotation.x=lightsOn ? -22.0f : 22.0f;
    switchFan.transform.rotation.x=fanOn ? -22.0f : 22.0f;
    switchLight.color=lightsOn ? glm::vec3(.7f,.87f,1) : glm::vec3(.37f,.43f,.5f);
    switchFan.color=fanOn ? glm::vec3(.7f,.87f,1) : glm::vec3(.37f,.43f,.5f);
    for(auto surface : lightSurfaces)scenery[surface.objectIndex].emissiveStrength=lightsOn ? surface.emission : 0;
    for(auto& object : scenery)if(object.name=="Window exterior") {
        object.color=dayMode ? glm::vec3(.42f,.64f,.86f) : glm::vec3(.07f,.14f,.23f);
        object.emissiveStrength=dayMode ? .45f : .25f;
    }
}
void GameHub::constrainCameras() {
    auto clampRoom=[](Camera& camera) {
        camera.position.x=std::clamp(camera.position.x,-HALF_W+.25f,HALF_W-.25f);
        camera.position.y=std::clamp(camera.position.y,-HALF_L+.25f,HALF_L-.25f);
        camera.position.z=std::clamp(camera.position.z,.25f,HEIGHT-.25f);
    };
    clampRoom(hubCamera);clampRoom(machine.camera);
}
glm::vec3 GameHub::skyColor() const {return dayMode ? glm::vec3(.42f,.6f,.75f) : glm::vec3(.04f,.09f,.17f);}
glm::vec3 GameHub::sceneAmbient() const {
    return lightsOn ? (dayMode ? glm::vec3(.32f,.37f,.42f) : glm::vec3(.22f,.27f,.34f)) : dayMode ? glm::vec3(.12f,.16f,.22f) : glm::vec3(.018f,.026f,.05f);
}
std::vector<PointLight> GameHub::effectiveLights() const {
    auto lights=roomLights;
    for(size_t i=0;i<lights.size();++i) {
        lights[i].enabled=i>=4 || lightsOn;
        if(i==6)lights[i].intensity=dayMode ? 2.2f : .25f;
    }
    return lights;
}
SpotLight GameHub::effectiveSpot() const {
    auto spot=tableSpot;spot.enabled=pinballCam || lightsOn;return spot;
}
RayScene GameHub::rayScene() const {
    RayScene scene;scene.ambient=sceneAmbient();scene.background=skyColor();
    scene.lights=effectiveLights();scene.spot=effectiveSpot();
    for(const auto& object : scenery)scene.addObject(object,object.transform.getModelMatrix());
    for(const GameObject* object : {&switchLight,&switchFan,&fanRod,&fanHub})scene.addObject(*object,object->transform.getModelMatrix());
    for(const auto& blade : fanBlade)scene.addObject(blade,blade.transform.getModelMatrix());
    for(const auto& object : transparent)scene.addObject(object,object.transform.getModelMatrix());
    machine.appendRayScene(scene,machinePlacement);
    return scene;
}
void GameHub::applyLighting(unsigned shader) const {
    auto viewPos=activeCamera().position;
    glUniform3fv(glGetUniformLocation(shader,"viewPos"),1,&viewPos[0]);
    glm::vec3 ambient=sceneAmbient();
    glUniform3fv(glGetUniformLocation(shader,"sceneAmbient"),1,&ambient[0]);
    glUniform1i(glGetUniformLocation(shader,"numPointLights"),static_cast<int>(roomLights.size()));
    auto lights=effectiveLights();
    for(size_t i=0;i<lights.size();++i) {
        uploadLight(shader,static_cast<int>(i),lights[i]);
    }
    auto spot=effectiveSpot();
    glUniform3fv(glGetUniformLocation(shader,"spotLight.position"),1,&spot.position[0]);
    glUniform3fv(glGetUniformLocation(shader,"spotLight.direction"),1,&spot.direction[0]);
    glUniform3fv(glGetUniformLocation(shader,"spotLight.ambient"),1,&spot.ambient[0]);
    glUniform3fv(glGetUniformLocation(shader,"spotLight.diffuse"),1,&spot.diffuse[0]);
    glUniform3fv(glGetUniformLocation(shader,"spotLight.specular"),1,&spot.specular[0]);
    glUniform1f(glGetUniformLocation(shader,"spotLight.intensity"),spot.intensity);
    glUniform1f(glGetUniformLocation(shader,"spotLight.cutOff"),spot.cutOff);
    glUniform1f(glGetUniformLocation(shader,"spotLight.outerCutOff"),spot.outerCutOff);
    glUniform1f(glGetUniformLocation(shader,"spotLight.exponent"),spot.exponent);
    glUniform1f(glGetUniformLocation(shader,"spotLight.kc"),spot.constant);
    glUniform1f(glGetUniformLocation(shader,"spotLight.kl"),spot.linear);
    glUniform1f(glGetUniformLocation(shader,"spotLight.kq"),spot.quadratic);
    glUniform1i(glGetUniformLocation(shader,"spotLight.enabled"),spot.enabled);
}
void GameHub::render(unsigned shader) {
    applyLighting(shader);
    const glm::mat4 identity(1);
    int placement=glGetUniformLocation(shader,"sceneTransform");
    glUniformMatrix4fv(placement,1,GL_FALSE,&identity[0][0]);
    for(const auto& object : scenery)object.draw(shader);
    switchLight.draw(shader);switchFan.draw(shader);
    fanRod.draw(shader);fanHub.draw(shader);
    for(const auto& blade : fanBlade)blade.draw(shader);
    glUniformMatrix4fv(placement,1,GL_FALSE,&machinePlacement[0][0]);
    machine.render(shader,false);
    glUniformMatrix4fv(placement,1,GL_FALSE,&identity[0][0]);
    // Glass is blended after opaque geometry in camera-distance order.
    std::vector<size_t> order;
    for(size_t i=0;i<transparent.size();++i)order.push_back(i);
    std::sort(order.begin(),order.end(),[&](size_t a,size_t b){
        return glm::length(transparent[a].transform.position-activeCamera().position)>glm::length(transparent[b].transform.position-activeCamera().position);
    });
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
    for(size_t i : order)transparent[i].draw(shader);
    glDepthMask(GL_TRUE);glDisable(GL_BLEND);
}
void GameHub::renderHUD(int width,int height) {
    const char* shading=rayTracing ? "RAY TRACE" : machine.getShadingMode()==ShadingMode::FLAT ? "FLAT" : machine.getShadingMode()==ShadingMode::GOURAUD ? "GOURAUD" : "PHONG";
    if(pinballCam)hud.renderConsole(width,height,machine,lightsOn,fanOn,dayMode,true,shading,rayFrameWidth);
    else hud.renderRoom(width,height,lightsOn,fanOn,dayMode,fanAngularSpeed,shading,doorOpen,doorAngle,
                        rayFrameMilliseconds,rayFrameWidth,rayRefining);
}
