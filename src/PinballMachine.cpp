#include "PinballMachine.h"
#include "RayTracer.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

PinballMachine::PinballMachine() {
    initialize();
}

void PinballMachine::initialize() {
    playfield = Playfield(7.0f, 12.0f, 8.0f);
    const float surface = playfield.baseHeight;

    ball = Ball(0.28f);
    ball.reset();
    seatBall();

    bumpers.clear();
    bumpers.emplace_back(0.42f, glm::vec3(-1.15f, 3.15f, surface + 0.225f));
    bumpers.emplace_back(0.42f, glm::vec3(0.15f, 4.15f, surface + 0.225f));
    bumpers.emplace_back(0.42f, glm::vec3(1.35f, 3.05f, surface + 0.225f));
    bumpers.emplace_back(0.28f, glm::vec3(-2.45f, 0.35f, surface + 0.18f));
    bumpers.emplace_back(0.28f, glm::vec3(-2.55f, -0.75f, surface + 0.18f));
    bumpers.emplace_back(0.28f, glm::vec3(-2.25f, -1.75f, surface + 0.18f));
    bumpers[0].restColor = glm::vec3(0.95f, 0.18f, 0.28f);
    bumpers[1].restColor = glm::vec3(0.95f, 0.95f, 0.98f);
    bumpers[2].restColor = glm::vec3(0.2f, 0.45f, 0.95f);
    bumpers[3].restColor = glm::vec3(0.35f, 0.55f, 1.0f);
    bumpers[4].restColor = glm::vec3(0.95f, 0.95f, 1.0f);
    bumpers[5].restColor = glm::vec3(0.35f, 0.55f, 1.0f);
    bumpers[0].points = 100;
    bumpers[1].points = 150;
    bumpers[2].points = 100;
    bumpers[3].points = 50;
    bumpers[4].points = 50;
    bumpers[5].points = 50;
    bumpers[3].impulse = 6.5f;
    bumpers[4].impulse = 6.5f;
    bumpers[5].impulse = 6.5f;
    for (auto& bumper : bumpers) {
        bumper.color = bumper.restColor;
    }

    targets.clear();
    targets.emplace_back(glm::vec3(-1.65f, 4.9f, surface + 0.22f));
    targets.emplace_back(glm::vec3(1.95f, 4.55f, surface + 0.22f));
    targets.emplace_back(glm::vec3(0.95f, 1.85f, surface + 0.22f));
    targets.emplace_back(glm::vec3(-0.15f, 2.05f, surface + 0.22f));
    targets[1].restColor = glm::vec3(0.2f, 0.85f, 0.95f);
    targets[2].restColor = glm::vec3(0.95f, 0.3f, 0.7f);
    targets[3].restColor = glm::vec3(0.95f, 0.55f, 0.2f);
    for (auto& t : targets) {
        t.color = t.restColor;
    }

    leftFlipper = Flipper(true);
    rightFlipper = Flipper(false);
    leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
    rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
    leftFlipper.transform.position = leftFlipper.pivot;
    rightFlipper.transform.position = rightFlipper.pivot;

    plunger = Plunger();
    plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;

    lights.resize(3);
    glm::vec3 lightColors[3] = {
        glm::vec3(1.0f, 0.92f, 0.78f),
        glm::vec3(0.85f, 0.95f, 1.0f),
        glm::vec3(1.0f, 0.9f, 0.7f)
    };
    glm::vec3 lightPos[3] = {
        glm::vec3(-4.5f, -2.0f, 7.2f),
        glm::vec3(4.5f, 3.0f, 7.2f),
        glm::vec3(0.0f, -1.0f, 3.6f)
    };
    for (int i = 0; i < 3; i++) {
        lights[i].position = lightPos[i];
        lights[i].intensity = 1.25f;
        lights[i].setColor(lightColors[i]);
        lights[i].ambient = glm::vec3(0.18f);
        lights[i].constant = 1.0f;
        lights[i].linear = 0.04f;
        lights[i].quadratic = 0.006f;
        lights[i].enabled = true;
    }

    createMachineStructure();

    // Keep the dedicated table camera below the GameHub ceiling.  The old
    // z=9.4 position sat above the opaque ceiling and could make the complete
    // pinball scene disappear when table camera mode was selected.
    camera = Camera(glm::vec3(0.0f, -8.4f, 8.1f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, -46.0f);
    camera.movementSpeed = 6.0f;
    camera.zoom = 64.0f;

    resetGame();
}

void PinballMachine::createMachineStructure() {
    const float tilt = glm::radians(playfield.tiltAngle);
    auto box = [&](GameObject& obj, const char* name, glm::vec3 pos, glm::vec3 size,
                   glm::vec3 color, bool inclined = false) {
        obj.name = name;
        obj.mesh = createCube(size.x, size.y, size.z);
        obj.transform.position = pos;
        obj.color = color;
        if (inclined) obj.transform.rotation.x = playfield.tiltAngle;
    };
    const glm::vec3 chrome(0.66f, 0.72f, 0.8f), red(0.48f, 0.035f, 0.08f);
    box(cabinet, "Cabinet", {0, 0, 0.22f}, {7.8f, 13.3f, 0.4f}, {0.045f, 0.035f, 0.09f});
    box(base, "Inclined deck", {0, 0, playfield.baseHeight - 0.15f},
        {7.5f, 12.4f / std::cos(tilt), 0.24f}, {0.08f, 0.09f, 0.13f}, true);
    auto leg = [&](GameObject& obj, float x, float y) {
        box(obj, "Chrome leg", {x,y,0.25f}, {0.15f,0.15f,0.5f}, chrome);
    };
    leg(legFL,-3.5f,-5.7f); leg(legFR,3.5f,-5.7f);
    leg(legBL,-3.5f,5.7f); leg(legBR,3.5f,5.7f);
    box(leftWall,"Left cabinet rail",{-3.61f,0,playfield.baseHeight+0.28f},
        {0.22f,12.25f/std::cos(tilt),0.56f},red,true);
    box(rightWall,"Right cabinet rail",{3.61f,0,playfield.baseHeight+0.28f},
        {0.22f,12.25f/std::cos(tilt),0.56f},red,true);
    box(backWall,"Back rail",{0,6.0f,playfield.calculateZ(6)+0.26f},{7.3f,0.18f,0.52f},red);
    box(frontWallLeft,"Left apron",{-1.96f,-5.75f,playfield.calculateZ(-5.75f)+0.07f},
        {2.9f,0.55f,0.12f},{0.045f,0.045f,0.085f},true);
    box(frontWallRight,"Right apron",{1.7f,-5.75f,playfield.calculateZ(-5.75f)+0.07f},
        {2.2f,0.55f,0.12f},{0.045f,0.045f,0.085f},true);
    box(leftRail,"Chrome trim",{-3.75f,0,playfield.baseHeight+0.6f},
        {0.08f,12.7f/std::cos(tilt),0.06f},chrome,true);
    box(rightRail,"Chrome trim",{3.75f,0,playfield.baseHeight+0.6f},
        {0.08f,12.7f/std::cos(tilt),0.06f},chrome,true);
    box(backbox,"Backbox",{0,6.6f,3.25f},{7.8f,0.65f,2.5f},{0.035f,0.04f,0.07f});
    box(backglass,"Illuminated backglass",{0,6.23f,3.35f},{7.2f,0.07f,1.95f},{0.72f,0.5f,1.0f});
    backglass.mesh.loadTexture("neon");
    backglass.emissiveStrength = 0.9f;
    box(glassCover,"Glass",{0,0,playfield.baseHeight+0.64f},
        {7.25f,12.0f/std::cos(tilt),0.02f},{0.5f,0.7f,0.85f},true);
    glassCover.alpha = 0.035f;

    // These exact segments drive both visible geometry and physical contact.
    // The shooter lane remains open at the top, with a left-turning guide.
    rails = {
        {{2.42f,-6.0f},{2.42f,4.25f},0.08f,0.45f,false},
        {{3.42f,4.65f},{2.35f,5.78f},0.08f,0.85f,false},
        {{2.35f,5.78f},{0.2f,5.78f},0.08f,0.8f,false},
        {{-3.3f,3.8f},{-2.9f,5.35f},0.09f,0.65f,false},
        {{-2.9f,5.35f},{-1.3f,5.78f},0.09f,0.65f,false},
        {{-3.3f,-2.4f},{-2.6f,-3.9f},0.09f,0.55f,false},
        {{-2.6f,-3.9f},{-1.75f,-4.6f},0.09f,0.55f,false},
        {{2.3f,-2.4f},{2.15f,-3.9f},0.09f,0.55f,false},
        {{2.15f,-3.9f},{1.75f,-4.6f},0.09f,0.55f,false},
        {{-2.5f,-2.45f},{-1.05f,-3.95f},0.10f,0.85f,true},
        {{1.65f,-2.45f},{0.95f,-3.95f},0.10f,0.85f,true}
    };
    decorations.clear();
    auto deco = [&](const char* name, Mesh&& mesh, glm::vec3 pos, glm::vec3 color,
                    float emission = 0.0f) -> GameObject& {
        decorations.emplace_back(name);
        auto& obj = decorations.back();
        obj.mesh = std::move(mesh); obj.transform.position = pos;
        obj.color = color; obj.emissiveStrength = emission;
        return obj;
    };
    // Meshes are built in world coordinates so slope and collisions cannot diverge.
    for (const auto& rail : rails) {
        Mesh mesh;
        const glm::vec3 a(rail.a,playfield.calculateZ(rail.a.y)+0.18f);
        const glm::vec3 b(rail.b,playfield.calculateZ(rail.b.y)+0.18f);
        const glm::vec3 side = glm::normalize(glm::vec3(-(b-a).y,(b-a).x,0))*rail.radius;
        glm::vec3 corners[8] = {a-side-glm::vec3(0,0,.18f),a+side-glm::vec3(0,0,.18f),
            b+side-glm::vec3(0,0,.18f),b-side-glm::vec3(0,0,.18f),
            a-side+glm::vec3(0,0,.18f),a+side+glm::vec3(0,0,.18f),
            b+side+glm::vec3(0,0,.18f),b-side+glm::vec3(0,0,.18f)};
        const int faces[6][4] = {{0,3,2,1},{4,5,6,7},{0,1,5,4},{3,7,6,2},{0,4,7,3},{1,2,6,5}};
        for (auto& face : faces) {
            auto normal = glm::normalize(glm::cross(corners[face[1]]-corners[face[0]],corners[face[2]]-corners[face[0]]));
            unsigned baseIndex = static_cast<unsigned>(mesh.vertices.size());
            for (int index : face) mesh.vertices.push_back({corners[index],normal,glm::vec3(1),{0,0}});
            for (unsigned index : {0u,1u,2u,0u,2u,3u}) mesh.indices.push_back(baseIndex+index);
        }
        mesh.setupMesh();
        deco(rail.sling ? "Sling" : "Guide rail",std::move(mesh),{0,0,0},rail.sling ? glm::vec3(0.85f,0.12f,0.25f) : chrome);
    }
    // A purple side area, concentric reactor inserts and warm arrow lamps echo
    // the reference board without placing decorative obstacles in the ball path.
    auto& ramp = deco("Purple artwork",createCube(1.1f,4.1f,0.015f),
        {-2.55f,0.5f,playfield.calculateZ(0.5f)+0.01f},{0.39f,0.16f,0.65f},0.3f);
    ramp.transform.rotation.x = playfield.tiltAngle;
    for (int i=0;i<24;++i) {
        float a=glm::radians(i*15.0f);
        glm::vec3 pos(-0.1f+std::cos(a)*1.2f,-0.65f+std::sin(a)*1.2f,0);
        pos.z=playfield.calculateZ(pos.y)+0.025f;
        auto& insert=deco("Reactor insert",createCylinder(0.10f,0.025f,12),pos,
            i%3==0 ? glm::vec3(1,.45f,.08f) : glm::vec3(.08f,.28f,.95f),0.85f);
        insert.transform.rotation.x=playfield.tiltAngle;
    }
    auto& reactor=deco("Reactor ring",createRing(.68f,.04f),
        {-.1f,-.65f,playfield.calculateZ(-.65f)+.025f},{.1f,.75f,1},0.85f);
    reactor.transform.rotation.x=playfield.tiltAngle;
    for (int side : {-1,1}) for (int i=0;i<7;++i) {
        float x=side<0 ? -1.8f+i*.05f : 1.95f-i*.03f;
        float y=-1.8f+i*.7f;
        auto& lamp=deco("Lane lamp",createCylinder(.075f,.025f,10),
            {x,y,playfield.calculateZ(y)+.025f},{.95f,.82f,.12f},.9f);
        lamp.transform.rotation.x=playfield.tiltAngle;
    }
    // Layered pop bumpers: chrome skirts, ivory caps, jewel lamps.
    for (size_t i=0;i<bumpers.size();++i) {
        auto pos=bumpers[i].transform.position;
        pos.z=playfield.calculateZ(pos.y);
        float r=bumpers[i].radius;
        deco("Bumper skirt",createRing(r+.02f,.055f),pos+glm::vec3(0,0,.12f),chrome);
        deco("Bumper cap",createCylinder(r*.9f,.09f,28),pos+glm::vec3(0,0,.47f),{.95f,.88f,.71f},.22f);
        deco(("Bumper lamp "+std::to_string(i)).c_str(),createSphere(r*.45f,12,18),
            pos+glm::vec3(0,0,.52f),bumpers[i].restColor,.6f);
    }
    for (auto& t : targets) {
        auto pos=t.transform.position;
        pos.z=playfield.calculateZ(pos.y)+.02f;
        auto& lamp=deco("Target insert",createCylinder(.12f,.025f,12),pos+glm::vec3(0,-.4f,0),{1,.7f,.1f},.8f);
        lamp.transform.rotation.x=playfield.tiltAngle;
    }
    auto& title=deco("Backglass title",createTextMesh("NEON CIRCUIT",.42f),{0,6.17f,3.4f},{.85f,.9f,1},1);
    (void)title;
    deco("Backglass subtitle",createTextMesh("THE GAME HUB",.17f),{0,6.16f,3.05f},{.2f,.8f,1},.9f);
    auto& apronText=deco("Apron title",createTextMesh("NEON CIRCUIT",.18f),{0,-5.8f,playfield.calculateZ(-5.8f)+.15f},{.7f,.5f,1},.8f);
    apronText.transform.rotation.x=-90.0f+playfield.tiltAngle;
    for(int side : {-1,1}) {
        Mesh triangle;
        glm::vec2 points[3]={{side<0 ? -2.5f : 1.65f,-2.45f},{side<0 ? -2.55f : 2.1f,-3.95f},{side<0 ? -1.05f : .95f,-3.95f}};
        for(auto pt : points)triangle.vertices.push_back({{pt.x,pt.y,playfield.calculateZ(pt.y)+.03f},{0,0,1},{1,1,1},{0,0}});
        triangle.indices={0,1,2};triangle.setupMesh();
        deco("Sling artwork",std::move(triangle),{0,0,0},{.25f,.12f,.4f},.3f);
    }
    // Unused legacy props have no mesh: guide geometry above replaces them.
    laneWall.mesh.clear(); laneDeflector.mesh.clear(); slingLeft.mesh.clear(); slingRight.mesh.clear();
    apronLeft.mesh.clear(); apronRight.mesh.clear(); centerPost.mesh.clear();
}

void PinballMachine::placeOnPlayfield(GameObject& obj, float heightOffset) {
    obj.transform.position.z = playfield.calculateZ(obj.transform.position.y) + heightOffset;
}

void PinballMachine::seatBall() {
    ball.sitOnPlayfield(playfield.calculateZ(ball.transform.position.y));
}

void PinballMachine::clampObjectToTable(glm::vec3& position, float radius) const {
    const float xmin = -tableHalfWidth() + radius + 0.12f;
    const float xmax = tableHalfWidth() - radius - 0.12f;
    const float ymin = -tableHalfLength() + radius + 0.12f;
    const float ymax = tableHalfLength() - radius - 0.12f;
    position.x = std::clamp(position.x, xmin, xmax);
    position.y = std::clamp(position.y, ymin, ymax);
}

void PinballMachine::addScore(int points) {
    if (gameOver || points <= 0) return;
    score = static_cast<int>(std::min(99999999LL, static_cast<long long>(score) + points));
    highScore = std::max(highScore, score);
}

void PinballMachine::awardScore(int points, const char* event) {
    if (gameOver || ballState != BallState::IN_PLAY) return;
    lastPoints = points * multiplier;
    addScore(lastPoints);
    lastEvent = event;
    eventTimer = 2.0f;
}

void PinballMachine::resetTargetBank() {
    bankResetTimer = 0.0f;
    for (auto& t : targets) {
        t.down = false; t.cooldown = 0.0f; t.color = t.restColor;
    }
}

void PinballMachine::prepareBall() {
    ball.reset(); seatBall();
    ballState = BallState::READY;
    rampArmed = false; rampCooldown = 0.0f;
    plunger.reset();
    plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;
    slingCooldown[0] = slingCooldown[1] = 0.0f;
}

void PinballMachine::resetGame() {
    score = 0; lives = 3; ballNumber = 1; gameOver = false;
    bonus = 0; multiplier = 1; lastPoints = 0;
    lastEvent = "READY TO LAUNCH"; eventTimer = 0;
    ballSaveTimer = 0; ballSaveAvailable = true; accumulator = 0;
    prepareBall(); resetTargetBank();
    leftFlipper.reset(); rightFlipper.reset();
    for (auto& bumper : bumpers) {
        bumper.hit = false; bumper.hitTimer = 0; bumper.color = bumper.restColor;
    }
}

void PinballMachine::launchFromLane(float speed) {
    // Space may launch a ready ball, but never teleport an active ball or erase
    // a pending drain. A new game is started explicitly with R.
    if (gameOver || ballState != BallState::READY) return;
    ballState = BallState::SHOOTER;
    ball.velocity = glm::vec3(0.0f,std::clamp(speed,14.0f,20.0f),0.0f);
    plunger.reset();
    if (ballSaveAvailable) { ballSaveTimer = 8.0f; ballSaveAvailable = false; }
    lastPoints = 0; lastEvent = "BALL LAUNCHED"; eventTimer = 1.5f;
}

void PinballMachine::drainBall() {
    if (gameOver || ballState == BallState::READY) return;
    if (ballSaveTimer > 0.0f) {
        ballSaveTimer = 0.0f;
        prepareBall();
        lastPoints = 0; lastEvent = "BALL SAVED"; eventTimer = 3;
        return;
    }
    const int collected = bonus * multiplier;
    addScore(collected);
    lastPoints = collected; lastEvent = "DRAIN BONUS"; eventTimer = 3;
    bonus = 0; multiplier = 1; resetTargetBank();
    lives = std::max(0,lives-1);
    prepareBall(); ballSaveTimer = 0; ballSaveAvailable = true;
    if (lives == 0) { gameOver = true; lastEvent = "GAME OVER"; }
    else ++ballNumber;
}

void PinballMachine::update(float dt) {
    if (!std::isfinite(dt) || dt <= 0) return;
    // 240 Hz contact resolution avoids tunneling through narrow targets and
    // fast flipper sweeps, independent of the rendering frame rate.
    constexpr float step = 1.0f / 240.0f;
    accumulator += std::min(dt, 0.25f);
    while (accumulator + 1e-7f >= step) {
        physicsStep(step);
        accumulator = std::max(0.0f,accumulator-step);
    }
}

void PinballMachine::physicsStep(float dt) {
    ballSaveTimer = std::max(0.0f,ballSaveTimer-dt);
    eventTimer = std::max(0.0f,eventTimer-dt);
    rampCooldown = std::max(0.0f,rampCooldown-dt);
    for (auto& cooldown : slingCooldown) cooldown = std::max(0.0f,cooldown-dt);
    if (bankResetTimer > 0) {
        bankResetTimer -= dt;
        if (bankResetTimer <= 0) resetTargetBank();
    }
    leftFlipper.update(dt); rightFlipper.update(dt); plunger.update(dt);
    plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y)+.16f;
    for (auto& t : targets) {
        t.update(dt);
        t.transform.position.z=playfield.calculateZ(t.transform.position.y)+.22f;
    }
    for (auto& bumper : bumpers) {
        bumper.update(dt);
        bumper.transform.position.z=playfield.calculateZ(bumper.transform.position.y)+bumper.height*.5f;
    }
    leftFlipper.pivot.z=playfield.calculateZ(leftFlipper.pivot.y)+.12f;
    rightFlipper.pivot.z=playfield.calculateZ(rightFlipper.pivot.y)+.12f;
    if (gameOver || ballState == BallState::READY) { seatBall(); return; }
    const float angle = glm::radians(playfield.tiltAngle);
    ball.velocity.y -= 9.81f*std::sin(angle)*std::cos(angle)*dt;
    ball.update(dt);
    checkCollisions();
    seatBall();
}

