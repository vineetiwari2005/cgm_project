## Penalty Shootout 3D (OpenGL/GLUT)

A small 3D penalty shootout game written in C++ using classic OpenGL and GLUT. You take turns with a simple AI to shoot and save penalties across multiple rounds, with sudden death if it’s tied after regulation.

This project renders a minimal 3D pitch with a goal, net, textured grass, a ball, and two players (kicker and goalkeeper) with basic animation and HUD text.

---

## Features

- 3D scene: goalposts, crossbar, net lines, and textured grass
- Simple player figures for the kicker and goalkeeper
- Turn-based shootout: player shoots, then defends vs. AI
- Basic AI picks a random dive/shot direction
- Scoreboard HUD, per-round status, and game over screen
- Smooth, time-based animation for ball and goalkeeper movement

---

## Project structure

- game.cpp — The entire game (rendering, input, state machine, and animation)
- README.md — This documentation

No external assets are required; the grass texture is generated procedurally at runtime.

---

## Controls

- SPACE — Start the game, advance between rounds, restart after game over
- While shooting (PLAYER KICKS):
	- L — Shoot left
	- M — Shoot middle
	- R — Shoot right
- While defending (AI KICKS):
	- L — Dive left
	- M — Stay middle
	- R — Dive right
- ENTER — Also restarts from game over

Tip: Input is context-sensitive. The prompt at the bottom of the window shows which keys are active in the current phase.

---

## Build and run

This is a straightforward GLUT application. The include in the source uses macOS’ header path (<GLUT/glut.h>). If you’re on Linux/Windows, see the platform notes below.

### macOS (Apple Clang + system GLUT)

Requirements:
- Xcode Command Line Tools (for clang and headers)

Build and run from the project folder:

zsh
g++ game.cpp -o penalty \
	-std=c++11 \
	-framework OpenGL -framework GLUT

./penalty


Notes:
- OpenGL/GLUT are deprecated on macOS but still available. You may see deprecation warnings; they’re safe to ignore for this project.

### Linux (GCC + freeglut)

Requirements (Ubuntu/Debian):

zsh
sudo apt update
sudo apt install -y build-essential freeglut3-dev mesa-utils mesa-common-dev


Two options for headers:
- Keep the code as-is and add a symlink or package that exposes <GLUT/glut.h>, or
- Change the include in game.cpp to #include <GL/glut.h> (common on Linux)

Build and run (typical flags):

zsh
g++ game.cpp -o penalty -std=c++11 -lglut -lGL -lGLU
./penalty


### Windows

You can use either:
- Visual Studio + vcpkg or NuGet packages for freeglut, or
- MSYS2/MinGW with mingw-w64-x86_64-freeglut

Link against freeglut, opengl32, and glu32, and include the correct header path (often <GL/glut.h>). Exact setup varies by environment.

---

## How it works (high level)

- Game state machine (GameState):
	- INTRO → WAITING_FOR_SHOT → SHOT_IN_PROGRESS → DISPLAY_RESULT → WAITING_FOR_DIVE … → GAME_OVER
- Directions (Direction): LEFT, MIDDLE, RIGHT, NONE
- Core callbacks (GLUT):
	- renderScene — Draws the world, players, ball, and HUD
	- reshape — Perspective and viewport updates
	- handleInput — Keyboard input, advances states, triggers animations
	- updateGameLogic — Timer-driven animation updates for ball and goalkeeper
- Animation: Eases ball toward its target and moves the goalkeeper to the chosen side over a fixed duration (ANIMATION_DURATION_MS at ANIMATION_FPS).
- Scoring: After the animation, if shooter’s direction ≠ goalkeeper’s dive, it’s a goal; otherwise it’s a save.
- Rounds: Best-of-MAX_ROUNDS with sudden death if tied.

---

## Tuning and customization

Open game.cpp and tweak these constants near the top to change gameplay:

- MAX_ROUNDS — Number of regulation rounds (default 5)
- ANIMATION_FPS, ANIMATION_DURATION_MS — Animation smoothness/speed
- GOAL_WIDTH, GOAL_HEIGHT, NET_DEPTH — Goal dimensions
- GK_LEFT_X, GK_RIGHT_X — How far the keeper can dive
- BALL_RADIUS, PLAYER_HEIGHT, etc. — Scene scale

---

## Troubleshooting

- Header not found: GLUT/glut.h
	- macOS: ensure Xcode Command Line Tools are installed
	- Linux: install freeglut3-dev and consider switching the include to <GL/glut.h>
- Linker errors for OpenGL/GLUT
	- Verify you’re using the right platform flags (see Build and run)
- Black window or no text
	- On some drivers, bitmap fonts can be small; resize the window. Ensure depth testing and lighting are enabled as in initGraphics()
- OpenGL deprecation warnings on macOS
	- Expected with legacy OpenGL; safe to ignore for learning/projects

If you hit a build issue, please share your OS, compiler version, and full command line.

---

## What’s next

Ideas to extend the project:
- Add shot power/curvature and variable ball height
- Better goalkeeper AI (predictive or probabilistic)
- Score history and per-round commentary
- Sound effects and a simple crowd
- Replace fixed-function pipeline with modern OpenGL (VBO/VAO + shaders)

---

## Acknowledgments

Built as a compact OpenGL/GLUT sample to demonstrate basic 3D rendering, input handling, and time-based animation.
