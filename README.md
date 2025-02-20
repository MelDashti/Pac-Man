# Pac-Man on LandTiger Board

## Overview
This project is an implementation of **Pac-Man** for the **LandTiger LPC1768 development board**, developed as part of a **Computer Architecture course**. The game follows classic Pac-Man mechanics with added features, including AI-controlled ghosts, sound effects, and CAN bus communication for score and status display.

![image](https://github.com/user-attachments/assets/dd9c6794-53af-47e7-a7ba-45bdab5abb05)


## Features
- **Joystick-controlled Pac-Man movement**
- **AI-driven Ghost (Blinky) with pathfinding** using the **A* algorithm**
- **Dynamic power pill placement** and frightened mode for ghosts
- **Sound effects and background music** using the speaker interface
- **Game status display using CAN bus communication**
- **Pause and Resume functionality** via INT0 button
- **Victory and Game Over conditions**

## Project Specifications
### Game Mechanics
- **Pac-Man navigates a maze** filled with **240 standard pills** and **6 power pills**.
- **Ghost AI (Blinky)** actively chases Pac-Man using the **A* pathfinding algorithm**.
- **Power pills enable a frightened mode**, turning Blinky blue for 10 seconds, allowing Pac-Man to eat him for extra points.
- The **score increases** based on the number of pills collected:
  - Standard pill: **+10 points**
  - Power pill: **+50 points**
  - Eating Blinky: **+100 points**
- **Game progression**:
  - A **countdown timer** starts from 60 seconds.
  - Pac-Man wins by **eating all pills before time runs out**.
  - The game ends in "Game Over" if the timer expires before clearing the maze.

### Hardware & Software Details
- **Development Board**: LandTiger LPC1768
- **Programming Language**: C
- **IDE**: Keil µVision
- **AI Pathfinding**: A* Algorithm for ghost movement
- **Audio**: Speaker interface for background music and sound effects
- **CAN Bus Communication**:
  - Displays **current score, remaining lives, and countdown timer**
  - Uses **external loopback mode** with CAN1 (send) and CAN2 (receive)

## Installation & Setup
### Prerequisites
- **Keil µVision IDE** installed and configured for **LPC1768**
- **LandTiger LPC1768 Development Board**
- **CAN bus interface setup** (for debugging and score display)

### Steps to Run the Project
1. **Clone the repository** and open the project in **Keil µVision**.
2. **Connect the LandTiger Board** to your PC via USB.
3. **Compile the project** with the required compilation options.
4. **Flash the program** onto the board.
5. **Start the game** and control Pac-Man using the **joystick**.

## Usage Instructions
1. **Start the Game**: Press **INT0** to begin (game starts in PAUSE mode).
2. **Control Pac-Man**: Use the **joystick** to navigate.
3. **Score Points**:
   - Collect **standard pills** (+10 points) and **power pills** (+50 points).
4. **Avoid Blinky**: The red ghost chases Pac-Man using an **A* pathfinding algorithm**.
5. **Use Power Pills**:
   - Activates **frightened mode** (Blinky turns blue and flees).
   - Eat Blinky for **+100 points**.
6. **Win Condition**:
   - Eat **all pills** before time expires.
7. **Game Over Condition**:
   - Countdown timer reaches **zero**.

## License
This project is developed for educational purposes as part of a **Computer Architecture course**. Any modifications and extensions should be credited accordingly.

## Acknowledgments
Special thanks to the course instructors and colleagues for feedback and insights during development.

---

This README provides a comprehensive and professional description of the project. Feel free to update the **image path** and add any necessary details before finalizing. Let me know if you need further refinements! 🚀
