#ifndef __WHACKREALTIME__
#define __WHACKREALTIME__

#include <mainwhack.hpp>

#ifdef COMPILE_FOR_PC
    #include <random>
    #include <mutex>
    extern std::random_device r;
    extern std::default_random_engine rand_engine;
    extern std::mutex true_board_mutex;
#else
#ifdef COMPILE_FOR_DUINO
    #include <mbed.h>
    extern rtos::Semaphore true_board_semaphore;
    extern mbed::Ticker board_ticker;
#endif
#endif

enum noarch_rnd {RND_HOLES, RND_STATES};
void setup_rtos_primitives(unsigned char num_holes);
unsigned short int random_int_noarch(enum noarch_rnd NOARCH_RND);

#endif
