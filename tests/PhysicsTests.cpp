#include "PinballMachine.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void place(PinballMachine& m, glm::vec2 p, glm::vec2 velocity) {
    m.ballState=BallState::IN_PLAY; m.ballSaveTimer=0;
    m.ball.transform.position=glm::vec3(p,m.playfield.calculateZ(p.y)+m.ball.radius);
    m.ball.velocity=glm::vec3(velocity,0);
}
void launchTest(float dt, float power) {
    PinballMachine m;
    m.update(2);
    require(m.ballState==BallState::READY && m.lives==3,"Idle ball must wait in lane");
    m.launchFromLane(power);
    auto p=m.ball.transform.position;auto v=m.ball.velocity;
    m.launchFromLane(20);
    require(glm::length(m.ball.transform.position-p)<.00001f && glm::length(m.ball.velocity-v)<.00001f,"Active Space must not reset ball");
    bool reached=false;
    for(float time=0;time<4;time+=dt) {
        m.update(dt);
        require(std::isfinite(m.ball.transform.position.x),"Finite launch physics");
        require(std::abs(m.ball.velocity.z)<.00001f,"No accumulated vertical velocity");
        require(std::abs(m.ball.transform.position.z-m.playfield.calculateZ(m.ball.transform.position.y)-m.ball.radius)<.0001f,"Ball follows board slope");
        if(m.ballState==BallState::IN_PLAY) {reached=true;break;}
    }
    require(reached,"Launch must leave shooter lane at dt="+std::to_string(dt)+" power="+std::to_string(power));
}
int main() {
    try {
        for(float dt : {1.0f/30,1.0f/60,1.0f/144,.1f})
            for(float power : {14.0f,18.0f,20.0f}) launchTest(dt,power);
        PinballMachine m;
        m.setPlungerPulling(true);m.update(.25f);m.update(.25f);m.setPlungerPulling(false);
        require(m.ballState==BallState::SHOOTER && m.ball.velocity.y>16,"Enter release launches charged ball");
        m.resetGame();m.launchFromLane(18);m.ball.transform.position.y=-7;m.ball.transform.position.x=0;m.checkCollisions();
        require(m.lives==3 && m.ballState==BallState::READY && m.lastEvent=="BALL SAVED","Early drain consumes save, not a life");
        m.resetGame();
        for(auto& t : m.targets) {
            place(m,{t.transform.position.x,t.transform.position.y-.2f},{0,5});
            m.checkCollisions();
        }
        require(m.score==1400 && m.multiplier==2 && m.bonus==200,"Four targets award 400, bank 1000 and 2x bonus");
        int before=m.score; m.checkCollisions();
        require(m.score==before,"Resting contact cannot score again");
        place(m,{0,-7},{0,-5});m.checkCollisions();
        require(m.score==1800 && m.lives==2 && m.ballNumber==2,"Drain collects 200 x 2 bonus once");
        m.checkCollisions();require(m.lives==2 && m.score==1800,"Waiting ball cannot repeatedly drain");
        for(int ball=0;ball<2;++ball) {place(m,{0,-7},{0,-5});m.checkCollisions();}
        require(m.gameOver && m.lives==0 && m.ballNumber==3,"Exactly three lives");
        m.launchFromLane(18);m.addScore(100);
        require(m.score==1800 && m.ballState==BallState::READY,"Game over blocks launches and scoring");
        m.resetGame(); require(m.highScore==1800 && m.score==0 && m.multiplier==1 && m.lives==3,"Reset keeps session best and clears game");
        auto& b=m.bumpers[0];
        place(m,{b.transform.position.x,b.transform.position.y-b.radius-m.ball.radius+.01f},{0,6});
        m.checkCollisions(); require(m.score==100,"Bumper impact awards points");
        int hitScore=m.score;
        place(m,{b.transform.position.x,b.transform.position.y-b.radius-m.ball.radius+.01f},{0,6});
        m.checkCollisions(); require(m.score==hitScore,"Bumper cooldown prevents duplicate points");
        b.hitTimer=0;place(m,{b.transform.position.x,b.transform.position.y},{0,6});m.checkCollisions();
        require(glm::length(m.ball.velocity)<=m.ball.maxSpeed+10,"Coincident bumper contact has bounded normal");
        m.resetGame();place(m,{0,-4.8f},{0,-2});
        for(int i=0;i<1000 && m.ballState!=BallState::READY;++i)m.update(1.0f/240);
        require(m.lives==2,"Unpowered flippers leave a usable central drain");
        m.resetGame();place(m,{-1.0f,-4.65f},{0,-4});m.setLeftFlipperPowered(true);
        float upward=0;for(int i=0;i<24;++i){m.update(1.0f/240);upward=std::max(upward,m.ball.velocity.y);}
        require(upward>5,"Powered flipper sends ball upfield");
        require(m.score==0,"Flipper contact never farms points");
        // Replay the same physics duration at different render rates.
        PinballMachine a,c;a.launchFromLane(18);c.launchFromLane(18);
        for(int i=0;i<60;++i)a.update(1.0f/30);
        for(int i=0;i<288;++i)c.update(1.0f/144);
        require(glm::length(a.ball.transform.position-c.ball.transform.position)<.03f && a.score==c.score,"Deterministic frame-rate independent motion");
        std::cout<<"PASS: 12 launch cases, charging, saves, scoring, lives, flippers and frame-rate consistency\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
