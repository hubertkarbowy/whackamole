#ifndef HELPERS_HPP__
#define HELPERS_HPP__

#define CHAR_ARR 1
#define UINT_ARR 2
#define INT_ARR 3
#define LOGGER mlogger()
#define _D mlogger() // tizen-style

#include <mainwhack.hpp>
#include <string>

#ifdef COMPILE_FOR_PC
#include <iostream>
#else
#ifdef COMPILE_FOR_DUINO
#include "Serial.h"
#endif
#endif

void print_arr(void* rewards, int len, int data_type=CHAR_ARR, bool eol=true);
std::string charr_to_str(char* rewards, int len); // only for unit tests
std::string intarr_to_str(unsigned short int* rewards, int len); // only for unit tests
void to_base3_buf(unsigned short int state, char* state_as_base3_buf, int num_holes);
unsigned short int base3_to_int(char* base3_buf, int num_holes);
char max_charr(char* arr, int len);
void argmax_charr(char* arr, int len, char* val, char* idx);
#ifdef COMPILE_FOR_DUINO
void say_ping();
#endif

class mlogger {
    public:
        mlogger() {}
        ~mlogger() {}
        template <class T>
        mlogger& operator<<(const T &v) { // ahh... to hell with body of a C++ template in a .cpp file
            #ifdef COMPILE_FOR_PC
            std::cout << v;
            // #else
            // put serial operations here
            #endif
            return *this;
        }
};

#endif
