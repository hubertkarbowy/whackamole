#ifndef __WHACKAMOLE_TOPLEVEL__
#define __WHACKAMOLE_TOPLEVEL__

#define NUM_HOLES 8
#define MISS -10
#define IDLE_BUT_WRONG -10
#define PENALTY -50
#define REWARD 15 // using the 8-bit char type as reward range can be quite a drawback, since the values are only between -128 and 127 (and integer), but keeps our Q-matrix small enough
#define BOARD_TICKER_INTERVAL 3
#define SIMULATION_WHACK_INTERVAL_MILLIS 1000
#define MAX_HIT_ATTEMPTS 10


/** The board state, as observed by the <b>agent</b>.
 * In simulation mode, this variable is directly set by the Board thread.
 *
 * On Arduino, it is set by the Camera thread after TensorFlow gives us the classification result.
 *
*/
extern unsigned short int OBSERVED_BOARD_STATE;

/** The "true" board state.
 *
 *  The Board thread continually updates this variable which allows us to measure the agent's
 *  performance.
*/
extern unsigned short int TRUE_BOARD_STATE;

//
//
//
// IMPORTANT: Cross-compilation defines
//
// If you are importing this to some IDE and you will not use the provided Makefile, then uncomment the following lines depending on your target:
//
// - for PC simulator:
//
// #define COMPILE_FOR_PC 1
//
// - for Arduino:
//
// #define COMPILE_FOR_DUINO 1
//
// There is no need to set these defines manually if you use the provided Makefile (and platforms.local.txt for Arduino) - the relevant -D parameters
// will be added to compiler flags for you by the recipe.
//

#endif
