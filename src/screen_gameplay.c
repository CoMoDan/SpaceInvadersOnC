/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"

#include <time.h>

static const int screenWidth = 800;
static const int screenHeight = 450;

const float playerWidth = 60;
const float playerHeight = 40;

const int enemiesX = 12;
const int enemiesY = 4;

const float enemiesXSpace = 15;
const float enemiesYSpace = 10;

const float shootWidth = 4;
const float shootHeight = 15;
const float shootSpeed = 10.0f;

int win;
int loose;

int score;

//----------------------------------------------------------------------------------
// Structs
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    float speed;
    int isAlive;
} Player;

typedef struct Enemy {
    Vector2 position;
    int isAlive;
    float speed;
    float width;
    float height;
} Enemy;

typedef struct Shoot {
    Vector2 position;
} Shoot;


//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;

int moverDerecha = 0;

Player player = { 0 };
Enemy enemies[4][12];

Texture2D textureBackground, playerTexture, playerTexture, covaTexture, invadersTextures[4];

Sound shootFx = { 0 }, invaderkilledFx = { 0 }, explosionFx = { 0 };

//----------------------------------------------------------------------------------
// Pilas para disparos
//----------------------------------------------------------------------------------
// Definition of Stack structure to hold Shoot elements
typedef struct {
    Shoot arr[10];  // Fixed-size stack with a capacity of 10
    int top;        // Index of the top element
} Stack;


Stack shootsPlayerStack;

Stack shootsEnemyStack;

// 
// Initializes the stack
void initializeStack(Stack* stack);

// Checks if the stack is full
int isFull(Stack* stack);

// Checks if the stack is empty
int isEmpty(Stack* stack);

// Pushes a Shoot element onto the stack
void push(Stack* stack, Shoot shoot);

// Pops a Shoot element from the stack
Shoot pop(Stack* stack);

void removeAt(Stack* stack, int i);


// Returns the top element of the stack without removing it
Shoot peek(Stack* stack);

// Traverses and prints all elements in the stack
void traverseStack(Stack* stack);

//Funcion dibujar disparos
dibujarDisparos(Stack *shootsStack, Color color) {
    if (isEmpty(shootsStack) == 0) {
        for (int i = shootsStack->top; i >= 0; i--) {
            Rectangle shootRect = {
            shootsStack->arr[i].position.x - shootWidth / 2,
            shootsStack->arr[i].position.y - shootHeight / 2,
            shootWidth, shootHeight
            };
            DrawRectangleRec(shootRect, color);
        }
    }
}


//----------------------------------------------------------------------------------
// Texto personlizado con bordes
//----------------------------------------------------------------------------------

