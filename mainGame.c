#include <SDL2/SDL_error.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

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
const int PLAYER_SPEED = 150;
int spriteInSpriteSheet = 0;
bool canJump = true;
float_t playerHealth = 100.0;
bool playerDead = false;

unsigned short int zombieCount = 3; // 10000 is max zombie that should be rendered, WHY? Just cause
float zombie_Y;
typedef struct {
    int x, y;           // Position
    int health;         // Health
    int current_frame;    // Animation frame
    int direction;        // Direction (1 for right, -1 for left)
    bool attackPlayer;     // If zombie is attacking the player
    bool idle;             // If zombie is idle
    float last_frame_time;
} Zombie;
Zombie zombies[MAX_ZOMBIES];

typedef struct{
    float x, y;
    bool direction;// True is right and false left
} Bullet;
const unsigned short int bulletVelocity = 300;
unsigned short int bulletCount = 0;


int InitializeWindow(void);
int SetUp(void);
void ProcessInput(void);
void Update(void);
    void bulletData(int X, int Y, bool facingDirection);
void Render(void);
    void zombieRender(void);
    void playerRender(void);
        void bulletRender(void);
    bool healthBar(int x, int y, float health, bool Player);
void DestroyWindow(void);

int collisionDetection(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);

int main(){
    game_is_running = InitializeWindow();
    SetUp();
    // Game Loop
    while (game_is_running) {
        ProcessInput();
        Update();
        Render();
    }
    DestroyWindow();
    return true;
}


// Function to Initialize Game State
int InitializeWindow(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        fprintf(stderr,"Error: %s",SDL_GetError());
        return false;
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

int ground_height;
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
        fprintf(stderr, "Error loading texture: %s\n", IMG_GetError());
        game_is_running = false;
    }

    // Other
    ground_height = WINDOW_HEIGHT / 12;
    background_img_dest_rect.x = 0;
    background_img_dest_rect.y = 0;
    background_img_dest_rect.w = WINDOW_WIDTH;
    background_img_dest_rect.h = WINDOW_HEIGHT - ground_height;

    player_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
    zombie_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;

    if (zombieCount > (MAX_ZOMBIES / 2)) {
        zombieCount = MAX_ZOMBIES/2;
    }

    for (int i = 0; i < zombieCount; i++) {
        zombies[i].x = WINDOW_WIDTH - 100 - (rand() % (500)); // SpawnPoint
        zombies[i].y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
        zombies[i].health = 100.0f;
        zombies[i].current_frame = 0;
        zombies[i].direction = -1;  // Initially facing left
        zombies[i].attackPlayer = false;
        zombies[i].idle = true;
        zombies[i].last_frame_time = SDL_GetTicks();
    }
    return true;
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
                    game_is_running = false;
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
                    jump = true;
                    break;
                case SDLK_j:
                    shoot = true;
                    break;
                case SDLK_k:
                    debug = !debug;
                    break;
                case SDLK_l:
                    zombies[0].health = 0;
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
                    jump = false;
                    canJump = false;
                    break;
                case SDLK_j:
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

void Update(void) {
    if (zombieCount > (MAX_ZOMBIES/2))
        zombieCount = MAX_ZOMBIES / 2;

    // IMP: DELAY LOGIC, 60 fps for now.
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    velocityY += gravity * delta_time;
    player_Y += velocityY * delta_time;
    if (player_Y >= WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT) {
        player_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
        velocityY = 0;
        canJump = true;
    }

    if (zombie_Y >= WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT)
        zombie_Y = WINDOW_HEIGHT - ground_height - SPRITE_HEIGHT;
    if (jump && canJump) {
        velocityY = jumpVelocity;
        canJump = false;
    }
}


int last_frame_time_for_idle = 0;
int addFactor = 43;
int n = 0;
float mulFactor = 1;

