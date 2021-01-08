#include <mainwhack.hpp>

#ifdef COMPILE_FOR_PC
#include <thread>
#include <chrono>
#else
#ifdef COMPILE_FOR_DUINO

#endif
#endif

void board_main();
