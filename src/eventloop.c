/*
    Awesome Tower Defense
    Copyright (C) 2008-2010  Trygve Vea and contributors (read AUTHORS)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <SDL.h>

#include "eventloop.h"
#include "video.h"
#include "level.h"

static char lastkeys[32] = { 0 };

int EventLoop(void) {
    SDL_Event eventqueue;
    while(SDL_PollEvent(&eventqueue)) {
        switch(eventqueue.type) {
            case SDL_QUIT:
                return -1;
            break;
            case SDL_VIDEORESIZE:
                VideoSetMode(eventqueue.resize.w,eventqueue.resize.h);
            break;
            case SDL_KEYDOWN:
                memmove(lastkeys, lastkeys + 1, sizeof(lastkeys) - 1);
                lastkeys[sizeof(lastkeys) - 1] = eventqueue.key.keysym.sym;
                if (memcmp(lastkeys + sizeof(lastkeys) - strlen("iddqd"),
                    "iddqd", strlen("iddqd")) == 0)
                    printf("/*iddqd();*/\n");
                if (memcmp(lastkeys + sizeof(lastkeys) - strlen("idkfa"),
                     "idkfa", strlen("idkfa")) == 0)
                     printf("/*idkfa();*/\n");

                switch(eventqueue.key.keysym.sym)
                {
                    case SDLK_UP: LevelCamera[1]--; break;
                    case SDLK_DOWN: LevelCamera[1]++; break;
                    case SDLK_RIGHT: LevelCamera[0]++; break;
                    case SDLK_LEFT: LevelCamera[0]--; break;
                }
            break;
            default:
            break;
        }
    }
    return 0;
}

