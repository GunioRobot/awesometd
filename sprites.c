#include <SDL.h>

#include <math.h>

#include "render.h"
#include "sprites.h"
#include "level.h"
#include "settings.h"
#include "game.h"

int level_monster[1][10][40] = {
    {
        { 1,1,1,1,1,1,1,1,1,1 },
        { 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 },
        { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 }
    }
};

int monster_batches[1][10] = {
    { 10, 15, 30, 0, 0, 0, 0, 0, 0, 0 }
};

int level_batches[1] = {
    10
};

struct monster monster_definitions[3] = {
    { 0,0,0,0,15,15,4,DIRECTION_S,0,0,10,5 },
    { 0,0,0,0,20,20,5,DIRECTION_S,0,0,20,10 },
    { 0,0,0,0,50,50,4,DIRECTION_S,0,0,35,15 }
};

struct tower tower_definitions[2] = {
    { 0,0,1,0,1,10,5,0,50,0,100 },
    { 0,0,1,0,1,50,30,10,100,0,50 }
};

int level = 0;
int batch = 0;
int monster = 0;


struct monster monsters[MAX_MONSTERS];
struct tower towers[MAX_TOWERS];
struct sprites sprites;


int sprites_inited = 0;

void load_sprite_from_pic(char *filename, int width, int height, int frames) {
    int i,d;
    sprites.sprites[sprites.spritecount] = SDL_LoadBMP(filename);
    if ( sprites.sprites[sprites.spritecount] != NULL ) {
        sprites.frames[sprites.spritecount] = frames;
        SDL_SetColorKey(sprites.sprites[sprites.spritecount], 
                        SDL_SRCCOLORKEY, 
                        SDL_MapRGB(
                            sprites.sprites[sprites.spritecount]->format,
                            255,0,255)
                        );
        for (i=0;i<frames;i++) {
            for (d=0;d<MAX_DIRECTIONS;d++) {
                sprites.slices[sprites.spritecount][d][i].x = 0 + (i*width);
                sprites.slices[sprites.spritecount][d][i].y = 0 + (d*height);
                sprites.slices[sprites.spritecount][d][i].w = width;
                sprites.slices[sprites.spritecount][d][i].h = height;
            }
        }
        sprites.spritecount++;
    } else {
        printf("Big bad error while loading file: \"%s\": %s\n", filename, SDL_GetError());
    }
}

void init_sprites(void) {
    memset(&monsters, 0x00, sizeof(struct monster)*MAX_MONSTERS);
    memset(&towers, 0x00, sizeof(struct tower)*MAX_TOWERS);
    memset(&sprites, 0x00, sizeof(struct sprites));

    load_sprite_from_pic("enemy1.bmp",32,32,2);
    load_sprite_from_pic("tower1.bmp",32,32,1);


    sprites_inited++;
}

void draw_sprite(SDL_Surface *s, int spid, int fid, int rot, int x, int y) {
    if ( sprites_inited == 0 ) init_sprites();

    int ax, ay;
    SDL_Rect dst = { x, y, RECTSIZE_X, RECTSIZE_Y };
    if ( spid < sprites.spritecount ) {
        if ( fid < sprites.frames[spid] ) {
            if ( x < VIDEOMODE_WIDTH && y < VIDEOMODE_HEIGHT ) {
                dst.w = sprites.slices[spid][rot][fid].w;
                dst.h = sprites.slices[spid][rot][fid].h;
                for (ax=(x/RECTSIZE_X);ax<=((dst.x+dst.w-1)/RECTSIZE_X);ax++) {
                    for (ay=(y/RECTSIZE_Y);ay<=((dst.y+dst.h-1)/RECTSIZE_Y);ay++) {
                        if ( ax < (VIDEOMODE_WIDTH/RECTSIZE_X) && ay < (VIDEOMODE_HEIGHT/RECTSIZE_Y) ) updaterect(ax, ay);
                    }
                }
                updaterect(0,0);
                SDL_BlitSurface(sprites.sprites[spid], &sprites.slices[spid][rot][fid], s, &dst);
            } else {
                printf("ERR -- draw_sprite(): Attempt to draw outside the screen.\n");
            }
        } else {
            printf("ERR -- draw_sprite(): Frame doesn't exist.\n");
        }
    } else {
        printf("ERR -- draw_sprite(): Sprite doesn't exist.\n");
    }
}

void spawn_monster(void) {
    int i;

    if ( monster_batches[level][batch] == monster ) {
        for (i=0;i<MAX_MONSTERS;i++) {
            if ( monsters[i].cur_hp > 0 ) {
                printf("Not spawning: Stuff is still alive.\n");
//   Not spawning any new monsters, 
//   because we still have live monsters on the field.
                return;
            }
        }
        monster = 0;
        batch++;
        return;
    }

    if ( batch == level_batches[level] ) {
//  level won, or something... \o/
        return;
    }

    for (i=0;i<MAX_MONSTERS;i++) {
        if ( monsters[i].cur_hp == 0 ) {
            monsters[i] = monster_definitions[level_monster[level][batch][monster]-1];
            monsters[i].loc_x = 3*RECTSIZE_X;
            monsters[i].loc_y = 0*RECTSIZE_Y;
            monsters[i].direction = DIRECTION_S;
            monster++;
            return;
        }
    }
}

