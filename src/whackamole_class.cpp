#include <stdlib.h>
#include <whackamole_class.hpp>
#include <whackrealtime.hpp> // this file uses the `rand_engine` global variable!
#include <states.hpp>
#include <helpers.hpp>
#include <climits>
#include <cmath>

#ifdef COMPILE_FOR_PC
#include <fstream>
#include <iostream>
#endif

using namespace std;

/** The agent
 *
 * The agent can:
 *
 * 1. Determine its current state from the camera output via the @OBSERVED_BOARD_STATE global variable
 * 2. Operate in the learning mode. In this mode it waits for the camera as described above
 *    and upon receiving data, performs one step of Q-learning.
 * 3. Operate in the playing mode. On each camera change it whacks the board on the hole that
 *    the Q matrix says is the next most probable state.
 * 4. Serialize and deserialize the Q matrix (PC only)
 * 5. Notify the board after it has hit a hole (this should normally be detected by the board)
 */
WhacQaMole::WhacQaMole(unsigned char num_holes, enum policy POLICY) {
    this->num_holes = num_holes;
    num_states = 1;
    for (int i=0; i<num_holes; i++) num_states *= 3;
    Q = new char*[num_states];
    
    int total_transitions = 0;
    for (int i=0; i<num_states; i++) {
        Q[i] = new char[num_holes+1];
        total_transitions += num_holes + 1;
    }
    this->gamma = 0.8;
    // this->current_state = random_int_noarch(RND_STATES);
    this->current_state = 9999;
    
    _D << "Created the game with " << num_states << " states and ";
    _D << total_transitions << " transitions. Initial state is " << this->current_state << "\n";
    // initialize temporary buffers:
    this->temp_base3_buf = new char[num_holes];
    this->temp_transitions = new unsigned short int[num_holes+1];
    this->temp_transition_rewards = new char[num_holes+1];
    this->POLICY = POLICY;
    this->_initialized = false;
}

WhacQaMole::~WhacQaMole() {
    delete[] this->temp_transitions;
    delete[] this->temp_base3_buf;
    delete[] this->temp_transition_rewards;
    for (int i=0; i<this->num_states; i++) {
        delete[] Q[i];
    }
    delete[] Q;
}

/** Performs one step of Q-learning
 *
 */
bool WhacQaMole::learn_step() {
    wait_on_cv_noarch(CAMERA_THREAD, 0); // wait for the camera to become available
    current_state = OBSERVED_BOARD_STATE;
    to_base3_buf(current_state, temp_base3_buf, num_holes);
    _D << "   Received from camera state " << current_state << ", as base3: "; print_arr(this->temp_base3_buf, this->num_holes);

    if (is_terminal_state(temp_base3_buf, num_holes)) { // do not perform updates if we detect a terminal state at the outset
        _D << "   This is the final state.\n";
        return true;
    }

    all_transitions_and_rewards(this->current_state, this->temp_transitions,
                                this->temp_transition_rewards, this->temp_base3_buf,
                                this->num_holes);
    _D << "   Next states: "; print_arr(this->temp_transitions, this->num_holes+1, UINT_ARR);
    // _D << "Next rewards:\n";
    // print_arr(this->temp_transition_rewards, this->num_holes+1);
    if (this->POLICY == RandomPolicy) {
        unsigned short int next_state_agent_thinks = current_state;
        int hole_to_hit = random_int_noarch(RND_HOLES);
        char transition_reward = temp_transition_rewards[hole_to_hit];
        _D << "   Will hit hole " << hole_to_hit << " with a transition reward of " << to_string(transition_reward);
        _D << ". Agent thinks next state will be " << temp_transitions[hole_to_hit];
        next_state_agent_thinks = temp_transitions[hole_to_hit];
        to_base3_buf(next_state_agent_thinks, temp_base3_buf, num_holes);
        all_transitions_and_rewards(next_state_agent_thinks, this->temp_transitions,
                                    this->temp_transition_rewards, this->temp_base3_buf,
                                    this->num_holes);
        char selected_max = max_charr(this->Q[next_state_agent_thinks], this->num_holes+1);
        _D << " and known Q-rewards from there are: ";
        print_arr(this->Q[next_state_agent_thinks], this->num_holes+1);
        _D << "   Therefore, the selected max is " << to_string(selected_max) << "\n";
        char updated_q = fmax(SCHAR_MIN, fmin(SCHAR_MAX, (char)round(transition_reward + (gamma*selected_max))));
        _D << "   Q(" << current_state<< ", " << next_state_agent_thinks << ") = " << to_string(transition_reward) << " + " << gamma << "*" << to_string(selected_max) << " = " << to_string(updated_q) << "\n";
        Q[current_state][hole_to_hit] = updated_q;
        notify_board(hole_to_hit);
    }
    else {
        _D << "Unknown policy!\n";
        exit(-1);
    }
    // return is_terminal_state(temp_base3_buf, num_holes);
    return false;
}

