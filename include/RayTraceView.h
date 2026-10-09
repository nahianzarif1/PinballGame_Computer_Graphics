#ifndef RAYTRACEVIEW_H
#define RAYTRACEVIEW_H

#include "RayTracer.h"
#include "Shader.h"
#include <future>
#include <memory>
#include <array>
#include <chrono>
#include <cstdint>

class GameHub;
struct RayQuality {int width{0},samples{1};};
// Owns presentation GL resources on the main thread and an immutable CPU job.
class RayTraceView {
public:
    RayTraceView();
    ~RayTraceView();
    RayTraceView(const RayTraceView&)=delete;
    RayTraceView& operator=(const RayTraceView&)=delete;
    bool update(const GameHub& hub,int viewportWidth,int viewportHeight,int rayWidth=0,int samples=0);
    static std::array<RayQuality,3> qualityPlan(int viewportWidth,int viewportHeight,
                                              int requestedWidth=0,int samples=0,bool playing=false);
    void capture(const GameHub& hub,int viewportWidth,int viewportHeight,int rayWidth,int samples);
    void draw() const;
    double milliseconds() const { return lastMilliseconds; }
    int width() const {return imageWidth;}
    int samples() const {return imageSamples;}
    bool refining() const {return !ready||imageWidth<target.width||imageSamples<target.samples;}
    void invalidate();
private:
    std::unique_ptr<Shader> shader;
    unsigned texture{0},vao{0};
    std::future<RayImage> pending;
    bool ready{false};
    unsigned generation{0},jobGeneration{0};
    std::shared_ptr<std::atomic_bool> cancelJob;
    std::chrono::steady_clock::time_point lastViewChange;
    unsigned phase{0},jobPhase{0};
    RayQuality target;
    int imageWidth{0},imageSamples{0},textureWidth{0},textureHeight{0};
    std::uint64_t previousSceneSignature{0},sceneVersion{0},jobSceneVersion{0},uploadedSceneVersion{0};
    double lastMilliseconds{0};
    Camera previousCamera;
    int previousWidth{0},previousHeight{0};
    int previousRequestedWidth{-1},previousRequestedSamples{-1};
    float previousDoorAngle{0};
    bool previousLights{false},previousDay{false},previousDoor{false},previousTable{false},previousFan{false},previousPlaying{false};
    void upload(const RayImage& image);
};
#endif
