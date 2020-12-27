#include <states.hpp>

/** Determine if we have arrived at a terminal state.
 *
 * A terminal state is defined as a board without any evil moles. A board with "good" moles
 * as well as a board with no moles are therefore terminal states.
 *
 * @param[in] base3_buf    A base-3 encoded state.
 * @returns                True if the input state is terminal, false otherwise.
*/

bool is_terminal_state(char* base3_buf, int num_holes) {
    bool is_terminal = true;
    for (int i=0; i<num_holes; i++) {
        if (base3_buf[i] == 2) { // at least one evil mole
            is_terminal = false;
            break;
        }
    }
    return is_terminal;
}

/** On-the-fly computations of the R-array.
 *
 * This function computes allowable transitions and their associated rewards/penalties.
 * We could do without it and just hardcode the value but this would use up as much memory
 * as the Q table (assuming 8 holes that's about 51 kb: 3^8 = 6561 states,
 * each state has up to 8 allowable transitions to another state and each transition carries
 * a reward/penalty stored in 1 byte, that's 52488 bytes).
 *
 * All buffers must be pre-allocated.
 *
 * @param[in] curr_state   Current state (from 0 to 3^num_holes)
 * @param[out] transitions  Pointer to a buffer of length of num_holes+1. It will contain the next states after hitting the i-th hole (or not hitting any hole).
 * @param[out] rewards      Pointer to a buffer of length of num_holes+1. It will contain the reward/penalty values associated with hitting the i-th hole given curr_state.
 * @param base3_buf    Pointer to a buffer of length of num_holes. Temporary buffer needed for function internals.
 */
void all_transitions_and_rewards(unsigned short int curr_state,
                                 unsigned short int* transitions, // next states after hitting i-th hole
                                 char* rewards,
                                 char* base3_buf,
                                 int num_holes) {
     to_base3_buf(curr_state, base3_buf, num_holes);
     for (int i=0; i<num_holes; i++) {
         char hole = base3_buf[i]; // save the state of a single hole
         base3_buf[i] = 0; // hit the hole with a hammer - if a mole was there, it now disappeared. if it was empty, it's still empty.
         transitions[i] = base3_to_int(base3_buf, num_holes); // state after hitting the ith hole
         //   std::cout << "After transition the state is " << transitions[i] << std::endl;
         switch (hole) {
             case 0: rewards[i] = MISS;    // hit an empty hole
                     break;
             case 1: rewards[i] = PENALTY; // hit a "good" mole
                     break;
             case 2: rewards[i] = REWARD;  // hit an "evil" mole
         }
         base3_buf[i] = hole; // restore the current state
     }
     to_base3_buf(curr_state, base3_buf, num_holes); // restore current state in the base3 buffer
     transitions[num_holes] = curr_state; // transition to self after NOT hitting anything...
     rewards[num_holes] = is_terminal_state(base3_buf, num_holes) == true ? REWARD : IDLE_BUT_WRONG;
}
