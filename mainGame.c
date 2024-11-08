#include <SDL2/SDL_error.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>
#include "./constants.h"

SDL_Window* Window = NULL;
SDL_Renderer* Renderer = NULL;

bool game_is_running = false;
bool gameIsOver = false;
float last_frame_time = 0;
float delta_time = 0.0f; // This is very important dont delete this ever.

typedef struct{
    bool jumping, goDown, dead, onPlatform, shoot, melee;
    float x, y, jumpForce, health, jumpVelocity, shooting_last_frame;
    short int speed, spriteInSpriteSheet, moveLR, current_frame, playerDirection, platformIndex;
} Player;
Player player;

int mouseX, mouseY;
int totalBulletInInventory = 200;
double distanceTravelledByPlayerFromSpawn = 0;

typedef struct {
    short int health, current_frame, direction;
    float x, y, last_frame_time, velocityYZombie;
    bool attackPlayer, idle, dead, onPlatform, playerReachable, jump;
} Zombie;
int zombieCount = 0;
int zombieKilled = 0;
Zombie *zombies = NULL;

char *items[] = {"bullet", "cash", "health", "nothing"};
typedef struct{
    int x, y;
    int itemIndex;
    bool onPlatform;
    float velocity;
} ItemDrop;
int itemCount = 0;
ItemDrop *itemDrops = NULL;

typedef struct{
    float x, y;
    bool direction;// True is right and false left
} Bullet;
Bullet *bullets = NULL;
unsigned short int bulletVelocity = 300;
unsigned short int bulletCount = 0;

typedef struct{
    int width, height;
    float x, y;
} Platform;
int platformsRendered = 0;
Platform platforms[MAX_PLATFORMS_TO_BE_RENDERED];


int InitializeWindow(void);
int SetUp(void);
    void spawnZombies(int numberOfZombies);
void ProcessInput(void);
void Update(void);
    void bulletData(int X, int Y, bool facingDirection);
void Render(void);
    void platformSpawn(int spawnAmount);
    void platformRender(void);
    void zombieRender(void);
    void playerRender(void);
        void bulletRender(void);
        void gameOver(void);
    bool healthBar(int x, int y, float health, bool Player);
void DestroyWindowInternal(void);

int collisionDetection(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);
bool gamePause = false;
float pause_start_time_tick = 0;
void Paused(void);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  #include <windows.h>
  int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevIns, LPSTR lpszArgument, int iShow) {
#else
  int main(int argc, char *argv[]) {
#endif
    game_is_running = InitializeWindow();
    SetUp();
    // Game Loop
    while (game_is_running) {
        ProcessInput();
        if (!gameIsOver){
            if (gamePause){
                Paused();
                continue;
            }
            Update();
            Render();
        }
    }
    DestroyWindowInternal();
    return 0; // 0 is success
}


// Function to Initialize Game State
int InitializeWindow(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        fprintf(stderr,"Error: %s",SDL_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    Window = SDL_CreateWindow(GameTitle,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);
    if (!Window){
        fprintf(stderr,"Error: %s",SDL_GetError());
        return false;
    }

    Renderer = SDL_CreateRenderer(Window, -1, 0);
    if (!Renderer){
        fprintf(stderr,"Error: %s",SDL_GetError());
        return false;
    }

    SDL_ShowCursor(SDL_DISABLE); // Disable Mouse
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "Error: %s", IMG_GetError());
        return false;
    }
    return true;
}

// Function for setup
SDL_Texture* BackgroundTexture;

SDL_Texture* PlayerIdleTexture;
SDL_Texture* PlayerRunTexture;
SDL_Texture* PlayerShootTexture;
SDL_Texture* PlayerJumpTexture;
SDL_Texture* PlayerDeadTexture;

SDL_Texture* BulletTexture;

SDL_Texture* ZombieRunTexture;
SDL_Texture* ZombieAtkTexture;
SDL_Texture* ZombieIdleTexture;
SDL_Texture* ZombieDeadTexture;
SDL_Texture* ZombieJumpTexture;

SDL_Texture* ResumeButtonTexture;
SDL_Texture* ExitGameButtonTexture;

int ground_height;
SDL_Rect background_img_src_rect;
SDL_Rect background_img_dest_rect;

TTF_Font* font = NULL;

int SetUp(void){
    srand(1007); // for fixed randomizing

    // Player Setup
    player.x = 100;
    player.speed = 150;
    player.spriteInSpriteSheet = 0;
    player.jumping = false;
    player.health = 100.0;
    player.jumpForce = 100.0;
    player.dead = false;
    player.onPlatform = false;
    player.goDown = false;
    player.moveLR = 0;
    player.shoot = false;
    player.melee = false;
    player.jumpVelocity = -200.0f;
    player.current_frame = 0;
    player.playerDirection = 1;
    player.shooting_last_frame = 0;

    // Sprite rendering:
    BackgroundTexture = IMG_LoadTexture(Renderer, "sprite/Background/1.png");

    PlayerIdleTexture = IMG_LoadTexture(Renderer, "sprite/Playable/AK/Idle.png");
    PlayerRunTexture = IMG_LoadTexture(Renderer, "sprite/Playable/AK/Run.png");
    PlayerShootTexture = IMG_LoadTexture(Renderer, "sprite/Playable/AK/Shoot.png");
    PlayerJumpTexture = IMG_LoadTexture(Renderer, "sprite/Playable/AK/Jump.png");
    PlayerDeadTexture = IMG_LoadTexture(Renderer, "sprite/Playable/AK/Dead.png");

    ZombieRunTexture = IMG_LoadTexture(Renderer, "sprite/NPC/Zombie/Run.png");
    ZombieAtkTexture = IMG_LoadTexture(Renderer, "sprite/NPC/Zombie/Attack.png");
    ZombieIdleTexture = IMG_LoadTexture(Renderer, "sprite/NPC/Zombie/Idle.png");
    ZombieJumpTexture = IMG_LoadTexture(Renderer, "sprite/NPC/Zombie/Jump.png");
    ZombieDeadTexture = IMG_LoadTexture(Renderer, "sprite/NPC/Zombie/Dead.png");

    BulletTexture = IMG_LoadTexture(Renderer, "sprite/bullet.png");

    ResumeButtonTexture = IMG_LoadTexture(Renderer, "sprite/icons/playGame.png");
    ExitGameButtonTexture = IMG_LoadTexture(Renderer, "sprite/icons/exitGame.png");

    if (!BackgroundTexture || !PlayerIdleTexture ||
        !PlayerRunTexture || !PlayerShootTexture ||
        !PlayerJumpTexture || !PlayerDeadTexture ||
        !ZombieRunTexture || !ZombieIdleTexture ||
        !ZombieAtkTexture || !ZombieDeadTexture ||
        !BulletTexture || !ResumeButtonTexture || !ExitGameButtonTexture
    ) {
        printf("Error loading texture: %s\n", IMG_GetError());
        game_is_running = false;
    }

    ground_height = WINDOW_HEIGHT / 12;
    SDL_QueryTexture(BackgroundTexture, NULL, NULL, &background_img_src_rect.w, &background_img_src_rect.h);
    background_img_src_rect.x = 0;
    background_img_src_rect.y = 0;
    background_img_src_rect.w /= 4;

    // Set destination rectangle to cover the full screen
    background_img_dest_rect.x = 0;
    background_img_dest_rect.y = 0;
    background_img_dest_rect.w = WINDOW_WIDTH;
    background_img_dest_rect.h = WINDOW_HEIGHT - ground_height;

    player.y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
    distanceTravelledByPlayerFromSpawn = 0;
    spawnZombies(1);
    platformSpawn(10);
    return true;
}

