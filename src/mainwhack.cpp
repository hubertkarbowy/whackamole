#ifdef COMPILE_FOR_PC
#include <vector>
#include <mainwhack.hpp>
#include <helpers.hpp>
#include <whackrealtime.hpp>
#include <whackamole_class.hpp>
#include <mole_board.hpp>
#include <tf_camera.hpp>
#include "getopt.h"

using namespace std;

enum SIMULATOR_OPERATION {train, play};
enum SERIALIZATION_OPERATION {serialize, deserialize, noop};

WhacQaMole* agent = nullptr; // using this together with extern is horrendous

// PC simulator only
int main(int argc, char** argv) {
    int num_episodes = -1;
    int num_holes = 8;
    char* fname = nullptr;
    enum SERIALIZATION_OPERATION ser_op = noop;
    enum SIMULATOR_OPERATION sim_op = train;
    int c;
    std::vector<std::thread> supporting_threads;
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

    // ======== SET UP THE AGENT AND OPTIONALLY DESERIALIZE PRETRAINED Q-MATRIX =====
    setup_rtos_primitives(num_holes); // very important!
    agent = new WhacQaMole(num_holes, RandomPolicy); // in the setup() function on mbed os
    if (ser_op == deserialize) {
        _D << "Attempting to deserialize from " << fname << "\n";
        agent->deserialize(fname);
    }

    // ========= SET UP AND START THREADS =======
    // std::thread mole_board(board_main);
    // std::thread camera_thread(run_camera);
    supporting_threads.emplace_back(std::thread(run_camera));
    supporting_threads.emplace_back(std::thread(board_main));

    if (sim_op == train) {
        for (int i=0; i<num_episodes; i++) {
            _D << "=================== EPISODE " << i << " ========================\n";
            int step = 0;
            while (true) {
                _D << "    ***** STEP " << step << " *****\n";
                bool is_final = agent->learn_step();
                if (is_final) break;
                step++;
            }
            notify_single_thread_noarch(BOARD_THREAD);
        }
        agent->set_initialized(true);
    }
    else if (sim_op == play) {
        if (!agent->is_initialized()) {
            _D << "Please deserialize the Q matrix before playing.\n";
            exit(-2);
        }
        for (int i=0; i<num_episodes; i++) { // loop forever on arduino?
            struct game_result res = {0, 0, 0, 0, 0};
            agent->play(10, &res);
            print_game_summary(res);
            notify_single_thread_noarch(BOARD_THREAD);
        }
    }

    if (ser_op == serialize) {
        if (!agent->is_initialized()) {
            _D << "Please train the Q matrix before serializing.\n";
            exit(-2);
        }
        agent->serialize(fname);
    }

    supporting_threads_active = false;
    for (auto& t : supporting_threads) t.join(); // camera_thread.join(); mole_board.join();
}
#endif
