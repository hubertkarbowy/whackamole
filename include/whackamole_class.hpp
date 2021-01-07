#ifndef __WHACKAMOLE_CLASS__
#define __WHACKAMOLE_CLASS__
#include <atomic>

enum policy {RandomPolicy};

struct game_result {
    short total_reward;
    unsigned char steps_taken;
    unsigned char num_evil_moles;
    unsigned char num_good_moles;
    unsigned char num_empty;
    bool whacked;
};

class WhacQaMole {
    unsigned char num_holes;
    unsigned short int num_states;
    std::atomic<unsigned short int> current_state;
    char** Q; // a 2D array of state-reward history. Rewards/penalties are within the range of -128 to 127
    float gamma;
    char* temp_base3_buf;   // We create these buffers once and for all in the constructor and then reuse them.
    unsigned short int* temp_transitions; // This is less elegant than creating and destroying them several times, but more efficient.
    char* temp_transition_rewards;
    policy POLICY;
    // std::default_random_engine rand_engine;
    // std::uniform_int_distribution<unsigned short int> uniform_dist;
    // std::uniform_int_distribution<int> random_hole_dist;

    // TODO: use the fields below after connecting to TF Lite
    std::atomic_bool _initialized;
    std::atomic_bool is_hitting;

    public:
    WhacQaMole(unsigned char num_holes, enum policy POLICY);
    ~WhacQaMole();
    bool learn_step();
    void reset();
    unsigned short int get_current_state();
    void set_current_state(unsigned short int new_state); // for simulation and debugging
    void set_current_state_from_camera(unsigned short int recognized_state); // for deployment
    void serialize(char* dest);
    void deserialize(char* src);
    bool is_initialized();
    void set_initialized(bool val);
    // bool play(unsigned char max_attempts, short* total_reward, unsigned short* steps_taken);
    bool play(unsigned char max_attempts, struct game_result* res);
}; 

bool notify_board(unsigned char which_hole);
void agent_play_main(int num_games);
extern WhacQaMole* agent;

#endif
