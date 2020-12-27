#include <mainwhack.hpp>
#include <whackamole_class.hpp>
#include <helpers.hpp>

using namespace std;

int main(int argc, char** argv) {

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
