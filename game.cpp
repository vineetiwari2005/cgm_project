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

// --- Game State Enums ---
enum GameState { INTRO, WAITING_FOR_SHOT, WAITING_FOR_DIVE, SHOT_IN_PROGRESS, DISPLAY_RESULT, GAME_OVER };
enum Direction { LEFT, MIDDLE, RIGHT, NONE };

// --- Global State Variables ---
GameState game_state = INTRO;
int player_goals = 0, ai_goals = 0, current_round = 1;
bool is_player_turn = true, last_shot_was_goal = false;
Direction player_shot_choice = MIDDLE, player_dive_choice = NONE;
Direction ai_shot_choice = MIDDLE, ai_dive_choice = MIDDLE;
float ball_x = GK_CENTER_X, ball_y = BALL_Y, ball_z = PENALTY_SPOT_Z;
float gk_x = GK_CENTER_X, gk_y = GROUND_Y + GK_BODY_Y_OFFSET, gk_z = GOAL_LINE_Z;
float target_ball_x = GK_CENTER_X, target_gk_x = GK_CENTER_X;
float start_ball_x = GK_CENTER_X, start_ball_z = PENALTY_SPOT_Z, start_gk_x = GK_CENTER_X;
int animation_steps = 0;
GLuint grassTextureID;
int window_width = 800, window_height = 600;

// --- Forward Declarations ---
void resetGame();
void drawText_2D(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18);
void initGraphics();
void startAnimation();
void drawPlayerFigure(float x, float y_base, float z, float r, float g, float b, bool is_goalkeeper);
void drawScene();
void drawUI();
void advanceRound();
void updateGameLogic(int value); // Renamed from animation_loop
void renderScene(); // New function for all drawing
void handleInput(unsigned char key, int x, int y); // New function for input


void initGraphics() {
    GLubyte grassPattern[2 * 2][3] = { {34, 139, 34}, {0, 100, 0}, {0, 100, 0}, {34, 139, 34} };
    glGenTextures(1, &grassTextureID); glBindTexture(GL_TEXTURE_2D, grassTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, grassPattern);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);
    GLfloat light_pos[] = { 0.0f, 5.0f, 5.0f, 1.0f }; glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glShadeModel(GL_SMOOTH);
}

/**
 * @brief Draws the static 3D scene elements. (Implementation unchanged)
 */
void drawScene() {
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, grassTextureID);
    glColor3f(0.8f, 0.8f, 0.8f); glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);   glVertex3f(-20.0f, GROUND_Y, 10.0f);
    glTexCoord2f(40.0f, 0.0f);  glVertex3f( 20.0f, GROUND_Y, 10.0f);
    glTexCoord2f(40.0f, 40.0f); glVertex3f( 20.0f, GROUND_Y, -20.0f);
    glTexCoord2f(0.0f, 40.0f);  glVertex3f(-20.0f, GROUND_Y, -20.0f);
    glEnd(); glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix(); glTranslatef(-GOAL_WIDTH / 2, GROUND_Y + GOAL_HEIGHT / 2, GOAL_LINE_Z); glScalef(POST_THICKNESS, GOAL_HEIGHT, POST_THICKNESS); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef( GOAL_WIDTH / 2, GROUND_Y + GOAL_HEIGHT / 2, GOAL_LINE_Z); glScalef(POST_THICKNESS, GOAL_HEIGHT, POST_THICKNESS); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, GROUND_Y + GOAL_HEIGHT, GOAL_LINE_Z); glScalef(GOAL_WIDTH + POST_THICKNESS, POST_THICKNESS, POST_THICKNESS); glutSolidCube(1.0); glPopMatrix();
    glColor3f(0.8f, 0.8f, 0.8f); glLineWidth(1.0); glDisable(GL_LIGHTING); glBegin(GL_LINES);
    for (float x = -GOAL_WIDTH / 2; x <= GOAL_WIDTH / 2; x += 0.4f) { glVertex3f(x, GROUND_Y, GOAL_LINE_Z); glVertex3f(x, GOAL_HEIGHT, GOAL_LINE_Z); glVertex3f(x, GROUND_Y, GOAL_LINE_Z); glVertex3f(x, GROUND_Y, GOAL_LINE_Z - NET_DEPTH); glVertex3f(x, GOAL_HEIGHT, GOAL_LINE_Z); glVertex3f(x, GOAL_HEIGHT, GOAL_LINE_Z - NET_DEPTH); }
    for (float y = GROUND_Y; y <= GOAL_HEIGHT; y += 0.4f) { glVertex3f(-GOAL_WIDTH / 2, y, GOAL_LINE_Z); glVertex3f( GOAL_WIDTH / 2, y, GOAL_LINE_Z); glVertex3f(-GOAL_WIDTH / 2, y, GOAL_LINE_Z - NET_DEPTH); glVertex3f( GOAL_WIDTH / 2, y, GOAL_LINE_Z - NET_DEPTH); }
    glVertex3f(-GOAL_WIDTH / 2, GROUND_Y, GOAL_LINE_Z); glVertex3f(-GOAL_WIDTH / 2, GROUND_Y, GOAL_LINE_Z - NET_DEPTH); glVertex3f( GOAL_WIDTH / 2, GROUND_Y, GOAL_LINE_Z); glVertex3f( GOAL_WIDTH / 2, GROUND_Y, GOAL_LINE_Z - NET_DEPTH); glVertex3f(-GOAL_WIDTH / 2, GOAL_HEIGHT, GOAL_LINE_Z); glVertex3f(-GOAL_WIDTH / 2, GOAL_HEIGHT, GOAL_LINE_Z - NET_DEPTH); glVertex3f( GOAL_WIDTH / 2, GOAL_HEIGHT, GOAL_LINE_Z); glVertex3f( GOAL_WIDTH / 2, GOAL_HEIGHT, GOAL_LINE_Z - NET_DEPTH); glVertex3f(0.0f, GOAL_HEIGHT, GOAL_LINE_Z); glVertex3f(0.0f, GOAL_HEIGHT, GOAL_LINE_Z - NET_DEPTH);
    glEnd(); glEnable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0); glLineWidth(2.0); glBegin(GL_LINES); glVertex3f(-0.1, GROUND_Y + 0.01, PENALTY_SPOT_Z); glVertex3f( 0.1, GROUND_Y + 0.01, PENALTY_SPOT_Z); glEnd(); glLineWidth(1.0);
}

