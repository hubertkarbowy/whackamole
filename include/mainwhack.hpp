#ifndef __WHACKAMOLE_TOPLEVEL__
#define __WHACKAMOLE_TOPLEVEL__

#define NUM_HOLES 8
#define MISS -10
#define IDLE_BUT_WRONG -10
#define PENALTY -50
#define REWARD 15 // moze byc zbyt mala rozdzielczosc dla typu char


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
// #define COMPILE_FOR_ARDUINO 1
//
// There is no need to set these defines manually if you use the provided Makefile (and platforms.local.txt for Arduino) - the relevant -D parameters
// will be added to compiler flags for you by the recipe.
//

#endif
