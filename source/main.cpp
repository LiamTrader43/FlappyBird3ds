#include <3ds.h>
#include <citro2d.h>
#include <tex3ds.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

u32 clrPipe = C2D_Color32(0x25, 0xE7, 0x37, 0xFF);
u32 clrClear = C2D_Color32(0x4D, 0xC3, 0xFF, 0xFF);
u32 clrPlayer = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);

float pipeX[4] = {400.0f, 525.0f, 650.0f, 775.0f};
int pipeY[4];

bool gameStarted = false;
bool gameOver = false;

float playerVelocity = 0.01;
float playerAcceleration = 0.05;
float playerY = 120;
float playerX = 100;

int score = 0;

static C2D_SpriteSheet spriteSheet;

void spawnPipe()
{
    std::srand(std::time(0));
    for(int i = 0; i < 4; i++)
    {
        pipeY[i] = std::rand() % 50 + 50; 
    }
}

void drawPipe(int x, int y)
{
    C2D_Sprite topPipe;
    C2D_SpriteFromSheet(&topPipe, spriteSheet, 2);
    C2D_SpriteSetRotationDegrees(&topPipe, 180);
    
    C2D_SpriteSetPos(&topPipe, (x + 50), y);
    C2D_DrawSprite(&topPipe);
    
    C2D_Sprite bottomPipe;
    C2D_SpriteFromSheet(&bottomPipe, spriteSheet, 1);
    C2D_SpriteSetPos(&bottomPipe, x, (0+y+90));
    C2D_DrawSprite(&bottomPipe);
}

void updatePipe()
{
    for(int i = 0; i < 4; i++)
    {
        drawPipe(pipeX[i], pipeY[i]);
        pipeX[i] -= 1.5;
        if(pipeX[i] <= -50)
        {
            pipeX[i] = 450;
            pipeY[i] = std::rand() % 110 + 20;            
        }

        if(pipeX[i] == 50.5 || pipeX[i] == 50 || pipeX[i] == 49.5)
        {
            score += 1;
        }

        //top pipe collision
        if ((pipeX[i] < playerX + 20) && ((pipeX[i] + 50) > playerX))
        {   
            if (playerY < pipeY[i])
            {
                gameStarted = false;
            }
        }

        //bottom pipe collision
        if ((pipeX[i] < playerX + 20) && (pipeX[i] + 50 > playerX))
        {
            if (playerY + 20 > pipeY[i] + 90)
            {
                gameStarted = false;
            }
        }
    }
}

void drawPlayer()
{
    C2D_Sprite player;
	C2D_SpriteFromSheet(&player, spriteSheet, 0);
	C2D_SpriteSetPos(&player, playerX, playerY);
    C2D_SpriteSetDepth(&player, 0);
    C2D_DrawSprite(&player);
}

int main(int argc, char* argv[])
{
    romfsInit();
    ndspInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_BOTTOM, NULL);

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    spawnPipe();

    spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");

    while (aptMainLoop())
    {
        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) break;

        printf("\x1b[20;11HPress Start to Exit");

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, clrClear);
        C2D_SceneBegin(top);
        
        printf("\x1b[1;16HScore: %i\n", score);
        
        if(gameStarted)
        {
            updatePipe();
            drawPlayer();
            playerVelocity += playerAcceleration;
            playerY += playerVelocity;
            printf("\x1b[17;11H                                 ");
            printf("\x1b[17;15HTap to Flap");
        }
        else
        {
            printf("\x1b[17;12HPress A to Start");
            if(kDown & KEY_A)
            {
                gameStarted = true;
                score = 0;
                printf("\x1b[1;16H                           ");
            }
            playerVelocity = 0.01;
            playerAcceleration = 0.01;
            playerY = 120;
            playerX = 100;
            for(int i = 0; i < 4; i++)
            {
                pipeX[i] = 400 + (i * 125);
            }
            spawnPipe();   
        }

        if(kDown & KEY_TOUCH)
        {
            playerVelocity = -2.2;
        }
        else
        {
            playerAcceleration = 0.1;
        }

        if(playerY >= 240 || playerY <= 0)
        {
            gameStarted = false;
        }

        C3D_FrameEnd(0);
    }
    
    C2D_SpriteSheetFree(spriteSheet);
    
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}