// level1.h
// nativeGraphics

#ifndef __nativeGraphics_levels_level1__
#define __nativeGraphics_levels_level1__

#include "basicLevel.h"

#define BOMB_TIMER_LENGTH 2.0f
#define BOMB_EXPLOSION_LENGTH .3f

class level1 : public basicLevel {
public:
    level1(const char * mazeFile, Vector3f target);
    void RenderFrame();
    void RestartLevel();
    
private:
    void addJellyfish();
    void addOctopus();

    Character * jellyfish;
    Character * octopus;
    PhysicsObject * bomb;

    Fluid * Water;

    RenderLight * smallLight;
    RenderLight * explosiveLight;
    RenderLight * spotLight;
    
    
    bool shotBomb;
    Timer frameRate;
    
    bool dead;
    Timer deathTimer;
    
    Vector3f goal;
};

level1::level1(const char * mazeFile, Vector3f target) : basicLevel(mazeFile) {
    
    jellyfish = new Character("jellyfish.obj", NULL, "albedo_f.glsl");
    jellyfish->AddTexture("jellyfish_albedo.jpg", false);
    
    octopus = new Character("octopus.obj", NULL, "albedo_f.glsl");
    octopus->AddTexture("octopus_albedo.jpg", false);
    
    bomb = new PhysicsObject("icosphere.obj", NULL, "solid_color_f.glsl");
        
    Water = new Fluid(NULL, "solid_color_f.glsl");

    smallLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_sat_f.glsl");
    explosiveLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_explosive_pointlight_f.glsl");
    spotLight = new RenderLight("cone.obj", "dr_standard_v.glsl", "dr_spotlight_f.glsl");
    
    shotBomb = false;
    
    goal = target;
    
    frameRate.reset();
    
    dead = false;

}

void level1::RestartLevel() {
    destructible = new RenderDestructible("submarine.obj", NULL, "albedo_f.glsl");
    destructible->AddTexture("submarine_albedo.jpg", false);
    
    cameraPos = Vector3f(0, 180, 100);
    cameraPan = Vector3f(0, 200, 0);

    health = 1.0;
    character->instances[0].position = Vector3f(0,0,0);
    character->instances[0].targetPosition = Vector3f(0,0,0);
    character->instances[0].velocity = Vector3f(0,0,0);
    jellyfish->instances.clear();
    octopus->instances.clear();
    bomb->instances.clear();
    
    transitionLight = 0.0;
    
    dead = false;
    goalReached = false;
    
}

void level1::addJellyfish() {
    struct characterInstance instance;
    instance.position = character->instances[0].position + 600.0f * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    instance.MaxAcceleration = 200.0f;
    instance.Drag = 100.0f;
    instance.MaxVelocity = 100.0f;
    jellyfish->instances.push_back(instance);
}

void level1::addOctopus() {
    struct characterInstance instance;
    instance.position = character->instances[0].position + 600.0f * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    instance.MaxAcceleration = 300.0f;
    instance.Drag = 100.0f;
    instance.MaxVelocity = 150.0f;
    octopus->instances.push_back(instance);
}

