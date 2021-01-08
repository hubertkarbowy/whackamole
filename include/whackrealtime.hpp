#ifndef __WHACKREALTIME__
#define __WHACKREALTIME__

#include <mainwhack.hpp>
#include <atomic>

// For PC we use condition variables to sync threads.
//
// On Arduino we use EventFlags as this is the only
// synchronization primitive in mBed OS that supports
// setting from an ISR context. Condition variables -
// unfortunately - cannot be accessed from an ISR.

    extern unsigned short int TRUE_BOARD_STATE;
    extern unsigned short int OBSERVED_BOARD_STATE;
    extern std::atomic<bool> is_whacked;
    extern std::atomic<unsigned char> agent_whacked_hole;
    extern std::atomic<bool> supporting_threads_active;
#ifdef COMPILE_FOR_PC
    #define CAMERA_SCAN_INTERVAL 1
    #include <random>
    #include <thread>
    #include <mutex>
    #include <condition_variable>
    extern std::random_device r;
    extern std::default_random_engine rand_engine;
    extern std::mutex true_board_mutex, agent_play_mutex, camera_mutex;
    extern std::condition_variable cv, cv_agent_play, cv_camera;
#else
#ifdef COMPILE_FOR_DUINO
    #define BOARD_WHACKED_FLAG (1UL << 0)
    #define CAMERA_SCAN_INTERVAL 10
    #include <mbed.h>
    // extern rtos::Semaphore true_board_semaphore;
    // extern mbed::Ticker board_ticker;
    extern rtos::EventFlags cv_flags;
#endif
#endif

enum noarch_rnd {RND_HOLES, RND_STATES};
enum noarch_which_thread {BOARD_THREAD, AGENT_THREAD, CAMERA_THREAD};
void setup_rtos_primitives(unsigned char num_holes);
unsigned short int random_int_noarch(enum noarch_rnd NOARCH_RND);
void sleep_for_noarch(long millis);
void notify_single_thread_noarch(enum noarch_which_thread thr);
bool wait_on_cv_noarch(enum noarch_which_thread thr, uint32_t secs, void* args=nullptr);

#endif
