/**
 * @file whackrealtime.cpp
 * @brief Real-time primitives for PC and for Arduino
 *
 * This file contains architecture-independent functions mainly for waiting on a conditional variable / flag.
 */
#include <whackrealtime.hpp>

   unsigned short int TRUE_BOARD_STATE=9999;
   unsigned short int OBSERVED_BOARD_STATE=9999;
   std::atomic<unsigned char> agent_whacked_hole(NUM_HOLES-1); // nothing whacked by default
   std::atomic<bool> supporting_threads_active(true);
#ifdef COMPILE_FOR_PC
    #include <random>
    std::random_device r;
    std::default_random_engine rand_engine = std::default_random_engine(r());
    std::mutex true_board_mutex, agent_play_mutex, camera_mutex;
    std::condition_variable cv, cv_agent_play, cv_camera;
    struct whackamole_generators_cpp {
        std::uniform_int_distribution<unsigned char> rnd_hole_dist;
        std::uniform_int_distribution<unsigned short int> rnd_state_dist;
    } whackamole_generators_cpp; /** This struct contains random number generators for PC */
#else
#ifdef COMPILE_FOR_DUINO
    rtos::EventFlags cv_flags("WHACKAMOLE_FLAGS");
    struct whackamole_generators_duino {
        unsigned char num_holes;
        unsigned short int num_states;
    } whackamole_generators_duino;
#endif
#endif


/**
 * Set up random number generation
 *
 * On PC: use the std::uniform_int_distribution
 * On Arduino: use random() function
 */
void setup_rtos_primitives(unsigned char num_holes) {
    unsigned short int num_states = 1;
    for (int i=0; i<num_holes; i++) num_states *= 3;
#ifdef COMPILE_FOR_PC
    whackamole_generators_cpp.rnd_hole_dist = std::uniform_int_distribution<unsigned char>(0, num_holes);
    whackamole_generators_cpp.rnd_state_dist = std::uniform_int_distribution<unsigned short int>(0, num_states);
#else
#ifdef COMPILE_FOR_DUINO
    whackamole_generators_duino.num_holes = num_holes + 1; // +1 because Arduino's random() function generates numbers up to but not including the upper bound
    whackamole_generators_duino.num_states = num_states + 1;
#endif
#endif
}

/**
 * Generate a random number (architecture-independent)
 *
 * @param NOARCH_RND    if set to RND_HOLES, will return a random number between 0 and the number of holes. If set to RND_STATES, will return a number between 0 and the maximal state.
 */
unsigned short int random_int_noarch(enum noarch_rnd NOARCH_RND) {
#ifdef COMPILE_FOR_PC
    if (NOARCH_RND == RND_HOLES) {
        return whackamole_generators_cpp.rnd_hole_dist(rand_engine);
    }
    else if (NOARCH_RND == RND_STATES) {
        return whackamole_generators_cpp.rnd_state_dist(rand_engine);
    }
    else {
        return 0; // todo: raise an exception
    }
#else
#ifdef COMPILE_FOR_DUINO
    long randomNumber;
    if (NOARCH_RND == RND_HOLES) {
        randomNumber = random(0, whackamole_generators_duino.num_holes);
        return (unsigned short int) randomNumber;
    }
    else if (NOARCH_RND == RND_STATES) {
        randomNumber = random(0, whackamole_generators_duino.num_states);
        return (unsigned short int) randomNumber;
    }
    else {
        return 0; // todo: raise an exception
    }
#endif
#endif
}

/**
 * Sleep (architecture-independent)
 *
 * PC: Uses std::this_thread::sleep_for
 * Arduino: Uses rtos::ThisThread::sleep_for
 */
void sleep_for_noarch(long millis) {
#ifdef COMPILE_FOR_PC
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
#else
#ifdef COMPILE_FOR_DUINO
    rtos::ThisThread::sleep_for(millis);
#endif
#endif
}

/**
 * Broadcasts state change across the system
 *
 * FIXME!!! This function has a confusing name and should be called `notify_all_threads_noarch` !!!
 *
 * Notifies all threads waiting on a given conditional variable (PC) or sets an event flag (Arduino).
 * This function requires a thread that calls it to identify the conditional variable/event flag
 * it wishes to set via the #thr enum. For example:
 *   - the agent whacks the board, so it has to call this function with #thr set to BOARD_THREAD since the board class waits on the cv_board conditional variable.
 *   - the camera captures an image, so it calls this function with #thr set to CAMERA_THREAD since the agent class waits on the cv_camera conditional variable.
 *   - etc.
 */
