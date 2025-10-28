#include <GLUT/glut.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>   
#include <cstdlib> 
#include <ctime>   
#include <algorithm> 

#define PI 3.14159265359

// --- Game Constants ---
const int MAX_ROUNDS = 5;
const int ANIMATION_FPS = 60;
const int ANIMATION_DURATION_MS = 500;
const int ANIMATION_TIMER_MS = 1000 / ANIMATION_FPS;
const int TOTAL_ANIMATION_STEPS = ANIMATION_DURATION_MS / ANIMATION_TIMER_MS;

// --- 3D Scene Coordinates ---
const float GOAL_LINE_Z = -8.0f;
const float PENALTY_SPOT_Z = 0.0f;
const float GROUND_Y = 0.0f;
const float BALL_RADIUS = 0.15f;
const float BALL_Y = GROUND_Y + BALL_RADIUS;
const float PLAYER_HEIGHT = 1.8f;
const float PLAYER_BODY_WIDTH = 0.4f;
const float PLAYER_BODY_DEPTH = 0.2f;
const float PLAYER_LIMB_THICKNESS = 0.12f;
const float KICKER_POS_Z = PENALTY_SPOT_Z + 0.8f;
const float GK_BODY_Y_OFFSET = 0.8f;
const float GK_CENTER_X = 0.0f;
const float GK_LEFT_X = -1.8f;
const float GK_RIGHT_X = 1.8f;
const float GOAL_WIDTH = 4.0f;
const float GOAL_HEIGHT = 2.0f;
const float POST_THICKNESS = 0.1f;
const float NET_DEPTH = 1.5f;

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(
        0.0f, 1.5f, KICKER_POS_Z + 3.0f,
        0.0f, 0.8f, GOAL_LINE_Z + 1.0f,
        0.0f, 1.0f, 0.0f
    );

    drawScene(); 

    glPushMatrix();
    glTranslatef(ball_x, ball_y, ball_z);
    glColor3f(1.0f, 1.0f, 1.0f); glutSolidSphere(BALL_RADIUS, 16, 16);
    glColor3f(0.0f, 0.0f, 0.0f); glDisable(GL_LIGHTING); glutWireSphere(BALL_RADIUS * 1.01, 10, 10); glEnable(GL_LIGHTING);
    glPopMatrix();

    if (game_state != INTRO) {
        if (is_player_turn) {
            drawPlayerFigure(GK_CENTER_X, GROUND_Y, KICKER_POS_Z, 0.0f, 0.0f, 1.0f, false); // Player Kicker
            drawPlayerFigure(gk_x, GROUND_Y, gk_z, 1.0f, 0.0f, 0.0f, true); // AI GK
        } else {
            drawPlayerFigure(GK_CENTER_X, GROUND_Y, KICKER_POS_Z, 1.0f, 0.0f, 0.0f, false); // AI Kicker
            drawPlayerFigure(gk_x, GROUND_Y, gk_z, 0.0f, 0.0f, 1.0f, true); // Player GK
        }
    }

    drawUI();

    glutSwapBuffers();
}
