#ifdef COMPILE_FOR_PC
#include <mainwhack.hpp>
#include <helpers.hpp>
#include <whackamole_class.hpp>
#include "getopt.h"

using namespace std;

enum SIMULATOR_OPERATION {train, play};
enum SERIALIZATION_OPERATION {serialize, deserialize, noop};

// PC simulator only
int main(int argc, char** argv) {
    int num_holes = 8;
    int num_episodes = -1;
    char* fname = nullptr;
    enum SERIALIZATION_OPERATION ser_op = noop;
    enum SIMULATOR_OPERATION sim_op = train;
    int c;
    while (1) {
        static struct option simulator_opts[] = {
            {"num-holes",              required_argument, 0, 'h'},
            {"num-episodes",           required_argument, 0, 'e'},
            {"serialize",              required_argument, 0, 's'},
            {"deserialize",            required_argument, 0, 'd'},
            // operations
            {"train",                  no_argument, 0, 't'}, // number of episodes
            {"play",                   no_argument, 0, 'p'}, // number of random states
            {0, 0, 0, 0}
        };
        int opt_index = 0;
        c = getopt_long_only(argc, argv, "", simulator_opts, &opt_index);
        if (c == -1) break;
        switch (c) {
            case 'h':
                num_holes = (int)strtol(optarg, nullptr, 10);
                break;
            case 'e':
                num_episodes = (int)strtol(optarg, nullptr, 10);
                break;
            case 's':
                ser_op = serialize;
                fname = optarg;
                break;
            case 'd':
                ser_op = deserialize;
                fname = optarg;
                break;
            case 't':
                sim_op = train;
                break;
            case 'p':
                sim_op = play;
                break;
        }
    }
    if (num_episodes == -1) {
        _D << "Please pass --num-episodes (number of games to learn from or to play)\n";
        exit(-2);
    }
    _D << "Running the simulator with the number of holes set to " << num_holes << "\n";

    WhacQaMole* game = new WhacQaMole(num_holes, Random);
    if (ser_op == deserialize) {
        _D << "Attempting to deserialize from " << fname << "\n";
        game->deserialize(fname);
    }

    if (sim_op == train) {
        for (int i=0; i<num_episodes; i++) {
            _D << "=================== EPISODE " << i << " ========================\n";
            int step = 0;
            while (game->learn_step() == false) {
                _D << "    ***** STEP " << step << " *****\n";
                step++;
            }
            game->reset();
        }
        game->set_initialized(true);
    }
    else if (sim_op == play) {
        if (!game->is_initialized()) {
            _D << "Please deserialize the Q matrix before playing.\n";
            exit(-2);
        }
        for (int i=0; i<num_episodes; i++) {
            _D << "=================== EPISODE " << i << " ========================\n";
            struct game_result res = {0, 0, 0, 0, 0};
            bool whacked = game->play(10, &res);
            _D << "  SUMMARY:\n    - Total reward: " << to_string(res.total_reward) << "\n";
            _D << "    - Number of steps: " << to_string(res.steps_taken) << "\n";
            _D << "    - Evil: " << to_string(res.num_evil_moles);
            _D << ", Good: " << to_string(res.num_good_moles) << ", Empty: ";
            _D << to_string(res.num_empty) << "\n";
            _D << "    - Whacked: " << to_string(whacked) << "\n";
            game->reset();
        }
    }

    if (ser_op == serialize) {
        if (!game->is_initialized()) {
            _D << "Please train the Q matrix before serializing.\n";
            exit(-2);
        }
        game->serialize(fname);
    }
}
#endif