/** Set a new state randomly.
 *
 * This function should generally not be used.
 */
void WhacQaMole::reset() {
    this->current_state = random_int_noarch(RND_STATES);
    to_base3_buf(current_state, temp_base3_buf, num_holes);
    _D << "[DEPRECATED] Agent's state reset to " << to_string(current_state) << " (base3: ";
    print_arr(temp_base3_buf, num_holes, CHAR_ARR, false);
    _D << ")\n";
}

/** Gets the current state
 * 
 * TODO: ensure state is accessed atomically
 */
unsigned short int WhacQaMole::get_current_state() {
    return this->current_state;
}

/** Sets the current state
 *
 * This function should be called each time the agent detects a change in its state.
 * In our case, this will happen when the camera module registers a new configuration of LEDs.
 * 
 * TODO: ensure state is accessed atomically
 */
void WhacQaMole::set_current_state(unsigned short int new_state) {
    this->current_state = new_state;
}

void WhacQaMole::set_current_state_from_camera(unsigned short int recognized_state) {
    this->current_state = recognized_state;
}

void WhacQaMole::serialize(char* dest) {
#ifdef COMPILE_FOR_PC
    char signature[] = {'M', '@', 'G', '1', 'C'};
    char signature2[] = {'Q', 'A', 'R', 'R', 'Y'};
    ofstream f(dest, ios::out | ios::binary);
    f.write(signature, 5);
    f.write(reinterpret_cast<const char*>(&num_holes), sizeof(num_holes));
    f.write(reinterpret_cast<const char*>(&num_states), sizeof(num_states));
    f.write(reinterpret_cast<const char*>(&gamma), sizeof(gamma));
    f.write(signature2, 5);
    for (int i=0; i<num_states; i++) {
        for (int j=0; j<num_holes+1; j++) {
            f.write(&Q[i][j], 1);
        }
    }
    f.close();
    _D << "Wrote the Q-array to " << dest << "\n";
#else
    _D << "Not implemented\n";
#endif
}

void WhacQaMole::deserialize(char* src) {
#ifdef COMPILE_FOR_PC
    unsigned char f_num_holes = 0;
    unsigned short int f_num_states = 0;
    float f_gamma = 0.0;
    ifstream f(src, ios::out | ios::binary);
    if (!f) {
        _D << "Cannot read file " << src << ".\n";
        exit(-10);
    }
    f.ignore(5, EOF); // M@G1C signature - todo: check for its presence
    f.read((char*)(&f_num_holes), sizeof(f_num_holes));
    f.read((char*)(&f_num_states), sizeof(f_num_states));
    if (f_num_holes != this->num_holes || f_num_states != this->num_states) {
        _D << "Error deserializing the Q matrix. Expected to see " << to_string(num_holes) << " holes ";
        _D << "and " << to_string(num_states) << " states, but file `" << src << "` has ";
        _D << to_string(f_num_holes) << " and " << to_string(f_num_states) << " respectively.\n";
        f.close();
        exit(-10);
    }
    f.read((char*)(&f_gamma), sizeof(f_gamma));
    f.ignore(5, EOF); // QARRY signature - todo: check for its presence
    char val = 0;
    for (int i=0; i<f_num_states; i++) {
        for (int j=0; j<f_num_holes+1; j++) {
            f.read((char*)(&val), sizeof(val));
            Q[i][j] = val;
        }
    } // todo: checksum?

    this->num_holes = f_num_holes; // this is redundant because of the above check, but I'm keeping it
    this->num_states = f_num_states; // in case I want to deserialize arbitrary files.
    this->gamma = f_gamma; // in case I want to deserialize arbitrary files.
    this->_initialized = true;
    _D << "Restored the game with " << to_string(num_holes) << " holes and ";
    _D << to_string(num_states) << " states \n";
    f.close();
#else

#endif
}

