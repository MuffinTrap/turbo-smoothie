#pragma once

#include <mgdl.h>
#include "fruitformation.h"

class Example
{
    Font* debugFont;
    Music* sampleMusic;
    Sound* blip;
    Menu* menu;
    Menu* controllerMenu;

    Texture* checkerTexture;
    Scene* gameScene;

    FruitFormation* fruits;

    // Terrain
    Mesh* quad;
    Node* terrainNode;

    // Ship
    Node* shipNode;
    Scene* shipScene;

    float cameraDistance = 1.0f;
    V3f sceneRotation;

    float deltaTime;
    float elapsedSeconds;

    // Toggles

    public:
        Example();
        void Init();
        void Update();
        void Draw();
        void Quit();

        void DrawFruits();

        void DrawInputInfo();

    private:
        void DrawMenu();

        // Togglable
        void DrawScene(Scene* scene, V3f scale);

        V2f cursorPos;
        bool mouseClick;
        bool mouseDown;
        bool musicLooping;
};
