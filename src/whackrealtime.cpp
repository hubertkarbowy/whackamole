#include <whackrealtime.hpp>

#ifdef COMPILE_FOR_PC
    #include <random>
    std::random_device r;
    std::default_random_engine rand_engine = std::default_random_engine(r());
    std::mutex true_board_mutex;
    struct whackamole_generators_cpp {
        std::uniform_int_distribution<unsigned char> rnd_hole_dist;
        std::uniform_int_distribution<unsigned short int> rnd_state_dist;
    } whackamole_generators_cpp;
#else
#ifdef COMPILE_FOR_DUINO
    rtos::Semaphore true_board_semaphore;
    struct whackamole_generators_duino {
        unsigned char num_holes;
        unsigned short int num_states;
    } whackamole_generators_duino;
    mbed::Ticker board_ticker;
#endif
#endif


void setup_rtos_primitives(unsigned char num_holes) {
    unsigned short int num_states = 1;
    for (int i=0; i<num_holes; i++) num_states *= 3;
#ifdef COMPILE_FOR_PC
    whackamole_generators_cpp.rnd_hole_dist = std::uniform_int_distribution<unsigned char>(0, num_holes);
    whackamole_generators_cpp.rnd_state_dist = std::uniform_int_distribution<unsigned short int>(0, num_states);
#else
#ifdef COMPILE_FOR_DUINO
    whackamole_generators_duino.num_holes = num_holes + 1; // +1 because Arduino's random() function generates numbers up to but not including the upper bound
    whackamole_generators_duino.num_states = num_states + 1;
#endif
#endif
}

unsigned short int random_int_noarch(enum noarch_rnd NOARCH_RND) {
#ifdef COMPILE_FOR_PC
    if (NOARCH_RND == RND_HOLES) {
        return whackamole_generators_cpp.rnd_hole_dist(rand_engine);
    }
    else if (NOARCH_RND == RND_STATES) {
        return whackamole_generators_cpp.rnd_state_dist(rand_engine);
    }
    else {
        return 0; // todo: raise an exception
    }
#else
#ifdef COMPILE_FOR_DUINO
    long randomNumber;
    if (NOARCH_RND == RND_HOLES) {
        randomNumber = random(0, whackamole_generators_duino.num_holes);
        return (unsigned short int) randomNumber;
    }
    else if (NOARCH_RND == RND_STATES) {
        randomNumber = random(0, whackamole_generators_duino.num_states);
        return (unsigned short int) randomNumber;
    }
    else {
        return 0; // todo: raise an exception
    }
#endif
#endif
}