/**
 * @brief Reshape callback (Implementation unchanged)
 */
void reshape(int w, int h) {
    if (h == 0) h = 1; float aspect = (float)w / (float)h; window_width = w; window_height = h;
    glViewport(0, 0, w, h); glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 100.0f); glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief Utility function to draw 2D text overlay. (Implementation unchanged)
 */
void drawText_2D(float x, float y, const char string, void font) {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, window_width, 0, window_height);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f); glRasterPos2f(x, y);
    while (*string) { glutBitmapCharacter(font, *string++); }
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

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

/**
 * @brief Handles keyboard input and related game state changes.
 */
void handleInput(unsigned char key, int x, int y) {
    key = tolower(key);
    switch (game_state) {
        case INTRO:
            if (key == ' ') game_state = WAITING_FOR_SHOT; break;
        case WAITING_FOR_SHOT:
            if (key == 'l') player_shot_choice = LEFT;
            else if (key == 'm') player_shot_choice = MIDDLE;
            else if (key == 'r') player_shot_choice = RIGHT;
            else break;
            game_state = SHOT_IN_PROGRESS; startAnimation(); break;
        case WAITING_FOR_DIVE:
            if (key == 'l') player_dive_choice = LEFT;
            else if (key == 'm') player_dive_choice = MIDDLE;
            else if (key == 'r') player_dive_choice = RIGHT;
            else break;
            game_state = SHOT_IN_PROGRESS; startAnimation(); break;
        case SHOT_IN_PROGRESS: break; // Input ignored during animation
        case DISPLAY_RESULT:
            if (key == ' ') advanceRound(); break; // advanceRound changes state
        case GAME_OVER:
            if (key == ' ' || key == 13) { resetGame(); game_state = WAITING_FOR_SHOT; } break;
    }
    glutPostRedisplay(); // Request redraw if state might have changed visually

}

/**
 * @brief Draws a more articulated player figure. (Implementation unchanged)
 */
void drawPlayerFigure(float x, float y_base, float z, float r, float g, float b, bool is_goalkeeper) {
    glColor3f(r, g, b); float head_radius = 0.15f; float torso_height = PLAYER_HEIGHT * 0.4f; float limb_length = PLAYER_HEIGHT * 0.3f; float torso_width = PLAYER_BODY_WIDTH; float torso_depth = PLAYER_BODY_DEPTH;
    glPushMatrix(); glTranslatef(x, y_base + torso_height / 2.0f, z); glScalef(torso_width, torso_height, torso_depth); glutSolidCube(1.0); glPopMatrix();
    glColor3f(0.9f, 0.7f, 0.6f); glPushMatrix(); glTranslatef(x, y_base + torso_height + head_radius * 0.8f, z); glutSolidSphere(head_radius, 16, 16); glPopMatrix();
    glColor3f(r, g, b); glPushMatrix(); glTranslatef(x + torso_width/2.0f + limb_length/2.0f, y_base + torso_height * 0.7f, z); glScalef(limb_length, PLAYER_LIMB_THICKNESS, PLAYER_LIMB_THICKNESS); glutSolidCube(1.0); glPopMatrix(); glPushMatrix(); glTranslatef(x - torso_width/2.0f - limb_length/2.0f, y_base + torso_height * 0.7f, z); glScalef(limb_length, PLAYER_LIMB_THICKNESS, PLAYER_LIMB_THICKNESS); glutSolidCube(1.0); glPopMatrix();
    glColor3f(r * 0.5f, g * 0.5f, b * 0.5f); glPushMatrix(); glTranslatef(x + torso_width * 0.2f, y_base + limb_length / 2.0f, z); glScalef(PLAYER_LIMB_THICKNESS, limb_length, PLAYER_LIMB_THICKNESS); glutSolidCube(1.0); glPopMatrix(); glPushMatrix(); glTranslatef(x - torso_width * 0.2f, y_base + limb_length / 2.0f, z); glScalef(PLAYER_LIMB_THICKNESS, limb_length, PLAYER_LIMB_THICKNESS); glutSolidCube(1.0); glPopMatrix();
}
/**
 * @brief Helper function to set targets and start animation timer. (Implementation unchanged)
 */
