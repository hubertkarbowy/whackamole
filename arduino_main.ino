// #include <mainwhack.hpp>
#include "Serial.h"
#include <mainwhack.hpp>
#include <helpers.hpp>
#include <whackrealtime.hpp>
#include <whackamole_class.hpp>
#include <mole_board.hpp>

// TODO: Switch between train and play + add serialization

// more ugly global variables in other files!
// WhacQaMole* agent;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  setup_rtos_primitives(NUM_HOLES);
//   agent = new WhacQaMole(NUM_HOLES, RandomPolicy);  // moving this to main agent threads
  pinMode(LED_BUILTIN, OUTPUT);
  // true_board_semaphore.acquire();
}

// the loop function runs over and over again forever
void loop() {
  rtos::Thread board_thread;
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
  board_thread.start(board_main);
  board_thread.join();
  // Serial.write("Hello, world of Arduino mobile!\n");
  // agent->reset();
  // _D << "Hello, world of Arduino mobile!\n";

}
