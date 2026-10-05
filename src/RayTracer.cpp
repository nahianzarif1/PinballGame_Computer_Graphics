#include "RayTracer.h"
#include "GameObject.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <array>

namespace {
constexpr float epsilon=1e-4f;
glm::vec3 inverseDirection(const Ray& ray) {
    glm::vec3 result(0);
    for(int axis=0;axis<3;++axis)if(std::abs(ray.direction[axis])>=1e-12f)result[axis]=1/ray.direction[axis];
    return result;
}
bool boxHit(const Ray& ray,glm::vec3 inverse,glm::vec3 low,glm::vec3 high,float minimum,float maximum,float& entry) {
    for(int axis=0;axis<3;++axis) {
        if(std::abs(ray.direction[axis])<1e-12f) {
            if(ray.origin[axis]<low[axis] || ray.origin[axis]>high[axis])return false;
        } else {
            float a=(low[axis]-ray.origin[axis])*inverse[axis];
            float b=(high[axis]-ray.origin[axis])*inverse[axis];
            if(a>b)std::swap(a,b);
            minimum=std::max(minimum,a);maximum=std::min(maximum,b);
            if(maximum<minimum)return false;
        }
    }
    entry=minimum;
    return true;
}
// Two-sided Moller-Trumbore test with barycentrics and a bounded ray segment.
bool triangleHit(const glm::vec3& origin,const glm::vec3& e1,const glm::vec3& e2,
                 const Ray& ray, float minimum, RayHit& hit) {
    glm::vec3 p=glm::cross(ray.direction,e2);
    float determinant=glm::dot(e1,p);
    if(std::abs(determinant)<1e-10f)return false;
    float inverse=1/determinant;
    glm::vec3 offset=ray.origin-origin;
    float u=glm::dot(offset,p)*inverse;
    if(u<0 || u>1)return false;
    glm::vec3 q=glm::cross(offset,e1);
    float v=glm::dot(ray.direction,q)*inverse;
    if(v<0 || u+v>1)return false;
    float distance=glm::dot(e2,q)*inverse;
    if(distance<minimum || distance>=hit.distance)return false;
    hit.distance=distance;hit.barycentric={1-u-v,u,v};return true;
}
glm::vec3 offsetOrigin(glm::vec3 p, glm::vec3 outwardNormal, glm::vec3 direction) {
    return p+outwardNormal*(glm::dot(outwardNormal,direction)>=0 ? epsilon : -epsilon);
}
glm::vec3 glassTint(const RayMaterial& material) {
    // alpha controls opacity, while most of the transmitted light stays clear.
    return glm::mix(glm::vec3(1),glm::clamp(material.color,glm::vec3(0),glm::vec3(1)),.15f);
}
}

