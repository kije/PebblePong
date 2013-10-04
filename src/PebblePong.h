struct Paddle {
  uint16_t v_position;
  uint16_t length;
  uint16_t thikness;
};

struct Player {
  uint16_t score;
  struct Paddle paddle;
};

struct Ball {
  GPoint position;
};