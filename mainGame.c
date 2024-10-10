#include <SDL2/SDL_error.h>
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
int last_frame_time = 0;
float delta_time = 0.0f; // This is very important dont delete this ever.
float player_X = 100;
float player_Y = 0;
int groundLevel = 0;
int PLAYER_SPEED = 150;
int spriteInSpriteSheet = 0;
bool canJump = true;
float playerHealth = 100.0;
float playerSprintLevel = 100.0;
bool playerDead = false;

bool playerOnGround = false;
float lastTimeOfPlayerOnGround = 0;
bool followPlayer = false;

float zombie_Y;
typedef struct {
    int x, y;           // Position
    int health;         // Health
    int current_frame;    // Animation frame
    int direction;        // Direction (1 for right, -1 for left)
    bool attackPlayer;     // If zombie is attacking the player
    bool idle;             // If zombie is idle
    double last_frame_time;
} Zombie;
int zombieCount = 0;
int zombieKilled = 0;
Zombie *zombies = NULL;

typedef struct{
    float x, y;
    bool direction;// True is right and false left
} Bullet;
Bullet *bullets = NULL;
const unsigned short int bulletVelocity = 500;
unsigned short int bulletCount = 0;


int InitializeWindow(void);
int SetUp(void);
    void spawnZombies(int numberOfZombies);
void ProcessInput(void);
void Update(void);
    void bulletData(int X, int Y, bool facingDirection);
void Render(void);
    void zombieRender(void);
    void playerRender(void);
        void bulletRender(void);
        void gameOver(void);
    bool healthBar(int x, int y, float health, bool Player);
void DestroyWindow(void);