bool PinballMachine::collideBallWithSegment(const glm::vec2& a, const glm::vec2& b, float radius, float bounce) {
    glm::vec2 p(ball.transform.position), ab=b-a;
    float abLen2=glm::dot(ab,ab);
    if (abLen2 < 1e-8f) return false;
    float t=std::clamp(glm::dot(p-a,ab)/abLen2,0.0f,1.0f);
    glm::vec2 delta=p-(a+t*ab);
    float dist=glm::length(delta), minDist=ball.radius+radius;
    if (dist >= minDist) return false;
    glm::vec2 n=dist>1e-5f ? delta/dist : glm::normalize(glm::vec2(-ab.y,ab.x));
    ball.transform.position += glm::vec3(n*(minDist-dist+.0001f),0);
    float incoming=glm::dot(glm::vec2(ball.velocity),n);
    if (incoming < 0) ball.velocity -= glm::vec3((1+bounce)*incoming*n,0);
    return incoming < -.1f;
}

void PinballMachine::collideBallWithFlipper(Flipper& flipper) {
    glm::vec2 pivot(flipper.pivot), along=flipper.alongDirection();
    glm::vec2 p(ball.transform.position), ab=along*flipper.length;
    float t=std::clamp(glm::dot(p-pivot,ab)/glm::dot(ab,ab),0.0f,1.0f);
    glm::vec2 closest=pivot+t*ab, delta=p-closest;
    float dist=glm::length(delta), minDist=ball.radius+flipper.width*.5f;
    if (dist >= minDist) return;
    glm::vec2 n=dist>1e-5f ? delta/dist : glm::vec2(0,1);
    ball.transform.position+=glm::vec3(n*(minDist-dist+.0001f),0);
    float omega=glm::radians(flipper.angularVelocity);
    glm::vec2 r=closest-pivot, surfaceVelocity(-omega*r.y,omega*r.x);
    glm::vec2 relative=glm::vec2(ball.velocity)-surfaceVelocity;
    float incoming=glm::dot(relative,n);
    if (incoming<0) ball.velocity-=glm::vec3(1.6f*incoming*n,0);
    // A held flipper supports the ball; only an upward stroke imparts a kick.
    if (flipper.powered && surfaceVelocity.y>0.1f && n.y>0) {
        ball.velocity.y=std::max(ball.velocity.y,8.0f+8.0f*t);
        ball.velocity.x+=(flipper.isLeft ? 1.0f : -1.0f)*2.0f*t;
    }
    float speed=glm::length(ball.velocity);
    if (speed>ball.maxSpeed) ball.velocity*=ball.maxSpeed/speed;
}

