#include <Arduino.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include "init.h"
#include "7segment.h"
#include "Array.h"
#include "timer.h"
#include "utilities.h"

// DEFINE CLASSES

enum class Direction {
  LEFT,
  UP,
  DOWN,
  RIGHT
};

class Snake {
public:
  int position[2];
  int velocity[2];
  int length = 3;
  Array segments_x;
  Array segments_y;
  Direction current_direction;
  uint32_t headColor = Adafruit_NeoPixel::Color(255, 0, 0);
  uint32_t bodyColor = Adafruit_NeoPixel::Color(0, 255, 0);
  bool is_switched = false;
  bool row_is_odd = false;
  bool is_alive = true;

  void update();
  void check_inputs();
  void change_direction(Direction new_direction);
  void show();
  bool head_on_position(int x, int y);
  void check_death();
};

class Apple {
public:
  int position[2];                                       // Position of the apple
  uint32_t color = Adafruit_NeoPixel::Color(0, 0, 255);  // BLUE COLOR

  void spawn_random();
  void spawn_at(const int newPosition[2]);
  void show();
  bool check_collision(Snake);
};

Snake snake1;
Apple apple1;
Adafruit_NeoPixel strip(TOTAL_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int left_button_state = 0;
int up_button_state = 0;
int down_button_state = 0;
int right_button_state = 0;
int prev_position[2];
int score = 0;

SevenSegment sevenSegment;
Timer timer;
SevenSegment sScore;

// ######################################

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LEFT_BUTTON_PIN, INPUT);
  pinMode(UP_BUTTON_PIN, INPUT);
  pinMode(DOWN_BUTTON_PIN, INPUT);
  pinMode(RIGHT_BUTTON_PIN, INPUT);

  strip.setBrightness(BRIGHTNESS);

  apple1.spawn_random();
  apple1.show();

  snake1.position[0] = 2;
  snake1.position[1] = 2;
  snake1.velocity[0] = 1;
  snake1.velocity[1] = 0;
  snake1.current_direction = Direction::RIGHT;

  sevenSegment.setup();
}

void loop() {
  timer.updateTimeByFrame();

  timer.showTime(sevenSegment);
  sScore.showScore(32);

  if (snake1.is_alive) {
    snake1.update();
    Serial.print("px = ");
    Serial.print(snake1.position[0]);
    Serial.print(", py = ");
    Serial.print(snake1.position[1]);
    Serial.print(", vx = ");
    Serial.print(snake1.velocity[0]);
    Serial.print(", vy = ");
    Serial.println(snake1.velocity[1]);
    // Yousif's 60FPS hack :D
    for (int i = 0; i < 30; i++) {
      snake1.check_inputs();
      delay(16);
    }
  } else {  // When you die
    //call end_screen
    Serial.println("ENDGAME STATE");
    strip.clear();  // Not working bc of TinkerCad(?)
    return;
  }
  //delay(wait);

  if (timer.isTimeOver) {  // GAMEOVER
    timer.restart();
  }
}

// INITIALIZE CLASSES
// #APPLE CLASS
void Apple::spawn_random() {
  do {
    // random(0, TOTAL_LEDS) -> We make sure that the limit is always within the number of LEDs
    position[0] = random(0, TOTAL_LEDS) % LEDS_PER_STRIP;
    position[1] = random(0, TOTAL_LEDS) % NUM_LED_STRIPS;
  } while (check_collision(snake1));  // Look for another position if and only if The apple is in the SNAKE


  // Show the position of the apple
  Serial.print("Apple spawned at (");
  Serial.print(position[0]);
  Serial.print(", ");
  Serial.print(position[1]);
  Serial.print(")\n");
}

// RECOMEND ADDIDNG THIS METHOD TO THE SNAKE
bool Apple::check_collision(Snake snake1) {
  for (int i = 0; i < snake1.length; i++) {                                            // Get the length of the snake
    if (snake1.segments_x[i] == position[0] && snake1.segments_y[i] == position[1]) {  // Check if the snake position is the same of the apple
      return true;
    }
  }
  // IF there is not any collision is false
  return false;
}

void Apple::show() {
  strip.setPixelColor(position_to_i(position), color);
  strip.show();
}
// ######################################

