#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"



#define MY_UUID { 0x61, 0xD1, 0x20, 0x75, 0xB7, 0x87, 0x4C, 0xCD, 0x86, 0xD5, 0xED, 0x5D, 0x30, 0x4C, 0x2A, 0xA4 }
PBL_APP_INFO(MY_UUID,
             "PebblePong", "kije dev",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayer titleLayer;
TextLayer scoreLayer;

Layer gameLayer;

typedef struct {
  GRect bounds;
} Paddle;

#define PADDLE_HEIGHT 20
#define PADDLE_WIDTH 2

typedef enum {
  WEST, EAST
} Side;


typedef struct {
  uint16_t score;
  Paddle paddle;
  Side side;
} Player;

Player pl1,pl2;

typedef struct {
  GPoint position;
} ball;



void init_player(Player *player, Side side) {
  GRect bounds = layer_get_bounds(&gameLayer);
  Paddle paddle = (Paddle){GRect(
    (side == WEST ? 0+PADDLE_WIDTH : bounds.size.w-PADDLE_WIDTH-2 /* Why 2 px offset? */),  
    (bounds.size.h/2)-(PADDLE_HEIGHT/2), 
    PADDLE_WIDTH,
    PADDLE_HEIGHT
  )};
  *player = (Player){0,paddle, side};
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

void ki() {
  // ist der ball weiter oben dann bewege die Platte nach oben
  // ist der ball weiter unten dann bewege die Platte nach unten
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

  //layer_mark_dirty(layer); /// !!!!!!!!!!! Maybe not so a good idea?  
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


  /* Better Way for Text?

  graphics_context_set_text_color(ctx, GColorBlack);

  graphics_text_draw(ctx,
         "Text here.",
         fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
         GRect(5, 5, 144-10, 100),
         GTextOverflowModeWordWrap,
         GTextAlignmentLeft,
         NULL);

  graphics_text_draw(ctx,
         "And text here as well.",
         fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
         GRect(90, 100, 144-95, 60),
         GTextOverflowModeWordWrap,
         GTextAlignmentRight,
         NULL);

         */

  // Game Field

  layer_init(&gameLayer, GRect(5, 40, 134, 90));
  layer_set_update_proc(&gameLayer, &draw_game_field);
  layer_add_child(&window.layer, &gameLayer);

  // Player 

  init_player(&pl1, EAST);
  init_player(&pl2, WEST);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init
  };
  app_event_loop(params, &handlers);
}