void PinballMachine::checkCollisions() {
    if (gameOver || ballState == BallState::READY) return;
    auto& pos=ball.transform.position;
    const float wallX=tableHalfWidth()-.11f-ball.radius;
    const float wallY=tableHalfLength()-.11f-ball.radius;
    if (pos.x>wallX) { pos.x=wallX; if(ball.velocity.x>0) ball.velocity.x*=-.7f; }
    if (pos.x<-wallX) { pos.x=-wallX; if(ball.velocity.x<0) ball.velocity.x*=-.7f; }
    if (pos.y>wallY) { pos.y=wallY; if(ball.velocity.y>0) ball.velocity.y*=-.8f; }
    int slingIndex=0;
    for (auto& rail : rails) {
        bool impact=collideBallWithSegment(rail.a,rail.b,rail.radius,rail.restitution);
        if (rail.sling) {
            if (impact && slingCooldown[slingIndex]<=0 && ballState==BallState::IN_PLAY) {
                ball.velocity+=glm::vec3(slingIndex==0 ? 2.8f : -2.8f,4.0f,0);
                awardScore(10,"SLINGSHOT"); slingCooldown[slingIndex]=.25f;
            }
            ++slingIndex;
        }
    }
    if (ballState==BallState::SHOOTER && pos.x<2.14f && pos.y>4.0f) {
        ballState=BallState::IN_PLAY;
        lastEvent="HIT THE TARGET BANK"; eventTimer=2;
    }
    if (ballState==BallState::IN_PLAY) {
        for (auto& bumper : bumpers) if (bumper.checkCollision(ball)) {
            awardScore(bumper.points,"POP BUMPER"); bonus+=10;
        }
        for (auto& target : targets) if (target.checkCollision(ball)) {
            awardScore(target.points,"DROP TARGET"); bonus+=50;
            if (bankResetTimer<=0 && std::all_of(targets.begin(),targets.end(),[](const Target& t){return t.down;})) {
                awardScore(1000,"BANK COMPLETE");
                multiplier=std::min(5,multiplier+1); bankResetTimer=1.25f;
            }
        }
        collideBallWithFlipper(leftFlipper); collideBallWithFlipper(rightFlipper);
        if (pos.x<-1.9f && pos.y>-1.8f && pos.y<2.4f && ball.velocity.y>1) rampArmed=true;
        if (rampArmed && rampCooldown<=0 && pos.y>4.35f && pos.x<-.8f && ball.velocity.y>0) {
            awardScore(250,"LEFT ORBIT"); bonus+=100; rampArmed=false; rampCooldown=1.5f;
        }
        if (pos.y<-2.2f) rampArmed=false;
    }
    // A ball that rolls back down the shooter lane can be launched again.
    if (pos.x>2.75f && pos.y<-4.6f && ball.velocity.y<0) {
        prepareBall(); lastEvent="READY TO LAUNCH"; return;
    }
    // Every ball leaving the bottom costs one life, including the side lanes.
    if (pos.y < -tableHalfLength()-ball.radius) drainBall();
}