void level1::RenderFrame() {
    
    if(health <= 0.0f && !dead) {
        dead = true;
        deathTimer.reset();
    }
    
    if(dead && deathTimer.getSeconds() > 4.0)
        RestartLevel();
    
    // Setup perspective matrices
    pLoadIdentity();
    perspective(90, (float) displayWidth / (float) displayHeight, 60, 800);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+cameraPan[0], cameraPos[1]+cameraPan[1], cameraPos[2]+cameraPan[2], cameraPan[0], cameraPan[1], cameraPan[2], 0, 1, 0);
    RotatePerspective();

    //////////////////////////////////
    // Render to g buffer.
    
    /** Any geometry that will be collision detected
        should be rendered here, before user input. **/
    mvPushMatrix();
    scalef(200);
    cave->Render();
    mvPopMatrix();
    
    // Process user input
    if(!dead) {
        if(touchDown) {
            uint8_t * geometry = pipeline->RayTracePixel(lastTouch[0], 1.0f - lastTouch[1], true);
            if(geometry[3] != 255) {
                float depth = geometry[3] / 128.0f - 1.0f;            
                Matrix4f mvp = projection.top()*model_view.top();
                Eigen::Vector4f pos = mvp.inverse() * Eigen::Vector4f((lastTouch[0]) * 2.0f - 1.0f, (1.0 - lastTouch[1]) * 2.0f - 1.0f, depth, 1.0);
                character->instances[0].targetPosition = Vector3f(pos(0) / pos(3), pos(1) / pos(3), pos(2) / pos(3));
            }
            delete[] geometry;
        }
        
        for(int i = 0; i < 3; i++)
            cameraPan[i] = (1.0 - PAN_LERP_FACTOR) * cameraPan[i] + PAN_LERP_FACTOR * character->instances[0].position[i];
            
        if(touchDown && !shotBomb) {
            struct physicsInstance newBomb;
            newBomb.position = character->instances[0].position;
            newBomb.velocity = 200.0f * Eigen::Vector3f(-cos(character->instances[0].rot[0]), 1.0f, sin(character->instances[0].rot[0]));
            bomb->instances.push_back(newBomb);
            shotBomb = true;
        }
        
        if(!touchDown)
            shotBomb = false;
        
        while(jellyfish->instances.size() < 15)
          addJellyfish();
     
        while(octopus->instances.size() < 7)
          addOctopus();
    }
   
    float timeSinceLast = frameRate.getSeconds();
    frameRate.reset();
    // Run physics.
    bomb->Update();
    character->Update();
    for(int i = 0; i < jellyfish->instances.size(); i++) {
        jellyfish->instances[i].targetPosition = character->instances[0].position;
        // Randomize movement
        float dist = (character->instances[0].position - jellyfish->instances[i].position).norm();
        if(dist < 50.0f)
            health -= .05f * timeSinceLast;
        jellyfish->instances[i].targetPosition += 1.1f * dist * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    }
    for(int i = 0; i < octopus->instances.size(); i++) {
        octopus->instances[i].targetPosition = character->instances[0].position;
        // Randomize movement
        float dist = (character->instances[0].position - octopus->instances[i].position).norm();
        if(dist < 50.0f)
            health -= .05f * timeSinceLast;
        octopus->instances[i].targetPosition += 1.1f * dist * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    }
    health = min(health + .01f * timeSinceLast, 1.0f);
    health = max(health, 0.0f);
    jellyfish->Update();
    octopus->Update();
    Water->Update();
    
    mvPushMatrix();
    translate(character->instances[0].position);
    rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    scalef(.15f);
    if(dead)
        destructible->Render();
    else
        character->Render();
    mvPopMatrix();

    mvPushMatrix();
    translate(character->instances[0].position);
    rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    scalef(10.00f);
    translate(Vector3f(2.5,-0.5,-2));
    rotatef(90, 0,0,-1);
    Water->Render(0,0,0);
    mvPopMatrix();
    
    for(int i = 0; i < jellyfish->instances.size(); i++) {
        mvPushMatrix();
        translate(jellyfish->instances[i].position);
        rotate(0.0, jellyfish->instances[i].rot[0], jellyfish->instances[i].rot[1]);
        rotate(0.0, 0.0, PI / 2);
        scalef(1.0f);
        jellyfish->Render(i);
        mvPopMatrix();
    }
    
    for(int i = 0; i < octopus->instances.size(); i++) {
        mvPushMatrix();
        translate(octopus->instances[i].position);
        rotate(0.0, octopus->instances[i].rot[0], octopus->instances[i].rot[1]);
        rotate(0.0, 0.0, PI / 2);
        scalef(10.0f);
        octopus->Render(i);
        mvPopMatrix();
    }
    
    for(int i = 0; i < bomb->instances.size(); i++) {
        if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(4);
            bomb->Render(i);
            mvPopMatrix();
        }
    }
    
    // Render the goal
    mvPushMatrix();
    translate(goal);
    scalef(50);
    bomb->Render(0);
    mvPopMatrix();

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    mvPushMatrix();
    translate(character->instances[0].position);
    bigLight->color[0] = 1.0 - .1 * transitionLight;
    bigLight->color[1] = 1.0;
    bigLight->color[2] = 0.8 - .1 * transitionLight;
    bigLight->brightness = 32000.0 * health + 320000.0 * transitionLight;
    bigLight->Render();
    mvPopMatrix();
    
    if(dead) {
        mvPushMatrix();
        translate(character->instances[0].position + 40.0f * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f));
        scalef(250);
        explosiveLight->color[0] = 1.00f;
        explosiveLight->color[1] = 0.33f;
        explosiveLight->color[2] = 0.07f;
        explosiveLight->brightness = 10000000.0f;
        explosiveLight->Render();
        mvPopMatrix();
    }
    
    for(int i = 0; i < bomb->instances.size(); i++) {
        if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(100);
            smallLight->color[0] = 1.00f;
            smallLight->color[1] = 0.33f;
            smallLight->color[2] = 0.07f;
            smallLight->brightness = 1500 + 1500 * sin(bomb->instances[i].timer.getSeconds() * 4.0f * PI);
            smallLight->Render();
            mvPopMatrix();
        } else if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH + BOMB_EXPLOSION_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(250);
            explosiveLight->color[0] = 1.00f;
            explosiveLight->color[1] = 1.00f;
            explosiveLight->color[2] = 1.00f;
            float explosionTime = (bomb->instances[i].timer.getSeconds() - BOMB_TIMER_LENGTH) / BOMB_EXPLOSION_LENGTH;
            float intensity = sin(PI * sqrt(explosionTime));
            explosiveLight->brightness = 10000000.0f * intensity;
            explosiveLight->Render();
            mvPopMatrix();
            for(int j = 0; j < jellyfish->instances.size(); j++) {
                if((jellyfish->instances[j].position - bomb->instances[i].position).norm() < 200) {
                    // Kill this jellyfish
                    jellyfish->instances.erase(jellyfish->instances.begin()+j);
                    j--;  
                }
	        }
	        for(int j=0; j < octopus->instances.size(); ++j) {
                if((octopus->instances[j].position - bomb->instances[i].position).norm() < 200) {
                    // Kill this octopus
                    octopus->instances.erase(octopus->instances.begin()+j);
                    j--;  
                }
            }
            // Shake screen
            cameraPan += 5.0f * intensity * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
        } else {
            bomb->instances.erase(bomb->instances.begin()+i);
            i--;
        }
    }
    
    if(!dead) {
        mvPushMatrix();
        translate(character->instances[0].position);
        rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
        rotate(0.0,0,-PI / 2);
        scalef(300.0f);
        spotLight->color[0] = 0.4f;
        spotLight->color[1] = 0.6f;
        spotLight->color[2] = 1.0f;
        spotLight->brightness = 16000.0;
        spotLight->Render();
        mvPopMatrix();
    }
    
    // Render the light around target
    mvPushMatrix();
    translate(goal);
    scalef(150);
    explosiveLight->color[0] = 0.8f;
    explosiveLight->color[1] = 1.0f;
    explosiveLight->color[2] = 0.8f;
    explosiveLight->brightness = 10000000.0f;
    explosiveLight->Render();
    mvPopMatrix();
    
    // TODO: clean this up
    Vector3f delta = goal - character->instances[0].position;
    Eigen::Vector2f delta2 = Eigen::Vector2f(delta(0), -delta(2));
    float angle = atan2(delta2(0), delta2(1));
    float distance = min(delta2.norm() / 1000.0f, 1.0f);
    float target[2] = {distance * sin(angle), distance * cos(angle)};
    
    if((goal - character->instances[0].position).norm() <= 200.0f)
        goalReached = true;
    
    if(goalReached) {
        transitionLight += .2f * timeSinceLast;
        health = min(health + 1.0f * timeSinceLast, 1.0f);
    }
    
    if(transitionLight >= 1.0f)
        RestartLevel();
    
    hud->Render(health, target);
}

#endif // __nativeGraphics_levels_simpleLevel1__
