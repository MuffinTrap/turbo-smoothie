
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

static const int CHESS_AMOUNT = 8;

static const float shipScale = 0.11f;
static const float terrainScale = 50.0f;
static V2f shipVelocity;
static V2f shipAcceleration;

static Texture* ChessPieces[CHESS_AMOUNT];
static Node* ChessNodes[CHESS_AMOUNT];
static V3f PiecePositions[CHESS_AMOUNT];
static const char* PieceNames[] = {"pw", "pb", "kw", "kb", "rw", "rb", "bw", "bb"};
const float pieceScale = 8.0f;


static bool shipHit = false;
static float lastHitTime = 0.0f;
static float hitDuration = 1.0f;
static Palette* cursorPalette;

Example::Example()
{

}

void Example::Init()
{
    // Sprites, images and fonts

    debugFont = DefaultFont_GetDefaultFont();

    // Audio
    blips[0] = mgdl_LoadSound("assets/Pickup1.wav");
    blips[1] = mgdl_LoadSound("assets/Pickup2.wav");
    blips[2] = mgdl_LoadSound("assets/Pickup3.wav");
    blips[3] = mgdl_LoadSound("assets/Pickup4.wav");
    hit = mgdl_LoadSound("assets/Hit.wav");
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
    Transform* shipTransform = Transform_Create(
        V3f_Create(0, -8, 0),
                                        V3f_Create(0, 0, 0),
                                                   V3f_Create(shipScale, shipScale, shipScale));
    shipNode->transform = shipTransform;

    Scene_AddChildNode(gameScene, root, shipNode);

    // Obstacles?!
    ChessPieces[0] = mgdl_LoadTexture("assets/Chess Pieces/pawn_white.png", TextureFilterModes::Nearest);
    ChessPieces[1] = mgdl_LoadTexture("assets/Chess Pieces/pawn_black.png", TextureFilterModes::Nearest);
    ChessPieces[2] = mgdl_LoadTexture("assets/Chess Pieces/knight_white.png", TextureFilterModes::Nearest);
    ChessPieces[3] = mgdl_LoadTexture("assets/Chess Pieces/knight_black.png", TextureFilterModes::Nearest);
    ChessPieces[4] = mgdl_LoadTexture("assets/Chess Pieces/rook_white.png", TextureFilterModes::Nearest);
    ChessPieces[5] = mgdl_LoadTexture("assets/Chess Pieces/rook_black.png", TextureFilterModes::Nearest);
    ChessPieces[6] = mgdl_LoadTexture("assets/Chess Pieces/bishop_white.png", TextureFilterModes::Nearest);
    ChessPieces[7] = mgdl_LoadTexture("assets/Chess Pieces/bishop_black.png", TextureFilterModes::Nearest);
    // Chess piece Nodes to terrain
    float boardLeft_x = -20.2f;
    float between_pieces = 5.7f;
    pieceQuad = Mesh_CreateQuad(FlagUVs);
    Random_SetSeed(0);
    for (int i = 0; i < CHESS_AMOUNT; i++)
    {
        ChessNodes[i] = Node_Create(1);
        Material* pieceMaterial = Material_Load(PieceNames[i], ChessPieces[i], MaterialType::Diffuse);
        Scene_AddMaterial(gameScene, pieceMaterial);
        Node_SetContent(ChessNodes[i], PieceNames[i], pieceQuad, pieceMaterial);
        ChessNodes[i]->transform = Transform_Create(
            V3f_Create(boardLeft_x + i * between_pieces,
                       -pieceScale + pieceScale/4.0f, 0),
            V3f_Create(0, 0, 0),
            V3f_Create(pieceScale, pieceScale, pieceScale));
        Scene_AddChildNode(gameScene, root, ChessNodes[i]);

        // Randomize starting positions
        PiecePositions[i].z = Random_Float(-80.0f, -30.0f);
    }


    menu = Menu_CreateWindowed(debugFont, 1.0f, 1.1f, 256, 256, "Tweak");

    musicLooping = Music_GetLooping(sampleMusic);

    cameraDistance = 30.0f;
    shipVelocity = V2f_Create(0,0);
    shipAcceleration = V2f_Create(1.0f, 1.0f);

    cursorPalette = Palette_GetDefault();

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
    float shipMoveSpeed = 80.0f;
    float maxvel_x = 10.0f;
    float maxvel_z = 10.0f;

    vec2 controller_dir = WiiController_GetNunchukJoystickDirection(cr0);
    float shipx = shipNode->transform->position.x;
    float shipz = shipNode->transform->position.z;

    vec2 dir;
    V2f_Normalize(controller_dir, dir);
    float deadzone = 0.3f;

    if (abs(dir.x) > deadzone)
    {
        shipVelocity.x += dir.x * shipAcceleration.x * shipMoveSpeed * deltaTime;
    }
    else {
        float decrease = shipVelocity.x * 0.9f;
        if (shipVelocity.x > 0)
        {
            decrease *= -1.0f;
        }
        shipVelocity.x += decrease * deltaTime;
    }

    if (abs(dir.y) > deadzone)
    {
        shipVelocity.y += dir.y * shipAcceleration.y * shipMoveSpeed * deltaTime;
    }
    else {
        float decrease = shipVelocity.y * 0.9f;
        if (shipVelocity.y > 0)
        {
            decrease *= -1.0f;
        }
        shipVelocity.y += decrease * deltaTime;
    }

    shipVelocity.x = clampF(shipVelocity.x, -maxvel_x, maxvel_x);
    shipVelocity.y = clampF(shipVelocity.y, -maxvel_z, maxvel_z);

    shipx += shipVelocity.x * deltaTime;
    shipz += shipVelocity.y * deltaTime;

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
        if (quad->uvs[i] > 2.0f)
        {
            //quad->uvs[i] -= 1.0f;
        }
    }

    // Move the pieces
    shipHit = false;
    float pieceSpeed = 15.0 + fruits->GetCollectedAmount() * 5;
    for(int i = 0; i < CHESS_AMOUNT; i++)
    {
        PiecePositions[i].z += pieceSpeed * deltaTime;
        if (PiecePositions[i].z > 30.0f)
        {
            PiecePositions[i].z = Random_Float(-30, -80);
            ChessNodes[i]->transform->rotationDegrees.x = 0;
            ChessNodes[i]->transform->position.y = -pieceScale + pieceScale/4.0f;
        }
        ChessNodes[i]->transform->position.z = PiecePositions[i].z;

        // Does the ship hit a piece?
        if (
            (abs( ChessNodes[i]->transform->position.z - shipNode->transform->position.z) < 1.3f) &&
            (abs( ChessNodes[i]->transform->position.x - shipNode->transform->position.x) < 2.0f))
        {
            // Hit!
            if (mgdl_GetElapsedSeconds() - lastHitTime > hitDuration)
            {
                //Flatten the piece
                ChessNodes[i]->transform->rotationDegrees.x = -90;
                ChessNodes[i]->transform->position.y = -pieceScale;
                shipHit = true;
                lastHitTime = mgdl_GetElapsedSeconds();
                Sound_Play(hit);
                for (int i = 0; i < FRUIT_COUNT; i++)
                {
                    if( fruits->collectedFruits[i] == 1)
                    {
                        fruits->collectedFruits[i] = 0;
                        break;
                    }
                }
            }
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
    //DrawScene(shipScene, scale);

    // Where is that ship!
    /*
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    for(int i = 0; i < CHESS_AMOUNT; i++)
    {
        // Does the ship hit a piece?
        glVertex3f(
        ChessNodes[i]->transform->position.x,
        ChessNodes[i]->transform->position.y,
        ChessNodes[i]->transform->position.z);
        glVertex3f(
            shipNode->transform->position.x,
            shipNode->transform->position.y,
            shipNode->transform->position.z);
    }
    glEnd();
    */


    mgdl_InitOrthoProjection();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    DrawFruits();

    float time = mgdl_GetElapsedSeconds() * 60;
    float crossHairRadius = 32.0f;
    float rad = crossHairRadius;
    float dirx = 1.0f;
    float diry = 1.0f;
    // Draw cursor
    glPushMatrix();
    glLineWidth(4.0f);
    glTranslatef(cursorPos.x, cursorPos.y, 0.0f);
    for (int i = 0; i < 3; i++)
    {
        glRotatef(time + i * 30, 0.0f, 0.0f, 1.0f);
        float x1 = 0.0f - dirx * rad/2.0f;
        float y1 = 0.0f + diry * rad/2.0f;

        Color4f c = Palette_GetColor4f(cursorPalette, 2+i);
        Draw2D_RectLinesWH(x1, y1, crossHairRadius, crossHairRadius, &c);
    }
    glLineWidth(1.0f);
    glPopMatrix();

    //Scene_DebugDraw(shipScene, menu, 10, 10, (Scene_DebugFlag::Index + Scene_DebugFlag::Position));
    //DrawMenu();


}

void Example::DrawFruits()
{
    bool got = fruits->Draw(cursorPos, mouseClick);
    if (got)
    {
        Sound_Play(blips[Random_Int(0,3)]);
    }
}

void Example::DrawScene ( Scene* scene, V3f scale)
{
    // Try to draw Wii 3D model
    mgdl_InitPerspectiveProjection(75.0f, 0.1f, 100.0f);
    mgdl_InitCamera(V3f_Create(0.0f, 0.0f, cameraDistance), V3f_Create(0.0f, 0.0f, 0.0f), V3f_Create(0.0f, 1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	//glDepthMask(GL_TRUE); //  is this needed?

	// This is the other way around on Wii, but
	// hopefully OpenGX handles it
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_FLAT);

    mgdl_glSetAlphaTest(true);
    mgdl_glSetTransparency(true);

    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();

    glTranslatef(0.0f, 0.0f, 0.0f);
    float elp = mgdl_GetElapsedSeconds();
    glRotatef(elp * sceneRotation.x * 10.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(elp * sceneRotation.y * 10.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(elp * sceneRotation.z * 10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(scale.x, scale.y, scale.z);

    Scene_Draw(scene);
    mgdl_glSetTransparency(false);
    mgdl_glSetAlphaTest(false);

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
    int w = 140;
    int x = mgdl_GetScreenWidth() - w;
    int y = mgdl_GetScreenHeight() - 8;

    Menu_Start(menu, x, y, w);

    int collected = fruits->GetCollectedAmount();

    Menu_TextF(menu, "Fruits %d/%d", collected, FRUIT_COUNT);

    AdjustNodeTransform(menu, terrainNode, terrainScale);
    AdjustNodeTransform(menu, shipNode, shipScale);
    AdjustNodeTransform(menu, ChessNodes[1], 1.0f );

    if (shipHit)
    {
        Menu_Text(menu, "HIT!");
    }


#if MGDL_ROCKET
    if (Menu_Button(menu, "Write Rocket"))
    {
        Rocket_SaveAllTracks();
    }
#endif
    Menu_DrawCursor(menu);
}

