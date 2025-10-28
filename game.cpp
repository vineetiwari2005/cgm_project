/*
 * penalty_game_3D_Refactored.cpp
 *
 * Code refactored into three main functions:
 * 1. updateGameLogic: Handles animation and time-based state.
 * 2. renderScene: Handles all drawing.
 * 3. handleInput: Handles keyboard input and input-driven state changes.
 *
 * Features: Perspective Camera, Deeper Net, Articulated Players, Direct L/M/R Controls.
 *
 * Mac Compilation (MUST USE C++11):
 * g++ -o penalty_game_3d penalty_game_3D_Refactored.cpp -std=c++11 -DGL_SILENCE_DEPRECATION -framework OpenGL -framework GLUT
 *
 * Linux Compilation:
 * g++ -o penalty_game_3d penalty_game_3D_Refactored.cpp -std=c++11 -lglut -lGLU -lGL -lm
 *
 */