void notify_single_thread_noarch(enum noarch_which_thread thr) {
    #ifdef COMPILE_FOR_PC
    switch (thr) {
        case BOARD_THREAD:
            {
                std::unique_lock<std::mutex> lck(true_board_mutex);
                cv.notify_all();
                break;
            }
        case AGENT_THREAD:
            {
                std::unique_lock<std::mutex> lck(agent_play_mutex);
                cv_agent_play.notify_all();
                break;
            }
        case CAMERA_THREAD:
            {
                std::unique_lock<std::mutex> lck(camera_mutex);
                cv_camera.notify_all();
                break;
            }
        default: {}
    }
    #else
    #ifdef COMPILE_FOR_DUINO
    switch (thr) {
        case BOARD_THREAD:
            cv_flags.set(BOARD_FLAG);
            break;
        case AGENT_THREAD:
            cv_flags.set(AGENT_FLAG);
            break;
        case CAMERA_THREAD:
            cv_flags.set(CAMERA_FLAG);
            break;
        default: {}
    }
    #endif
    #endif
}

/**
 * Wait on a conditional variable / event flag (architecture-independent)
 *
 * The thread calling this function has to identify the conditional variable (PC) / event flag (Arduino)
 * it will wait on via the #thr enum. 
 *
 * @param thr           Which conditional variable / event flag to wait on (board, agent or camera)
 * @param timeout_secs  How many seconds to wait before giving up. Setting this to 0 means waiting indefinitely.
 * @param args          Optional arguments (currently not used)
 * @returns             If timeout is 0: always returns false. Otherwise returns true if notification was received before the timer elapsed, or false in case of a timeout.
 */

bool wait_on_cv_noarch(enum noarch_which_thread thr, uint32_t timeout_secs, void* args) {
     bool ret = false;
     #ifdef COMPILE_FOR_PC
     switch (thr) {
        case BOARD_THREAD:
            {
                std::unique_lock<std::mutex> lck(true_board_mutex);
                if (timeout_secs != 0) {
                    std::cv_status status = cv.wait_for(lck, std::chrono::seconds(timeout_secs));
                    if (status == std::cv_status::timeout) ret = false;
                    else ret = true;
                }
                else {
                    cv.wait(lck);
                }
                break;
            }
        case AGENT_THREAD:
            {
                std::unique_lock<std::mutex> lck(agent_play_mutex);
                if (timeout_secs != 0) {
                    std::cv_status status = cv_agent_play.wait_for(lck, std::chrono::seconds(timeout_secs));
                    if (status == std::cv_status::timeout) ret = false;
                    else ret = true;
                }
                else {
                    cv_agent_play.wait(lck);
                }
                break;
            }
        case CAMERA_THREAD: 
            {
                std::unique_lock<std::mutex> lck(camera_mutex);
                if (timeout_secs != 0) {
                    std::cv_status status = cv_camera.wait_for(lck, std::chrono::seconds(timeout_secs));
                    if (status == std::cv_status::timeout) ret = false;
                    else ret = true;
                }
                else {
                    cv_camera.wait(lck);
                }
                break;
            }
        default: {}
    }
    #else
    #ifdef COMPILE_FOR_DUINO
    switch (thr) {
        case BOARD_THREAD:
            if (timeout_secs != 0) {
                uint32_t status = cv_flags.wait_any(BOARD_FLAG, timeout_secs*1000);
                if (status == osFlagsError || status == osFlagsErrorTimeout) ret = false;
                else ret = true;
            }
            else {
                uint32_t status = cv_flags.wait_any(BOARD_FLAG);
                if (status == osFlagsError || status == osFlagsErrorTimeout) ret = false;
                else ret = true;
            }
            break;
        case AGENT_THREAD:
            if (timeout_secs != 0) {
                uint32_t status = cv_flags.wait_any(AGENT_FLAG, timeout_secs*1000);
                if (status == osFlagsError || status == osFlagsErrorTimeout) ret = false;
                else ret = true;
            }
            else {
                uint32_t status = cv_flags.wait_any(AGENT_FLAG);
                if (status == osFlagsError || status == osFlagsErrorTimeout) ret = false;
                else ret = true;
            }
            break;
        case CAMERA_THREAD:
            if (timeout_secs != 0) {
                uint32_t status = cv_flags.wait_any(CAMERA_FLAG, timeout_secs*1000);
                if (status == osFlagsError || status == osFlagsErrorTimeout) ret = false;
                else ret = true;
            }
            else {
                uint32_t status = cv_flags.wait_any(CAMERA_FLAG);
                if (status == osFlagsError || status == osFlagsErrorTimeout) ret = false;
                else ret = true;
            }
            break;
        }
    #endif
    #endif
    return ret;
}