bool WhacQaMole::is_initialized() {
    return (bool) this->_initialized;
}

void WhacQaMole::set_initialized(bool val) {
    this->_initialized = val;
}

/** Plays a single game, beginning from WhacQaMole#current_state
 *
 * Using the pre-trained Q matrix, the agent attempts to hit all "evil" moles
 * existing in the WhacQaMole#current_state without hitting either empty
 * holes or "good" moles.
 *
 * @param[in] max_attempts      The maximum number of attempts before the agent gives up.
 * @param[out] res              A #game_result struct to store various game statistics (or nullptr)
 * @returns                     True if the agent found the final state, false if it gave up.
 *
 *
 */
bool WhacQaMole::play(unsigned char max_attempts, struct game_result* res) {
    short _reward = 0;
    unsigned char _steps = 0;
    unsigned char _evil_moles = 0;
    unsigned char _good_moles = 0;
    unsigned char _empty_holes = 0;
    bool ret = false;
    wait_on_cv_noarch(CAMERA_THREAD, 0); // wait for the camera to become available
    current_state = 9999; // undefined state at initialization
    unsigned char i = max_attempts;
    _D << "************ NEW GAME  (max " << std::to_string(max_attempts) << " attempts) ***********\n";
    while (i>0) {
        wait_on_cv_noarch(CAMERA_THREAD, 0);
        _D << "   OK, received data from camera\n";
        current_state = OBSERVED_BOARD_STATE;
        to_base3_buf(current_state, temp_base3_buf, num_holes);
        _D << "   Observed state is " << to_string(current_state) << ", as base3: ";
        print_arr(this->temp_base3_buf, this->num_holes, CHAR_ARR, false);
        _D << ", rewards: ";
        print_arr(this->Q[current_state], this->num_holes+1, CHAR_ARR);
        if (is_terminal_state(temp_base3_buf, num_holes)) {
            ret = true;
            break;
        }
        char val = SCHAR_MIN;
        char idx = -1;
        argmax_charr(Q[current_state], num_holes+1, &val, &idx);
        _D << "   I= " << to_string(i) << " DECISION: ";
        if (idx != -1) {
            if (idx != num_holes) {
                _D << "Hitting hole " << to_string(idx) << " with a reward of " << to_string(val) << ".\n";
                switch (temp_base3_buf[(unsigned char)idx]) {
                    case 0: _empty_holes++; break;
                    case 1: _good_moles++; break;
                    case 2: _evil_moles++; break;
                }
            }
            else {
                _D << "No hit.\n";
            }
        }
        else { _D << "Oops, something looks broken.\n"; }
        _reward += val;
        _steps++;
        i--;
        notify_board(idx); // wait for response only indirectly (through camera) - this is just a "whack" signal to the board to respond
    }
    if (res != nullptr) {
        res->total_reward = _reward;
        res->steps_taken = _steps;
        res->num_evil_moles = _evil_moles;
        res->num_good_moles = _good_moles;
        res->num_empty = _empty_holes;
    }
    return ret;
}

bool notify_board(unsigned char which_hole) {
    agent_whacked_hole.store(which_hole);
    is_whacked.store(true);
    notify_single_thread_noarch(BOARD_THREAD);
    return true;
}