int collisionDetection(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);
bool gamePause = false;
int main(){
    game_is_running = InitializeWindow();
    SetUp();
    // Game Loop
    while (game_is_running) {
        ProcessInput();
        if (gamePause)
            continue;
        Update();
        Render();
    }
    DestroyWindow();
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

TTF_Font* font;
char text[] = "Game Over";
SDL_Surface * textSurface;
SDL_Texture *textTexture;
SDL_Color textColor = {255, 0, 0, 255};

int ground_height;
SDL_Rect background_img_src_rect;
SDL_Rect background_img_dest_rect;
int SetUp(void){
    srand(time(NULL));
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
    ZombieDeadTexture = IMG_LoadTexture(Renderer, "sprite/NPC/Zombie/Dead.png");

    BulletTexture = IMG_LoadTexture(Renderer, "sprite/bullet.png");

    if (!BackgroundTexture || !PlayerIdleTexture ||
        !PlayerRunTexture || !PlayerShootTexture ||
        !PlayerJumpTexture || !PlayerDeadTexture ||
        !ZombieRunTexture || !ZombieIdleTexture ||
        !ZombieAtkTexture || !ZombieDeadTexture || !BulletTexture
    ) {
        printf("Error loading texture: %s\n", IMG_GetError());
        game_is_running = false;
    }

    // Text Display
    font = TTF_OpenFont("textFont/AmaticSC-Regular.ttf", 40);
    if (font == NULL){
        printf("Error opening Font: %s", TTF_GetError());
        game_is_running = false;
    }
    textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (textSurface == NULL) {
        printf("Error rendering text: %s\n", TTF_GetError());
        game_is_running = false;
    }
    textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
    if (textTexture == NULL) {
        printf("Error creating texture: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
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

    player_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
    zombie_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;

    spawnZombies(1);
    return true;
}

void spawnZombies(int numberOfZombies) {
    float factor;
    // Reallocate memory for the new zombies
    if (zombies == NULL) {
        zombies = malloc(sizeof(Zombie) * numberOfZombies);
    } else {
        zombies = realloc(zombies, sizeof(Zombie) * (numberOfZombies + zombieCount));
    }

    // Spawn each zombie
    for (int i = 0; i < numberOfZombies; i++) {
        factor = (rand() % 2 == 0) ? -1 : 1;
        float value = player_X + factor * (rand() % 100 + (float)WINDOW_WIDTH / 2);

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

        zombieCount++;
    }
}

// Function to process keyboard input
short int moveLR = 0;
bool jump = false;
bool shoot = false;
bool debug = false;
void ProcessInput(void){
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT:
            game_is_running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    gamePause = !gamePause;
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    moveLR = -1;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    moveLR = 1;
                    break;
                case SDLK_w:
                case SDLK_UP:
                case SDLK_SPACE:
                    jump = true;
                    break;
                case SDLK_f:
                    shoot = true;
                    break;
                case SDLK_k:
                    debug = !debug;
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_a:
                case SDLK_LEFT:
                case SDLK_d:
                case SDLK_RIGHT:
                    moveLR = 0;
                    break;
                case SDLK_w:
                case SDLK_UP:
                case SDLK_SPACE:
                    jump = false;
                    canJump = false;
                    break;
                case SDLK_f:
                    shoot = false;
                    break;
            }
            break;
        default:
            break;
    }
}

// Function to Update Game Objects Per Frame
float jumpVelocity = -400.0f;
float velocityY = 0.0f;
float gravity = 980.0f; // 9.8m/s is earth's gravity i think
float_t zombie_frame_time = 0;
float rightCam = 3.0;
float leftCam = 2.0;

void Update(void) {
    // IMP: DELAY LOGIC, 60 fps for now.
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    if (SDL_GetTicks() - zombie_frame_time > (10000 + rand() % 30000)){ //spawn one zombie every 10-30 sec
        spawnZombies(rand() % 4 + 1);
        zombie_frame_time = SDL_GetTicks();
    }

    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    velocityY += gravity * delta_time;
    player_Y += velocityY * delta_time;
    if (player_Y >= WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT) {
        player_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
        velocityY = 0;
        canJump = true;
        playerOnGround = true;
        followPlayer = true;
        lastTimeOfPlayerOnGround = SDL_GetTicks();
    }

    if (playerDead)
        gameOver();

    int parallaxValue = delta_time * PLAYER_SPEED;
    if (player_X > (float)WINDOW_WIDTH * (rightCam/5)){
        background_img_src_rect.x += 100 * delta_time;
        if (background_img_src_rect.x >= background_img_src_rect.w)
            background_img_src_rect.x = 0;
        player_X = (rightCam/5) * (float)WINDOW_WIDTH;
        for (int i = 0; i < zombieCount; i++)
            zombies[i].x -= parallaxValue;
        for (int i = 0; i < bulletCount; i++)
            bullets[i].x -= parallaxValue;
    }
    else if (player_X < (float)WINDOW_WIDTH * (leftCam/5)){
        background_img_src_rect.x -= 50 * delta_time;
        if (background_img_src_rect.x <= 0)
            background_img_src_rect.x = background_img_src_rect.w;
        player_X = (leftCam/5) * (float)WINDOW_WIDTH;
        for (int i = 0; i < zombieCount; i++)
            zombies[i].x += parallaxValue;
        for (int i = 0; i < bulletCount; i++)
            bullets[i].x += parallaxValue;
    }

    // Healing Ability
    playerHealth += delta_time * 5;
    if (playerHealth > 100)
        playerHealth = 100;

    if (SDL_GetTicks() - lastTimeOfPlayerOnGround > 1000) // This means that the player is above the zombie
        followPlayer = false;

    if (zombie_Y >= WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT)
        zombie_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
    if (jump && canJump) {
        velocityY = jumpVelocity;
        canJump = false;
    }
}

void spawnMainBoss(void){
    if (zombieKilled % 15 == 0 && zombieKilled != 0){
        printf("Spawning The BOSS...\n");
    }
}

void otherRender(void){

}

int last_frame_time_for_idle = 0;
int addFactor = 43;
int n = 0;
float mulFactor = 1;

// Function to render game objects in the scene
void Render(void){
    // Background Color
    SDL_SetRenderDrawColor(Renderer, 211, 211, 211, 1);
    SDL_RenderClear(Renderer);

    SDL_RenderCopy(Renderer, BackgroundTexture, &background_img_src_rect, &background_img_dest_rect); //Background Image

    // Draw ground
    SDL_Rect ground_rect = {
        0, //x-position
        WINDOW_HEIGHT - ground_height, //y-position
        WINDOW_WIDTH, //width
        ground_height //height
    };
    SDL_SetRenderDrawColor(Renderer, 49, 91, 43, 255);
    SDL_RenderFillRect(Renderer, &ground_rect);
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
    ground_rect.h = 2;
    SDL_RenderFillRect(Renderer, &ground_rect);

    // Sprite Animation:
    zombieRender();
    playerRender();
        bulletRender();
    spawnMainBoss();

    // Render Other:
    otherRender();

    SDL_RenderPresent(Renderer); // Show rendered frame to user.
}

int current_frame = 0;
int playerDirection = 1;
int ActualSpriteHeight = 2 * (int)(SPRITE_HEIGHT / 5);
float shooting_last_frame = 0;
void playerRender(void) {
    SDL_RendererFlip flip = SDL_FLIP_NONE;  // No flip by default

    // Update playerDirection
    if (moveLR > 0)
        playerDirection = 1;  // Moving right
    else if (moveLR < 0)
        playerDirection = -1; // Moving left

    // If the direction is left, flip the sprite horizontally
    if (playerDirection == -1)
        flip = SDL_FLIP_HORIZONTAL;


    if (!canJump) {
        // TODO Complete: made it so that jump animation only plays in air.
        spriteInSpriteSheet = 7;  //frames in jump animation
        n = -8;
        mulFactor = 0.25;

        // finish the jump animation
        if (player_Y >= WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT - 10)
            current_frame = (current_frame + 1) % spriteInSpriteSheet;
        else
            current_frame = 4;  // Keep 4th sprite in jump sheet

        player_X += moveLR * delta_time * PLAYER_SPEED;
    }
    else if (moveLR && !shoot) {
        spriteInSpriteSheet = 8;
        n = -8;
        mulFactor = 1.5;
        player_X += moveLR * delta_time * PLAYER_SPEED;
        playerSprintLevel -= delta_time * 10;
        if (playerSprintLevel < 0.0f)
            playerSprintLevel = 0.0f;
    }
    else if (shoot) {
        spriteInSpriteSheet = 4;  // frames in shooting animation
        n = 0;
        mulFactor = 4;
    }
    else {
        spriteInSpriteSheet = 8;  // Idle animation frames
        n = -8;
        mulFactor = 1;
    }
    if (!moveLR){
        playerSprintLevel += delta_time * 20;
        if(playerSprintLevel > 100){
            playerSprintLevel = 100;
        }
    }

    SDL_Rect src_rect = {
        45 + n + addFactor * 2.98 * current_frame,
        0, 50, SPRITE_HEIGHT
    };

    SDL_Rect dst_rect = {(int)player_X, (int)player_Y, src_rect.w, src_rect.h};

    // Draw a red outline around the sprite (for debugging purposes)
    if (debug){
        SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
        dst_rect.h -= ActualSpriteHeight;
        dst_rect.y += ActualSpriteHeight;
        SDL_RenderDrawRect(Renderer, &dst_rect);
        dst_rect.h += ActualSpriteHeight;
        dst_rect.y -= ActualSpriteHeight;
    }

    if (shoot && (SDL_GetTicks() - shooting_last_frame) > (120)){
        if (flip == SDL_FLIP_NONE){
            bulletData(player_X, player_Y, true);
            player_X -= delta_time * (rand() % 10 + 20);
        }
        else{
            bulletData(player_X, player_Y, false);
            player_X += delta_time * (rand() % 10 + 20);
        }
        shooting_last_frame = SDL_GetTicks();
    }

    // Only allow the jump animation while in the air
    if (!canJump){
        playerSprintLevel -= delta_time * 30;
        if (playerSprintLevel < 0)
            playerSprintLevel = 0;
        SDL_RenderCopyEx(Renderer, PlayerJumpTexture, &src_rect, &dst_rect, 0, NULL, flip);
    }
    else if (moveLR && !shoot)
        SDL_RenderCopyEx(Renderer, PlayerRunTexture, &src_rect, &dst_rect, 0, NULL, flip);
    else if (shoot)
        SDL_RenderCopyEx(Renderer, PlayerShootTexture, &src_rect, &dst_rect, 0, NULL, flip);
    else
        SDL_RenderCopyEx(Renderer, PlayerIdleTexture, &src_rect, &dst_rect, 0, NULL, flip);

    // indipendent animation for player instead of bound my game FPS
    if ((SDL_GetTicks() - last_frame_time_for_idle) > 1000 / (spriteInSpriteSheet * mulFactor)) {
        current_frame = (current_frame + 1) % spriteInSpriteSheet;
        last_frame_time_for_idle = SDL_GetTicks();
    }
    if (!healthBar(player_X, player_Y, playerHealth, true))
        playerDead = true;
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
    bullets[bulletCount].y = (Y + ActualSpriteHeight + 22); // Set y-coordinate
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
            if (collisionDetection((int)zombies[j].x, (int)zombies[j].y, 50, (int)ActualSpriteHeight, (int)bullets[i].x, (int)bullets[i].y - 60, 10, 10)){
                removeIndex[totalremove++] = i;
                zombies[j].health -= DAMAGE_BY_BULLET;
                break;
            }
        }
    }

    bulletRemove(removeIndex, totalremove);
    free(removeIndex);  // Free the remove index array
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
    // Displaying GameOverText at Center Of Screen

    SDL_Rect textRect;
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = (WINDOW_WIDTH - textRect.w) / 2;
    textRect.y = (WINDOW_HEIGHT - textRect.h) / 2;

    SDL_RenderCopy(Renderer, textTexture, NULL, &textRect);

    SDL_RenderPresent(Renderer);
    game_is_running = false;

    SDL_Delay(5000);
}