void DrawTextWithBorder(const char* text, int posX, int posY, int fontSize, Color textColor, int borderSize, Color borderColor) {
    // Dibujar el texto en posiciones desplazadas para crear el efecto de borde
    DrawText(text, posX - borderSize, posY, fontSize, borderColor); // Izquierda
    DrawText(text, posX + borderSize, posY, fontSize, borderColor); // Derecha
    DrawText(text, posX, posY - borderSize, fontSize, borderColor); // Arriba
    DrawText(text, posX, posY + borderSize, fontSize, borderColor); // Abajo

    // Dibujar el texto original encima
    DrawText(text, posX, posY, fontSize, textColor);
}

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    char resourcesPath[200];
    strcpy(resourcesPath, GetWorkingDirectory());
    size_t length = strlen(resourcesPath);
    resourcesPath[length - 27] = '\0';  // Ajusta el terminador nulo para recortar
    strcat(resourcesPath, "src\\resources\\");

    char temporalChain[200];

    // TODO: Initialize GAMEPLAY screen variables here!
    framesCounter = 0;
    finishScreen = 0;

    //reset win loose
    win = 0;
    loose = 0;

    //reset score
    score = 0;

    //Iniciar jugador
    player.position = (Vector2){ screenWidth / 2 , screenHeight - playerHeight};
    player.speed = 6.0f;
    player.isAlive = 1;


    //Iniciar Enemigos
    for (int i = 0; i < enemiesY; i++)
    {
        for (int j = 0; j < enemiesX; j++) {

            switch (i) {
            case 0:
                enemies[i][j].width = 40;
                enemies[i][j].height = 25;
                break;
            case 1:
                enemies[i][j].width = 40;
                enemies[i][j].height = 30;
                break;
            case 2:
                enemies[i][j].width = 40;
                enemies[i][j].height = 40;
                break;
            case 3:
                enemies[i][j].width = 40;
                enemies[i][j].height = 40;
                break;
            }


            enemies[i][j].isAlive = 1;

            enemies[i][j].speed = 1.3f;
            enemies[i][j].position = (Vector2){ 
                ((float)j * (enemies[i][j].width + enemiesXSpace)) + enemies[i][j].width / 2,
                ((float)i * (enemies[i][j].height + enemiesYSpace )) + 50
            };
        }
    }
    

    initializeStack(&shootsPlayerStack);  // Initialize the stack
    initializeStack(&shootsEnemyStack);  // Initialize the stack de disparos enemigos


    //Inicializar Texturas
    strcpy(temporalChain, resourcesPath);  // Copy str1 into result
    strcat(temporalChain, "background.png");
    Image background = LoadImage (temporalChain);
    temporalChain[0] = '\0';

    ImageResize(&background, screenWidth, screenHeight);
    textureBackground = LoadTextureFromImage(background);
    UnloadImage(background);

    strcpy(temporalChain, resourcesPath);  // Copy str1 into result
    strcat(temporalChain, "player.png");
    Image player = LoadImage(temporalChain);
    temporalChain[0] = '\0';

    ImageResize(&player, playerWidth, playerHeight);
    playerTexture = LoadTextureFromImage(player);
    UnloadImage(player);

    for (int i = 0;i < 4;i++) {
        strcpy(temporalChain, resourcesPath);  // Copy str1 into result
        strcat(temporalChain, "invaders");
        
        switch (i) {
        case 0:
            strcat(temporalChain, "1");
            break;
        case 1:
            strcat(temporalChain, "2");
            break;
        case 2:
            strcat(temporalChain, "3");
            break;
        case 3:
            strcat(temporalChain, "4");
            break;
        }

        strcat(temporalChain, ".png");
        Image invader = LoadImage(temporalChain);
        temporalChain[0] = '\0';

        ImageResize(&invader, enemies[i][0].width, enemies[i][0].height);
        invadersTextures[i] = LoadTextureFromImage(invader);
        UnloadImage(invader);
    }

    strcpy(temporalChain, resourcesPath);  // Copy str1 into result
    strcat(temporalChain, "CovaWhite.png");
    Image cova = LoadImage(temporalChain);
    temporalChain[0] = '\0';

    ImageResize(&cova, 28 , 28);
    covaTexture = LoadTextureFromImage(cova);
    UnloadImage(cova);

    //Sonidos
    strcpy(temporalChain, resourcesPath);  // Copy str1 into result
    strcat(temporalChain, "shoot.wav");
    shootFx = LoadSound(temporalChain);
    temporalChain[0] = '\0';

    strcpy(temporalChain, resourcesPath);  // Copy str1 into result
    strcat(temporalChain, "invaderkilled.wav");
    invaderkilledFx = LoadSound(temporalChain);
    temporalChain[0] = '\0';

    strcpy(temporalChain, resourcesPath);  // Copy str1 into result
    strcat(temporalChain, "explosion.wav");
    explosionFx = LoadSound(temporalChain);
    temporalChain[0] = '\0';

}


// Gameplay Screen Draw logic
void DrawGameplayScreen()
{

    // TODO: Draw GAMEPLAY screen here!
    //DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), PURPLE);
    //Vector2 pos = { 20, 10 };
    //DrawTextEx(font, "GAMEPLAY SCREEN", pos, font.baseSize * 3.0f, 4, MAROON);
    
    //Dibujar Fondo
    DrawTexture(textureBackground, screenWidth / 2 - textureBackground.width / 2, screenHeight / 2 - textureBackground.height / 2, WHITE);


    //Dibujar Jugador
    if(player.isAlive)
        DrawTexture(playerTexture, player.position.x - playerWidth / 2, player.position.y - playerHeight / 2, GREEN);

    //Dibujar invasores
    for (int i = 0; i < enemiesY; i++){
        for (int j = 0; j < enemiesX; j++){
            if (enemies[i][j].isAlive == 1){
                DrawTexture(invadersTextures[i], enemies[i][j].position.x - enemies[i][j].width/2, enemies[i][j].position.y - enemies[i][j].height / 2, WHITE);
            }
        }
    }

    //Si hay, dibujar disparos
    dibujarDisparos(&shootsPlayerStack, YELLOW);
    dibujarDisparos(&shootsEnemyStack, WHITE);


    char temporalChain[200];
    strcpy(temporalChain, "Score: ");  // Copy str1 into result
    
    
    char strScore[20];

    itoa(score, strScore, 10);

    strcat(temporalChain, strScore);
    
    DrawTextWithBorder(temporalChain, 20, 5, 20, MAROON, 1, WHITE);

    if (win) {
        DrawTextWithBorder(" Ganaste !!!", 200, screenHeight / 2 - 30, 60, MAROON, 3, WHITE);
    }
    else if (loose) {
        DrawTextWithBorder("Game Over :(", 200, screenHeight / 2 - 30, 60, MAROON, 3, WHITE);
    }


    DrawTexture(covaTexture, screenWidth - covaTexture.width -10 , 2, WHITE);
}


// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{

    // TODO: Update GAMEPLAY screen variables here!

    // Jugador
    if (IsKeyDown(KEY_RIGHT) && (player.position.x + playerWidth / 2) < screenWidth ) player.position.x += player.speed;
    if (IsKeyDown(KEY_LEFT) && (player.position.x - playerWidth / 2) > 0) player.position.x -= player.speed;

    // Disparo de jugador
    if (IsKeyPressed(KEY_SPACE) && player.isAlive) {
        PlaySound(shootFx);

        Shoot shoot;
        shoot.position = player.position;
        push(&shootsPlayerStack, shoot);
    }

    //Mover Disparo jugador
    if (isEmpty(&shootsPlayerStack) == 0){
        for (int i = shootsPlayerStack.top; i >= 0; i--){
            shootsPlayerStack.arr[i].position.y -= shootSpeed;
            if (shootsPlayerStack.arr[i].position.y <= 0){
                removeAt(&shootsPlayerStack, i);
            }
        }
    }

    //Disparo enemigo
    if(isFull(&shootsEnemyStack) == 0){ //si hay disparos disponibles
        if(rand() % 30 == 1){
            int randomY = rand() % enemiesY; // osea del 0 a enemiesY - 1
            int randomX = rand() % enemiesX; // igual

            if (enemies[randomY][randomX].isAlive == 1 ) {

                Shoot shoot;
                shoot.position = enemies[randomY][randomX].position;
                push(&shootsEnemyStack, shoot);
            }
        }
    }

    //Mover Disparo enemigo
    if (isEmpty(&shootsEnemyStack) == 0) {
        for (int i = shootsEnemyStack.top; i >= 0; i--) {
            shootsEnemyStack.arr[i].position.y += shootSpeed * .4;
            if (shootsEnemyStack.arr[i].position.y >= screenHeight) {
                removeAt(&shootsEnemyStack, i);
            }
        }
    }


    //Hacia donde es la siguiente direccion del movimiento enemigo 
    int estadoDeMoverDerecha = moverDerecha;

    if (moverDerecha == 1 &&
        enemies[0][11].position.x < screenWidth - enemies[0][11].width/2)//Muestra de ultimo enemmigo a la derecha, a ver si esta en el borde
    {
        moverDerecha = 1;
    }
    else {
        moverDerecha = 0;
    }
    if (moverDerecha == 0 &&
        enemies[0][0].position.x > enemies[0][0].width/2)//Muestra de ultimo enemmigo a la derecha, a ver si esta en el borde
    {
        moverDerecha = 0;
    }
    else {
        moverDerecha = 1;
    }
    
    //Mover enemigos izquierda derecha
    for (int i = 0; i < enemiesY; i++)
    {
        for (int j = 0; j < enemiesX; j++) {

            if (moverDerecha) enemies[i][j].position.x += enemies[i][j].speed;
            else enemies[i][j].position.x -= enemies[i][j].speed;
        }
    }

    //Mover hacia abajo en cada cambio de direccion
    if (estadoDeMoverDerecha != moverDerecha)//Hubo cambio de direccion
    {
        for (int i = 0; i < enemiesY; i++)
        {
            for (int j = 0; j < enemiesX; j++) {
               enemies[i][j].position.y += enemies[i][j].speed * 5;
            }
        }
    }

    //Comprobar colision de enemigo
    if (isEmpty(&shootsPlayerStack) == 0) {

        for (int i = 0; i < enemiesY; i++){
            for (int j = 0; j < enemiesX; j++) {

                for (int iS = shootsPlayerStack.top; iS >= 0; iS--) {

                    if (enemies[i][j].isAlive == 1
                        && shootsPlayerStack.arr[iS].position.x >= enemies[i][j].position.x - enemies[i][j].width / 2
                        && shootsPlayerStack.arr[iS].position.x <= enemies[i][j].position.x + enemies[i][j].width / 2
                        && shootsPlayerStack.arr[iS].position.y >= enemies[i][j].position.y - enemies[i][j].height / 2
                        && shootsPlayerStack.arr[iS].position.y <= enemies[i][j].position.y + enemies[i][j].height / 2) {
                        PlaySound(invaderkilledFx);
                        enemies[i][j].isAlive = 0;
                        score += 100;
                        removeAt(&shootsPlayerStack, iS);
                    }
                }
            }
        }
    }

    if (isEmpty(&shootsEnemyStack) == 0) {
        for (int iS = shootsEnemyStack.top; iS >= 0; iS--) {

            if (shootsEnemyStack.arr[iS].position.x >= player.position.x - playerWidth / 2
                && shootsEnemyStack.arr[iS].position.x <= player.position.x + playerWidth / 2
                && shootsEnemyStack.arr[iS].position.y >= player.position.y - playerHeight / 2
                && shootsEnemyStack.arr[iS].position.y <= player.position.y + playerHeight / 2) {

                PlaySound(explosionFx);
                player.isAlive = 0;

                removeAt(&shootsEnemyStack, iS);
            }
        }
    }

    //Comprobar si existen vivos y rescatar quien es el vivo mas cercano al fondo
    int existenVivos = 0;
    int masCercaDeinvadir = 0;
    Vector2 posicionMasCercaDeinvadir = { 0 };
    for (int i = 0; i < enemiesY; i++) {
        for (int j = 0; j < enemiesX; j++) {
            if (enemies[i][j].isAlive == 1) {
                existenVivos++;
                if (i > masCercaDeinvadir) {
                    masCercaDeinvadir = i;
                    posicionMasCercaDeinvadir = enemies[i][j].position;
                }
            }
        }
    }

    if (posicionMasCercaDeinvadir.y >= player.position.y - playerHeight) {
        player.isAlive = 0;
    }
    
    if (existenVivos == 0) {
        win = 1;
    }
    if (player.isAlive == 0) {
        loose = 1;
    }

    if ((loose || win) && (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)))
    {
        finishScreen = 2;
        PlaySound(fxCoin);
    }

    /* Press enter or tap to change to ENDING screen 
    * 
    * Ya decidiremos despues como acaba
    * 
    
    }*/
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