// Function to render game objects in the scene
void Render(void){
    // Background Color
    SDL_SetRenderDrawColor(Renderer, 150, 180, 230, 255);
    SDL_RenderClear(Renderer);

    // Background Image Rendering
    SDL_RenderCopy(Renderer, BackgroundTexture, NULL, &background_img_dest_rect); //Background Image

    // Draw ground
    SDL_Rect ground_rect = {
        (int)0, //x-position
        (int)WINDOW_HEIGHT - ground_height, //y-position
        (int)WINDOW_WIDTH, //width
        (int)ground_height //height
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

        // Ensure player doesn't move beyond screen edges
        if (player_X > WINDOW_WIDTH - 50)
            player_X = WINDOW_WIDTH - 50;
        else if (player_X < 0)
            player_X = 0;

    }
    else if (moveLR && !shoot) {
        spriteInSpriteSheet = 8;
        n = -8;
        mulFactor = 1.5;
        player_X += moveLR * delta_time * PLAYER_SPEED;
        if (player_X > WINDOW_WIDTH - 50)
            player_X = WINDOW_WIDTH - 50;

        if (player_X < 0)
            player_X = 0;
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

    if (shoot && (SDL_GetTicks() - shooting_last_frame) > 100){
        flip == SDL_FLIP_NONE? bulletData(player_X, player_Y, true): bulletData(player_X, player_Y, false);
        shooting_last_frame = SDL_GetTicks();
    }

    // Only allow the jump animation while in the air
    if (!canJump)
        SDL_RenderCopyEx(Renderer, PlayerJumpTexture, &src_rect, &dst_rect, 0, NULL, flip);
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

Bullet *bullets = NULL;
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
    bool checkForBulletToRight = true;
    bool checkForBulletToLeft = true;
    int totalremove = 0;
    int *removeIndex = malloc(bulletCount * sizeof(int));
    bool anyCollision = false;

    for (int i = 0; i < bulletCount; i++){
        anyCollision = false;

        // Check if the bullet is out of bounds
        if (bullets[i].x < 0 || bullets[i].x > WINDOW_WIDTH){
            removeIndex[totalremove++] = i;
            continue;
        }

        if (bullets[i].direction) {
            if (!checkForBulletToRight) continue;
        } else { // Bullet to left
            if (!checkForBulletToLeft) continue;
        }

        // Check collision with zombies
        for (int j = 0; j < zombieCount; j++){
            if (collisionDetection((int)zombies[j].x, (int)zombies[j].y, 50, (int)ActualSpriteHeight, (int)bullets[i].x, (int)bullets[i].y - 75, 10, 10)){
                removeIndex[totalremove++] = i;
                anyCollision = true;
                zombies[j].health -= DAMAGE_BY_BULLET;
                break;
            }
        }

        // Stop further checks in this direction if no collisions found
        if (!anyCollision){
            if (bullets[i].direction)
                checkForBulletToRight = false;
            else
                checkForBulletToLeft = false;
        }
    }

    bulletRemove(removeIndex, totalremove);
    free(removeIndex);  // Free the remove index array
}


void bulletRender(void){
    for (int i = 0; i < bulletCount; i++){
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (bullets[i].direction){
            bullets[i].x += delta_time * bulletVelocity;
            flip = SDL_FLIP_HORIZONTAL;
        }
        else
            bullets[i].x -= delta_time * bulletVelocity;

        SDL_Rect src_rect ={
            0,
            0,
            100,
            100
        };

        SDL_Rect dstRect = {
            bullets[i].x,
            bullets[i].y - 4,
            10, 10
        };
        if (debug){
            SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(Renderer,&dstRect);
        }
        SDL_RenderCopyEx(Renderer, PlayerIdleTexture, &src_rect, &dstRect, 0, NULL, flip);
    }
    bulletCollisionSystem(); // Called properly
};

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
}

void zombieRender(void) {
    int run[] = {25, 115, 215, 313, 415, 507, 600};
    int atk[] = {25, 117, 220, 310};
    int idle[] = {24, 115, 215, 315, 411, 504, 600, 698, 795};

    int *arr = NULL;
    int size = 0;

    for (int z = 0; z < zombieCount; z++){
        srcRect.w = 50;
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
        if ((SDL_GetTicks() - zombie[z].last_frame_time) > (1000 / (float)spriteChangeRate * (1.3 + ((float)rand() / RAND_MAX) * (1.6 - 1.3)))) {
            zombie->current_frame = (zombie->current_frame + 1) % (spriteChangeRate - 1);
            zombie[z].last_frame_time = SDL_GetTicks();
        }

        // Movement logic based on player's position
        int dist_To_player = player_X - zombie->x;
        float movementSpeed = delta_time * 100;

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
        } else if (abs(dist_To_player) > 25 && abs(dist_To_player) < ((int)(WINDOW_WIDTH / 2) * 1.2) && (player_Y - zombie->y >= -40)) {
            zombie->attackPlayer = false;
            zombie->idle = false;
            if (dist_To_player < -25)
                zombie->x -= movementSpeed * (float)(-dist_To_player) / abs(dist_To_player) - delta_time * 50;
            else if (dist_To_player >= 25)
                zombie->x += movementSpeed * (float)dist_To_player / abs(dist_To_player);
        } else {
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
    if (size > 0){
        removeZombie(arr, size);
        free(arr);
    }
}

bool healthBar(int x, int y, float health, bool Player) {
    if (health <= 0) {
        return false;
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
    // free(zombies);
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
    IMG_Quit();
    SDL_Quit();
}