void RayScene::addMesh(const Mesh& mesh,const glm::mat4& world,const RayMaterial& material) {
    unsigned materialIndex=static_cast<unsigned>(materials.size());
    materials.push_back(material);
    glm::mat3 normalMatrix=glm::transpose(glm::inverse(glm::mat3(world)));
    size_t count=mesh.indices.empty() ? mesh.vertices.size() : mesh.indices.size();
    for(size_t i=0;i+2<count;i+=3) {
        RayTriangle triangle;triangle.material=materialIndex;
        for(int j=0;j<3;++j) {
            unsigned index=mesh.indices.empty() ? static_cast<unsigned>(i+j) : mesh.indices[i+j];
            const auto& vertex=mesh.vertices.at(index);
            triangle.position[j]=glm::vec3(world*glm::vec4(vertex.position,1));
            glm::vec3 n=normalMatrix*vertex.normal;
            triangle.normal[j]=glm::length(n)>1e-8f ? glm::normalize(n) : glm::vec3(0);
            triangle.color[j]=vertex.color;triangle.uv[j]=vertex.texCoord;
        }
        glm::vec3 n=glm::cross(triangle.position[1]-triangle.position[0],triangle.position[2]-triangle.position[0]);
        if(glm::dot(n,n)<1e-16f)continue; // Sphere poles and flattened faces.
        // Some existing procedural meshes use inward winding but outward normals.
        // The geometric normal must point outward to classify glass entry/exit.
        if(glm::dot(n,triangle.normal[0]+triangle.normal[1]+triangle.normal[2])<0)n=-n;
        triangle.geometricNormal=glm::normalize(n);
        triangles.push_back(triangle);
    }
}
void RayScene::addObject(const GameObject& object,const glm::mat4& world) {
    RayMaterial material;
    material.color=object.color;material.emission=object.emissiveStrength;
    material.shininess=object.shininess;material.reflection=glm::clamp(object.reflectivity,0.0f,1.0f);
    material.transmission=1-glm::clamp(object.alpha,0.0f,1.0f);
    material.ior=object.refractiveIndex;material.texture=object.mesh.textureData;
    addMesh(object.mesh,world,material);
}
void RayScene::buildBVH() {
    order.resize(triangles.size());std::iota(order.begin(),order.end(),0u);
    nodes.clear();nodes.reserve(triangles.size()*2);
    intersections.resize(triangles.size());bounds.resize(triangles.size());
    for(size_t i=0;i<triangles.size();++i) {
        const auto& t=triangles[i];
        intersections[i]={t.position[0],t.position[1]-t.position[0],t.position[2]-t.position[0]};
        bounds[i]={glm::min(t.position[0],glm::min(t.position[1],t.position[2])),
                   glm::max(t.position[0],glm::max(t.position[1],t.position[2])),
                   (t.position[0]+t.position[1]+t.position[2])/3.0f};
    }
    if(!triangles.empty())buildNode(0,static_cast<unsigned>(triangles.size()));
    bounds.clear(); // Only the compact intersection records are needed by rays.
}
unsigned RayScene::buildNode(unsigned begin,unsigned end,unsigned depth) {
    unsigned index=static_cast<unsigned>(nodes.size());nodes.emplace_back();
    glm::vec3 low(std::numeric_limits<float>::infinity()),high(-low),centerLow(low),centerHigh(high);
    for(unsigned i=begin;i<end;++i) {
        const auto& b=bounds[order[i]];
        low=glm::min(low,b.low);high=glm::max(high,b.high);
        centerLow=glm::min(centerLow,b.center);centerHigh=glm::max(centerHigh,b.center);
    }
    nodes[index].low=low-glm::vec3(1e-6f);nodes[index].high=high+glm::vec3(1e-6f);
    if(end-begin<=4) {nodes[index].begin=begin;nodes[index].count=end-begin;return index;}
    // Binned surface-area splits reduce overlap between children. Fall back to
    // equal counts for degenerate centroids and to bound traversal stack depth.
    constexpr int binCount=12;
    struct Bin {glm::vec3 low{std::numeric_limits<float>::infinity()},high{-low};unsigned count{0};};
    auto area=[](glm::vec3 a,glm::vec3 b){auto d=glm::max(b-a,glm::vec3(0));return d.x*d.y+d.x*d.z+d.y*d.z;};
    glm::vec3 extent=centerHigh-centerLow;
    int bestAxis=-1,bestSplit=-1;float bestCost=std::numeric_limits<float>::infinity();
    if(depth<24)for(int axis=0;axis<3;++axis) {
        if(extent[axis]<=1e-8f)continue;
        std::array<Bin,binCount> bins;
        auto bucket=[&](unsigned id){return std::min(binCount-1,static_cast<int>((bounds[id].center[axis]-centerLow[axis])/extent[axis]*binCount));};
        for(unsigned i=begin;i<end;++i) {
            auto& bin=bins[bucket(order[i])];const auto& b=bounds[order[i]];
            ++bin.count;bin.low=glm::min(bin.low,b.low);bin.high=glm::max(bin.high,b.high);
        }
        std::array<float,binCount-1> leftArea;
        std::array<unsigned,binCount-1> leftCount;
        Bin left;
        for(int split=0;split<binCount-1;++split) {
            if(bins[split].count){left.low=glm::min(left.low,bins[split].low);left.high=glm::max(left.high,bins[split].high);}
            left.count+=bins[split].count;leftCount[split]=left.count;
            leftArea[split]=left.count?area(left.low,left.high):0;
        }
        Bin right;
        for(int split=binCount-2;split>=0;--split) {
            const auto& bin=bins[split+1];
            if(bin.count){right.low=glm::min(right.low,bin.low);right.high=glm::max(right.high,bin.high);}
            right.count+=bin.count;
            if(!leftCount[split]||!right.count)continue;
            float cost=leftCount[split]*leftArea[split]+right.count*area(right.low,right.high);
            if(cost<bestCost){bestCost=cost;bestAxis=axis;bestSplit=split;}
        }
    }
    unsigned middle=begin;
    if(bestAxis>=0) {
        auto pivot=std::partition(order.begin()+begin,order.begin()+end,[&](unsigned id){
            int bucket=std::min(binCount-1,static_cast<int>((bounds[id].center[bestAxis]-centerLow[bestAxis])/extent[bestAxis]*binCount));
            return bucket<=bestSplit;
        });
        middle=static_cast<unsigned>(pivot-order.begin());
    }
    if(middle==begin||middle==end) {
        int axis=extent.y>extent.x?1:0;if(extent.z>extent[axis])axis=2;
        middle=begin+(end-begin)/2;
        std::nth_element(order.begin()+begin,order.begin()+middle,order.begin()+end,
            [&](unsigned a,unsigned b){return bounds[a].center[axis]<bounds[b].center[axis];});
    }
    unsigned left=buildNode(begin,middle,depth+1),right=buildNode(middle,end,depth+1);
    nodes[index].left=left;nodes[index].right=right;return index;
}
bool RayScene::intersect(const Ray& ray,RayHit& hit,float minimum,float maximum) const {
    if(nodes.empty())return false;
    hit.distance=maximum;bool found=false;
    // SAH switches to equal counts at depth 24, keeping this stack bounded.
    glm::vec3 inverse=inverseDirection(ray);
    float entries[64];
    if(!boxHit(ray,inverse,nodes[0].low,nodes[0].high,minimum,maximum,entries[0]))return false;
    unsigned stack[64],size=1;stack[0]=0;
    while(size) {
        --size;
        if(entries[size]>=hit.distance)continue;
        const auto& node=nodes[stack[size]];
        if(node.count) {
            for(unsigned i=node.begin;i<node.begin+node.count;++i) {
                unsigned triangle=order[i];
                const auto& t=intersections[triangle];
                if(triangleHit(t.origin,t.edge1,t.edge2,ray,minimum,hit)) {hit.triangle=triangle;found=true;}
            }
        } else {
            float leftEntry=0,rightEntry=0;
            const auto& left=nodes[node.left];const auto& right=nodes[node.right];
            bool leftHit=boxHit(ray,inverse,left.low,left.high,minimum,hit.distance,leftEntry);
            bool rightHit=boxHit(ray,inverse,right.low,right.high,minimum,hit.distance,rightEntry);
            auto push=[&](unsigned index,float entry){stack[size]=index;entries[size++]=entry;};
            // Near child first: its closest hit prunes farther queued boxes.
            if(leftHit && rightHit) {
                if(leftEntry<rightEntry){push(node.right,rightEntry);push(node.left,leftEntry);}
                else {push(node.left,leftEntry);push(node.right,rightEntry);}
            } else if(leftHit)push(node.left,leftEntry);
            else if(rightHit)push(node.right,rightEntry);
        }
    }
    return found;
}
glm::vec3 RayScene::transmittance(const Ray& ray,float maximum) const {
    if(nodes.empty())return glm::vec3(1);
    // One traversal finds opaque blockers immediately and gathers transparent
    // interfaces. The former implementation restarted the BVH at every pane.
    struct Interface {float distance;unsigned triangle;};
    std::array<Interface,64> interfaces;unsigned count=0;
    glm::vec3 inverse=inverseDirection(ray);
    unsigned stack[64],size=1;float entries[64];stack[0]=0;
    if(!boxHit(ray,inverse,nodes[0].low,nodes[0].high,epsilon,maximum,entries[0]))return glm::vec3(1);
    while(size) {
        const auto& node=nodes[stack[--size]];
        if(node.count)for(unsigned i=node.begin;i<node.begin+node.count;++i) {
            unsigned id=order[i];const auto& t=intersections[id];RayHit hit;hit.distance=maximum;
            if(!triangleHit(t.origin,t.edge1,t.edge2,ray,epsilon,hit))continue;
            if(materials[triangles[id].material].transmission<=0)return glm::vec3(0);
            if(count==interfaces.size()) {
                // Preserve the exact layered-segment rule in a pathological
                // scene with more interfaces than the small stack buffer.
                glm::vec3 visibility(1);float minimum=epsilon;
                for(int layer=0;layer<32;++layer) {
                    RayHit h;if(!intersect(ray,h,minimum,maximum))return visibility;
                    const auto& tri=triangles[h.triangle];const auto& material=materials[tri.material];
                    if(material.transmission<=0)return glm::vec3(0);
                    visibility*=glassTint(material)*std::sqrt(material.transmission)*
                        (1-RayTracer::fresnel(std::abs(glm::dot(ray.direction,tri.geometricNormal)),1,material.ior));
                    if(glm::length(visibility)<.005f)return glm::vec3(0);
                    minimum=h.distance+epsilon;
                }
                return glm::vec3(0);
            }
            interfaces[count++]={hit.distance,id};
        } else {
            float a=0,b=0;const auto& left=nodes[node.left];const auto& right=nodes[node.right];
            bool l=boxHit(ray,inverse,left.low,left.high,epsilon,maximum,a);
            bool r=boxHit(ray,inverse,right.low,right.high,epsilon,maximum,b);
            if(l&&r){stack[size++]=a<b?node.right:node.left;stack[size++]=a<b?node.left:node.right;}
            else if(l)stack[size++]=node.left;else if(r)stack[size++]=node.right;
        }
    }
    std::sort(interfaces.begin(),interfaces.begin()+count,[](const auto& a,const auto& b){return a.distance<b.distance;});
    glm::vec3 visibility(1);float minimum=epsilon;int layers=0;
    for(unsigned i=0;i<count;++i) {
        const auto& hit=interfaces[i];if(hit.distance<minimum)continue;
        const auto& t=triangles[hit.triangle];const auto& m=materials[t.material];
        if(m.transmission<=0)return glm::vec3(0);
        float cosine=std::abs(glm::dot(ray.direction,t.geometricNormal));
        visibility*=glassTint(m)*std::sqrt(m.transmission)*(1-RayTracer::fresnel(cosine,1,m.ior));
        if(glm::length(visibility)<.005f)return glm::vec3(0);
        minimum=hit.distance+epsilon;
        if(++layers>=32)return glm::vec3(0);
    }
    return visibility;
}

