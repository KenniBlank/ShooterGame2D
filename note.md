Compilation
```bash
gcc -o hello hello2_window.c $(pkg-config --cflags --libs sdl2)
```

Because it is difficult writing this everytime, making an alias:
```bash
alias gccsdl='gcc $(pkg-config --cflags --libs sdl2) -o'
source ~/.bashrc  # reloading the shell configuration
```
This makes it so that I can compile with just:
```py
gccsdl hello hello.c
```
> In my case this didn't work so I worked with Makefile which I ran with a shellscript to make it dynamic


# SDL2 Doc for me
[Orgininal Doc](https://wiki.libsdl.org/SDL2/APIByCategory)

SDL works with the idea of dubble buffer technique while rendering.
User sees Front buffer.
Back buffer is hidden, we render things into it. After we have completed populating the back buffer,
we swap the front and back buffer and then we work on the next buffer.

- To create DeltaTime:
SDL_GetTicks(): this keeps track of time that it took after SDL_Init.
SDL_Sleep(): you can waste time like this OR
while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME)));
// This is a bad idea because processor uses 100% of CPU during while Loop: it takes resources from other processes

- Changing object pixel per second:
We use delta_time to get the latest position of an object so as to update it to the latest position rather that leave
it be running at the previous position.

> The Beauty of Delta time is that you dont need sdl_delay after using it

```c
// IMP: DELAY LOGIC IF NEEDED, const frame for all
int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    SDL_Delay(time_to_wait);
```


# The Story:
Set in a post apocalyptic world, you are tasked to shut down the lab that is causing this catastroupe.
As zombies, bugs, and inhumane species fill the world, you move forward to save this world from doom.

- Background:
You are a man who has lost his wife and child to this laboratory.
You are
