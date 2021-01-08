#ifndef HELPERS_HPP__
#define HELPERS_HPP__

#define CHAR_ARR 1
#define UINT_ARR 2
#define INT_ARR 3
#define LOGGER mlogger()
#define _D mlogger() // tizen-style

#include <mainwhack.hpp>
#include <string>
#include <sstream>

#ifdef COMPILE_FOR_PC
#include <iostream>
#else
#ifdef COMPILE_FOR_DUINO
#include "Serial.h"
#endif
#endif

void print_arr(void* rewards, int len, int data_type=CHAR_ARR, bool eol=true);
void print_game_summary(struct game_result res);
std::string charr_to_str(char* rewards, int len); // only for unit tests
std::string intarr_to_str(unsigned short int* rewards, int len); // only for unit tests
void to_base3_buf(unsigned short int state, char* state_as_base3_buf, int num_holes);
unsigned short int base3_to_int(char* base3_buf, int num_holes);
char max_charr(char* arr, int len);
void argmax_charr(char* arr, int len, char* val, char* idx);

struct game_result {
    short total_reward;
    unsigned char steps_taken;
    unsigned char num_evil_moles;
    unsigned char num_good_moles;
    unsigned char num_empty;
    bool whacked;
};

class mlogger {
    public:
        mlogger() {}
        ~mlogger() {}
        template <class T>
        mlogger& operator<<(const T &v) { // ahh... to hell with body of a C++ template in a .cpp file
            #ifdef COMPILE_FOR_PC
            std::cout << v;
            // std::cout.flush();
            #else
            #ifdef COMPILE_FOR_DUINO
            std::stringstream ss;
            ss << v;
            const char* rett = ss.str().c_str();
            Serial.write(rett);
            #endif
            #endif
            return *this;
        }
};

#endif