void PinballMachine::applyLighting(unsigned int shaderProgram) const {
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &camera.position[0]);
    glUniform1i(glGetUniformLocation(shaderProgram, "numPointLights"), static_cast<int>(lights.size()));
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 48.0f);
    glm::vec3 ambient(0.18f, 0.18f, 0.22f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "sceneAmbient"), 1, &ambient[0]);

    for (int i = 0; i < static_cast<int>(lights.size()); i++) {
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        const PointLight& light = lights[i];
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "position").c_str()), 1, &light.position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "ambient").c_str()), 1, &light.ambient[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "diffuse").c_str()), 1, &light.diffuse[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "specular").c_str()), 1, &light.specular[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "intensity").c_str()), light.intensity);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "kc").c_str()), light.constant);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "kl").c_str()), light.linear);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "kq").c_str()), light.quadratic);
        glUniform1i(glGetUniformLocation(shaderProgram, (prefix + "enabled").c_str()), light.enabled ? 1 : 0);
    }
}

void PinballMachine::render(unsigned int shaderProgram, bool withLighting) {
    if (withLighting) {
        applyLighting(shaderProgram);
    }

    cabinet.draw(shaderProgram);
    base.draw(shaderProgram);
    legFL.draw(shaderProgram);
    legFR.draw(shaderProgram);
    legBL.draw(shaderProgram);
    legBR.draw(shaderProgram);
    playfield.draw(shaderProgram);
    for (auto& deco : decorations) {
        if (deco.name.rfind("Bumper lamp ",0)==0) {
            size_t i=static_cast<size_t>(deco.name.back()-'0');
            deco.color=bumpers[i].isHit() ? glm::vec3(1,1,.55f) : bumpers[i].restColor;
            deco.emissiveStrength=bumpers[i].isHit() ? 1.4f : .6f;
        }
        deco.draw(shaderProgram);
    }
    leftWall.draw(shaderProgram);
    rightWall.draw(shaderProgram);
    backWall.draw(shaderProgram);
    frontWallLeft.draw(shaderProgram);
    frontWallRight.draw(shaderProgram);
    laneWall.draw(shaderProgram);
    slingLeft.draw(shaderProgram);
    slingRight.draw(shaderProgram);
    laneDeflector.draw(shaderProgram);
    apronLeft.draw(shaderProgram);
    apronRight.draw(shaderProgram);
    centerPost.draw(shaderProgram);
    leftRail.draw(shaderProgram);
    rightRail.draw(shaderProgram);
    backbox.draw(shaderProgram);
    backglass.draw(shaderProgram);

    for (const auto& bumper : bumpers) {
        bumper.draw(shaderProgram);
    }
    for (const auto& target : targets) {
        target.draw(shaderProgram);
    }
    leftFlipper.draw(shaderProgram);
    rightFlipper.draw(shaderProgram);
    plunger.draw(shaderProgram);
    ball.draw(shaderProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glassCover.draw(shaderProgram);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void PinballMachine::appendRayScene(RayScene& scene,const glm::mat4& placement) const {
    auto add=[&](const GameObject& object){scene.addObject(object,placement*object.transform.getModelMatrix());};
    for(const GameObject* object : {&cabinet,&base,&legFL,&legFR,&legBL,&legBR,
        &leftWall,&rightWall,&backWall,&frontWallLeft,&frontWallRight,&laneWall,
        &slingLeft,&slingRight,&laneDeflector,&apronLeft,&apronRight,&centerPost,
        &leftRail,&rightRail,&backbox,&backglass,&glassCover})add(*object);
    for(const auto& object : decorations) {
        if(object.name.rfind("Bumper lamp ",0)==0) {
            size_t i=static_cast<size_t>(object.name.back()-'0');
            RayMaterial material;
            material.color=bumpers[i].isHit() ? glm::vec3(1,1,.55f) : bumpers[i].restColor;
            material.emission=bumpers[i].isHit() ? 1.4f : .6f;material.shininess=object.shininess;
            scene.addMesh(object.mesh,placement*object.transform.getModelMatrix(),material);
        } else add(object);
    }
    auto special=[&](const GameObject& object,glm::mat4 model,float shininess,float emission,float reflection=0) {
        RayMaterial material;material.color=object.color;material.shininess=shininess;
        material.emission=emission;material.reflection=reflection;material.texture=object.mesh.textureData;
        scene.addMesh(object.mesh,placement*model,material);
    };
    special(playfield,playfield.getModelMatrix(),40,.25f);
    for(const auto& bumper : bumpers)special(bumper,bumper.transform.getModelMatrix(),48,bumper.hit ? 1.0f : .15f);
    for(const auto& target : targets)special(target,target.getModelMatrix(),24,target.down ? .8f : .2f);
    special(leftFlipper,leftFlipper.getModelMatrix(),32,.1f);
    special(rightFlipper,rightFlipper.getModelMatrix(),32,.1f);
    special(plunger,plunger.transform.getModelMatrix(),96,0,.4f);
    special(ball,ball.transform.getModelMatrix(),96,.12f,.72f);
}

void PinballMachine::selectNextObject() {
    selectedObjectIndex = (selectedObjectIndex + 1) % 7;
    selectionMode = SelectionMode::OBJECT;
}

void PinballMachine::moveSelectedObject(const glm::vec3& delta) {
    if (selectionMode != SelectionMode::OBJECT) return;

    glm::vec3 movement = delta * 2.0f * 0.016f;

    switch (selectedObjectIndex) {
        case 0:
            ball.transform.position += movement;
            ball.velocity = glm::vec3(0.0f);
            clampObjectToTable(ball.transform.position, ball.radius);
            seatBall();
            break;
        case 1:
        case 2:
        case 3:
            bumpers[selectedObjectIndex - 1].transform.position += movement;
            clampObjectToTable(bumpers[selectedObjectIndex - 1].transform.position, bumpers[selectedObjectIndex - 1].radius);
            bumpers[selectedObjectIndex - 1].transform.position.z =
                playfield.calculateZ(bumpers[selectedObjectIndex - 1].transform.position.y) +
                bumpers[selectedObjectIndex - 1].height * 0.5f;
            break;
        case 4:
            leftFlipper.pivot += movement;
            clampObjectToTable(leftFlipper.pivot, 0.2f);
            leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5:
            rightFlipper.pivot += movement;
            clampObjectToTable(rightFlipper.pivot, 0.2f);
            rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6:
            plunger.transform.position += movement;
            clampObjectToTable(plunger.transform.position, 0.12f);
            plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;
            break;
    }
}

void PinballMachine::rotateSelectedObject(float delta) {
    if (selectionMode != SelectionMode::OBJECT) return;
    if (selectedObjectIndex == 4) leftFlipper.rotate(delta);
    if (selectedObjectIndex == 5) rightFlipper.rotate(delta);
}

void PinballMachine::resetSelectedObject() {
    if (selectionMode != SelectionMode::OBJECT) return;
    const float surface = playfield.baseHeight;

    switch (selectedObjectIndex) {
        case 0:
            ball.reset();
            seatBall();
            break;
        case 1:
            bumpers[0].transform.position = glm::vec3(-1.45f, 2.9f, surface + 0.225f);
            break;
        case 2:
            bumpers[1].transform.position = glm::vec3(0.0f, 4.05f, surface + 0.225f);
            break;
        case 3:
            bumpers[2].transform.position = glm::vec3(1.45f, 2.9f, surface + 0.225f);
            break;
        case 4:
            leftFlipper.reset();
            leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5:
            rightFlipper.reset();
            rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6:
            plunger.reset();
            plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;
            break;
    }
}

void PinballMachine::selectLight(int index) {
    if (index >= 0 && index < static_cast<int>(lights.size())) {
        selectedLightIndex = index;
        selectionMode = SelectionMode::LIGHT;
    }
}

void PinballMachine::moveSelectedLight(const glm::vec3& delta) {
    if (selectionMode != SelectionMode::LIGHT) return;
    lights[selectedLightIndex].position += delta * 2.0f * 0.016f;
}

void PinballMachine::adjustLightIntensity(float delta) {
    if (selectionMode != SelectionMode::LIGHT) return;
    lights[selectedLightIndex].intensity = std::clamp(lights[selectedLightIndex].intensity + delta * 0.1f, 0.0f, 3.0f);
}

void PinballMachine::resetSelectedLight() {
    if (selectionMode != SelectionMode::LIGHT) return;
    lights[selectedLightIndex].position = bumpers[selectedLightIndex].transform.position + glm::vec3(0.0f, 0.0f, 3.5f);
    lights[selectedLightIndex].intensity = 1.35f;
}

void PinballMachine::setShadingMode(ShadingMode mode) {
    shadingMode = mode;
}

void PinballMachine::setLeftFlipperPowered(bool on) {
    leftFlipper.setPowered(on);
}

void PinballMachine::setRightFlipperPowered(bool on) {
    rightFlipper.setPowered(on);
}

void PinballMachine::setPlungerPulling(bool on) {
    if (gameOver || ballState != BallState::READY) { plunger.setPulling(false); return; }
    if (!on && plunger.pulling) {
        launchFromLane(plunger.releaseLaunchSpeed());
        return;
    }
    plunger.setPulling(on);
}
