#include <tf_camera.hpp>
#include <whackrealtime.hpp>
#include <helpers.hpp>

unsigned short int OBSERVED_BOARD_STATE = 0;

void run_camera() {
    while (supporting_threads_active) {
        bool agent_ready = wait_on_cv_noarch(CAMERA_THREAD, 1); // wait for decision
        if (!agent_ready) continue;
        _D << "Camera identifying the new state...\n";
        sleep_for_noarch(500);
        _D << "Camera + TF ready \n";
        // set observed board state global variable here
        notify_single_thread_noarch(CAMERA_THREAD);
    }
    _D << "Exiting camera thread\n";
}