// #SNAKE CLASS
void Snake::change_direction(Direction direction) {
  switch (direction) {
    case Direction::LEFT:
      if (current_direction != Direction::RIGHT) {
        velocity[0] = -1;
        velocity[1] = 0;
        current_direction = Direction::LEFT;
      }
      break;
    case Direction::UP:
      if (current_direction != Direction::DOWN) {
        velocity[0] = 0;
        velocity[1] = -1;
        current_direction = Direction::UP;
      }
      break;
    case Direction::DOWN:
      if (current_direction != Direction::UP) {
        velocity[0] = 0;
        velocity[1] = 1;
        current_direction = Direction::DOWN;
      }
      break;
    case Direction::RIGHT:
      if (current_direction != Direction::LEFT) {
        velocity[0] = 1;
        velocity[1] = 0;
        current_direction = Direction::RIGHT;
      }
      break;
  }
}

void Snake::check_inputs() {
  left_button_state = digitalRead(LEFT_BUTTON_PIN);
  up_button_state = digitalRead(UP_BUTTON_PIN);
  down_button_state = digitalRead(DOWN_BUTTON_PIN);
  right_button_state = digitalRead(RIGHT_BUTTON_PIN);

  if (left_button_state == HIGH) {
    change_direction(Direction::LEFT);
  }
  if (up_button_state == HIGH) {
    change_direction(Direction::UP);
  }
  if (down_button_state == HIGH) {
    change_direction(Direction::DOWN);
  }
  if (right_button_state == HIGH) {
    change_direction(Direction::RIGHT);
  }
}

void Snake::check_death() {
  for (int i = 1; i < length; i++) {
    if (head_on_position(segments_x[i], segments_y[i])) {
      is_alive = false;
    }
  }
}

bool Snake::head_on_position(int xPos, int yPos) {
  return (segments_x[0] == xPos && segments_y[0] == yPos);
}

void Snake::show() {
  strip.setPixelColor(xy_to_i(segments_x[0], segments_y[0]), headColor);
  for (int i = 1; i < length; i++) {
    strip.setPixelColor(xy_to_i(segments_x[i], segments_y[i]), bodyColor);
    strip.show();
  }
}

void Snake::update() {
  change_direction(current_direction);
  prev_position[1] = position[1];
  prev_position[0] = position[0];

// Add velocity to position
  position[0] += velocity[0];
  position[0] %= LEDS_PER_STRIP;
  position[1] += velocity[1];
  position[1] %= NUM_LED_STRIPS;
  if (position[0] < 0) {
    position[0] += LEDS_PER_STRIP;
  }
  if (position[1] < 0) {
    position[1] += NUM_LED_STRIPS;
  }

  // if (!row_is_odd && !is_switched) {
  //   position[0] = (((position[0] + velocity[0]) % LEDS_PER_STRIP) + LEDS_PER_STRIP) % LEDS_PER_STRIP;
  // } else if (row_is_odd && !is_switched) {
  //   position[0] = LEDS_PER_STRIP - 1 - (((position[0] + velocity[0]) % LEDS_PER_STRIP) + LEDS_PER_STRIP) % LEDS_PER_STRIP;
  //   is_switched = true;
  // } else if (!row_is_odd && is_switched) {
  //   position[0] = LEDS_PER_STRIP - 1 - (((position[0] + velocity[0]) % LEDS_PER_STRIP) + LEDS_PER_STRIP) % LEDS_PER_STRIP;
  //   is_switched = false;
  // } else if (is_switched) {
  //   position[0] = (((position[0] - velocity[0]) % LEDS_PER_STRIP) + LEDS_PER_STRIP) % LEDS_PER_STRIP;
  // }


  segments_x.push_back(position[0]);
  segments_y.push_back(position[1]);

  if (head_on_position(apple1.position[0], apple1.position[1])) {
    length++;
    score++;
    Serial.print("Length: ");  // once you eat the second apple, length does not print to serialmonitor
    Serial.println(length);
    apple1.spawn_random();
    apple1.show();
  } else if (length < segments_x.get_size()) {
    strip.setPixelColor(xy_to_i(segments_x[0], segments_y[0]), strip.Color(0, 0, 0));
    segments_x.pop_back();
    segments_y.pop_back();
    Serial.println("POPPED");
  }
  if (snake1.is_alive) {
    snake1.check_death();
  }
  if (!snake1.is_alive) {
    Serial.println("YOU'RE DEAD");
    strip.clear();
  } else {
    snake1.show();
  }
}
