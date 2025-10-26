#pragma once

#include <mgdl.h>
#include "fruitformation.h"

enum GameState
{
    Intro,
    Play,
    Win
};

class Example
{
    Font* debugFont;
    Music* sampleMusic;
    Sound* blips[4];
    Sound* hit;
    Menu* menu;
    Menu* controllerMenu;

    Texture* checkerTexture;
    Texture* cloudBg;
    Scene* gameScene;

    FruitFormation* fruits;

    GameState state;

    // Terrain
    Mesh* quad;
    Node* terrainNode;

    Mesh* pieceQuad;

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
        bool Update_Init();
        bool Update_Game();
        bool Update_Win();
        void Draw();
        void Draw_Init();
        void Draw_Game();
        void Draw_Win();
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
