#include <stdlib.h>
#include <whackamole_class.hpp>
#include <states.hpp>
#include <helpers.hpp>
#include <random>
#include <climits>
#include <cmath>

#ifdef COMPILE_FOR_PC
#include <fstream>
#include <iostream>
#endif


using namespace std;

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

    std::random_device r;
    this->random_hole_dist = uniform_int_distribution<int>(0, num_holes); // not num_holes+1 because inclusive
    // Randomly initialize the agent's state - this should be irrelevant when connected to TF and camera
    this->rand_engine = std::default_random_engine(r());
    this->uniform_dist = uniform_int_distribution<unsigned short int>(0, num_states);
    this->current_state = this->uniform_dist(rand_engine);
    
    cout << "Created the game with " << num_states << " states and ";
    cout << total_transitions << " transitions. Initial state is " << this->current_state << std::endl;
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
    to_base3_buf(current_state, temp_base3_buf, num_holes);
    if (is_terminal_state(temp_base3_buf, num_holes)) { // do not perform updates if we detect a terminal state at the outset
        return true;
    }

    all_transitions_and_rewards(this->current_state, this->temp_transitions,
                                this->temp_transition_rewards, this->temp_base3_buf,
                                this->num_holes);
    _D << "Current state is " << current_state << ", as base3: ";
    print_arr(this->temp_base3_buf, this->num_holes);
    cout << "Next states:\n";
    print_arr(this->temp_transitions, this->num_holes+1, UINT_ARR);
    // cout << "Next rewards:\n";
    // print_arr(this->temp_transition_rewards, this->num_holes+1);
    if (this->POLICY == Random) {
        // int hole_to_hit = rand() % (num_holes + 1);
        unsigned short int prev_state = current_state;
        int hole_to_hit = this->random_hole_dist(rand_engine);
        char transition_reward = temp_transition_rewards[hole_to_hit];
        cout << "Will hit hole " << hole_to_hit << " with a transition reward of " << to_string(transition_reward);
        cout << ". Next state is " << temp_transitions[hole_to_hit];
        // cout << " with a reward of " << to_string(temp_transition_rewards[hole_to_hit]) << endl;
        current_state = temp_transitions[hole_to_hit];
        to_base3_buf(current_state, temp_base3_buf, num_holes);
        all_transitions_and_rewards(this->current_state, this->temp_transitions,
                                    this->temp_transition_rewards, this->temp_base3_buf,
                                    this->num_holes);
        char selected_max = max_charr(this->Q[current_state], this->num_holes+1);
        cout << " and known Q-rewards from there are: ";
        print_arr(this->Q[current_state], this->num_holes+1);
        cout << "Therefore, the selected max is " << to_string(selected_max) << endl;
        char updated_q = fmax(SCHAR_MIN, fmin(SCHAR_MAX, (char)round(transition_reward + (gamma*selected_max))));
        cout << "Q(" << prev_state << ", " << current_state << ") = " << to_string(transition_reward) << " + " << gamma << "*" << to_string(selected_max) << " = " << to_string(updated_q) << endl;
        Q[prev_state][hole_to_hit] = updated_q;
    }
    else {
        cout << "Unknown policy!\n";
        exit(-1);
    }
   
    return is_terminal_state(temp_base3_buf, num_holes);
}

/** Set a new state randomly.
 *
 * At learning time used to switch between episodes.
 */
void WhacQaMole::reset() {
    this->current_state = this->uniform_dist(rand_engine);
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
    // this->is_hitting = true;
    this->current_state = new_state;
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
    cout << "Wrote the Q-array to " << dest << endl;
#else
    cout << "Not implemented\n";
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
    cout << "Restored the game with " << to_string(num_holes) << " holes and ";
    cout << to_string(num_states) << " states \n";
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
 * @param[out] total_reward     Total number of points collected as rewards on state transitions.
 * @param[out] total_whacks     Total number of whacks before reaching the final state or giving up.
 * @returns                     True if the agent found the final state, false if it gave up.
 *
 *
 */
bool WhacQaMole::play(unsigned char max_attempts, short* total_reward, unsigned short* steps_taken) {
    short _reward = 0;
    unsigned short _steps = 0;
    bool ret = false;
    to_base3_buf(current_state, temp_base3_buf, num_holes);
    _D << "   *** Playing from state " << to_string(current_state) << ", as base3: ";
    print_arr(this->temp_base3_buf, this->num_holes, CHAR_ARR, false);
    _D << ", rewards: ";
    print_arr(this->Q[current_state], this->num_holes+1, CHAR_ARR);
    unsigned char i = max_attempts;
    while (i>0) {
        if (is_terminal_state(temp_base3_buf, num_holes)) {
            ret = true;
            break;
        }
        char val = SCHAR_MIN;
        char idx = -1;
        unsigned short int next_state = current_state;
        argmax_charr(Q[current_state], num_holes+1, &val, &idx);
        _D << "  I= " << to_string(i) << " DECISION: ";
        if (idx != -1) {
            if (idx != num_holes) {
                _D << "Hitting hole " << to_string(idx) << " with a reward of " << to_string(val) << ".\n";
                temp_base3_buf[(unsigned char)idx] = 0; // remove anything in a hole
                next_state = base3_to_int(temp_base3_buf, num_holes);
            }
            else {
                _D << "No hit.\n";
            }
        }
        else { _D << "Oops, something looks broken.\n"; }
        to_base3_buf(next_state, temp_base3_buf, num_holes);
        _D << "  I= " << to_string(i) << " NEXT STATE: " << to_string(next_state) << ", as base3: ";
        print_arr(temp_base3_buf, num_holes, CHAR_ARR, false);
        _D << ", rewards: ";
        print_arr(this->Q[next_state], this->num_holes+1, CHAR_ARR);
        this->current_state = next_state;
        _reward += val;
        _steps++;
        i--;
    }
    *total_reward = _reward;
    *steps_taken = _steps;
    return ret;
}
