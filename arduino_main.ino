// #include <mainwhack.hpp>
#include "Serial.h"
#include <mainwhack.hpp>
#include <helpers.hpp>
#include <whackrealtime.hpp>
#include <whackamole_class.hpp>
#include <mole_board.hpp>
#include <tf_camera.hpp>

// TODO: Switch between train and play + add serialization

// more ugly global variables in other files!
// WhacQaMole* agent;

// the setup function runs once when you press reset or power the board

WhacQaMole* agent = nullptr;
int num_episodes = 10;

// first shift register for "good" moles:
int good_latch = 11;
int good_clock = 13;
int good_data = 12;
int bad_data = A7;
int bad_latch = A6;
int bad_clock = A3;
unsigned char good_leds = 0;
unsigned char bad_leds = 0;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  setup_rtos_primitives(NUM_HOLES);
  agent = new WhacQaMole(NUM_HOLES, RandomPolicy);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(good_latch, OUTPUT);
  pinMode(good_clock, OUTPUT);
  pinMode(good_data, OUTPUT);
  pinMode(bad_latch, OUTPUT);
  pinMode(bad_clock, OUTPUT);
  pinMode(bad_data, OUTPUT);
  // true_board_semaphore.acquire();
}

// the loop function runs over and over again forever
void loop() {
  rtos::Thread camera_thread;
  rtos::Thread board_thread;

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
  camera_thread.start(run_camera);
  board_thread.start(board_main);

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
  //      agent->set_initialized(true);

  // supporting_threads_active = false;
  board_thread.join();
  camera_thread.join();
  // Serial.write("Hello, world of Arduino mobile!\n");
  // agent->reset();
  // _D << "Hello, world of Arduino mobile!\n";

}