void spawnZombies(int numberOfZombies) {
    float factor;
    if (zombies == NULL)
        zombies = malloc(sizeof(Zombie) * numberOfZombies);
    else
        zombies = realloc(zombies, sizeof(Zombie) * (numberOfZombies + zombieCount));

    // Spawn each zombie
    for (int i = 0; i < numberOfZombies; i++) {
        factor = (rand() % 2 == 0) ? -1 : 1;
        float value = player.x + factor * (rand() % 100 + (float)WINDOW_WIDTH / 2);

        // Clamp the value to be within valid screen bounds
        if (value < -((float)WINDOW_WIDTH / 5))
            value = -100;
        else if (value > WINDOW_WIDTH + ((float)WINDOW_WIDTH / 5))
            value = WINDOW_WIDTH + 100;

        // Initialize the zombie properties
        zombies[zombieCount].x = value;
        zombies[zombieCount].y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
        zombies[zombieCount].health = 100.0f;
        zombies[zombieCount].current_frame = 0;
        zombies[zombieCount].direction = -1;  // Initially facing left
        zombies[zombieCount].attackPlayer = false;
        zombies[zombieCount].idle = true;
        zombies[zombieCount].last_frame_time = SDL_GetTicks();
        zombies[zombieCount].dead = false;
        zombies[zombieCount].jump = false;
        zombieCount++;
    }
}


SDL_Rect playButton_dest_rect;
SDL_Rect exitButton_dest_rect;
SDL_Rect cursorRect = {0, 0, 10, 10};
void Paused(void) {
    SDL_ShowCursor(true);
    SDL_SetRenderDrawColor(Renderer, 22, 22, 22, 255);
    SDL_RenderClear(Renderer);

    cursorRect.x = mouseX - 5;
    cursorRect.y = mouseY - 5;

    font = TTF_OpenFont("./textFont/AmaticSC-Regular.ttf", 40);
    SDL_Color textColor = {255, 0, 0, 255};

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Game Paused", textColor);
    if (textSurface == NULL) {
        printf("Error: %s", SDL_GetError());
        return;
    }

    SDL_Rect textRect;
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_FreeSurface(textSurface);

    textRect.x = (WINDOW_WIDTH - textRect.w) / 2;
    textRect.y = (WINDOW_HEIGHT - textRect.h) / 2 - WINDOW_HEIGHT / 5;

    SDL_RenderCopy(Renderer, textTexture, NULL, &textRect);

    // Play button positioning
    SDL_Rect playButton_src_rect = {0, 0, 160, 160};
    playButton_dest_rect.w = 40;
    playButton_dest_rect.h = 40;
    playButton_dest_rect.x = (WINDOW_WIDTH - 2 * playButton_dest_rect.w - 50) / 2; // Center and add padding
    playButton_dest_rect.y = textRect.y + 60;

    SDL_RenderCopy(Renderer, ResumeButtonTexture, &playButton_src_rect, &playButton_dest_rect);

    // Exit button positioning
    exitButton_dest_rect.w = 40;
    exitButton_dest_rect.h = 40;
    exitButton_dest_rect.x = playButton_dest_rect.x + playButton_dest_rect.w + 50; // Offset by button width + padding
    exitButton_dest_rect.y = playButton_dest_rect.y;

    SDL_RenderCopy(Renderer, ExitGameButtonTexture, &playButton_src_rect, &exitButton_dest_rect);

    SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
    if(collisionDetection(cursorRect.x, cursorRect.y, cursorRect.w, cursorRect.h, playButton_dest_rect.x, playButton_dest_rect.y, playButton_dest_rect.w, playButton_dest_rect.h))
        SDL_RenderDrawRect(Renderer, &playButton_dest_rect);
    if(collisionDetection(cursorRect.x, cursorRect.y, cursorRect.w, cursorRect.h, exitButton_dest_rect.x, exitButton_dest_rect.y, exitButton_dest_rect.w, exitButton_dest_rect.h))
        SDL_RenderDrawRect(Renderer, &exitButton_dest_rect);

    TTF_CloseFont(font);
    SDL_DestroyTexture(textTexture);
    SDL_RenderPresent(Renderer);
}