void startAnimation()
{
    if (animation_steps != 0)
        return;
    animation_steps = 1;
    if (is_player_turn)
    {
        ai_dive_choice = (Direction)(rand() % 3);
        if (player_shot_choice == LEFT)
            target_ball_x = GK_LEFT_X;
        else if (player_shot_choice == RIGHT)
            target_ball_x = GK_RIGHT_X;
        else
            target_ball_x = GK_CENTER_X;
        if (ai_dive_choice == LEFT)
            target_gk_x = GK_LEFT_X;
        else if (ai_dive_choice == RIGHT)
            target_gk_x = GK_RIGHT_X;
        else
            target_gk_x = GK_CENTER_X;
    }
    else
    {
        ai_shot_choice = (Direction)(rand() % 3);
        if (ai_shot_choice == LEFT)
            target_ball_x = GK_LEFT_X;
        else if (ai_shot_choice == RIGHT)
            target_ball_x = GK_RIGHT_X;
        else
            target_ball_x = GK_CENTER_X;
        if (player_dive_choice == LEFT)
            target_gk_x = GK_LEFT_X;
        else if (player_dive_choice == RIGHT)
            target_gk_x = GK_RIGHT_X;
        else
            target_gk_x = GK_CENTER_X;
    }
    start_ball_x = ball_x;
    start_ball_z = ball_z;
    start_gk_x = gk_x;
    glutTimerFunc(ANIMATION_TIMER_MS, updateGameLogic, 0); // Call the logic update function
}

/**
 * @brief Draws the scoreboard and UI text. (Implementation unchanged)
 */
void drawUI() {
    if (game_state != INTRO) { std::stringstream ss_round; ss_round << "Round: " << current_round; if (current_round > MAX_ROUNDS) ss_round << " (SUDDEN DEATH)"; else ss_round << " of " << MAX_ROUNDS; drawText_2D(20, window_height - 30, ss_round.str().c_str(), GLUT_BITMAP_HELVETICA_18); std::stringstream ss_score; ss_score << "Player: " << player_goals << "  |  AI: " << ai_goals; drawText_2D(window_width - 180, window_height - 30, ss_score.str().c_str(), GLUT_BITMAP_HELVETICA_18); }
    float center_x = window_width / 2.0f; float bottom_y = 50;
    switch (game_state) { case INTRO: drawText_2D(center_x - 100, center_x - 50, "PENALTY SHOOTOUT 3D", GLUT_BITMAP_TIMES_ROMAN_24); drawText_2D(center_x - 110, center_x - 80, "Press SPACE to Start", GLUT_BITMAP_HELVETICA_18); break; case WAITING_FOR_SHOT: drawText_2D(center_x - 50, window_height - 60, "PLAYER KICKS", GLUT_BITMAP_HELVETICA_18); drawText_2D(center_x - 180, bottom_y, "Shoot Directly: [L] Left, [M] Middle, [R] Right", GLUT_BITMAP_HELVETICA_18); break; case WAITING_FOR_DIVE: drawText_2D(center_x - 30, window_height - 60, "AI KICKS", GLUT_BITMAP_HELVETICA_18); drawText_2D(center_x - 180, bottom_y, "Dive Directly: [L] Left, [M] Middle, [R] Right", GLUT_BITMAP_HELVETICA_18); break; case SHOT_IN_PROGRESS: drawText_2D(center_x - 10, bottom_y, "...", GLUT_BITMAP_HELVETICA_18); break; case DISPLAY_RESULT: if (last_shot_was_goal) drawText_2D(center_x - 30, center_x, "GOAL!", GLUT_BITMAP_TIMES_ROMAN_24); else drawText_2D(center_x - 30, center_x, "SAVED!", GLUT_BITMAP_TIMES_ROMAN_24); drawText_2D(center_x - 100, bottom_y, "Press SPACE to continue", GLUT_BITMAP_HELVETICA_18); break; case GAME_OVER: drawText_2D(center_x - 70, center_x + 50, "--- GAME OVER ---", GLUT_BITMAP_TIMES_ROMAN_24); std::stringstream ss_final; ss_final << "Final Score: Player " << player_goals << " - " << ai_goals << " AI"; drawText_2D(center_x - 120, center_x + 20, ss_final.str().c_str(), GLUT_BITMAP_HELVETICA_18); if (player_goals > ai_goals) drawText_2D(center_x - 120, center_x - 10, "WORLD CLASS PERFORMANCE!", GLUT_BITMAP_HELVETICA_18); else if (ai_goals > player_goals) drawText_2D(center_x - 70, center_x - 10, "NEEDS PRACTICE!", GLUT_BITMAP_HELVETICA_18); else drawText_2D(center_x - 70, center_x - 10, "A TIE!", GLUT_BITMAP_HELVETICA_18); drawText_2D(center_x - 140, bottom_y, "Press SPACE or ENTER to play again", GLUT_BITMAP_HELVETICA_18); break; }
}


