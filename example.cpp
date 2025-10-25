
#include "example.h"
#include <mgdl/mgdl-draw2d.h>

#include <string>

#ifdef MGDL_ROCKET
    #include <mgdl/mgdl-rocket.h>

    #ifdef SYNC_PLAYER
        #include MGDL_ROCKET_FILE_H
    #else
        static ROCKET_TRACK sync_value;
    #endif
#endif


static const float shipScale = 0.12f;
static const float terrainScale = 50.0f;

Example::Example()
{

}

void Example::Init()
{
    // Sprites, images and fonts

    debugFont = DefaultFont_GetDefaultFont();

    // Audio
    blip = mgdl_LoadSound("assets/blipSelect.wav");
    sampleMusic = mgdl_LoadOgg("assets/sample3.ogg");





    fruits = new FruitFormation();
    fruits->Load();

    // Game scene
    gameScene = Scene_CreateEmpty();
    Node* root = Node_Create(2);
    Scene_AddChildNode(gameScene, nullptr, root);

    // Terrain node
    checkerTexture = Texture_GenerateCheckerBoard();
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, checkerTexture->textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    Material* checkerMaterial = Material_Load("checker", checkerTexture, MaterialType::Diffuse);
    Scene_AddMaterial(gameScene, checkerMaterial );

    quad = Mesh_CreateQuad(FlagNormals | FlagUVs);
    // Mesh* icosaMesh = Mesh_CreateIcosahedron(FlagNormals | FlagUVs);
    terrainNode = Node_Create(1);
    // TODO Gizmos for movement!
    Transform* terrainTransform = Transform_Create(V3f_Create(0,-10,-7.4f),
                                                   V3f_Create(-90, 0, 0),
                                                   V3f_Create(terrainScale, terrainScale, terrainScale));
    Node_SetContent(terrainNode, "terrain", quad, checkerMaterial);
    terrainNode->transform = terrainTransform;
    Scene_AddChildNode(gameScene, root, terrainNode);

    // Ship node
    shipScene = mgdl_LoadFBX("assets/ship.fbx");
    shipNode = Node_FindChildByIndex(Scene_GetRootNode(shipScene), 1);
    Texture* shipTexture = mgdl_LoadTexture("assets/ship_texture.png", TextureFilterModes::Nearest);
    Material* shipMaterial = Material_Load("ship_texture.png", shipTexture, MaterialType::Diffuse);
    Scene_SetAllMaterialTextures(shipScene, shipTexture);

    shipNode->material = shipMaterial;
    Transform* shipTransform = Transform_Create(V3f_Create(0, -9, 0),
                                                   V3f_Create(0, 0, 0),
                                                   V3f_Create(shipScale, shipScale, shipScale));
    shipNode->transform = shipTransform;

    printf("Ship vertices\n");
    for (int i = 0 ; i < 100; i++)
    {
        V3f p = Mesh_GetPosition(shipNode->mesh, i);
        printf("Pos: %d %.2f, %.2f, %.2f\n", i, p.x, p.y, p.z);
    }

    //Scene_AddChildNode(gameScene, root, shipNode);


    menu = Menu_CreateWindowed(debugFont, 1.0f, 1.1f, 256, 256, "Tweak");

    musicLooping = Music_GetLooping(sampleMusic);

    cameraDistance = 30.0f;

    #ifdef MGDL_ROCKET
        // Connect to editor
        RocketTrackFormat trackSource = TrackEditor;
        #ifdef SYNC_PLAYER
            // Read from CPP file
            RocketTrackFormat trackSource = TrackCPP;
        #endif

        bool rocketInit = Rocket_Connect(trackSource, TrackCPP, sampleMusic, 120, 4);
        if (rocketInit == false)
        {
            mgdl_DoProgramExit();
        }

        #ifdef SYNC_PLAYER
            // In release mode start the music and tracks
            Rocket_PlayTracks();
        #else
            // When tracks are in CPP file, this should not be called
            sync_value = Rocket_AddTrack("sync_value");
        #endif
    #endif
}

void Example::Quit()
{
#ifdef MGDL_ROCKET
    Rocket_Disconnect();
#endif
}

void Example::Update()
{
    #ifdef MGDL_ROCKET

        Rocket_UpdateRow();
        float r = Rocket_Float(sync_value);

        #ifndef SYNC_PLAYER

            if (WiiController_ButtonPress(mgdl_GetController(0), WiiButtons::Button2))
            {
                Rocket_SaveAllTracks();
            }
        #endif
    #endif

    elapsedSeconds = mgdl_GetElapsedSeconds();
    deltaTime = mgdl_GetDeltaTime();

    WiiController* cr0 = Platform_GetController(0);
    cursorPos = WiiController_GetCursorPosition(cr0);
    mouseClick = WiiController_ButtonPress(cr0, ButtonA);
    mouseDown = WiiController_ButtonHeld(cr0, ButtonA);


    // Move the ship
    float shipMoveSpeed = 100.0f;

    vec2 dir = WiiController_GetNunchukJoystickDirection(cr0);
    float shipx = shipNode->transform->position.x;
    float shipz = shipNode->transform->position.z;

    shipx += dir.x * shipMoveSpeed * deltaTime;
    shipz += dir.y * shipMoveSpeed * deltaTime;


    // Movement limits
    float minx = -11.0f;
    float maxx = 11.0f;
    float minz = -3.0f;
    float maxz = 13.5f;

    shipNode->transform->position.x = clampF(shipx, minx, maxx);
    shipNode->transform->position.z = clampF(shipz, minz, maxz);

    // Scroll the texture uvs
    for (int i = 1; i < 8; i += 2)
    {
        quad->uvs[i] += deltaTime; // X1
        if (quad->uvs[i] > 1.0f)
        {
            //quad->uvs[i] -= 1.0f;
        }
    }
}
#if 0