// Function to process keyboard input
bool debug = false;
void ProcessInput(void){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (!gamePause)
                    break;
                SDL_GetMouseState(&mouseX, &mouseY);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(!gamePause)
                    break;
                if (event.button.button == SDL_BUTTON_LEFT){
                    if(collisionDetection(cursorRect.x, cursorRect.y, cursorRect.w, cursorRect.h, playButton_dest_rect.x, playButton_dest_rect.y, playButton_dest_rect.w, playButton_dest_rect.h))
                        gamePause = false;
                    if(collisionDetection(cursorRect.x, cursorRect.y, cursorRect.w, cursorRect.h, exitButton_dest_rect.x, exitButton_dest_rect.y, exitButton_dest_rect.w, exitButton_dest_rect.h))
                        game_is_running = false;
                }
                break;
            case SDL_QUIT:
                game_is_running = false;
                break;
            case SDL_KEYDOWN:
                if (gameIsOver)
                    game_is_running = false;
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        gamePause = !gamePause;
                        if (gamePause){
                            pause_start_time_tick = SDL_GetTicks();
                        } else{
                            last_frame_time += SDL_GetTicks() - pause_start_time_tick;
                            delta_time = SDL_GetTicks() - last_frame_time;
                            SDL_ShowCursor(SDL_DISABLE);
                        }
                        break;
                    if (!player.dead){
                        case SDLK_a:
                        case SDLK_LEFT:
                            player.moveLR = -1;
                            break;
                        case SDLK_d:
                        case SDLK_RIGHT:
                            player.moveLR = 1;
                            break;
                        case SDLK_w:
                        case SDLK_UP:
                        case SDLK_SPACE:
                            if (player.jumpForce > 20)
                                player.jumping = true;
                            break;
                        case SDLK_s:
                        case SDLK_DOWN:
                            player.goDown = true;
                            break;
                        case SDLK_f:
                            player.shoot = true;
                            break;
                        case SDLK_k:
                            debug = !debug;
                            break;
                        default:
                            break;
                    }
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                    case SDLK_LEFT:
                    case SDLK_d:
                    case SDLK_RIGHT:
                        player.moveLR = 0;
                        break;
                    case SDLK_w:
                    case SDLK_UP:
                    case SDLK_SPACE:
                        player.jumping = false;
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                            player.goDown = false;
                            break;
                    case SDLK_f:
                        player.shoot = false;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

// Function to Update Game Objects Per Frame
float velocityY = 0.0f;
float gravity = 980.0f; // 9.8m/s is earth's gravity i think
float_t zombie_frame_time = 0;
float rightCam = 3.0;
float leftCam = 2.0;

int indexOfNearestPlatform(int x, int y){
    int distanceToPlatform[platformsRendered];
    int minDistance = WINDOW_WIDTH;
    int index;
    for (int i = 0; i <= platformsRendered; i++) {
        // distanceToPlatform[i] = pow(pow(x - platforms[i].x, 2) + pow(y - platforms[i].y, 2), 0.5);
        distanceToPlatform[i] = pow(pow(player.x - platforms[i].x, 2) + pow(player.y - platforms[i].y, 2), 0.5);
        if (distanceToPlatform[i] < minDistance){
            minDistance = distanceToPlatform[i];
            index = i;
        }
    }
    if (minDistance == WINDOW_WIDTH)
        return MAX_PLATFORMS_TO_BE_RENDERED + 1;
    return index;
}

void Update(void) {
    // IMP: DELAY LOGIC, 60 fps for now.
    int time_to_wait = (int)FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    if (SDL_GetTicks() - zombie_frame_time > (10000 + rand() % 30000)){ //spawn one zombie every 10-30 sec
        spawnZombies(rand() % 3 + 1);
        zombie_frame_time = SDL_GetTicks();
    }

    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    for (int i = 0; i < zombieCount; i++){
        zombies[i].y += zombies[i].velocityYZombie;
        zombies[i].velocityYZombie = gravity * delta_time;
        if (zombies[i].y > WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT){
            zombies[i].y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
            zombies[i].velocityYZombie = 0;
            zombies[i].onPlatform = true;
        }
        if (player.y - zombies[i].y >= -40){
            zombies[i].playerReachable = true;
            continue;
        } else
            zombies[i].playerReachable = false;

        if (!zombies[i].dead && !player.dead && !zombies[i].playerReachable && player.onPlatform){
            int index = indexOfNearestPlatform(zombies[i].x, zombies[i].y);
            if (index == MAX_PLATFORMS_TO_BE_RENDERED + 1)
                zombies[i].idle = true;
            else{
                // TODO: Teleport to platform: change animation to jumping
                // printf("...\n Platform: %f, Zombie: %d ...\n", platforms[index].y, zombies[i].y);
                if (zombies[i].y == platforms[index].y - SPRITE_HEIGHT)
                    continue;
                // zombies[i].y = platforms[index].y - SPRITE_HEIGHT;
                // zombies[i].x = platforms[index].x + (int)(platforms[index].width / 2);
                // zombies[i].idle = true;
            }
        }
    }

    for (int i = 0; i < itemCount; i++){
        itemDrops[i].y += itemDrops[i].velocity;
        itemDrops[i].velocity = gravity * delta_time;
        if (itemDrops[i].y > WINDOW_HEIGHT - ground_height - 15){
            itemDrops[i].y = WINDOW_HEIGHT - ground_height - 15;
            itemDrops[i].velocity = 0;
            itemDrops[i].onPlatform = true;
        }
    }

    // Setting a limit to player zet max reach
    if (player.y <= (-SPRITE_HEIGHT))
        player.y = -SPRITE_HEIGHT;

    velocityY += gravity * delta_time;
    if (gamePause)
        velocityY = 0;
    player.y += velocityY * delta_time;
    if (player.y > WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT) {
        player.y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
        velocityY = 0;
        player.onPlatform = true;
    }

    if (player.onPlatform)
        player.jumpForce += 20 * delta_time;

    float parallaxValue = (delta_time * player.speed);
    if (player.x > (float)WINDOW_WIDTH * (rightCam/5)){
        background_img_src_rect.x += 100 * delta_time;
        if (background_img_src_rect.x >= background_img_src_rect.w)
            background_img_src_rect.x = 0;
        player.x = (rightCam/5) * (float)WINDOW_WIDTH;
        for (int i = 0; i < zombieCount; i++)
            zombies[i].x -= parallaxValue;
        for (int i = 0; i < bulletCount; i++)
            bullets[i].x -= parallaxValue;
        for (int i = 0; i < platformsRendered; i++)
            platforms[i].x -= parallaxValue;
        for (int i = 0; i < itemCount; i++)
            itemDrops[i].x -= parallaxValue;
    }
    else if (player.x < (float)WINDOW_WIDTH * (leftCam/5)){
        background_img_src_rect.x -= 50 * delta_time;
        if (background_img_src_rect.x <= 0)
            background_img_src_rect.x = background_img_src_rect.w;
        player.x = (leftCam/5) * (float)WINDOW_WIDTH;
        for (int i = 0; i < zombieCount; i++)
            zombies[i].x += parallaxValue;
        for (int i = 0; i < bulletCount; i++)
            bullets[i].x += parallaxValue;
        for (int i = 0; i < platformsRendered; i++)
            platforms[i].x += parallaxValue;
        for (int i = 0; i < itemCount; i++)
            itemDrops[i].x += parallaxValue;
    }
    for (int i = 0; i < itemCount; i++){
        itemDrops[i].x += player.x - itemDrops[i].x < 0 ? -1 : 1;
        itemDrops[i].y += (player.y - itemDrops[i].y < 0 ? -1 : 1) * sqrt(pow(player.x - itemDrops[i].x, 2) + pow(player.y - itemDrops[i].y, 2));
    }

    distanceTravelledByPlayerFromSpawn += player.moveLR * delta_time * player.speed;
    // Healing
    player.health += delta_time;
    if (player.health > 100)
        player.health = 100;
    if (player.jumping && player.jumpForce > 0){
        velocityY = player.jumpVelocity;
        player.jumpForce -= delta_time * 50;
    }
    if (player.jumpForce < 5)
        player.jumping = false;

    if (player.jumpForce <= 0)
        player.jumpForce = 0;
    if (player.jumpForce >= 100)
        player.jumpForce = 100;

    // removing items on collision with player
    for (int i = 0; i < itemCount; i++){
        bool collided = collisionDetection(itemDrops[i].x, itemDrops[i].y, 50, 20, player.x, player.y, 70, SPRITE_HEIGHT);
        if (collided)
            if (itemDrops[i].itemIndex == 0){
                totalBulletInInventory += 12; // TODO: Animate that
                // removeItemFromItemDrops();
                itemDrops[i].itemIndex = 3;
            }
    }
    if (totalBulletInInventory > 999)
        totalBulletInInventory = 999;
}

void spawnMainBoss(void){
    if (zombieKilled % 15 == 0 && zombieKilled != 0){
        printf("Spawning The BOSS...\n");
    }
}

void otherRender(void){
    // Zombie Killed
    SDL_Rect srcRect = {215, 0, 50, SPRITE_HEIGHT};
    SDL_Rect dstRect = {WINDOW_WIDTH - 100, -20, srcRect.w, SPRITE_HEIGHT};
    SDL_RenderCopy(Renderer, ZombieIdleTexture, &srcRect, &dstRect);

    // Bullet in Mag
    srcRect.x = 0;
    srcRect.w = 900;
    srcRect.h = 450;
    dstRect.x = WINDOW_WIDTH -90;
    dstRect.y = 20;
    dstRect.w = 40;
    dstRect.h = 20;
    SDL_RenderCopyEx(Renderer, BulletTexture, &srcRect, &dstRect, -45, NULL, 0);

    font = TTF_OpenFont("./textFont/textFonts.ttf", 24);
    SDL_Color textColor = {0, 0, 0, 255};  // Black color for text
    SDL_Rect textRect;
    char text[20];  // Buffer large enough to hold both texts

    // First line: Display totalBulletInInventory
    sprintf(text, "x %d", totalBulletInInventory);  // Format the first line
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (textSurface == NULL) {
        printf("Error: %s", TTF_GetError());
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
    if (textTexture == NULL) {
        printf("Error: %s", SDL_GetError());
        return;
    }
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = WINDOW_WIDTH - 55;  // Position near the right side of the screen
    textRect.y = 20;                 // First line at 20px from the top
    SDL_RenderCopy(Renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);     // Clean up the surface after use
    SDL_DestroyTexture(textTexture);  // Clean up the texture after rendering

    // Second line: Display zombieKilled
    sprintf(text, "x %d", zombieKilled);  // Format the second line
    textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (textSurface == NULL) {
        printf("Error: %s", TTF_GetError());
        return;
    }
    textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
    if (textTexture == NULL) {
        printf("Error: %s", SDL_GetError());
        return;
    }
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = WINDOW_WIDTH - 55;
    textRect.y = 65;
    SDL_RenderCopy(Renderer, textTexture, NULL, &textRect);

    TTF_CloseFont(font);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void itemRender(void){
    for (int i = 0; i < itemCount; i++){
        if (itemDrops[i].itemIndex == 0){
            // bullet
            SDL_Rect src_rect ={0, 0, 900, 450};
            SDL_Rect dstRect = {0, 0, 20, 10};
            dstRect.x = itemDrops[i].x;
            dstRect.y = itemDrops[i].y;
            for (int j = 0; j < 6; j++){
                dstRect.x += 5;
                SDL_RenderCopyEx(Renderer, BulletTexture, &src_rect, &dstRect, -90.0, NULL, SDL_FLIP_NONE);
            }
        }
    }
}

int last_frame_time_for_idle = 0;
int addFactor = 43;
int n = 0;
float mulFactor = 1;

// Function to render game objects in the scene
void Render(void){
    // Background Color
    SDL_SetRenderDrawColor(Renderer, 211, 211, 211, 255);
    SDL_RenderClear(Renderer);

    SDL_RenderCopy(Renderer, BackgroundTexture, &background_img_src_rect, &background_img_dest_rect); //Background Image

    // Spawn Platforms for player to jump into
    platformRender();

    // Draw ground
    SDL_Rect ground_rect = {
        0, //x-position
        WINDOW_HEIGHT - ground_height, //y-position
        WINDOW_WIDTH, //width
        ground_height //height
    };
    SDL_SetRenderDrawColor(Renderer, 49, 91, 43, 255);
    SDL_RenderFillRect(Renderer, &ground_rect);
    ground_rect.h = 2;
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(Renderer, &ground_rect);
    // TODO: Find a color that covers below ground grass
    // ground_rect.h = ground_height;
    // ground_rect.y = WINDOW_HEIGHT - ground_height + 25;
    // SDL_SetRenderDrawColor(Renderer, 125, 25, 69, 5);
    // SDL_RenderFillRect(Renderer, &ground_rect);

    // Sprite Animation:
    zombieRender();
    playerRender();
        bulletRender();
        itemRender();
    spawnMainBoss();

    // Render Other:
    otherRender();
    if (gameIsOver)
        gameOver();
    SDL_RenderPresent(Renderer); // Show rendered frame to user.
}
int ActualSpriteHeightStartY = 2 * (int)(SPRITE_HEIGHT / 5);

bool isRectOnTop(SDL_Rect Rect, SDL_Rect dst_rect) {
    return (Rect.y + Rect.h <= (dst_rect.y+3) && Rect.y + Rect.h >= (dst_rect.y - 3) && Rect.x < dst_rect.x + dst_rect.w && Rect.x + Rect.w > dst_rect.x);
}

void platformSpawn(int spawnAmount) {
    float maxPlatformHeight = (2.0f / 4.0f) * WINDOW_HEIGHT - ground_height;

    // Spawn additional platforms
    for (int i = platformsRendered; i < platformsRendered + spawnAmount; i++) {
        platforms[i].width = 100;
        platforms[i].y = maxPlatformHeight + rand() % (int)(WINDOW_HEIGHT / 2);
        if (platforms[i].y > WINDOW_HEIGHT - ground_height - 20)
            platforms[i].y = WINDOW_HEIGHT - ground_height - 20;
        else if (platforms[i].y < maxPlatformHeight)
            platforms[i].y = 2.0 / 4 * WINDOW_HEIGHT - ground_height;

        platforms[i].height = (WINDOW_HEIGHT - ground_height) - platforms[i].y;

        if (rand() % 2 == 1)
            platforms[i].x = platforms[i].width + 2 + 50 * i;
        else
            platforms[i].x = -1 * (platforms[i].width + 2 + 50 * i);
        continue;
    }
    platformsRendered += spawnAmount;

    // sort
    for (int i = 0; i < platformsRendered; i++){
        for (int j = i + 1; j < platformsRendered; j++) {
            if (platforms[i].y > platforms[j].y) {
                Platform temp = platforms[i];
                platforms[i] = platforms[j];
                platforms[j] = temp;
            }
        }
    }
}

void platformRender(void) {
    // const int renderMaxWindowLength = +- 7;

    SDL_Rect playerRect = {player.x + 13, player.y, 25, SPRITE_HEIGHT};

    for (int i = 0; i < platformsRendered; i++) {
        SDL_Rect dst_rect = {platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height};

        // Check if the player is on top of the platform
        if (isRectOnTop(playerRect, dst_rect) && (!player.jumping || player.jumpForce <= 10) && !player.goDown) {
            player.y = platforms[i].y - SPRITE_HEIGHT; // Place player on top of the platform
            velocityY = 0;
            player.onPlatform = true;
            player.platformIndex = i;
        }

        // check if zombie on top of platform
        for (int j = 0; j < zombieCount; j++){
            SDL_Rect zombieRect = {zombies[j].x + 13, zombies[j].y, 25, SPRITE_HEIGHT}; // +13 to center the rect
            if (isRectOnTop(zombieRect, dst_rect)) {
                zombies[j].y = platforms[i].y - SPRITE_HEIGHT;
                zombies[j].onPlatform = true;
                zombies[j].velocityYZombie = 0;
            }
        }

        // check if items on top of platform
        for (int j = 0; j < itemCount; j++){
            SDL_Rect itemRect = {itemDrops[j].x + 13, itemDrops[j].y, 20, 10};
            if (isRectOnTop(itemRect, dst_rect)){
                itemDrops[j].y = platforms[i].y - 15;
                itemDrops[j].onPlatform = true;
                itemDrops[j].velocity = 0;
            }
        }

        // Platform
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255); // Outer color (black)
        SDL_RenderFillRect(Renderer, &dst_rect);
        if (dst_rect.x != 0 && dst_rect.y != 0 && dst_rect.w != 0 && dst_rect.h != 0){
            dst_rect.x += 2;
            dst_rect.y += 2;
            dst_rect.w -= 4;
            dst_rect.h -= 2;
        }
        SDL_SetRenderDrawColor(Renderer, 190, 100, 56, 255);
        SDL_RenderFillRect(Renderer, &dst_rect);

        dst_rect.h = 20;
        SDL_SetRenderDrawColor(Renderer, 49, 91, 43, 255); // Inner color (green)
        SDL_RenderFillRect(Renderer, &dst_rect);
    }
}

void playerRender(void) {
    SDL_RendererFlip flip = SDL_FLIP_NONE;  // No flip by default

    // Update playerDirection
    if (player.moveLR > 0)
        player.playerDirection = 1;  // Moving right
    else if (player.moveLR < 0)
        player.playerDirection = -1; // Moving left

    // If the direction is left, flip the sprite horizontally
    if (player.playerDirection == -1)
        flip = SDL_FLIP_HORIZONTAL;

    if (player.dead){
        player.spriteInSpriteSheet = 4;
        if (player.current_frame > player.spriteInSpriteSheet){
            gameIsOver = true;
            SDL_Delay(1000);
        }
        int dead[]= {25, 150, 280, 400, 560};
        mulFactor = 0.5;
        SDL_Rect src_rect = {
            dead[player.current_frame],
            0, 80, SPRITE_HEIGHT
        };
        SDL_Rect dst_rect = {(int)player.x, (int)player.y, src_rect.w, src_rect.h};

        SDL_RenderCopyEx(Renderer, PlayerDeadTexture, &src_rect, &dst_rect, 0, NULL, flip);

        if ((SDL_GetTicks() - last_frame_time_for_idle) > 1000 / (player.spriteInSpriteSheet * mulFactor)) {
            player.current_frame = (player.current_frame + 1);
            last_frame_time_for_idle = SDL_GetTicks();
        }
        return;
    }

    if (player.jumping || !player.onPlatform) {
        player.spriteInSpriteSheet = 7;  //frames in jump animation
        n = -8;
        mulFactor = 0.25;

        // finish the jump animation
        // TODO: with implementation of platform, need another method for finishing this animation
        if (player.y >= WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT - 10)
            player.current_frame = (player.current_frame + 1) % player.spriteInSpriteSheet;
        else
            player.current_frame = 4;  // Keep 4th sprite in jump sheet
        player.x += player.moveLR * delta_time * player.speed;
        player.onPlatform = false;
    }
    else if (player.moveLR && !player.shoot && player.onPlatform){
        player.spriteInSpriteSheet = 8;
        n = -8;
        mulFactor = 1.5;
        player.x += player.playerDirection * delta_time * player.speed;
    }
    else if (player.shoot) {
        player.spriteInSpriteSheet = 4;  // frames in shooting animation
        n = 0;
        mulFactor = 4;
    }
    else {
        player.spriteInSpriteSheet = 8;  // Idle animation frames
        n = -8;
        mulFactor = 1;
    }

    SDL_Rect src_rect = {
        45 + n + addFactor * 2.98 * player.current_frame,
        0, 50, SPRITE_HEIGHT
    };

    SDL_Rect dst_rect = {(int)player.x, (int)player.y, src_rect.w, src_rect.h};

    // Draw a red outline around the sprite (for debugging purposes)
    if (debug){
        SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
        dst_rect.h -= ActualSpriteHeightStartY;
        dst_rect.y += ActualSpriteHeightStartY;
        SDL_RenderDrawRect(Renderer, &dst_rect);
        dst_rect.h += ActualSpriteHeightStartY;
        dst_rect.y -= ActualSpriteHeightStartY;
    }

    if (player.shoot && (SDL_GetTicks() - player.shooting_last_frame) > (120)){
        if (totalBulletInInventory > 0){
            if (src_rect.x != 45 + addFactor * 2.98 * player.current_frame){
                if (player.playerDirection == 1){
                    bulletData(player.x, player.y, true);
                    player.x -= delta_time * (rand() % 10 + 20);
                }
                else{
                    bulletData(player.x, player.y, false);
                    player.x += delta_time * (rand() % 10 + 20);
                }
                player.shooting_last_frame = SDL_GetTicks();
                totalBulletInInventory--;
            }
        }
    }


    // Only allow the jump animation while in the air
    if (player.jumping || (!player.onPlatform && player.shoot))
        SDL_RenderCopyEx(Renderer, PlayerJumpTexture, &src_rect, &dst_rect, 0, NULL, flip);
    else if (player.moveLR && !player.shoot)
        SDL_RenderCopyEx(Renderer, PlayerRunTexture, &src_rect, &dst_rect, 0, NULL, flip);
    else if (player.shoot)
        SDL_RenderCopyEx(Renderer, PlayerShootTexture, &src_rect, &dst_rect, 0, NULL, flip);
    else
        SDL_RenderCopyEx(Renderer, PlayerIdleTexture, &src_rect, &dst_rect, 0, NULL, flip);

    // indipendent animation for player instead of bound my game FPS
    if ((SDL_GetTicks() - last_frame_time_for_idle) > 1000 / (player.spriteInSpriteSheet * mulFactor)) {
        player.current_frame = (player.current_frame + 1) % player.spriteInSpriteSheet;
        last_frame_time_for_idle = SDL_GetTicks();
    }
    if (!player.dead){
        if (!healthBar(player.x, player.y, player.health, true)){
            player.dead = true;
            player.current_frame = 0;
        }
    }
}

void bulletData(int X, int Y, bool facingDirection) {
    Bullet *newBullets = realloc(bullets, (bulletCount + 1) * sizeof(Bullet));
    if (newBullets == NULL) {
        printf("Memory allocation failed!\n");
        game_is_running = false;
        return;
    }

    bullets = newBullets;
    if (facingDirection) {
        bullets[bulletCount].x = X + 47; // Facing right
        bullets[bulletCount].direction = true;
    } else {
        bullets[bulletCount].x = X - 7; // Facing left
        bullets[bulletCount].direction = false;
    }
    bullets[bulletCount].y = (Y + ActualSpriteHeightStartY + 22); // Set y-coordinate
    bulletCount++;
}

void bulletRemove(int *array, int size){
    for (int i = 0; i < size; i++) {
        int index = array[i] - i;
        for (int j = index; j < bulletCount - 1; j++)
            bullets[j] = bullets[j + 1];
    }
    bulletCount-=size;
}

void bulletCollisionSystem(void){
    int totalremove = 0;
    int *removeIndex = malloc(bulletCount * sizeof(int));

    for (int i = 0; i < bulletCount; i++){
        // Check if the bullet is out of bounds
        if (bullets[i].x < 0 || bullets[i].x > WINDOW_WIDTH){
            removeIndex[totalremove++] = i;
            continue;
        }

        // Check collision with zombies
        for (int j = 0; j < zombieCount; j++){
            if (!zombies[j].dead){
                if (collisionDetection(zombies[j].x, zombies[j].y + ActualSpriteHeightStartY, 50, (int)SPRITE_HEIGHT - ActualSpriteHeightStartY, (int)bullets[i].x, (int)bullets[i].y, 10, 10)){
                    if (zombies[j].direction == 1)
                        zombies[j].x += rand() % 5;
                    else
                        zombies[j].x -= rand() % 5;
                    removeIndex[totalremove++] = i;
                    int damage = DAMAGE_BY_BULLET * (rand() % 2 + 1);
                    zombies[j].health -= damage;
                    break;
                }
            }
        }
    }

    bulletRemove(removeIndex, totalremove);
    free(removeIndex);  // Free the remove index array
    removeIndex = NULL;
}


void bulletRender(void){
    bulletCollisionSystem(); // Called properly
    SDL_Rect src_rect ={0, 0, 900, 450};
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SDL_Rect dstRect = {0, 0, 10, 5};
    for (int i = 0; i < bulletCount; i++){
        if (bullets[i].direction)
            bullets[i].x += delta_time * bulletVelocity;
        else{
            bullets[i].x -= delta_time * bulletVelocity;
            flip = SDL_FLIP_HORIZONTAL;
        }
        dstRect.x = bullets[i].x;
        dstRect.y = bullets[i].y + 3;
        if (debug){
            SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(Renderer,&dstRect);
        }
        SDL_RenderCopyEx(Renderer, BulletTexture, &src_rect, &dstRect, 0, NULL, flip);
    }
};

void gameOver(void){
    font = TTF_OpenFont("./textFont/AmaticSC-Regular.ttf", 40);
    if (font == NULL){
        printf("Error opening Font: %s", TTF_GetError());
        game_is_running = false;
    }
    char line1[] = "GameOver";
    char line2[] = "...Enter Any Key To Exit...";

    SDL_Color textColor = {255, 0, 0, 255};

    SDL_Surface* textSurface1 = TTF_RenderText_Solid(font, line1, textColor);
    if (textSurface1 == NULL) {
        printf("Error rendering text (line 1): %s\n", TTF_GetError());
        game_is_running = false;
    }

    textColor.g = 255;
    textColor.b = 255;
    TTF_CloseFont(font);
    font = TTF_OpenFont("./textFont/AmaticSC-Regular.ttf", 30);
    SDL_Surface* textSurface2 = TTF_RenderText_Solid(font, line2, textColor);
    if (textSurface2 == NULL) {
        printf("Error rendering text (line 2): %s\n", TTF_GetError());
        SDL_FreeSurface(textSurface1);
        game_is_running = false;
    }

    SDL_Texture* textTexture1 = SDL_CreateTextureFromSurface(Renderer, textSurface1);
    if (textTexture1 == NULL) {
        printf("Error creating texture (line 1): %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface1);
        SDL_FreeSurface(textSurface2);
        game_is_running = false;
    }

    SDL_Texture* textTexture2 = SDL_CreateTextureFromSurface(Renderer, textSurface2);
    if (textTexture2 == NULL) {
        printf("Error creating texture (line 2): %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface1);
        SDL_FreeSurface(textSurface2);
        SDL_DestroyTexture(textTexture1);
        game_is_running = false;
    }

    // Free the surfaces after creating textures
    SDL_FreeSurface(textSurface1);
    SDL_FreeSurface(textSurface2);

    // Show cursor
    SDL_ShowCursor(true);

    // Set background color and clear the renderer
    SDL_SetRenderDrawColor(Renderer, 22, 22, 22, 255);
    SDL_RenderClear(Renderer);

    // Render line 1 (GameOver) at the center
    SDL_Rect textRect1;
    SDL_QueryTexture(textTexture1, NULL, NULL, &textRect1.w, &textRect1.h);
    textRect1.x = (WINDOW_WIDTH - textRect1.w) / 2;
    textRect1.y = (WINDOW_HEIGHT - textRect1.h) / 2 - 20; // Slight adjustment to move the text up

    SDL_RenderCopy(Renderer, textTexture1, NULL, &textRect1);

    // Render line 2 (Enter Any key...) slightly below the first line
    SDL_Rect textRect2;
    SDL_QueryTexture(textTexture2, NULL, NULL, &textRect2.w, &textRect2.h);
    textRect2.x = (WINDOW_WIDTH - textRect2.w) / 2;
    textRect2.y = textRect1.y + textRect1.h + 10; // 10 pixels below the first line

    SDL_RenderCopy(Renderer, textTexture2, NULL, &textRect2);

    // Clean up textures
    TTF_CloseFont(font);
    SDL_DestroyTexture(textTexture1);
    SDL_DestroyTexture(textTexture2);
}

int i = 0;
int zombie_X = WINDOW_WIDTH - 100;
SDL_RendererFlip flip;
int attackPlayer = false;
int spriteChangeRate = 0;
float zombieHealth = 100.0f;
int zombieIdle = false;


int temp = 0;
float tempTime = 0;
void removeZombie(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        int index = arr[i] - i;
        for (int j = index; j < zombieCount - 1; j++)
            zombies[j] = zombies[j + 1];  // Shift the other array element one step left: Clever RIGHT!!!
    }
    zombieCount-=size;
    zombieKilled+=size;
}

void zombieRender(void) {
    int run[] = {25, 115, 215, 313, 415, 507, 600};
    int atk[] = {25, 117, 220, 310};
    int idle[] = {24, 115, 215, 315, 411, 504, 600, 698, 795};
    int jump[] = {15, 113, 210, 310, 407, 503};
    int dead[] = {15, 110, 200, 305, 405};

    int size = 0;
    int *arr = NULL;
    SDL_Rect srcRect = {0, 0, 50, SPRITE_HEIGHT};

    for (int z = 0; z < zombieCount; z++){
        Zombie *zombie = &zombies[z];  // Pointer to current zombie

        if (zombie->dead){
            spriteChangeRate = 7;
            srcRect.w = 80;
            srcRect.x = dead[zombie->current_frame];
            if (zombie->current_frame == 5){
                arr = realloc(arr, size * sizeof(int));
                if (arr == NULL) {
                    // TODO: Handle Case?
                    printf("Memory Allocation failed!");
                    game_is_running = false;
                }
                arr[size++] = z;

                if (itemCount == 0) {
                    itemDrops = malloc(sizeof(ItemDrop));
                    if (itemDrops == NULL) {
                        // TODO: Handle this case?
                        continue;
                    }
                    itemCount = 1;
                } else {
                    itemCount++;
                    ItemDrop* temp = realloc(itemDrops, itemCount * sizeof(ItemDrop));
                    if (temp == NULL) {
                        // TODO: Handle this case?
                        continue;
                    }
                    itemDrops = temp;
                }
                itemDrops[itemCount - 1].x = zombie->x;
                itemDrops[itemCount - 1].y = zombie->y + SPRITE_HEIGHT - 10;
                itemDrops[itemCount - 1].itemIndex = 0;
                continue;
            }
        } else if (zombie->jump){
            spriteChangeRate = 6;
            srcRect.w = 60;
            if (!zombies[i].onPlatform)
                zombie->current_frame = 4;
            srcRect.x = jump[zombie->current_frame];
        } else if (zombie->idle) {
            spriteChangeRate = 9;
            srcRect.x = idle[zombie->current_frame];
        } else if (zombie->attackPlayer) {
            spriteChangeRate = 4;
            srcRect.w = 60;
            srcRect.x = atk[zombie->current_frame];
        } else {
            spriteChangeRate = 8;
            srcRect.x = run[zombie->current_frame];
        }

        // Update animation frame for the zombie
        if ((SDL_GetTicks() - zombie->last_frame_time) > (1000 / (float)spriteChangeRate * 1.3)){
            zombie->current_frame = (zombie->current_frame + 1) % (spriteChangeRate - 1);
            zombie->last_frame_time = SDL_GetTicks();
        }

        // Movement logic based on player's position
        int dist_To_player = player.x - zombie->x;

        SDL_Rect dstRect = {
            zombie->x,
            zombie->y,
            srcRect.w,
            srcRect.h
        };

        if (abs(dist_To_player) <= 25 && zombie->playerReachable && !zombie->dead && !player.dead){
            if (collisionDetection(player.x, player.y, 50, ActualSpriteHeightStartY, zombie->x, zombie->y, 50, ActualSpriteHeightStartY)) {
                zombie->idle = false;
                zombie->attackPlayer = true;
                if (zombie->current_frame >= 4){
                    zombie->current_frame = 0;
                    zombie->attackPlayer = false;
                }
            }
            else{
               zombie->attackPlayer = false;
            }
        } else if (abs(dist_To_player) > 25 && abs(dist_To_player) < (WINDOW_WIDTH - 10) && !zombie->dead && !player.dead) {
            zombie->attackPlayer = false;
            zombie->idle = false;
            int resultOfRandom = rand() % 100 + 50;
            float movementSpeed = delta_time * resultOfRandom;
            if (dist_To_player < -25)
                zombie->x -= movementSpeed * (float)(-dist_To_player) / abs(dist_To_player) - delta_time * resultOfRandom / 2;
            else if (dist_To_player >= 25)
                zombie->x += movementSpeed * (float)dist_To_player / abs(dist_To_player) - delta_time * resultOfRandom / 2;
        }

        if (player.dead)
            zombie->current_frame = 3;

        if(zombie->attackPlayer && !zombie->dead && !player.dead)
            if (zombie->current_frame >= 2){
                if (collisionDetection(player.x, player.y, 50, ActualSpriteHeightStartY, zombie->x, zombie->y, 50, ActualSpriteHeightStartY)){
                    player.health = (int)player.health;
                    player.health -= 15 * delta_time;
                    player.health = (float)player.health;
                }
            }

        flip = (dist_To_player < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        zombie->direction = (dist_To_player < 0) ? 1: -1;

        // ZombieSprite
        if (zombie->dead)
            SDL_RenderCopyEx(Renderer, ZombieDeadTexture, &srcRect, &dstRect, 0, 0, flip);
        else if (zombie->idle)
            SDL_RenderCopy(Renderer, ZombieIdleTexture, &srcRect, &dstRect);
        else if (zombie->attackPlayer)
            SDL_RenderCopyEx(Renderer, ZombieAtkTexture, &srcRect, &dstRect, 0, 0, flip);
        else if(zombie->jump)
            SDL_RenderCopyEx(Renderer, ZombieJumpTexture, &srcRect, &dstRect, 0, 0, flip);
        else
            SDL_RenderCopyEx(Renderer, ZombieRunTexture, &srcRect, &dstRect, 0, 0, flip);

        // Render zombie health bar
        if (!zombie->dead)
            if (!healthBar(zombies[z].x, zombies[z].y, zombies[z].health, false)){
                zombie->current_frame = 0;
                zombie->dead = true;
                zombie->idle = false;
                zombie->attackPlayer = false;
            }

        // Debug Setting
        if (debug) {
            SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
            dstRect.h -= ActualSpriteHeightStartY;
            dstRect.y += ActualSpriteHeightStartY;
            SDL_RenderDrawRect(Renderer, &dstRect);
            dstRect.h += ActualSpriteHeightStartY;
            dstRect.y -= ActualSpriteHeightStartY;
        }
    };
    if (size > 0)
        removeZombie(arr, size);
    free(arr);
    arr = NULL;
}

bool healthBar(int x, int y, float health, bool Player) {
    if (health <= 0)
        return false;

    // Draw Sprint Bar
    if (Player){
        SDL_SetRenderDrawColor(Renderer, 0, 0, 10, 255);
        SDL_Rect sprintbar = {x + 5, y + 45 - 4, 40, 5};

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(Renderer, &sprintbar);

        sprintbar.w = (player.jumpForce / 100 * 40);
        if (sprintbar.w > 40)
            sprintbar.w = 40;
        SDL_SetRenderDrawColor(Renderer, 50, 50, 255, 255);
        SDL_RenderFillRect(Renderer, &sprintbar);
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(Renderer, &sprintbar);
    }
    // Draw the background of the health bar
    SDL_SetRenderDrawColor(Renderer, 200, 200, 200, 255);
    SDL_Rect healthbar = {x + 5, y + 45, 40, 10};
    SDL_RenderFillRect(Renderer, &healthbar);

    // Draw the border of the health bar
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(Renderer, &healthbar);

    // Calculate the width of the filled health bar
    healthbar.w = (health / 100) * 40;
    if (healthbar.w > 40) {
        healthbar.w = 40; // Ensure the width doesn't exceed the max width
    }

    int healthInt = (int)health;
    int b = healthInt > 0 ? 255 % healthInt : 0; // Avoid division by zero

    // Set color based on whether it's a player or not
    Player? SDL_SetRenderDrawColor(Renderer, 255 - healthInt * 2.55, healthInt * 2.55, b, 255): SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(Renderer, &healthbar);
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(Renderer, &healthbar);
    return true;
}

int collisionDetection(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {
    return ((x1 + width1 > x2) && (x1 < x2 + width2) && (y1 + height1 > y2) && (y1 < y2 + height2));
}

// Function to clear all the things the game has started
void DestroyWindowInternal(void){
    free(bullets);
    free(zombies);
    bullets = NULL;
    zombies = NULL;

    SDL_DestroyTexture(BackgroundTexture);

    SDL_DestroyTexture(PlayerRunTexture);
    SDL_DestroyTexture(PlayerIdleTexture);
    SDL_DestroyTexture(PlayerShootTexture);
    SDL_DestroyTexture(PlayerJumpTexture);
    SDL_DestroyTexture(PlayerDeadTexture);

    SDL_DestroyTexture(ZombieRunTexture);
    SDL_DestroyTexture(ZombieAtkTexture);
    SDL_DestroyTexture(ZombieIdleTexture);
    SDL_DestroyTexture(ZombieJumpTexture);
    SDL_DestroyTexture(ZombieDeadTexture);

    SDL_DestroyTexture(BulletTexture);

    SDL_DestroyTexture(ResumeButtonTexture);
    SDL_DestroyTexture(ExitGameButtonTexture);

    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}
