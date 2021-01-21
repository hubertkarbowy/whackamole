##Whack-a-mole with Q-learning on Arduino BLE 33 Sense [WIP]

###0. Overview
This is an implementation of a classic [Whack-a-mole](https://en.wikipedia.org/wiki/Whac-A-Mole)  game with some twists. The objective is to build a simple robot (well, that's too much said) which can learn to play this game without being tols explicitly what the rules are. Even though the project was designed to run with Arduino BLE 33 Sense, there is also a command-line version that runs on a PC with a simulated camera.

In my implementation, the robot is the **agent**. It has a camera that constatly observes changes on the board (the **environment**). Depending on what it can see on the board, it decides which hole to hit (choose the next **state**).
####The game and its modifications
The board in this implementation consists of 8 "holes" (this number can be modified). However, unlike in the classical game, the moles come in <b>two varieties</b>: *good moles* and *evil moles*. The objective is, of course, to whack only the evil species and keep the good ones unhurt. This idea was borrowed from a BSc dissertation called [Robot Learning through Crowd-Based Games](https://core.ac.uk/download/pdf/212992099.pdf) by Scott Cornman and Andy Wolff.

Building a robot that actually wields a toy hammer would be quite challenging and the mechanical side is not that relevant to demonstrate Q-learning. For this reason, instead of the actual board with holes, I have a board with 8 LEDs that light up as GREEN (if it's a good mole) or AMBER (if it's an evil mole). Here it is:

![](img/board_1.jpg)   ![](img/board_2.jpg)

####Wiring diagram

###1. Implementation details
####Main components
* the agent - defined in  `whackamole_class.cpp` and its "eyes" in `tf_camera.cpp` (NOT IMPLEMENTED!)
* the board (environment) - defined in `mole_board.cpp`



* state representation
* main components

####2. Requirements
* for PC
* for Arduino

####3. Building
* for PC
* for Arduino

####4. Running
* training
* playing


####5. Todo