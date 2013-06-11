#ifndef __nativeGraphics_levels_basicLevel__
#define __nativeGraphics_levels_basicLevel__

#include "HUD.h"

#define PAN_LERP_FACTOR .04 // TODO
#define PI 3.1415f // TODO

class basicLevel {
public:
    basicLevel(const char * mazeFile);
    void RotatePerspective();
    virtual void RenderFrame();
    void RestartLevel();
    void FreeLevel();
    
    Character * character;
    RenderObject * cave;
    RenderLight * bigLight;
    RenderDestructible *destructible;
    HUD * hud;
    float health;

    Vector3f cameraPos;
    Vector3f cameraPan;
    
    bool goalReached;
    float transitionLight;
};

void basicLevel::RestartLevel() {
    
}

void basicLevel::FreeLevel() {
    free(character);
    free(cave);
    free(bigLight);
    free(hud);
}

basicLevel::basicLevel(const char * mazeFile) {

    cameraPos = Vector3f(0, 180, 100);
    cameraPan = Vector3f(0, 200, 0);

    cave = new RenderObject(mazeFile, NULL, "caustics_f.glsl");
    cave->AddTexture("caustic_albedo.jpg", false);
    
    character = new Character("submarine.obj", NULL, "albedo_f.glsl");
    character->AddTexture("submarine_albedo.jpg", false);
    struct characterInstance instance;
    character->instances.push_back(instance);
    
    destructible = new RenderDestructible("submarine.obj", NULL, "albedo_f.glsl");
    destructible->AddTexture("submarine_albedo.jpg", false);
    
    bigLight = new RenderLight("square.obj", "dr_square_v.glsl", "dr_pointlight_f.glsl");
    
    hud = new HUD();
    health = 1.0f;
    goalReached = false;
    transitionLight = 0.0f;
};

// Clamps input to (-max, max) according to curve.
inline float tanClamp(float input, float max) {
    return max * atan(input / max);
}

// Rotate around the subject based on orientation
void basicLevel::RotatePerspective() {
    float rot0 = tanClamp( orientation[2], PI / 10.0f);
    float rot2 = tanClamp(-orientation[1], PI / 10.0f);
    
    translatef(cameraPan[0], cameraPan[1], cameraPan[2]);
    rotate(rot0, 0, rot2);
    translatef(-cameraPan[0], -cameraPan[1], -cameraPan[2]);
}

void basicLevel::RenderFrame() {
    // Setup perspective matrices
    pLoadIdentity();
    perspective(90, (float) displayWidth / (float) displayHeight, 60, 800);
    
    mvLoadIdentity();
    lookAt(cameraPos(0)+cameraPan(0), cameraPos(1)+cameraPan(1), cameraPos(2)+cameraPan(2), cameraPan(0), cameraPan(1), cameraPan(2), 0, 1, 0);
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
    
    cameraPan = (1.0 - PAN_LERP_FACTOR) * cameraPan + PAN_LERP_FACTOR * character->instances[0].position;
        
    // Run physics.
    character->Update();
    
    mvPushMatrix();
    translate(character->instances[0].position);
    rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    scalef(.15f);
    if (health < .05) {
        destructible->Render();
    } else {
        character->Render();
    }
    mvPopMatrix();
    
    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    mvPushMatrix();
    translate(character->instances[0].position);
    bigLight->color[0] = 1.0;
    bigLight->color[1] = 1.0;
    bigLight->color[2] = 0.8;
    bigLight->brightness = 16000.0;// * health;
    bigLight->Render();
    mvPopMatrix();
    
    //hud->Render(0.8f);
}


#endif // __nativeGraphics_levels_basicLevel__
