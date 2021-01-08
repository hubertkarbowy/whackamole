#include <tf_camera.hpp>
#include <whackrealtime.hpp>
#include <helpers.hpp>

void run_camera() {
    int num_states = 1;
    for (int i=0; i<NUM_HOLES; i++) num_states *= 3;
    while (supporting_threads_active) {
    #ifdef COMPILE_FOR_PC
        // _D << "Camera scanning for new states...\n";
        sleep_for_noarch(CAMERA_SCAN_INTERVAL);
        if (TRUE_BOARD_STATE > num_states) continue; // undefined camera state, e.g. 9999
        // _D << "Camera + TF ready \n";
        // set observed board state global variable here just before calling notify
        //if (OBSERVED_BOARD_STATE != TRUE_BOARD_STATE) {
        OBSERVED_BOARD_STATE = TRUE_BOARD_STATE;
        notify_single_thread_noarch(CAMERA_THREAD);
        // }
    #else
    #ifdef COMPILE_FOR_DUINO
        // _D << "Camera scanning for new states...\n";
        sleep_for_noarch(CAMERA_SCAN_INTERVAL);
        if (TRUE_BOARD_STATE > num_states) continue; // undefined camera state, e.g. 9999
        // _D << "Camera + TF ready \n";
        // set observed board state global variable here just before calling notify
        //if (OBSERVED_BOARD_STATE != TRUE_BOARD_STATE) {
        OBSERVED_BOARD_STATE = TRUE_BOARD_STATE;
        notify_single_thread_noarch(CAMERA_THREAD);
        // }
    #endif
    #endif
    }
    _D << "Exiting camera thread\n";
}