void move_monster(void) {
    if ( sprites_inited != 1 ) return;
    int i, old_x, old_y;
    for (i=0;i<MAX_MONSTERS;i++) {
        if ( monsters[i].cur_hp > 0 ) {
            old_x = monsters[i].loc_x / RECTSIZE_X;
            old_y = monsters[i].loc_y / RECTSIZE_Y;
            if ( monsters[i].loc_x/RECTSIZE_X == 19 && monsters[i].loc_y/RECTSIZE_Y == 2 ) {
                update_lives(-1);
                draw_numbers();
                monsters[i].cur_hp = 0;
            }
            monsters[i].progress = monsters[i].progress + monsters[i].speed;
            if ( monsters[i].progress > 10 ) {
                switch(monsters[i].direction) {
                    case DIRECTION_N:
                        if ( is_path( monsters[i].loc_x/RECTSIZE_X, ((monsters[i].loc_y-1)/RECTSIZE_Y)) ) monsters[i].loc_y--;
                        else if ( is_path(((monsters[i].loc_x)/RECTSIZE_X)+1,(monsters[i].loc_y/RECTSIZE_Y)) ) {
                            monsters[i].loc_x++;
                            monsters[i].direction = DIRECTION_E;
                        }
                        else if ( is_path(((monsters[i].loc_x-1)/RECTSIZE_X),(monsters[i].loc_y/RECTSIZE_Y)) ) {
                            monsters[i].loc_x--;
                            monsters[i].direction = DIRECTION_W;
                        }
                    break;
                    case DIRECTION_E:
                        if ( is_path(((monsters[i].loc_x)/RECTSIZE_X)+1,monsters[i].loc_y/RECTSIZE_Y) ) monsters[i].loc_x++;
                        else if ( is_path(monsters[i].loc_x/RECTSIZE_X, ((monsters[i].loc_y)/RECTSIZE_Y)+1) ) {
                            monsters[i].loc_y++;
                            monsters[i].direction = DIRECTION_S;
                        }
                        else if ( is_path( monsters[i].loc_x/RECTSIZE_X,((monsters[i].loc_y)/RECTSIZE_Y)-1) ) {
                            monsters[i].loc_y--;
                            monsters[i].direction = DIRECTION_N;
                        }
                    break;
                    case DIRECTION_S:
                        if ( is_path(monsters[i].loc_x/RECTSIZE_X,((monsters[i].loc_y)/RECTSIZE_Y)+1) ) monsters[i].loc_y++;
                        else if ( is_path(((monsters[i].loc_x)/RECTSIZE_X)+1,(monsters[i].loc_y/RECTSIZE_Y)) ) {
                            monsters[i].loc_x++;
                            monsters[i].direction = DIRECTION_E;
                        }
                        else if ( is_path(((monsters[i].loc_x-1)/RECTSIZE_X),(monsters[i].loc_y/RECTSIZE_Y))) {
                            monsters[i].loc_x--;
                            monsters[i].direction = DIRECTION_W;
                        }
                    break;
                    case DIRECTION_W:
                        if ( is_path(((monsters[i].loc_x-1)/RECTSIZE_X),(monsters[i].loc_y/RECTSIZE_Y)) ) monsters[i].loc_x--;
                        else if ( is_path( monsters[i].loc_x/RECTSIZE_X,((monsters[i].loc_y)/RECTSIZE_Y)-1) ) {
                            monsters[i].loc_y--;
                            monsters[i].direction = DIRECTION_N;
                        }
                        else if ( is_path(monsters[i].loc_x/RECTSIZE_X, ((monsters[i].loc_y)/RECTSIZE_Y)+1) ) {
                            monsters[i].loc_y++;
                            monsters[i].direction = DIRECTION_S;
                        }
                    break;
                }
                monsters[i].progress = monsters[i].progress - 10;
            }
//            if ( old_x != (monsters[i].loc_x / RECTSIZE_X) || old_y != (monsters[i].loc_y / RECTSIZE_Y)) updaterect(old_x, old_y);
            updaterect(old_x, old_y);
            draw_sprite(get_screen(),monsters[i].spid, monsters[i].frameno,monsters[i].direction, monsters[i].loc_x, monsters[i].loc_y);
            draw_health(get_screen(),monsters[i].loc_x, monsters[i].loc_y, monsters[i].cur_hp, monsters[i].max_hp);
        }
    }
//    printf("Moving monsters...\n");
}