int i = 0;
int zombie_X = WINDOW_WIDTH - 100;
SDL_RendererFlip flip;
int attackPlayer = false;
int spriteChangeRate = 0;
float zombieHealth = 100.0f;
int zombieIdle = false;
SDL_Rect srcRect = {0, 0, 50, SPRITE_HEIGHT};

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

    int *arr = NULL;
    int size = 0;

    for (int z = 0; z < zombieCount; z++){
        srcRect.w = 50;
        srcRect.h = SPRITE_HEIGHT;
        Zombie *zombie = &zombies[z];  // Pointer to current zombie

        // Render zombie health bar
        if (!healthBar(zombies[z].x, zombies[z].y, zombies[z].health, false)) {
            arr = realloc(arr, size * sizeof(int));
            if (arr == NULL) {
                printf("Memory Allocation failed!");
                game_is_running = false;
            }
            arr[size++] = z;
            continue;
        }

        if (zombie->idle) {
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
            zombie->current_frame = (zombie->current_frame + 1) % (spriteChangeRate-1);
            zombie->last_frame_time = SDL_GetTicks();
        }

        // Movement logic based on player's position
        int dist_To_player = player_X - zombie->x;

        SDL_Rect dstRect = {
            zombie->x,
            WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT,
            srcRect.w, SPRITE_HEIGHT
        };

        if (abs(dist_To_player) <= 25 && (player_Y - zombie->y >= -40)) {
            if (collisionDetection(player_X, player_Y, 50, ActualSpriteHeight, zombie->x, zombie->y, 50, ActualSpriteHeight)) {
                zombie->idle = false;
                zombie->attackPlayer = true;
                flip = (dist_To_player < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            }
        } else if (abs(dist_To_player) > 25 && abs(dist_To_player) < (WINDOW_WIDTH - 10) && followPlayer) {
            zombie->attackPlayer = false;
            zombie->idle = false;
            int resultOfRandom = rand() % 100 + 50;
            float movementSpeed = delta_time * resultOfRandom;
            if (dist_To_player < -25)
                zombie->x -= movementSpeed * (float)(-dist_To_player) / abs(dist_To_player) - delta_time * resultOfRandom / 2;
            else if (dist_To_player >= 25)
                zombie->x += movementSpeed * (float)dist_To_player / abs(dist_To_player);
        } else if (!followPlayer){
            zombie->attackPlayer = false;
        }
        else{
            zombie->attackPlayer = false;
            zombie->idle = true;
        }

        if(zombie->attackPlayer){
            if (zombie->current_frame == 2){
                if (collisionDetection(player_X, player_Y, 50, ActualSpriteHeight, zombie->x, zombie->y, 50, ActualSpriteHeight)){
                    playerHealth = (int)playerHealth;
                    playerHealth -= 20 * delta_time;
                    playerHealth = (float)playerHealth;
                }
            }
        }
        if (debug) {
            SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
            dstRect.h -= ActualSpriteHeight;
            dstRect.y += ActualSpriteHeight;
            SDL_RenderDrawRect(Renderer, &dstRect);
            dstRect.h += ActualSpriteHeight;
            dstRect.y -= ActualSpriteHeight;
        }

        flip = (dist_To_player < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        // ZombieSprite
        if (zombie->idle) {
            SDL_RenderCopy(Renderer, ZombieIdleTexture, &srcRect, &dstRect);
        } else if (zombie->attackPlayer) {
            SDL_RenderCopyEx(Renderer, ZombieAtkTexture, &srcRect, &dstRect, 0, 0, flip);
        } else {
            SDL_RenderCopyEx(Renderer, ZombieRunTexture, &srcRect, &dstRect, 0, 0, flip);
        }
    };
    if (size > 0)
        removeZombie(arr, size);
    free(arr);
}

bool healthBar(int x, int y, float health, bool Player) {
    if (health <= 0) {
        return false;
    }

    // Draw Sprint Bar
    if (Player){
        SDL_SetRenderDrawColor(Renderer, 0, 0, 10, 255);
        SDL_Rect sprintbar = {x + 5, y + 45 - 4, 40, 5};

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(Renderer, &sprintbar);

        sprintbar.w = (playerSprintLevel / 100 * 40);
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
void DestroyWindow(void){
    free(bullets);
    free(zombies);
    SDL_DestroyTexture(BackgroundTexture);

    SDL_DestroyTexture(PlayerRunTexture);
    SDL_DestroyTexture(PlayerIdleTexture);
    SDL_DestroyTexture(PlayerShootTexture);
    SDL_DestroyTexture(PlayerJumpTexture);
    SDL_DestroyTexture(PlayerDeadTexture);

    SDL_DestroyTexture(ZombieRunTexture);
    SDL_DestroyTexture(ZombieAtkTexture);
    SDL_DestroyTexture(ZombieIdleTexture);
    SDL_DestroyTexture(ZombieDeadTexture);

    SDL_DestroyTexture(BulletTexture);

    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    IMG_Quit();
    SDL_Quit();
}
