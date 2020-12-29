#ifndef __STATES_HPP__
#define __STATES_HPP__

#include <mainwhack.hpp>
#include <helpers.hpp>

bool is_terminal_state(char* base3_buf, int num_holes);
void all_transitions_and_rewards(unsigned short int curr_state,
                                 unsigned short int* transitions,
                                 char* rewards,
                                 char* base3_buf,
                                 int num_holes);

#endif
