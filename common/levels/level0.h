// level0.h
// nativeGraphics

#ifndef __nativeGraphics_levels_level0__
#define __nativeGraphics_levels_level0__

#include "basicLevel.h"

#define BOMB_TIMER_LENGTH 2.0f
#define BOMB_EXPLOSION_LENGTH .3f

class level0 : public basicLevel {
public:
    level0();
    void RenderFrame();
    
private:
    void addJellyfish();

    Character * jellyfish;
    PhysicsObject * bomb;

    Fluid * Water;

    RenderLight * smallLight;
    RenderLight * explosiveLight;
    RenderLight * spotLight;
    RenderDestructible *destructible;
    
    bool shotBomb;

};

level0::level0() : basicLevel() {
    
    jellyfish = new Character("jellyfish.obj", NULL, "albedo_f.glsl"); // TODO: jellyfish shader. It seemed to lower fps.
    jellyfish->AddTexture("jellyfish_albedo.jpg", false);
    
    destructible = new RenderDestructible("cube.obj", NULL, "albedo_f.glsl");
    destructible->AddTexture("submarine_albedo.jpg", false);
    
    bomb = new PhysicsObject("icosphere.obj", NULL, "solid_color_f.glsl");
        
    Water = new Fluid(NULL, "solid_color_f.glsl");

    smallLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_sat_f.glsl");
    explosiveLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_explosive_pointlight_f.glsl");
    spotLight = new RenderLight("cone.obj", "dr_standard_v.glsl", "dr_spotlight_f.glsl");
    
    shotBomb = false;

}

void level0::addJellyfish() {
    struct characterInstance instance;
    instance.position = character->instances[0].position + 600.0f * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    instance.MaxAcceleration = 200.0f;
    instance.Drag = 100.0f;
    instance.MaxVelocity = 100.0f;
    jellyfish->instances.push_back(instance);
}

void level0::RenderFrame() {
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
    scalef(40);
    cave->Render();
    mvPopMatrix();
    
    // Process user input
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
    
    // Run physics.
    bomb->Update();
    character->Update();
    for(int i = 0; i < jellyfish->instances.size(); i++) {
        jellyfish->instances[i].targetPosition = character->instances[0].position;
        // Randomize movement
        float dist = (character->instances[0].position - jellyfish->instances[i].position).norm();
        jellyfish->instances[i].targetPosition += 1.1f * dist * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    }
    jellyfish->Update();
    Water->Update();
    
    mvPushMatrix();
    destructible->Render();
    mvPopMatrix();
    
    mvPushMatrix();
    translate(character->instances[0].position);
    rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    scalef(.15f);
    character->Render();
    mvPopMatrix();

    mvPushMatrix();
    translate(character->instances[0].position);
    //rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    scalef(10.00f);
    Water->Render();
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
    
    for(int i = 0; i < bomb->instances.size(); i++) {
        if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(4);
            bomb->Render(i);
            mvPopMatrix();
        }
    }

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    
    bigLight->PreTranslate();
    mvPushMatrix();
    translate(character->instances[0].position);
    bigLight->color[0] = 1.0;
    bigLight->color[1] = 1.0;
    bigLight->color[2] = 0.8;
    bigLight->brightness = 16000.0;
    bigLight->Render();
    mvPopMatrix();
    
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
            // Shake screen
            cameraPan += 5.0f * intensity * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
        } else {
            bomb->instances.erase(bomb->instances.begin()+i);
            i--;
        }
    }
    
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

#endif // __nativeGraphics_levels_simpleLevel0__
