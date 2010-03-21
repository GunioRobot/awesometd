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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "level.h"
#include "game.h"

//static GSList *Gamedata.EnemyList;

void MessageAdd(char *string)
{
    String *s = g_malloc(sizeof(String));
    SDL_Color c = { 255,255,255,255 };
    printf("Message: %s\n",string);
    s = VideoLoadText(string,c);
    s->timeleft = 1000;
    s->alpha = 100;
    Gamedata.TextList = g_slist_insert(Gamedata.TextList,s,0);
}

void MessageDo(gpointer data, gpointer user_data)
{
    String *s = (String*)data;
    if ( s->timeleft > 0 ) s->timeleft--;
    else if ( s->timeleft == 0 && s->alpha > 0 ) s->alpha--;
    else if ( s->timeleft == 0 && s->alpha == 0 ) 
    {
        glDeleteTextures(1,&(s->texid));
        Gamedata.TextList = g_slist_remove(Gamedata.TextList,data);
    }
    else return;
}

void EnemyPrint(gpointer data, gpointer user_data);
void EnemyTemplatePrint(gpointer key, gpointer value, gpointer user_data);

void EnemyAdd(gint id,gint delay,StartPosition *sp)
{
    Enemy *e;
    Enemy *t = g_hash_table_lookup(Gamedata.EnemyTemplates,&id);
    if ( t )
    {
        e = g_malloc(sizeof(Enemy));
        memcpy(e,t,sizeof(Enemy));
        e->cur_hp = e->max_hp;
        e->spawn_in = delay;
        e->x = sp->x*32;
        e->y = sp->y*32;
        e->direction = sp->dir;
        e->rotation = sp->dir * 90;
        Gamedata.EnemyList = g_slist_insert(Gamedata.EnemyList,e, -1);
    }
    else
    {
        printf("Could not spawn undefined enemy.\n");
    }
}

void EnemyFreeIfDead(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*)data;
    if ( e->cur_hp <= 0 ) {
        Gamedata.EnemyList = g_slist_remove(Gamedata.EnemyList,e);
        free(e);
    }
}

void EnemyFreeDead(void)
{
    g_slist_foreach(Gamedata.EnemyList,EnemyFreeIfDead,NULL);
}

void EnemyFree(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*)data;
    Gamedata.EnemyList = g_slist_remove(Gamedata.EnemyList,e);
    free(e);
}

void EnemyFreeAll(void)
{
    g_slist_foreach(Gamedata.EnemyList,EnemyFree,NULL);
}

void EnemyLoseHP(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*) data;
    // eh wat?
}

void EnemyCheckSquare(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*)user_data;
    EndPosition *ep = (EndPosition*)data;
    if ( ep->x == e->x / 32 && ep->y == e->y / 32 )
    {
        e->cur_hp = 0;
        MessageAdd("Enemy killed you.");
    }
}

