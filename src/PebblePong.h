typedef struct Paddle {
  uint16_t v_position;
  uint16_t length;
  uint16_t thikness;
} Paddle;

typedef struct Player {
  uint16_t score;
  struct Paddle paddle;
} Player;

typedef struct Ball {
  GPoint position;
} Ball;