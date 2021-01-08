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
    char* temp_base3_buf;
    
    public:
    unsigned short int current_state;
    Board(char num_holes) {
        this->temp_base3_buf = new char[num_holes];
        this->num_holes = num_holes;
    }

    /**
     * Whack
     *
     * Whacks the board - removes anything (good, evil or void) from which_hole and updates
     * the board's internal state. This function also copies the updated board state to
     * a global variable TRUE_BOARD_STATE which is normally needed only for simulation.
     * When compiled on PC (simulation), the fake camera will then "observe" changes in that global
     * variable instead of the actual image.
     *
    */
    void whack(char which_hole) {    
        _D << "Whacking hole " << std::to_string(which_hole) << "\n";
        to_base3_buf(current_state, temp_base3_buf, num_holes);
        temp_base3_buf[(unsigned char)which_hole] = 0;
        this->current_state = base3_to_int(temp_base3_buf, num_holes);
        TRUE_BOARD_STATE=this->current_state;
    }

    /**
     * Reset the board
     *
     * This function is triggered by the board's main thread on two events (see @board_main):
     * 1) Manual reset was requested - when the agent has reached the final state
     * 2) A timer has elapsed
    */
    void permute() {
        this->current_state = random_int_noarch(RND_STATES);
        _D << "Board permuted\n";
        TRUE_BOARD_STATE = current_state;
    }

} mole_board(NUM_HOLES);

void board_main() {
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
                mole_board.whack(agent_whacked_hole);
                _D << "Board was just whacked on hole " << std::to_string(agent_whacked_hole.load()) << " and went to state " << mole_board.current_state << ".";
                _D << " Resetting the timer.\n";
                is_whacked.store(false);
            }
            else {
                _D << "Attempting a forced reset... ";
                mole_board.permute();
                _D << "Done, board was manually reset to next state " << mole_board.current_state << "\n";
            }
        }
    }
    _D << "Exiting board thread\n";
}
