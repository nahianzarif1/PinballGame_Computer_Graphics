#include "GameHub.h"
#include "RayTracer.h"
#include "RayTraceView.h"
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
#include <chrono>
#include <future>
#include <thread>

namespace {
void require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
bool near(float a,float b,float tolerance=1e-4f) {return std::abs(a-b)<tolerance;}
RayScene darkScene() {RayScene s;s.ambient=glm::vec3(0);s.background=glm::vec3(0);s.spot.enabled=false;return s;}
void box(RayScene& scene,glm::vec3 center,glm::vec3 size,const RayMaterial& material) {
    auto mesh=createCube(size.x,size.y,size.z);
    scene.addMesh(mesh,glm::translate(glm::mat4(1),center),material);
}
float bruteDistance(const RayScene& scene,const Ray& ray) {
    float closest=std::numeric_limits<float>::infinity();
    // Independent plane intersection followed by a dot-product barycentric test.
    for(const auto& triangle : scene.triangles) {
        auto a=triangle.position[0],e=triangle.position[1]-a,f=triangle.position[2]-a;
        auto normal=glm::cross(e,f);float denominator=glm::dot(normal,ray.direction);
        if(std::abs(denominator)<1e-8f)continue;
        float distance=glm::dot(normal,a-ray.origin)/denominator;
        if(distance<1e-4f || distance>=closest)continue;
        auto p=ray.origin+distance*ray.direction-a;
        float ee=glm::dot(e,e),ef=glm::dot(e,f),ff=glm::dot(f,f),pe=glm::dot(p,e),pf=glm::dot(p,f);
        float divisor=ee*ff-ef*ef;
        float u=(ff*pe-ef*pf)/divisor,v=(ee*pf-ef*pe)/divisor;
        if(u>=0 && v>=0 && u+v<=1)closest=distance;
    }
    return closest;
}
void intersections() {
    RayMaterial material;auto scene=darkScene();
    box(scene,{0,0,0},{2,2,2},material);box(scene,{0,0,-5},{2,2,2},material);scene.buildBVH();
    RayHit hit;
    require(scene.intersect({{0,0,3},{0,0,-1}},hit) && near(hit.distance,2),"Nearest triangle hit wins");
    require(!scene.intersect({{3,0,3},{0,0,-1}},hit),"Axis-parallel ray outside bounds misses");
    require(scene.intersect({{0,0,0},{1,0,0}},hit) && near(hit.distance,1),"Ray inside box finds exit");
    require(!scene.intersect({{0,0,3},{0,0,-1}},hit,1e-4f,1.9f),"Finite segment excludes distant hits");
    for(int i=0;i<25;++i) {
        Transform transform;transform.position={float(i%5)*2.7f-5,float(i/5)*2.3f-5,float(i%3)-1};
        transform.rotation={17.0f*i,23.0f*i,11.0f*i};transform.scale={.4f,1.3f,.7f};
        auto mesh=createCube();scene.addMesh(mesh,transform.getModelMatrix(),material);
    }
    scene.buildBVH();require(scene.nodeCount()>1,"Triangle BVH is built");
    std::mt19937 random(57);std::uniform_real_distribution<float> range(-8,8);
    for(int i=0;i<600;++i) {
        Ray ray{{range(random),range(random),10},glm::normalize(glm::vec3(range(random),range(random),-10))};
        float reference=bruteDistance(scene,ray);bool found=scene.intersect(ray,hit);
        require(found==std::isfinite(reference),"BVH agrees with independent exhaustive intersections");
        if(found)require(near(hit.distance,reference,.002f),"BVH preserves closest intersection distance");
    }
    for(const auto& triangle : scene.triangles) {
        auto e=triangle.position[1]-triangle.position[0],f=triangle.position[2]-triangle.position[0];
        require(std::abs(glm::dot(e,triangle.normal[0]))<1e-4f && std::abs(glm::dot(f,triangle.normal[0]))<1e-4f,
                "Inverse-transpose normals remain perpendicular under rotation and nonuniform scale");
    }
    RayScene empty;empty.buildBVH();require(!empty.intersect({{0,0,0},{0,0,1}},hit),"Empty BVH safely misses");
}
void shadowsAndLighting() {
    auto scene=darkScene();RayMaterial glass;glass.transmission=.81f;
    box(scene,{0,0,2},{2,2,.1f},glass);scene.buildBVH();
    auto visibility=scene.transmittance({{0,0,0},{0,0,1}},3.5f);
    require(near(visibility.x,.746496f,.001f),"Glass transmits shadow rays through both dielectric interfaces");
    box(scene,{0,0,4},{2,2,.1f},RayMaterial{});scene.buildBVH();
    require(scene.transmittance({{0,0,0},{0,0,1}},5)==glm::vec3(0),"Opaque blocker behind glass blocks light");
    require(scene.transmittance({{0,0,0},{0,0,1}},3.5f).x>.7f,"Blocker beyond light segment has no effect");
    auto lit=darkScene();auto plane=createPlane(4,4);lit.addMesh(plane,glm::mat4(1),RayMaterial{});
    PointLight light;light.position={0,0,3};light.ambient=glm::vec3(0);light.specular=glm::vec3(0);
    light.constant=1;light.linear=light.quadratic=0;lit.lights.push_back(light);lit.buildBVH();
    Ray ray{{0,0,1},{0,0,-1}};
    require(near(RayTracer::trace(lit,ray).x,1),"Unoccluded Lambert surface receives direct light");
    box(lit,{0,0,2},{1,1,.1f},RayMaterial{});lit.buildBVH();
    require(RayTracer::trace(lit,ray)==glm::vec3(0),"Shadow ray suppresses occluded direct lighting");
    lit.lights[0].enabled=false;
    require(RayTracer::trace(lit,ray)==glm::vec3(0),"Disabled light contributes no radiance");
}
void secondaryRays() {
    auto mirror=darkScene();RayMaterial reflective;reflective.reflection=1;
    auto plane=createPlane(8,8);mirror.addMesh(plane,glm::mat4(1),reflective);
    RayMaterial red;red.color={1,0,0};red.emission=1;
    box(mirror,{0,0,2},{4,4,.1f},red);mirror.buildBVH();
    Ray ray{{0,0,1},{0,0,-1}};
    require(glm::length(RayTracer::trace(mirror,ray,1)-glm::vec3(1,0,0))<1e-4f,"Reflection ray sees actual emissive geometry");
    require(RayTracer::trace(mirror,ray,0)==glm::vec3(0),"Recursion depth bounds reflected transport");
    require(near(RayTracer::fresnel(1,1,1.5f),.04f),"Glass normal-incidence Fresnel reflectance is four percent");
    require(RayTracer::fresnel(.2f,1.5f,1)==1,"Glass exit beyond critical angle gives total internal reflection");
    auto refraction=darkScene();RayMaterial glass;glass.transmission=1;
    box(refraction,{0,0,0},{8,8,1},glass);
    RayMaterial green;green.color={0,1,0};green.emission=1;
    // Snell: sin(theta_t)=.6/1.5=.4. Beam crosses the slab with x
    // displacement .4/sqrt(1-.4^2), then resumes .6/.8 in air.
    float x=1.125f+.4f/std::sqrt(.84f)+.75f;
    box(refraction,{x,0,-1.5f},{.16f,1,.02f},green);refraction.buildBVH();
    auto color=RayTracer::trace(refraction,{{0,0,2},{.6f,0,-.8f}});
    require(color.y>.85f && color.x==0,"Refracted ray bends through glass according to Snell's law and hits expected target");
}
glm::vec3 referenceShadow(const RayScene& scene,const Ray& ray,float maximum) {
    glm::vec3 visibility(1);float minimum=1e-4f;
    for(int layer=0;layer<32;++layer) {
        RayHit hit;if(!scene.intersect(ray,hit,minimum,maximum))return visibility;
        const auto& triangle=scene.triangles[hit.triangle];const auto& material=scene.materials[triangle.material];
        if(material.transmission<=0)return glm::vec3(0);
        auto tint=glm::mix(glm::vec3(1),glm::clamp(material.color,glm::vec3(0),glm::vec3(1)),.15f);
        visibility*=tint*std::sqrt(material.transmission)*
            (1-RayTracer::fresnel(std::abs(glm::dot(ray.direction,triangle.geometricNormal)),1,material.ior));
        if(glm::length(visibility)<.005f)return glm::vec3(0);
        minimum=hit.distance+1e-4f;
    }
    return glm::vec3(0);
}
void transparentShadowTraversal() {
    auto scene=darkScene();
    for(int i=0;i<8;++i) {
        RayMaterial material;material.transmission=.85f+.015f*i;material.ior=1.2f+.05f*i;
        material.color={.2f+.08f*i,.8f,.45f};
        box(scene,{0,0,.5f+i*.45f},{3,3,.12f},material);
    }
    box(scene,{.8f,0,4.7f},{.45f,2,.2f},RayMaterial{});scene.buildBVH();
    for(int i=0;i<200;++i) {
        Ray ray{{-.8f+i*.008f,.03f,0},glm::normalize(glm::vec3(.02f*(i%5),.01f,1))};
        float maximum=1.0f+.025f*i;
        require(glm::length(scene.transmittance(ray,maximum)-referenceShadow(scene,ray,maximum))<1e-5f,
                "One-pass colored-glass shadows match nearest-interface traversal and bounded blockers");
    }
    Ray edge{{0,0,0},{0,0,1}};
    require(glm::length(scene.transmittance(edge,4.2f)-referenceShadow(scene,edge,4.2f))<1e-5f,
            "Shared triangle edges do not count a glass interface twice");
    auto layered=darkScene();RayMaterial clear;clear.transmission=1;
    for(int i=0;i<40;++i)box(layered,{0,0,1.0f+i*.2f},{2,2,.05f},clear);
    layered.buildBVH();
    require(layered.transmittance(edge,20)==referenceShadow(layered,edge,20),
            "Many glass layers preserve the conservative traversal bound");
}
void liveQualityAndCancellation() {
    auto plan=RayTraceView::qualityPlan(1868,1440);
    require(plan[0].width>=480&&plan[2].width>=1280&&plan[2].samples>=2,
            "Live ray mode refines beyond a small preview with final antialiasing");
    auto native=RayTraceView::qualityPlan(400,300);
    require(native[2].width==400,"Refinement does not waste rays beyond the native viewport");
    auto manual=RayTraceView::qualityPlan(1868,1440,960,4);
    require(manual[2].width==960&&manual[2].samples==4,"Explicit live width and samples are honored");
    auto playing=RayTraceView::qualityPlan(1868,1440,0,0,true);
    require(playing[2].width>256&&playing[2].width<plan[2].width&&playing[2].samples==1,
            "Active pinball balances image detail and refresh time");
    auto portrait=RayTraceView::qualityPlan(200,3000,1920,8);
    require(portrait[2].width<=200&&portrait[2].width*std::lround(portrait[2].width*3000.0/200)<=2073600,
            "Tall windows respect native dimensions and the CPU pixel budget");
    auto invalid=RayTraceView::qualityPlan(0,0);
    require(invalid[2].width==1,"Minimized viewports retain valid image dimensions");
    auto scene=darkScene();box(scene,{0,0,0},{2,2,2},RayMaterial{});scene.buildBVH();Camera camera;
    std::atomic_bool stop{true};auto cancelled=RayTracer::render(scene,camera,64,48,1,4,&stop);
    require(cancelled.cancelled&&cancelled.pixels.empty(),"Cancelled jobs never expose a partial RGB frame");
    stop.store(false);
    auto job=std::async(std::launch::async,[&]{return RayTracer::render(scene,camera,1920,1080,64,4,&stop);});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));stop.store(true);
    require(job.wait_for(std::chrono::seconds(2))==std::future_status::ready,"Camera cancellation promptly releases ray workers");
    require(job.get().cancelled,"An obsolete running image is marked cancelled");
}
void cameraAndTextures() {
    Camera camera({1,2,3},{0,0,1},31,-22);camera.zoom=57;
    auto center=RayTracer::primaryRay(camera,320,240,640,480);
    require(glm::length(center.direction-camera.front)<1e-5f,"Center primary ray follows active camera");
    auto ray=RayTracer::primaryRay(camera,89.5f,157.5f,640,480);
    auto projected=camera.getProjectionMatrix(640.0f/480)*camera.getViewMatrix()*glm::vec4(ray.origin+ray.direction*10.0f,1);
    projected/=projected.w;
    require(near(projected.x,2*89.5f/640-1) && near(projected.y,2*157.5f/480-1),"Primary ray agrees with raster projection");
    TextureData data;data.width=data.height=2;data.pixels={255,0,0,0,255,0,0,0,255,255,255,255};
    require(data.sample({.25f,.25f})==glm::vec3(1,0,0),"Ray textures use matching texel coordinates");
    require(glm::length(data.sample({.5f,.5f})-glm::vec3(.5f))<1e-5f,"Texture bilinear filtering");
    require(data.sample({1.25f,-.75f})==glm::vec3(1,0,0),"Texture repeat handles negative and wrapped UVs");
    auto mesh=createPlane(2,2);mesh.loadTexture("white_tile");
    require(mesh.textureData && mesh.hasTexture,"Procedural textures retain CPU data without a GL context");
}
void integration() {
    GameHub hub;hub.toggleFan();hub.update(0);
    auto scene=hub.rayScene();scene.buildBVH();
    require(scene.triangles.size()>10000 && scene.materials.size()>100,"Room and pinball export their full geometry");
    auto image=RayTracer::render(scene,hub.activeCamera(),64,48,2,1);
    auto parallel=RayTracer::render(scene,hub.activeCamera(),64,48,2,4);
    require(image.pixels==parallel.pixels,"Multithreaded render is deterministic");
    require(image.pixels.size()==64*48*3,"Complete RGB frame returned");
    auto previous=scene.triangles.back().position[0];
    hub.machine.launchFromLane(18);hub.update(.1f);
    auto moving=hub.rayScene();
    require(glm::length(previous-moving.triangles.back().position[0])>.01f,"Ray snapshot follows animated pinball with room placement");
    require(scene.triangles.back().position[0]==previous,"A worker snapshot remains immutable after game changes");
    hub.toggleDoor();hub.update(.25f);auto opening=hub.rayScene();
    int changes=0;
    for(size_t i=0;i<opening.triangles.size();++i)if(glm::length(opening.triangles[i].position[0]-moving.triangles[i].position[0])>.01f)++changes;
    require(changes>=60,"Glass door, frame and handle share animated hinge transform");
    hub.toggleLights();hub.update(0);auto off=hub.rayScene();
    require(!off.lights[0].enabled && off.lights[4].enabled,"Ray mode honors independent room-light switch policy");
    require(glm::length(off.ambient)<glm::length(scene.ambient),"Ray mode honors dark-room ambient");
    require(!off.spot.enabled,"Room spotlight follows lighting switch");
    hub.pinballCam=true;require(hub.rayScene().spot.enabled,"Table camera retains table spotlight");
}
}
int main() {
    try {
        intersections();shadowsAndLighting();secondaryRays();transparentShadowTraversal();
        liveQualityAndCancellation();cameraAndTextures();integration();
        std::cout<<"PASS: closest hits, BVH reference agreement, transforms, camera rays, finite shadows, glass transmission, reflection, Snell refraction, Fresnel/TIR, textures, threaded rendering and animated scene integration\n";
    } catch(const std::exception& error) {std::cerr<<"FAIL: "<<error.what()<<"\n";return 1;}
}