//----------------------------------------------------------------------------------
// Funciones para pilas
//----------------------------------------------------------------------------------


// Function to initialize the stack
void initializeStack(Stack* stack) {
    stack->top = -1;  // The stack is initially empty
}

// Function to check if the stack is full
int isFull(Stack* stack) {
    return stack->top == 9;  // Stack is full if top is 9 (0-based index)
}

// Function to check if the stack is empty
int isEmpty(Stack* stack) {
    return stack->top == -1;  // Stack is empty if top is -1
}

// Function to push a Shoot element onto the stack
void push(Stack* stack, Shoot shoot) {
    if (isFull(stack)) {
        printf("Error: Stack is full!\n");
        return;
    }
    stack->arr[++(stack->top)] = shoot;  // Increment top and add the shoot to the stack
    printf("Shoot pushed: position(%.2f, %.2f), exist=%d, speed=%.2f\n", shoot.position.x, shoot.position.y);
}

// Function to pop a Shoot element from the stack
Shoot pop(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Error: Stack is empty!\n");
        Shoot emptyShoot = { {0.0, 0.0} };
        return emptyShoot;
    }
    return stack->arr[(stack->top)--];  // Return the top element and decrement top
}

// Function to remove an element at index i in the stack
void removeAt(Stack* stack, int i) {
    if (i < 0 || i > stack->top) {
        printf("Error: Invalid index!\n");
        return;
    }

    // Shift elements down to fill the gap left by the removed element
    for (int j = i; j < stack->top; j++) {
        stack->arr[j] = stack->arr[j + 1];
    }

    // Decrease the top index to reflect the removal
    stack->top--;
    printf("Element at index %d removed\n", i);
}

// Function to get the top element of the stack without removing it
Shoot peek(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Error: Stack is empty!\n");
        Shoot emptyShoot = { {0.0, 0.0} };
        return emptyShoot;
    }
    return stack->arr[stack->top];  // Return the top element without modifying the stack
}

// Function to traverse and print all elements in the stack
void traverseStack(Stack* stack) {
    if (isEmpty(stack)) {
        printf("The stack is empty.\n");
        return;
    }
    printf("Stack elements:\n");
    for (int i = stack->top; i >= 0; i--) {
        printf("Shoot at position(%.2f, %.2f), exist=%d, speed=%.2f\n",
            stack->arr[i].position.x, stack->arr[i].position.y);
    }
}