/**
 * @brief Advances the game state. (Implementation unchanged)
 */
void advanceRound()
{
    ball_x = GK_CENTER_X;
    ball_y = BALL_Y;
    ball_z = PENALTY_SPOT_Z;
    gk_x = GK_CENTER_X;
    gk_z = GOAL_LINE_Z;
    gk_y = GROUND_Y + GK_BODY_Y_OFFSET;
    animation_steps = 0;
    player_dive_choice = NONE;
    if (is_player_turn)
    {
        is_player_turn = false;
        game_state = WAITING_FOR_DIVE;
    }
    else
    {
        is_player_turn = true;
        if (current_round >= MAX_ROUNDS)
        {
            if (player_goals != ai_goals)
                game_state = GAME_OVER;
            else
            {
                current_round++;
                game_state = WAITING_FOR_SHOT;
            }
        }
        else
        {
            current_round++;
            game_state = WAITING_FOR_SHOT;
        }
    }

}
/**
 * @brief Resets game state variables. (Implementation unchanged)
 */
void resetGame()
{
    player_goals = 0;
    ai_goals = 0;
    current_round = 1;
    is_player_turn = true;
    game_state = INTRO;
    player_dive_choice = NONE;
    player_shot_choice = MIDDLE;
    ball_x = GK_CENTER_X;
    ball_y = BALL_Y;
    ball_z = PENALTY_SPOT_Z;
    gk_x = GK_CENTER_X;
    gk_y = GROUND_Y + GK_BODY_Y_OFFSET;
    gk_z = GOAL_LINE_Z;
    animation_steps = 0;
}

/**
 * @brief Handles time-based updates, primarily animation.
 */
void updateGameLogic(int value) {
    if (game_state != SHOT_IN_PROGRESS || animation_steps == 0) return;

    if (animation_steps <= TOTAL_ANIMATION_STEPS) {
        float t = (float)animation_steps / (float)TOTAL_ANIMATION_STEPS;
        float t_ball = t * t; float t_gk = t;
        ball_x = (1.0f - t_ball) * start_ball_x + t_ball * target_ball_x;
        ball_z = (1.0f - t_ball) * start_ball_z + t_ball * GOAL_LINE_Z;
        ball_y = BALL_Y;
        gk_x = (1.0f - t_gk) * start_gk_x + t_gk * target_gk_x;
        gk_y = GROUND_Y + GK_BODY_Y_OFFSET;
        animation_steps++;
        glutTimerFunc(ANIMATION_TIMER_MS, updateGameLogic, 0); // Continue loop
    } else {
        // Animation finished: Determine result and change state
        game_state = DISPLAY_RESULT;
        ball_x = target_ball_x; ball_z = GOAL_LINE_Z; gk_x = target_gk_x;
        if (is_player_turn) { last_shot_was_goal = (player_shot_choice != ai_dive_choice); if (last_shot_was_goal) player_goals++; }
        else { last_shot_was_goal = (ai_shot_choice != player_dive_choice); if (last_shot_was_goal) ai_goals++; }
    }
    glutPostRedisplay(); // Request redraw after state change
}

// --- Main Function and GLUT Setup ---

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Penalty Shootout 3D - Refactored");

    srand(static_cast<unsigned int>(time(NULL)));

    initGraphics();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.2f, 0.4f, 1.0f); // Sky blue

    // Register callbacks to the new primary functions
    glutDisplayFunc(renderScene); // Drawing function
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleInput); // Input function
    // The timer is started initially by handleInput calling startAnimation,
    // which then calls updateGameLogic recursively via glutTimerFunc.

    resetGame();
    glutMainLoop();
    return 0;
}