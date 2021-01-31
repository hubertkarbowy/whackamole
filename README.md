## Whack-a-mole with Q-learning on Arduino BLE 33 Sense [WIP]

### 0. Overview
This is an implementation of a classic [Whack-a-mole](https://en.wikipedia.org/wiki/Whac-A-Mole)  game with some twists. The objective is to build a simple robot (well, that's too much said) which can learn to play this game without being tols explicitly what the rules are. Even though the project was designed to run with Arduino BLE 33 Sense, there is also a command-line version that runs on a PC with a simulated camera.

In my implementation, the robot is the **agent**. It has a camera that constatly observes changes on the board (the **environment**). Depending on what it can see on the board, it decides which hole to hit (choose the next **state**).
#### The game and its modifications
The board in this implementation consists of 8 "holes" (this number can be modified). However, unlike in the classical game, the moles come in <b>two varieties</b>: *good moles* and *evil moles*. The objective is, of course, to whack only the evil species and keep the good ones unhurt. This idea was borrowed from a BSc dissertation called [Robot Learning through Crowd-Based Games](https://core.ac.uk/download/pdf/212992099.pdf) by Scott Cornman and Andy Wolff.

Building a robot that actually wields a toy hammer would be quite challenging and the mechanical side is not that relevant to demonstrate Q-learning. For this reason, instead of the actual board with holes, I have a board with 8 LEDs that light up as GREEN (if it's a good mole) or AMBER (if it's an evil mole). Here it is:

![](img/board_1.jpg)   ![](img/board_2.jpg)

#### Wiring diagram

### 1. Implementation details
#### Main components
* *the agent* - defined in  [`whackamole_class.cpp`](src/whackamole_class.cpp) and its "eyes" in `tf_camera.cpp` (NOT IMPLEMENTED!). The agent can operate in two modes:
    * ** learning** - in this mode you can make repeated calls to the `learn_step` method in order to build the Q-matrix of state/reward transitions. Note that you can save and load the Q-matrix using `serialize` and `deserialize` methods (so far implemented only for PC - for Arduino you should add a board-dependent implementation that can read from/write to your NVRAM or EEPROM)
    * ** playing**  - in the playing mode you need to call the `play` method once on the current state of the environment

    In both modes, the agent acts (learns / plays) only when it detects a change in the environment. This is done by the agent waiting on a condition variable set by its "eyes", that is the camera. The camera takes pictures every 0.5 seconds, runs TensorFlow lite inference on the picture, writes the recognized state into the `OBSERVED_BOARD_STATE` global variable and finally notifies the agent (and possibly all other threads) about the change by setting the condition variable.
    
    After performing the whack, the agent writes the hole number to the `agent_whacked_hole` global atomic variable and notifies the board via condition variable. This notification simulates the physical contact between the hammer and the board in the real world.

* *the board (environment)* - defined in `mole_board.cpp`<br/>
	The board contains current state and two core functions: `permute` - which randomly shows and hides the moles every 3 seconds if the agent is idle, and `whack` - which hides the moles chosen by the agent. Additionally, there are some functions for controlling the LEDs on Arduino. 

* *state representation*<br/>
    In this implementation there are **8 holes**. For each hole, there are three options: **0** - the hole is empty, **1** - there is a "good" mole (green LED on Arduino), **2** - there is a "bad" mole (amber LED on Arduino). Therefore, there are 3^8 = 6561 possible states of our environment. A number of the `unsigned short` type in the range between 0 and 6561 uniquely identifies the current state. If you convert this number to (an eight-digit) base-3, you will know how each hole looks like (most significant digit = hole 0 least significant digit = hole 7).
    
    Example: state 6001 is 22020021 in base-3. The most significant digit is 2, so there is an evil mole in the zeroeth hole. The next most significant digit is also 1, so there is also an evil mole in hole number one. Hole number two is empty, because the next digit is 0. And so on until we reach the least significant digit, which is 1, so there is a good mole in the seventh hole.
    The `helpers.cpp` file contains methods which convert between unsigned short int and array of `char` types (`to_base3_buf` and `base3_to_int`).
    
* main components

####2. Requirements
* for PC
* for Arduino

####3. Building
* for PC
* for Arduino
* generating documentation

####4. Running
* training
* playing


####5. Todo