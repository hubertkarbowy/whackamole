#include <helpers.hpp>
#include <limits.h>

using namespace std;

/**
 * Prints an array of chars or unsigned ints
 *
 */
void print_arr(void* rewards, int len, int data_type, bool eol) {
    for (int i=0; i<len; i++) {
        switch (data_type) {
            case (CHAR_ARR): {
                _D << std::to_string(((char*)rewards)[i]) << "*";
                break;
            }
            case (UINT_ARR): {
                _D << std::to_string(((unsigned short int*)rewards)[i]) << "*";
                break;
            }
            default: {
                _D << std::to_string(((int*)rewards)[i]) << "*";
            }
        }
    }
    if (eol) _D << "\n";
}

/**
 * Prints the game summary.
 *
 * Prints: total reward, number of steps, number of whacked holes recognized as good, evil or empty.
 * @param[in] res           A struct of the type #game_result holding the game results
 */
void print_game_summary(struct game_result res) {
    _D << "  SUMMARY:\n    - Total reward: " << to_string(res.total_reward) << "\n";
    _D << "    - Number of steps: " << to_string(res.steps_taken) << "\n";
    _D << "    - Evil: " << to_string(res.num_evil_moles);
    _D << ", Good: " << to_string(res.num_good_moles) << ", Empty: ";
    _D << to_string(res.num_empty) << "\n";
    _D << "    - Whacked: " << to_string(res.whacked) << "\n";
}

string charr_to_str(char* rewards, int len) {
    string ret = "";
    for (int i=0; i<len; i++) {
       ret += std::to_string(rewards[i]);
    }
    return ret;
}

string intarr_to_str(unsigned short int* rewards, int len) {
    string ret = "";
    for (int i=0; i<len; i++) {
       ret += std::to_string(rewards[i]);
    }
    return ret;
}

// note 1: state_as_base3_buf has to be allocated
// note 2: state_as_base3_buf will hold the number in least-significant-digit first order
/**
 * Converts a single-number representation of a board state to an array of chars
 *
 * Example: state = 6000, num_holes = 8, state_as_base3_buf will then be "02002022" because
 * 6000(dec) is 22020020(base3) and our convention is to put the most significant digit first.
 *
 * @param[in] state         A number in the range of 0 - 3^num_holes
 * @param[out] state_as_base3_buf    Buffer of length num_holes where the result will be written. This buffer has to be pre-allocated.
 * @param[out] num_holes    The number of holes on the board
 */
void to_base3_buf(unsigned short int state, char* state_as_base3_buf, int num_holes) {
    char rem = 0;
    unsigned char cnt = 0;
    for (int i=0; i<num_holes; i++) state_as_base3_buf[i] = 0;
    // print_arr(state_as_base3_buf);
    do {
        rem = state % 3;
        state = state / 3;
        state_as_base3_buf[cnt] = rem;
        cnt++;
    } while (state > 0);
    // To reverse the array so that it can be printed as a base-3 number:
    // for (int i=0; i<num_holes/2; i++) {
    //     _D << "Swapping " << i << " with " << (num_holes) - i - 1 << endl;
    //     char tmp = state_as_base3_buf[i];
    //     state_as_base3_buf[i] = state_as_base3_buf[num_holes - i - 1];
    //     state_as_base3_buf[num_holes - i - 1] = tmp;
    // }
}

unsigned short int base3_to_int(char* base3_buf, int num_holes) {
    unsigned short int ret = 0;
    unsigned short int base3_pow = 1;
    for (int i=0; i<num_holes; i++) {
        ret += base3_buf[i]*base3_pow;
        base3_pow *= 3;
    }
    return ret;
}

/**
 * Finds maximum element in an array of signed chars (-128 to 127)
 *
 */
char max_charr(char* arr, int len) {
    char found = SCHAR_MIN; // -128   
    for (int i=0; i<len; i++) {
        if (arr[i] > found) found = arr[i];
    }
    return found;
}

/**
 * Finds maximum element in an array of signed chars (-128 to 127) and its index.
 *
 * @param[in] arr           Pointer to the array
 * @param len               Array length
 * @param[out] val          The maximum value
 * @param[out] idx          Index of the maximum value
 */
void argmax_charr(char* arr, int len, char* val, char* idx) {
    char found = SCHAR_MIN; // -128
    char pos = -1;
    for (int i=0; i<len; i++) {
        if (arr[i] > found) {
            found = arr[i];
            pos = (char)i;
        }
    }
    *val = found;
    *idx = pos;
}
