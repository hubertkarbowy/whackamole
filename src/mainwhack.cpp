#include <iostream>
// #include <stdlib.h>
// #include <time.h>
#include <limits.h>
#include <mainwhack.hpp>
#include <whackamole_class.hpp>
#include <states.hpp>
#include <helpers.hpp>

using namespace std;

void R(unsigned short int state, char* state_as_base3_buf) {
    to_base3_buf(state, state_as_base3_buf);
}

void test_tobase3() {
    cout << ">>>>>>> RUNNING test_tobase3 <<<<<<<<\n";
    char* base3buf = new char[NUM_HOLES];
    unsigned short int test_arr[] = {1, 18, 6530, 6560, 0, 3, 2, 4};
    for (int i=0; i<8; i++) {
        // R(test_arr[i], base3buf);
        cout << int(test_arr[i]) << " is ";
        to_base3_buf(test_arr[i], base3buf);
        print_arr(base3buf, NUM_HOLES);
    }
    delete base3buf;
}

int main(int argc, char** argv) {
    cout << "Hello, Whack-a-mole\n";
        test_tobase3();

    WhacQaMole* game = new WhacQaMole(NUM_HOLES, Random);
    game->deserialize("./kret.bin");

    for (int i=0; i<5000; i++) {
        _D << "=================== EPISODE " << i << " ========================\n";
        int step = 0;
        while (game->learn_step() == false) {
            _D << "    ***** STEP " << step << " *****\n";
            step++;
        }
        game->reset();
    }
    // game->serialize("./kret.bin");
}