Ray RayTracer::primaryRay(const Camera& camera,float x,float y,int width,int height) {
    float tangent=std::tan(glm::radians(camera.zoom)*.5f);
    float sx=(2*x/width-1)*tangent*width/height,sy=(2*y/height-1)*tangent;
    return {camera.position,glm::normalize(camera.front+sx*camera.right+sy*camera.up)};
}
float RayTracer::fresnel(float cosine,float incidentIOR,float transmittedIOR) {
    cosine=glm::clamp(cosine,0.0f,1.0f);
    float sinTransmitted=incidentIOR/transmittedIOR*std::sqrt(std::max(0.0f,1-cosine*cosine));
    if(sinTransmitted>=1)return 1; // Total internal reflection.
    float cosTransmitted=std::sqrt(1-sinTransmitted*sinTransmitted);
    float parallel=(transmittedIOR*cosine-incidentIOR*cosTransmitted)/(transmittedIOR*cosine+incidentIOR*cosTransmitted);
    float perpendicular=(incidentIOR*cosine-transmittedIOR*cosTransmitted)/(incidentIOR*cosine+transmittedIOR*cosTransmitted);
    return (parallel*parallel+perpendicular*perpendicular)*.5f;
}
glm::vec3 RayTracer::trace(const RayScene& scene,const Ray& ray,int depth,float contribution) {
    RayHit hit;if(!scene.intersect(ray,hit))return scene.background;
    const auto& t=scene.triangles[hit.triangle];const auto& m=scene.materials[t.material];
    glm::vec3 position=ray.origin+ray.direction*hit.distance;
    bool entering=glm::dot(ray.direction,t.geometricNormal)<0;
    glm::vec3 geometric=entering ? t.geometricNormal : -t.geometricNormal;
    glm::vec3 normal(0),vertexColor(0);glm::vec2 uv(0);
    for(int j=0;j<3;++j) {normal+=t.normal[j]*hit.barycentric[j];vertexColor+=t.color[j]*hit.barycentric[j];uv+=t.uv[j]*hit.barycentric[j];}
    normal=glm::length(normal)>1e-6f ? glm::normalize(normal) : geometric;
    if(glm::dot(normal,geometric)<0)normal=-normal;
    glm::vec3 albedo=m.color*vertexColor;if(m.texture)albedo*=m.texture->sample(uv);
    glm::vec3 local(0);
    // Glass is a specular dielectric. Its missing transmitted energy is
    // absorption, not diffuse light; direct Phong evaluation is for opaque
    // surfaces. Glass reflects the actual emissive fixtures through ray paths.
    if(m.transmission<=0) {
        local=scene.ambient*albedo;
        auto illuminate=[&](glm::vec3 lightPosition,glm::vec3 ambient,glm::vec3 diffuse,glm::vec3 specular,
                            float intensity,float kc,float kl,float kq,float cone) {
            glm::vec3 delta=lightPosition-position;float distance=glm::length(delta);
            if(distance<epsilon)return;
            glm::vec3 direction=delta/distance;
            float attenuation=intensity/std::max(kc+kl*distance+kq*distance*distance,.001f);
            local+=ambient*albedo*attenuation;
            float lambert=std::max(glm::dot(normal,direction),0.0f);
            if(lambert<=0 || cone<=0)return;
            float spec=std::pow(std::max(glm::dot(-ray.direction,glm::reflect(-direction,normal)),0.0f),std::max(m.shininess,1.0f));
            glm::vec3 visibility=scene.transmittance({offsetOrigin(position,t.geometricNormal,direction),direction},distance-epsilon);
            local+=visibility*(diffuse*albedo*lambert+specular*spec)*attenuation*cone;
        };
        for(const auto& light : scene.lights)if(light.enabled)
            illuminate(light.position,light.ambient,light.diffuse,light.specular,light.intensity,light.constant,light.linear,light.quadratic,1);
        const auto& spot=scene.spot;
        if(spot.enabled) {
            glm::vec3 delta=spot.position-position;
            float theta=glm::dot(glm::normalize(delta),glm::normalize(-spot.direction));
            float inner=std::cos(glm::radians(spot.cutOff)),outer=std::cos(glm::radians(spot.outerCutOff));
            float cone=std::pow(glm::clamp((theta-outer)/std::max(inner-outer,.0001f),0.0f,1.0f),std::max(spot.exponent*.05f,.2f));
            illuminate(spot.position,spot.ambient,spot.diffuse,spot.specular,spot.intensity,spot.constant,spot.linear,spot.quadratic,cone);
        }
    }
    float reflected=m.reflection,transmitted=0;
    glm::vec3 transmittedDirection(0);
    if(m.transmission>0) {
        float incident=entering ? 1.0f : m.ior,exiting=entering ? m.ior : 1.0f;
        float f=fresnel(std::max(-glm::dot(ray.direction,geometric),0.0f),incident,exiting);
        reflected=f;transmitted=(1-f)*std::sqrt(m.transmission);
        transmittedDirection=glm::refract(ray.direction,geometric,incident/exiting);
    }
    glm::vec3 radiance=local*std::max(0.0f,1-reflected-transmitted)+albedo*m.emission;
    if(depth>0 && contribution>.005f) {
        if(reflected*contribution>.005f) {
            glm::vec3 direction=glm::reflect(ray.direction,m.transmission>0 ? geometric : normal);
            if(glm::dot(direction,geometric)<=0)direction=glm::reflect(ray.direction,geometric);
            radiance+=reflected*trace(scene,{offsetOrigin(position,t.geometricNormal,direction),direction},depth-1,contribution*reflected);
        }
        if(transmitted*contribution>.005f && glm::dot(transmittedDirection,transmittedDirection)>0) {
            radiance+=transmitted*glassTint(m)*trace(scene,{offsetOrigin(position,t.geometricNormal,transmittedDirection),glm::normalize(transmittedDirection)},depth-1,contribution*transmitted);
        }
    }
    return radiance;
}
RayImage RayTracer::render(const RayScene& scene,const Camera& camera,int width,int height,int samples,unsigned workers,
                          const std::atomic_bool* cancel) {
    if(width<1 || height<1 || samples<1 || samples>64)throw std::invalid_argument("Invalid ray image dimensions or samples");
    auto start=std::chrono::steady_clock::now();
    RayImage result;result.width=width;result.height=height;result.samples=samples;result.triangleCount=scene.triangles.size();
    if(cancel&&cancel->load(std::memory_order_relaxed)){result.cancelled=true;return result;}
    result.pixels.resize(static_cast<size_t>(width)*height*3);
    if(!workers)workers=std::max(1u,std::min(8u,std::thread::hardware_concurrency()));
    workers=std::min(workers,static_cast<unsigned>(height));
    std::atomic<int> nextRow{0};std::vector<std::thread> threads;
    std::array<float,64> offsetsX,offsetsY;
    for(int sample=0;sample<samples;++sample) {
        offsetsX[sample]=samples==1?.5f:(sample+.5f)/samples;
        offsetsY[sample]=samples==1?.5f:std::fmod((sample+.5f)*.61803398875f,1.0f);
    }
    const float tangent=std::tan(glm::radians(camera.zoom)*.5f);
    const float scaleX=tangent*width/height;
    auto draw=[&] {
        int y;
        while((y=nextRow.fetch_add(1))<height) {
          if(cancel&&cancel->load(std::memory_order_relaxed))break;
          for(int x=0;x<width;++x) {
            if((x&15)==0&&cancel&&cancel->load(std::memory_order_relaxed))break;
            glm::vec3 radiance(0);
            for(int sample=0;sample<samples;++sample) {
                // Deterministic stratification, including the exact pixel center
                // for a single sample. All tone mapping happens after averaging.
                float sx=(2*(x+offsetsX[sample])/width-1)*scaleX;
                float sy=(2*(y+offsetsY[sample])/height-1)*tangent;
                Ray ray{camera.position,glm::normalize(camera.front+sx*camera.right+sy*camera.up)};
                radiance+=trace(scene,ray);
            }
            radiance=glm::max(radiance/static_cast<float>(samples)*.75f,glm::vec3(0));
            glm::vec3 mapped=radiance/(glm::vec3(1)+radiance);
            size_t index=(static_cast<size_t>(y)*width+x)*3;
            for(int channel=0;channel<3;++channel)result.pixels[index+channel]=static_cast<unsigned char>(glm::clamp(mapped[channel],0.0f,1.0f)*255+.5f);
          }
        }
    };
    for(unsigned i=1;i<workers;++i)threads.emplace_back(draw);
    draw();for(auto& thread : threads)thread.join();
    result.cancelled=cancel&&cancel->load(std::memory_order_relaxed);
    if(result.cancelled)result.pixels.clear();
    result.milliseconds=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();return result;
}
