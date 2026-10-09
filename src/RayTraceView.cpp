#include "RayTraceView.h"
#include "GameHub.h"
#include <glad/glad.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>

namespace {
std::uint64_t sceneSignature(const GameHub& hub) {
    // Geometry/material states. Camera and lighting have separate invalidation.
    std::uint64_t hash=14695981039346656037ull;
    auto add=[&](float value){std::uint32_t bits;std::memcpy(&bits,&value,sizeof(bits));hash^=bits;hash*=1099511628211ull;};
    add(hub.fanAngle);add(hub.doorAngle);
    const auto& m=hub.machine;
    for(int axis=0;axis<3;++axis)add(m.ball.transform.position[axis]);
    add(m.leftFlipper.angle);add(m.rightFlipper.angle);add(m.plunger.springCompression);
    for(const auto& bumper:m.bumpers)add(bumper.hitTimer>0?1.0f:0.0f);
    for(const auto& target:m.targets)add(target.down?1.0f:0.0f);
    return hash;
}
}

RayTraceView::RayTraceView() {
    shader=std::make_unique<Shader>("shaders/ray_present.vert","shaders/ray_present.frag");
    glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glGenVertexArrays(1,&vao);
    lastViewChange=std::chrono::steady_clock::now();
}
RayTraceView::~RayTraceView() {
    if(cancelJob)cancelJob->store(true,std::memory_order_relaxed);
    if(pending.valid())pending.wait();
    glDeleteTextures(1,&texture);glDeleteVertexArrays(1,&vao);
}
void RayTraceView::invalidate() {
    ++generation;ready=false;lastMilliseconds=0;imageWidth=imageSamples=0;phase=0;
    if(cancelJob)cancelJob->store(true,std::memory_order_relaxed);
    lastViewChange=std::chrono::steady_clock::now();
}
std::array<RayQuality,3> RayTraceView::qualityPlan(int viewWidth,int viewHeight,int requestedWidth,int samples,bool playing) {
    viewWidth=std::max(1,viewWidth);viewHeight=std::max(1,viewHeight);
    // Preserve aspect ratio, native size and a 2 MP CPU image budget, including
    // when the window is extremely narrow or tall.
    const int pixelBudget=2073600;
    int nativeLimit=std::max(1,std::min(viewWidth,static_cast<int>(std::sqrt(double(pixelBudget)*viewWidth/viewHeight))));
    int detail=requestedWidth>0?std::clamp(requestedWidth,1,1920):(playing?640:1280);
    detail=std::min(detail,nativeLimit);
    int finalSamples=samples>0?std::clamp(samples,1,64):(playing?1:2);
    return {{{std::min(480,detail),1},{std::min(960,detail),1},{detail,finalSamples}}};
}
void RayTraceView::upload(const RayImage& image) {
    if(image.cancelled||image.pixels.empty())return;
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    if(image.width==textureWidth&&image.height==textureHeight)
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,image.width,image.height,GL_RGB,GL_UNSIGNED_BYTE,image.pixels.data());
    else {
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,image.width,image.height,0,GL_RGB,GL_UNSIGNED_BYTE,image.pixels.data());
        textureWidth=image.width;textureHeight=image.height;
    }
    ready=true;lastMilliseconds=image.milliseconds;imageWidth=image.width;imageSamples=image.samples;
}
bool RayTraceView::update(const GameHub& hub,int viewportWidth,int viewportHeight,int rayWidth,int samples) {
    viewportWidth=std::max(1,viewportWidth);viewportHeight=std::max(1,viewportHeight);
    const auto& camera=hub.activeCamera();
    bool playing=hub.pinballCam&&(hub.machine.ballState!=BallState::READY||hub.machine.plunger.pulling);
    auto plan=qualityPlan(viewportWidth,viewportHeight,rayWidth,samples,playing);target=plan.back();
    if(previousWidth!=viewportWidth||previousHeight!=viewportHeight||previousLights!=hub.lightsOn||
       previousDay!=hub.dayMode||previousDoor!=hub.doorOpen||previousTable!=hub.pinballCam||previousFan!=hub.fanOn||
       previousPlaying!=playing||previousRequestedWidth!=rayWidth||previousRequestedSamples!=samples||
       std::abs(previousDoorAngle-hub.doorAngle)>.001f||
       previousCamera.position!=camera.position||previousCamera.front!=camera.front||previousCamera.zoom!=camera.zoom) {
        invalidate();previousCamera=camera;previousWidth=viewportWidth;previousHeight=viewportHeight;
        previousLights=hub.lightsOn;previousDay=hub.dayMode;previousDoor=hub.doorOpen;
        previousTable=hub.pinballCam;previousFan=hub.fanOn;previousPlaying=playing;
        previousRequestedWidth=rayWidth;previousRequestedSamples=samples;previousDoorAngle=hub.doorAngle;
    }
    auto signature=sceneSignature(hub);
    if(signature!=previousSceneSignature){previousSceneSignature=signature;++sceneVersion;}
    if(pending.valid()&&pending.wait_for(std::chrono::milliseconds(0))==std::future_status::ready) {
        auto image=pending.get();
        if(jobGeneration==generation&&!image.cancelled) {
            upload(image);uploadedSceneVersion=jobSceneVersion;phase=std::min(3u,jobPhase+1);
        }
        cancelJob.reset();
    }
    // Camera/door motion remains a clear raster view. A short debounce avoids
    // starting jobs that the next mouse event would immediately cancel.
    if(std::chrono::steady_clock::now()-lastViewChange<std::chrono::milliseconds(120))return ready;
    if(!pending.valid()) {
        while(phase<3&&ready&&plan[phase].width==imageWidth&&plan[phase].samples==imageSamples)++phase;
        // Completed static scenes cost no further CPU until an object changes.
        if(phase>=3&&uploadedSceneVersion==sceneVersion)return ready;
        jobPhase=std::min(phase,2u);
        const auto quality=plan[jobPhase];
        auto scene=hub.rayScene();auto snapshotCamera=camera;
        int height=std::max(1,static_cast<int>(std::lround(quality.width*double(viewportHeight)/viewportWidth)));
        jobGeneration=generation;jobSceneVersion=sceneVersion;
        cancelJob=std::make_shared<std::atomic_bool>(false);auto cancel=cancelJob;
        pending=std::async(std::launch::async,[scene=std::move(scene),snapshotCamera,quality,height,cancel]() mutable {
            if(cancel->load(std::memory_order_relaxed)){RayImage image;image.cancelled=true;return image;}
            scene.buildBVH();
            // Leave one logical CPU for input, physics and the HUD.
            unsigned hardware=std::thread::hardware_concurrency();
            unsigned workers=std::max(1u,std::min(8u,hardware>1?hardware-1:1));
            return RayTracer::render(scene,snapshotCamera,quality.width,height,quality.samples,workers,cancel.get());
        });
    }
    return ready;
}
void RayTraceView::capture(const GameHub& hub,int viewportWidth,int viewportHeight,int rayWidth,int samples) {
    if(cancelJob)cancelJob->store(true,std::memory_order_relaxed);
    if(pending.valid())pending.get();
    cancelJob.reset();
    auto scene=hub.rayScene();scene.buildBVH();
    int height=std::max(1,static_cast<int>(std::lround(rayWidth*double(std::max(1,viewportHeight))/std::max(1,viewportWidth))));
    auto image=RayTracer::render(scene,hub.activeCamera(),rayWidth,height,samples);upload(image);
    target={rayWidth,samples};
    std::cout<<"Ray traced "<<image.width<<"x"<<image.height<<", "<<samples<<" samples/pixel, "
             <<image.triangleCount<<" triangles, "<<scene.nodeCount()<<" BVH nodes, "<<image.milliseconds<<" ms\n";
}
void RayTraceView::draw() const {
    if(!ready)return;
    glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);glDepthMask(GL_FALSE);
    shader->use();shader->setInt("rayImage",0);
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);
    glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,3);glBindVertexArray(0);
    glDepthMask(GL_TRUE);glEnable(GL_DEPTH_TEST);
}
