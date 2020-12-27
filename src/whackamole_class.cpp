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
    this->is_initialized = true;
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
 * Used at learning time.
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
    ifstream f(src, ios::out | ios::binary);
    f.ignore(5, EOF);
    f.read((char*)(&num_holes), sizeof(num_holes));
    f.read((char*)(&num_states), sizeof(num_states));

    cout << "Restored the game with " << to_string(num_holes) << " holes and ";
    cout << to_string(num_states) << " states \n";
    f.close();
#else

#endif
}
