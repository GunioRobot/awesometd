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

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "video-game.h"
#include "game.h"
#include "level.h"

void VideoGameDrawLevel(void)
{
    int i;
    glPushMatrix();
    glTranslatef(LevelCamera[0],LevelCamera[1],0.0);
    for (i=1;i<=(Level.w*Level.h);++i)
    {
        GLfloat vcoords[] = {
             0.0, 0.0,
            CELL_SIZE_W, 0.0,
            CELL_SIZE_W,CELL_SIZE_H,
             0.0,CELL_SIZE_H
        };
        switch(Level.map[i-1])
        {
            case 0: glColor4d( 0.0f, 1.0f, 0.0f, 1.0f); break;
            case 1: glColor4d( 0.5f,0.25f, 0.0f, 1.0f); break;
            case 2: glColor4d( 1.0f, 0.0f, 0.0f, 1.0f); break;
            case 3: glColor4d( 0.0f, 0.0f, 1.0f, 1.0f); break;
        }
        glVertexPointer(2, GL_FLOAT, 0, vcoords);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glTranslatef(CELL_SIZE_W,0.0f,0.0f);
        if ( i % Level.w == 0 && i > 0 ) 
            glTranslatef(-CELL_SIZE_W*Level.w,CELL_SIZE_H,0.0f);
    }
    glPopMatrix();
}

void VideoGameDrawCursor(void)
{
    int mx,my;
    SDL_GetMouseState(&mx,&my);
    glPushMatrix();
    if ( mx-LevelCamera[0] >= 0 &&
         my-LevelCamera[1] >= 0 &&
         mx-LevelCamera[0] < CELL_SIZE_W*Level.w &&
         my-LevelCamera[1] < CELL_SIZE_H*Level.h )
    {
        glTranslatef(LevelCamera[0]+(((mx-LevelCamera[0])/CELL_SIZE_W)*CELL_SIZE_W),
                     LevelCamera[1]+(((my-LevelCamera[1])/CELL_SIZE_H)*CELL_SIZE_H),
                     0.0);
        GLfloat vcoords[] = {
             0.0, 0.0,
            CELL_SIZE_W, 0.0,
            CELL_SIZE_W,CELL_SIZE_H,
             0.0,CELL_SIZE_H
        };
        glColor4d(1.0f,1.0f,1.0f,0.5f);
        glVertexPointer(2, GL_FLOAT, 0, vcoords);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glPopMatrix();
}

void VideoGameDrawEnemy(gpointer data, gpointer user_data)
{
    Enemy *e = (Enemy*)data;
}

void VideoGameDrawWave(gpointer data, gpointer user_data)
{
    Wave *w = (Wave*)data;
    glPushMatrix();
    glTranslatef(w->start-Gamedata.GameStepN,0.0f,0.0f);
    GLfloat vcoords[] = {
        0.0, 0.0,
        50.0, 0.0,
        50.0, 20.0,
        0.0, 20.0
    };
    glColor4d(1.0f,1.0f,1.0f,1.0f);
    glVertexPointer(2, GL_FLOAT, 0, vcoords);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);
    glPopMatrix();
}

void VideoGameDraw(void)
{
    VideoGameDrawLevel();
    g_slist_foreach(Gamedata.EnemyList,VideoGameDrawEnemy,NULL);
    g_slist_foreach(Gamedata.WaveList,VideoGameDrawWave,NULL);
    VideoGameDrawCursor();
}