void DrawTextDouble(const char* text, short x, short y, float textHeight, Font* font)
{
    glPushMatrix();
        glTranslatef(x, y, 0.0f);
        font->Print(Colors::Black, textHeight, LJustify, LJustify, text);

        glTranslatef(-1, -1, 0.0f);
        font->Print(Colors::LightGreen, textHeight, LJustify, LJustify, text);
    glPopMatrix();
}
#endif

void Example::Draw()
{
    Color4f black = Palette_GetColor4f(Palette_GetDefault(), 5);
    mgdl_glClearColor4f(&black);


    V3f scale = V3f_Create(1,1,1);
    DrawScene(gameScene, scale);
    DrawScene(shipScene, scale);

    // Where is that ship!


    mgdl_InitOrthoProjection();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    DrawFruits();
    //Scene_DebugDraw(shipScene, menu, 10, 10, (Scene_DebugFlag::Index + Scene_DebugFlag::Position));
    DrawMenu();

}

void Example::DrawFruits()
{
    fruits->Draw(cursorPos, mouseClick);
}

void Example::DrawScene ( Scene* scene, V3f scale)
{
    // Try to draw Wii 3D model
    mgdl_InitPerspectiveProjection(75.0f, 0.1f, 100.0f);
    mgdl_InitCamera(V3f_Create(0.0f, 0.0f, cameraDistance), V3f_Create(0.0f, 0.0f, 0.0f), V3f_Create(0.0f, 1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE); //  is this needed?

	// This is the other way around on Wii, but
	// hopefully OpenGX handles it
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_FLAT);

    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();

    glTranslatef(0.0f, 0.0f, 0.0f);
    float elp = mgdl_GetElapsedSeconds();
    glRotatef(elp * sceneRotation.x * 10.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(elp * sceneRotation.y * 10.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(elp * sceneRotation.z * 10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(scale.x, scale.y, scale.z);

    Scene_Draw(scene);

    glPopMatrix();
    glDisable(GL_DEPTH_TEST);


}

void Example::DrawInputInfo()
{
    int x = 10;
    int y = mgdl_GetScreenHeight() - 10;


    Menu_StartInput(controllerMenu, x, y, 100, cursorPos, false, false);

    Menu_Text(controllerMenu, "Buttons");

    // Draw button states
    int buttons[8] = {
        WiiButtons::ButtonA,
        WiiButtons::ButtonB,
        WiiButtons::ButtonPlus,
        WiiButtons::ButtonMinus,
        WiiButtons::Button1,
        WiiButtons::Button2,
        WiiButtons::ButtonZ,
        WiiButtons::ButtonC
        };

    for(int i = 0; i < 8;i++ )
    {
        bool held = WiiController_ButtonHeld(mgdl_GetController(0), buttons[i]);
        Menu_Flag(controllerMenu, WiiController_GetButtonSymbol(buttons[i]), held);
    }

    Menu_Text(controllerMenu, "D pad");
    //DrawDPad(controllerMenu->drawx + 50, controllerMenu->drawy, controllerMenu->textSize);
    Menu_Skip(controllerMenu, controllerMenu->textSize*3);

    Menu_Text(controllerMenu, "Joystick");
    //DrawJoystick(controllerMenu->drawx + 50, controllerMenu->drawy, controllerMenu->textSize);
    Menu_Skip(controllerMenu, controllerMenu->textSize*3);

    float pitch = WiiController_GetPitch(mgdl_GetController(0));
    float yaw = WiiController_GetYaw(mgdl_GetController(0));
    float roll = WiiController_GetRoll(mgdl_GetController(0));
    Menu_TextF(controllerMenu, "Pitch %.0f ", Rad2Deg(pitch));
    Menu_TextF(controllerMenu, "Yaw %.0f", Rad2Deg(yaw));
    Menu_TextF(controllerMenu, "Roll %.0f", Rad2Deg(roll));
}

void AdjustNodeTransform(Menu* menu, Node* node, float scaleMult)
{
    Menu_TextF(menu, "Node %s", node->name);
    float scale = node->transform->scale.x / scaleMult;

    if (Menu_Slider(menu, "Scale", 1.0f, 5.0f, &scale))
    {
        node->transform->scale.x = scale * scaleMult;
        node->transform->scale.y = scale * scaleMult;
        node->transform->scale.z = scale * scaleMult;
    }

    Menu_Slider(menu, "Pos X", -10.0f, 10.0f, &node->transform->position.x);
    Menu_Slider(menu, "Pos Y", -20.0f, 20.0f, &node->transform->position.y);
    Menu_Slider(menu, "Pos Z", -20.0f, 20.0f, &node->transform->position.z);
}

void Example::DrawMenu()
{
    int w = 256;
    int x = mgdl_GetScreenWidth() - w;
    int y = mgdl_GetScreenHeight() - 8;

    Menu_Start(menu, x, y, w);

    int collected = fruits->GetCollectedAmount();

    Menu_TextF(menu, "Fruits %d/%d", collected, FRUIT_COUNT);

    AdjustNodeTransform(menu, terrainNode, terrainScale);
    AdjustNodeTransform(menu, shipNode, shipScale);


#if MGDL_ROCKET
    if (Menu_Button(menu, "Write Rocket"))
    {
        Rocket_SaveAllTracks();
    }
#endif
    Menu_DrawCursor(menu);
}