void EnemyMove(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*)data;
    if ( e->spawn_in > 0 )
    {
        e->spawn_in--;
    }
    else
    {   
        e->moved++;
        if ( e->moved % 15 == 0 ) {
            e->frame++;
            if ( e->frame == e->tex->frames ) e->frame = 0;
        }
        if ( e->rotation != e->direction*90 )
        {
            e->rotation += e->rotdir*5;
            if ( e->rotation == 360 ) e->rotation = 0;
            if ( e->rotation < 0 ) e->rotation += 360;
        }
        e->progress += e->speed;
        if ( e->progress > 100 )
        {
            if ( e->y % 32 == 0 && e->x % 32 == 0 )
                g_slist_foreach(Level.ep,EnemyCheckSquare,e);
            int x = e->x / 32 - 1;
            int y = e->y / 32 - 1;
            switch(e->direction)
            {
                case DIR_N:
                    e->y--;
                    y = e->y / 32 - 1;
                    if ( e->y % 32 == 0 && Level.map[(y-1)*Level.w+x] == 0 && y > 0 )
                    {
                        if ( Level.map[y*Level.w+(x-1)] == 1 )
                        {
                            e->rotdir = -1;
                            e->direction = DIR_W;
                        }
                        else
                        {
                            e->rotdir = 1;
                            e->direction = DIR_E;
                        }
                    }
                    break;
                case DIR_E:
                    e->x++;
                    x = e->x / 32 - 1;
                    if ( e->x % 32 == 0 && Level.map[y*Level.w+(x+1)] == 0 && x < Level.w-1)
                    {
                        if ( Level.map[(y-1)*Level.w+x] == 1 )
                        {
                            e->rotdir = -1;
                            e->direction = DIR_N;
                        }
                        else
                        {
                            e->rotdir = 1;
                            e->direction = DIR_S;
                        }
                    }
                    break;
                case DIR_S:
                    e->y++;
                    y = e->y / 32 - 1;
                    if ( e->y % 32 == 0 && Level.map[(y+1)*Level.w+x] == 0 && y < Level.h-1)
                    {
                        if ( Level.map[y*Level.w+(x+1)] == 1 )
                        {
                            e->rotdir = -1;
                            e->direction = DIR_E;
                        }
                        else
                        {
                            e->rotdir = 1;
                            e->direction = DIR_W;
                        }
                    }
                    break;
                case DIR_W:
                    e->x--;
                    x = e->x / 32 - 1;
                    if ( e->x % 32 == 0 && Level.map[y*Level.w+(x-1)] == 0 && x > 0)
                    {
                        if ( Level.map[(y+1)*Level.w+x] == 1 )
                        {
                            e->rotdir = -1;
                            e->direction = DIR_S;
                        }
                        else
                        {
                            e->rotdir = 1;
                            e->direction = DIR_N;
                        }
                    }
                    break;
            }
            e->progress -= 100;
        }
        return;
    }
    if ( e->spawn_in == 0 )
    MessageAdd("Spawning enemy !");
}

void EnemySpawn(Wave *w)
{
    int i,interval = 0;
    StartPosition *sp;
    MessageAdd("Wave spawning!");
    for (i=0;i<w->enemies;i++)
    {
        if ( w->sp[i] > 0 )
            sp = g_hash_table_lookup(Level.st,&(w->sp[i]));
        interval += w->intervals[i];
        EnemyAdd(w->types[i],interval,sp);
    }
    return;
}

void EnemyPrint(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*)data;
    printf("The enemy has %d in hp - spawning in %d!\n",e->cur_hp, e->spawn_in);
}

void EnemyTemplateAdd(int id,Enemy *e)
{
    gint *eid = g_malloc(sizeof(gint));
    Enemy *et = g_malloc(sizeof(Enemy));
    memcpy(et,e,sizeof(Enemy));
    *eid = id;
    g_hash_table_insert(Gamedata.EnemyTemplates,eid,et);
}

void EnemyTemplatePrint(gpointer key, gpointer value, gpointer user_data)
{
    gint *id = (gint*)key;
    Enemy *e = (Enemy*)value;
    printf("Template ID#%d\n",*id);
    printf("\tMax HP: %d\n\tSpeed: %d\n\tName: %s\n",e->max_hp,e->speed,e->name);
}

void WaveAdd(Wave *w)
{
    Gamedata.WaveList = g_slist_insert(Gamedata.WaveList,w,-1);
}

void WavePrint(gpointer data, gpointer user_data)
{
    Wave *w = (Wave*)data;
    printf("Wave starts in %d ticks.\n",w->start);
}

void WaveMove(gpointer data, gpointer user_data)
{
    Wave *w = (Wave*)data;
    if ( w->start == 0 && w->blowup > 20 ) return;
    if ( w->start > 0 ) w->start--;
    else {
        w->blowup++;
        return;
    }
    if ( w->start == 0 ) EnemySpawn(w);
}

