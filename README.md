# Tank Game

## Overview
Tank Game is a simple 3D game built using OpenGL, GLFW, and GLEW. The game features a hero tank that shoots bullets to destroy enemy tanks. The player wins by achieving a target score.

## Features
- **Hero Tank**: Controlled by computer, capable of shooting bullets.
- **Enemy Tanks**: Spawn periodically and move randomly.
- **Collision Detection**: Bullets can destroy enemy tanks.
- **Camera Views**: Switch between perspective and top-down views.
- **Game Logic**: Includes scoring, spawning, and game-over conditions.

## Requirements
To build and run the project, ensure the following dependencies are installed:
- **g++**: GNU Compiler Collection
- **GLFW**: OpenGL framework for window and input handling
- **GLEW**: OpenGL Extension Wrangler Library
- **OpenGL**: Graphics library for rendering

### Installing Dependencies on Linux
```bash
sudo apt update
sudo apt install g++ libglfw3-dev libglew-dev libgl-dev
```

## Build Instructions
1. Clone the repository:
   ```bash
   git clone <repository_url>
   cd compilerincpp
   ```

2. Compile the project:
   ```bash
   g++ main.cpp -o tank_game -lglfw -lGLEW -lGL
   ```

3. Run the executable:
   ```bash
   ./tank_game
   ```

## Controls
- **W/A/S/D**: Move the camera in perspective view.
- **Mouse Movement**: Rotate the camera in perspective view.
- **Scroll Wheel**: Adjust the field of view (FOV).
- **T**: Switch to top-down view.
- **P**: Switch to perspective view.
- **Escape**: Exit the game.

## Game Rules
- Destroy enemy tanks by shooting bullets.
- Achieve the target score to win.
- Avoid letting enemy tanks overwhelm the play area.


## License
This project is licensed under the MIT License. See `LICENSE` for details.

## Acknowledgments
- OpenGL for graphics rendering
- GLFW for window and input handling
- GLEW for managing OpenGL extensions
- GLM for mathematical operations

Enjoy the game!