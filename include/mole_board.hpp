#include <mainwhack.hpp>

#ifdef COMPILE_FOR_PC
#include <thread>
#include <chrono>
#else
#ifdef COMPILE_FOR_DUINO
extern int good_latch;
extern int good_clock;
extern int good_data;
extern unsigned char good_leds;
#endif
#endif

void board_main();
