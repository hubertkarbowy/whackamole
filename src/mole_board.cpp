#include <mole_board.hpp>
#include <helpers.hpp>
#include <whackrealtime.hpp>

/**
 * The board object.
 *
 * Keeps the current board state in the same convention as the agent (which see).
 *
 * Since this class should not be accessible to other threads and is quite small,
 * we don't put it in the header file
*/

std::atomic<bool> is_whacked(false);

class Board {
    private:
    char num_holes;
    
    public:
    unsigned short int current_state;
    Board(char num_holes) {
        this->num_holes = num_holes;
    }

    /**
     * Whack
    */
    void whack(char which_hole) {    
        _D << "Whacking hole " << std::to_string(which_hole) << "\n";
    // #ifdef COMPILE_FOR_DUINO
    //     bool acquired = true_board_semaphore.try_acquire();
    //     if (acquired) {
    //     }
    //     else {
    //        _D << "Hit while board was changing - not processing this one due to ambiguity\n";
    //     }
    // #endif
    }

    void permute() {
        this->current_state = random_int_noarch(RND_STATES);
        _D << "Board permuted\n";
    }

} mole_board(NUM_HOLES);

/**
 * Change the board when the ticker elapses.
 *
 * This function will prevent the tick from completing if a hammer is hitting the hole at the same time
 *
 */

// void isr_ticker_board_change() {
//     #ifdef COMPILE_FOR_PC
// 
//     #else
//     #ifdef COMPILE_FOR_DUINO
//     bool acquired = true_board_semaphore.try_acquire();
//     if (acquired) {
//         board_ticker.detach(); // stop ticking
//         mole_board.permute();
//         board_ticker.attach(&isr_ticker_board_change, BOARD_TICKER_INTERVAL);
//         true_board_semaphore.release();
//     }
//     // do nothing otherwise - just let the ticker move on to the next cycle
//     #endif
//     #endif
// }

void _board_lock_unconditionally() {
    #ifdef COMPILE_FOR_PC
       // std::unique_lock<std::mutex> lck(true_board_mutex); // initialization - TODO: RAII
    #else
    #ifdef COMPILE_FOR_DUINO
       // true_board_semaphore.acquire();
    #endif
    #endif
}

// bool board_lock_try_lock_for(int secs) { // todo: refactor - this code repeats itself in other classes
//     #ifdef COMPILE_FOR_PC
//     std::unique_lock<std::mutex> lck(true_board_mutex);
//     std::cv_status status = cv.wait_for(lck, std::chrono::seconds(secs)); // seconds
//     if (status == std::cv_status::timeout) {
//         return false;
//     }
//     else return true;
//     // return true_board_mutex.try_lock_for(std::chrono::seconds(secs)); // seconds
//     #else
//     #ifdef COMPILE_FOR_DUINO
//     uint32_t ret = cv_flags.wait_any(BOARD_WHACKED_FLAG, (uint32_t)1000*secs, true);
//     _D << "Got " << std::to_string(ret) << "\n";
//     // printf("Got: 0x%08lx\r\n", flags_read);    
//     // return true_board_semaphore.try_acquire_for(1000*secs); // miliseconds
//     return false;
//     #endif
//     #endif
// }

void board_main() {
    // _board_lock_unconditionally();
    _D << "INIT board - lock successful\n";
    while (supporting_threads_active) {
        // bool released = board_lock_try_lock_for(BOARD_TICKER_INTERVAL);
        bool released = wait_on_cv_noarch(BOARD_THREAD, BOARD_TICKER_INTERVAL);
        if (!released) {
            mole_board.permute();
            _D << "Board cycling to the next state " << mole_board.current_state << "\n";
        }
        else {
            if (is_whacked.load()) {
                _D << "Board was just whacked on hole " << std::to_string(agent_whacked_hole.load()) << " and went to state " << mole_board.current_state << ".";
                _D << " Resetting the timer.\n";
                is_whacked.store(false);
            }
            else {
                _D << "Spurious wakeup?! We probably shouldn't see this...\n";
            }
        }
    }
    _D << "Exiting board thread\n";
}
