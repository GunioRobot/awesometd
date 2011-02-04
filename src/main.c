#include <stdio.h>

#include <SDL.h>

#include "video.h"
#include "input.h"
#include "game.h"
//#include "audio.h"

struct main {
    int last_draw;
    int f;
};

struct all {
    struct video v;
    struct input i;
    struct game g;
//    struct audio a;

    struct main m;
};

int main(int argc, char *argv[])
{
    struct all all;

    // Is there any way I can make sure that all is 0 without doing this manually?
    memset(&all, '\0', sizeof(struct all));

    if ( SDL_Init(SDL_INIT_VIDEO) != 0 )
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }
    if ( SDL_InitSubSystem(SDL_INIT_AUDIO) != 0 )
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    all.v = vSetup();
//    all.a = aSetup();
    all.g = gNew();

    while ( !iEventLoop(&all.i, &all.g) ) // Inputhandling is done here.
    {
        // Moving the game forward is done here.
        gDo(&all.g);

//        aDo(&all.g, &all.a);

        // Drawing is done here.
        vDraw(&all.v, &all.g);

        all.m.f++;
        if ( all.i.f.debug )
            printf("Frame: %d\n", all.m.f);

        int t = (1000/FPS)-(SDL_GetTicks()-all.m.last_draw);
        if ( t > 0 && t < 1000 ) SDL_Delay(t);
        all.m.last_draw = SDL_GetTicks();
    }

    return 0;
}