void StartPositionPrint(gpointer key, gpointer value, gpointer user_data)
{
    StartPosition *sp = (StartPosition*)value;
    gint *k = (gint*)key;
    printf("Key: %d, x:%d y:%d dir:%d\n",*k,sp->x,sp->y,sp->dir);
}

void TowerLocatedAt(gpointer data, gpointer user_data)
{
    gint *loc = (gint*)user_data;
    Tower *t = (Tower*)data;
    if ( t->x == loc[0] && t->y == loc[1] ) loc[2] = 1;
}

void TowerMove(gpointer data, gpointer user_data)
{
    Tower *t = (Tower*)data;
    t->rotation++;
    // TODO: We want to figure out which enemy we should aim for, and then
    //       change t->rotation to the appropriate angle.
}

void TowerAdd(int id, int x, int y)
{
    if ( x > Level.w || x < 0 || y > Level.h || y < 0 ) return;

    gint loc[] = { x,y,0 };
    g_slist_foreach(Gamedata.TowerList,TowerLocatedAt,&loc);
    if ( loc[2] == 1 )
    {
        MessageAdd("You can't build a tower on top of another tower!");
        return;
    }

    Tower *t = g_hash_table_lookup(Gamedata.TowerTemplates,&id);
    if ( t ) 
    {
        if ( Level.map[(x-1)+(y-1)*Level.w] != 0 ) {
            MessageAdd("You can only build towers on grass.");
            return;
        }
        // TODO: Check if we have enough money to buy the tower.
        Tower *nt = g_malloc(sizeof(Tower));
        memcpy(nt,t,sizeof(Tower));
        nt->x = x;
        nt->y = y;
        Gamedata.TowerList = g_slist_insert(Gamedata.TowerList,nt,-1);
    }
    else
    {
        MessageAdd("Could not build tower (tower not found in hash table - this is a bug!)");
    }
}

void TowerTemplateAdd(int id,Tower *t)
{
    gint *tid = g_malloc(sizeof(gint));
    Tower *tt = g_malloc(sizeof(Tower));
    memcpy(tt,t,sizeof(Tower));
    *tid = id;
    g_hash_table_insert(Gamedata.TowerTemplates,tid,tt);
}
void TowerTemplatePrint(gpointer key, gpointer value, gpointer user_data)
{
    Tower *t = (Tower*)value;
    printf("Tower:\n\tname\t: %s\n\treload\t: %d\n",t->name,t->reloadtime);
}

void GameNew(void)
{
    Gamedata.TowerList = NULL;
    Gamedata.EnemyTemplates = g_hash_table_new(g_int_hash,g_int_equal);
    Gamedata.TowerTemplates = g_hash_table_new(g_int_hash,g_int_equal);
    Level.st = g_hash_table_new(g_int_hash,g_int_equal);
    // TODO: Validate if the load was a success.
    LevelLoad("original.lvl");
    Gamedata.GameStepN = 0;
//    EnemyAdd(1);
//    EnemyAdd(42);
//    EnemyAdd(52);
//    g_slist_foreach(Gamedata.EnemyList,EnemyPrint,NULL);
    g_hash_table_foreach(Gamedata.EnemyTemplates,EnemyTemplatePrint,NULL);
    g_slist_foreach(Gamedata.WaveList,WavePrint,NULL);
    g_hash_table_foreach(Level.st,StartPositionPrint,NULL);
    g_hash_table_foreach(Gamedata.TowerTemplates,TowerTemplatePrint,NULL);
}

void GameStep(void)
{
    if ( Gamedata.GameStepN == 0 ) GameNew();
    Gamedata.GameStepN++;
    g_slist_foreach(Gamedata.EnemyList,EnemyMove,NULL);
    g_slist_foreach(Gamedata.TowerList,TowerMove,NULL);
    g_slist_foreach(Gamedata.WaveList,WaveMove,NULL);
    g_slist_foreach(Gamedata.TextList,MessageDo,NULL);
    EnemyFreeDead();
}