void draw_health(SDL_Surface *s, int x, int y, int cur, int max) {
    SDL_Rect green = { x+2,y,(28*cur/max),3 };
    SDL_Rect red = { x+2+green.w,y,28-green.w,3 };
    SDL_FillRect(s, &green, SDL_MapRGB(s->format,0,255,0));
    SDL_FillRect(s, &red, SDL_MapRGB(s->format, 255,0,0));
}

void draw_towers(void) {
    int i;
    for (i=0;i<MAX_TOWERS;i++) {
        if ( towers[i].active == 1 ) {
            updaterect(towers[i].loc_x, towers[i].loc_y);
            draw_sprite(get_screen(),towers[i].spid, towers[i].frameno,DIRECTION_N,towers[i].loc_x*RECTSIZE_X,towers[i].loc_y*RECTSIZE_Y);
        }
    }
}

void draw_tower(int x, int y) {
    int i;
    for (i=0;i<MAX_TOWERS;i++) {
        if ( towers[i].active == 1 ) {
            if ( towers[i].loc_x == x && towers[i].loc_y == y ) {
                updaterect(x,y);
                draw_sprite(get_screen(),towers[i].spid, towers[i].frameno, DIRECTION_N,towers[i].loc_x*RECTSIZE_X,towers[i].loc_y*RECTSIZE_Y);
                return;
            }
        }
    }
}

int has_tower(int x, int y) {
    int i;
    for (i=0;i<MAX_TOWERS;i++) {
        if ( towers[i].active == 1 ) {
            if ( towers[i].loc_x == x && towers[i].loc_y == y ) {
                return 1;
            }
        }
    }
    return 0;
}

void add_tower(int x, int y, int type) {
    int i;
    if ( have_money(tower_definitions[type].price) == 1 ) {
        if ( has_tower(x, y) == 0 ) {
            for (i=0;i<MAX_TOWERS;i++) {
                if ( towers[i].active == 0 ) {
                    towers[i] = tower_definitions[type];
                    towers[i].loc_x = x;
                    towers[i].loc_y = y;
                    update_money(tower_definitions[type].price*-1);
                    draw_numbers();
                    return;
                }
            }
        }
    }
}

void animate_sprites(void) {
    int i;
    for (i=0;i<MAX_MONSTERS;i++) {
        if (monsters[i].cur_hp > 0) {
            if ( (monsters[i].frameno+1) < sprites.frames[monsters[i].spid] ) monsters[i].frameno++;
            else monsters[i].frameno = 0;
        }
    }
    for (i=0;i<MAX_TOWERS;i++) {
        if (towers[i].active == 1 ) {
            if ( (towers[i].frameno+1) < sprites.frames[towers[i].spid] ) towers[i].frameno++;
            else towers[i].frameno = 0;
            updaterect(towers[i].loc_x,towers[i].loc_y);
        }
    }
}

void shoot_towers(void) {
    int i,y;
    double k1,k2,h;
    double shortest = 0;

    int target;

    for (i=0;i<MAX_TOWERS;i++) {
        if (towers[i].active == 1) {
            if (towers[i].reloadtimeleft == 0) {
                target = -1;
                shortest = 0;
                for (y=0;y<MAX_MONSTERS;y++) {
                    if (monsters[y].cur_hp > 0) {
                        k1 = ((towers[i].loc_x*32)+16)-(monsters[y].loc_x+16);
                        if ( k1 < 0 ) k1 = k1 * -1;

                        k2 = ((towers[i].loc_y*32)+16)-(monsters[y].loc_y+16);
                        if ( k2 < 0 ) k2 = k2 * -1;

                        h = sqrt(pow(k1,2) + pow(k2,2));


                        if ( h < towers[i].range ) {
                            if ( shortest == 0 || shortest > h ) {
                                shortest = h;
                                target = y;
                            }
                        }
                    }
                }
                if ( target > -1 ) {
                    monsters[target].cur_hp = monsters[target].cur_hp-towers[i].damage;
                    if ( monsters[target].cur_hp <= 0 ) {
                        monsters[target].cur_hp = 0;

                        updaterect(monsters[target].loc_x/RECTSIZE_X, monsters[target].loc_y/RECTSIZE_Y);
                        updaterect((monsters[target].loc_x+31)/RECTSIZE_X,monsters[target].loc_y/RECTSIZE_Y);
                        updaterect((monsters[target].loc_x+31)/RECTSIZE_X,(monsters[target].loc_y+31)/RECTSIZE_Y);
                        updaterect(monsters[target].loc_x/RECTSIZE_X,(monsters[target].loc_y+31)/RECTSIZE_Y);

                        update_score(monsters[target].score);
                        update_money(monsters[target].money);
                        draw_numbers();
                    }

                    towers[i].reloadtimeleft = towers[i].reload;
                }
            } else {
                towers[i].reloadtimeleft--;
            }
        }
    }
}
