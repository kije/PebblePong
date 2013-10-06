#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <math.h>



#define MY_UUID { 0x61, 0xD1, 0x20, 0x75, 0xB7, 0x87, 0x4C, 0xCD, 0x86, 0xD5, 0xED, 0x5D, 0x30, 0x4C, 0x2A, 0xA4 }
PBL_APP_INFO(MY_UUID,
             "PebblePong", "kije dev",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

#define PADDLE_HEIGHT 20
#define PADDLE_WIDTH 2

#define BALL_SIZE_HEIGHT 5
#define BALL_SIZE_WIDTH 5
#define BALL_SPEED 3 // Pixel per Update

#define PADDLE_SPEED 2


#define UPDATE_FREQUENCY 1000/10
#define UPDATE_TIMER_COOKIE 1

const float ONE_DEGREE = TRIG_MAX_ANGLE/360.0F;


typedef struct PrecisePoint PrecisePoint;

struct PrecisePoint {
  float x;
  float y;
};



typedef enum Side Side;
typedef enum Direction Direction;
typedef struct Paddle Paddle;
typedef struct Player Player;
typedef struct Ball Ball;

enum Side {
  WEST, EAST, SOUTH, NORTH
};

enum Direction {
  LEFT, RIGHT
};

struct Paddle {
  GRect bounds;
};

struct Player {
  uint16_t score;
  Paddle paddle;
  Side side;
  bool isKI;
  void (*control_handler)(Player *); // Function pointer, which should "controll" the player (e.g. a function that emulates the ki or a function that processes real user input)
};

struct Ball {
  PrecisePoint position;
  GSize size;
  int32_t angle;
};


Window window;
TextLayer titleLayer;
TextLayer scoreLayer;

AppTimerHandle timer_handle;

Layer gameLayer;
Player pl1,pl2;
Ball ball;

GRect validField;




void ki(Player *self) {
  int32_t ball_vertical_position = ball.position.y;
  int32_t paddle_vertical_position = (*self).paddle.bounds.origin.y;

  if (ball_vertical_position > paddle_vertical_position) { // ball is above paddle
    (*self).paddle.bounds.origin.y += PADDLE_SPEED;

  } else if (ball_vertical_position == paddle_vertical_position) {

  } else { // ball is below paddle
    (*self).paddle.bounds.origin.y -= PADDLE_SPEED;
  }
}

void human(Player *self) {
}

// TODO: Multiplayer
void multiplayer(Player *self) {

}


float reflect_angle(int32_t angle) {
  return (float)180.0F-angle;
}


void move_ball(Ball *b) {

  // cos(angle) -> how much the ball goes right ( > 0 means it goes left)
  // sin(angle) -> the slope

  float x = (*b).position.x;
  float y = (*b).position.y;

   if (x < 0 || x > gameLayer.bounds.size.w || y < 0 || y > gameLayer.bounds.size.h) {
      (*b).angle = reflect_angle((*b).angle);
    } 


  float c = (cos_lookup(((*b).angle)*ONE_DEGREE)/(float)TRIG_MAX_RATIO);
  float s = (sin_lookup(((*b).angle)*ONE_DEGREE)/(float)TRIG_MAX_RATIO);


  (*b).position.y = y-(c*(float)BALL_SPEED);
  (*b).position.x = x+(s*(float)BALL_SPEED);

 

  /*if ((*b).direction == LEFT) {
    (*b).position.x = ((*b).position.x - BALL_SPEED);
  } else {
    (*b).position.x = ((*b).position.x + BALL_SPEED);
  }*/
}


void init_player(Player *player, Side side, bool is_ki) {
  GRect bounds = layer_get_bounds(&gameLayer);
  Paddle paddle = (Paddle){GRect(
    (side == WEST ? 0+PADDLE_WIDTH : bounds.size.w-PADDLE_WIDTH-2 /* Why 2 px offset? */),  
    (bounds.size.h/2)-(PADDLE_HEIGHT/2), 
    PADDLE_WIDTH,
    PADDLE_HEIGHT
  )};
  *player = (Player){
    .score = 0,
    .paddle = paddle, 
    .side = side, 
    .isKI = is_ki, 
    .control_handler = (is_ki ? &ki : &human)
  };
}

void init_ball(Ball *b) {
  GRect bounds = layer_get_bounds(&gameLayer);
  *b = (Ball){
    {(bounds.size.w/2)-(BALL_SIZE_WIDTH/2), (bounds.size.h/2)-(BALL_SIZE_HEIGHT/2)},
    GSize(BALL_SIZE_WIDTH, BALL_SIZE_HEIGHT),
    170
  };
}

void draw_dotted_line(GContext *ctx, GPoint p1, GPoint p2, uint16_t space) {
  uint16_t start = p1.y;
  uint16_t stop = p2.y;

  for (uint16_t i = start; i < stop; i+=(space*2)) {
    graphics_draw_line(ctx, GPoint(p1.x,i), GPoint(p2.x,i+space));
  }
}

void draw_paddle(GContext *ctx, Paddle paddle) {
  graphics_fill_rect(ctx, paddle.bounds, 0, GCornerNone);
}

void draw_ball(GContext *ctx, Ball b) {
  graphics_fill_rect(ctx, GRect(b.position.x, b.position.y, b.size.w, b.size.h), 0, GCornerNone);
}

void draw_game_field(struct Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color (ctx, GColorWhite); 
  graphics_context_set_fill_color (ctx, GColorWhite);   

  // Frame
  graphics_draw_rect(ctx, bounds);

  // Middle-line
  draw_dotted_line(ctx, GPoint(bounds.size.w/2,0), GPoint(bounds.size.w/2,bounds.size.h), 3); 


  draw_paddle(ctx,pl1.paddle);
  draw_paddle(ctx,pl2.paddle);


  draw_ball(ctx, ball); 
}

void handle_init(AppContextRef ctx) {

  resource_init_current_app(&PEBBLE_PONG_RESOURCES);

  window_init(&window, "PebblePong");
  window_set_background_color(&window, GColorBlack);
  window_stack_push(&window, true /* Animated */);


  // Title Layer
  text_layer_init(&titleLayer, GRect(0,0,144,23));
  text_layer_set_text_alignment(&titleLayer, GTextAlignmentCenter);
  text_layer_set_text(&titleLayer, "PebblePong");
  text_layer_set_font(&titleLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_THIN_15)));
  text_layer_set_background_color(&titleLayer, GColorBlack);
  text_layer_set_text_color(&titleLayer, GColorWhite);
  layer_add_child(&window.layer, &titleLayer.layer);

  //Score Layer
  text_layer_init(&scoreLayer, GRect(0,20,144,18));
  text_layer_set_text_alignment(&scoreLayer, GTextAlignmentCenter);
  text_layer_set_text(&scoreLayer, "0 | 0");
  text_layer_set_font(&scoreLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RALEWAY_THIN_15)));
  text_layer_set_background_color(&scoreLayer, GColorBlack);
  text_layer_set_text_color(&scoreLayer, GColorWhite);
  layer_add_child(&window.layer, &scoreLayer.layer);


  // Game Field

  layer_init(&gameLayer, GRect(5, 40, 134, 90));
  layer_set_update_proc(&gameLayer, &draw_game_field);
  layer_add_child(&window.layer, &gameLayer);

  // Player 

  init_player(&pl1, WEST, true);
  init_player(&pl2, EAST, false);

  // Ball 
  init_ball(&ball);

  // Setup timer

  timer_handle = app_timer_send_event(ctx, UPDATE_FREQUENCY, UPDATE_TIMER_COOKIE);
}

void handle_timeout(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie) {
  switch (cookie) {
    case UPDATE_TIMER_COOKIE:
      pl1.control_handler(&pl1);
      pl2.control_handler(&pl2);
      move_ball(&ball);
      layer_mark_dirty(&gameLayer);
      layer_mark_dirty(&window.layer);
      timer_handle = app_timer_send_event(app_ctx, UPDATE_FREQUENCY, UPDATE_TIMER_COOKIE);
      break;
  }
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timeout
  };
  app_event_loop(params, &handlers);
}
