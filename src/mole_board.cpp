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
class Board {
    private:
    char num_holes;
    unsigned short int current_state;
    
    public:
    Board(char num_holes) {
        this->num_holes = num_holes;
    }

    /**
     * Whack
    */
    void whack(char which_hole) {    
    #ifdef COMPILE_FOR_DUINO
    bool acquired = true_board_semaphore.try_acquire();
    if (acquired) {
        
    }
    else {
        _D << "Hit while board was changing - not processing this one due to ambiguity\n";
    }
    #endif
    }

    void permute() {
        _D << "Board permuted\n";
    }

} mole_board(NUM_HOLES);

/**
 * Change the board when the ticker elapses.
 *
 * This function will prevent the tick from completing if a hammer is hitting the hole at the same time
 *
 */

void isr_ticker_board_change() {
    #ifdef COMPILE_FOR_PC

    #else
    #ifdef COMPILE_FOR_DUINO
    bool acquired = true_board_semaphore.try_acquire();
    if (acquired) {
        board_ticker.detach(); // stop ticking
        mole_board.permute();
        board_ticker.attach(&isr_ticker_board_change, BOARD_TICKER_INTERVAL);
        true_board_semaphore.release();
    }
    // do nothing otherwise - just let the ticker move on to the next cycle
    #endif
    #endif
}

void board() {

    
}
