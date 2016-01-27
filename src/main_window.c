#include <pebble.h>
#include "main_window.h"

#define ASTEROID_COUNT 0
#define KEY_TEMPERATURE 1
#define KEY_CONDITIONS 2

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_bitham_42_bold;
static GFont s_res_gothic_18_bold;
static GFont s_res_gothic_14;
static GFont s_res_gothic_24_bold;
static TextLayer *s_timelayer_main;
static TextLayer *s_asteroidlayer_main;
static TextLayer *s_asteroidlabel_1;
static TextLayer *s_weather_main;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_bitham_42_bold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  // s_timelayer_main
  s_timelayer_main = text_layer_create(GRect(8, 27, 129, 44));
  text_layer_set_text(s_timelayer_main, "00:00");
  text_layer_set_text_alignment(s_timelayer_main, GTextAlignmentCenter);
  text_layer_set_font(s_timelayer_main, s_res_bitham_42_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_timelayer_main);
  
  // s_asteroidlayer_main
  s_asteroidlayer_main = text_layer_create(GRect(118, 141, 24, 20));
  text_layer_set_text_alignment(s_asteroidlayer_main, GTextAlignmentCenter);
  text_layer_set_font(s_asteroidlayer_main, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_asteroidlayer_main);
  
  // s_asteroidlabel_1
  s_asteroidlabel_1 = text_layer_create(GRect(43, 142, 76, 17));
  text_layer_set_text(s_asteroidlabel_1, "AsteroidToday");
  text_layer_set_font(s_asteroidlabel_1, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_asteroidlabel_1);
  
  // s_weather_main
  s_weather_main = text_layer_create(GRect(3, 2, 139, 24));
  text_layer_set_text_alignment(s_weather_main, GTextAlignmentRight);
  text_layer_set_font(s_weather_main, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_weather_main);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_timelayer_main);
  text_layer_destroy(s_asteroidlayer_main);
  text_layer_destroy(s_asteroidlabel_1);
  text_layer_destroy(s_weather_main);
}
// END AUTO-GENERATED UI CODE

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, 
           sizeof(s_buffer),
           clock_is_24h_style() ?
           "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_timelayer_main, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char asteroid_buffer[32];
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  Tuple *asteroid_tuple = dict_find(iterator, ASTEROID_COUNT);
  if(asteroid_tuple) {
     snprintf(asteroid_buffer,
              sizeof(asteroid_buffer),
              "%d",
               (int)asteroid_tuple->value->int32);
     APP_LOG(APP_LOG_LEVEL_INFO, "data received with value %d", (int)asteroid_tuple->value->int32);
  }
  text_layer_set_text(s_asteroidlayer_main, asteroid_buffer);

  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_main, weather_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_main_window(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  
  window_stack_push(s_window, true);
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void hide_main_window(void) {
  window_stack_remove(s_window, true);
}

int main(void) {
  show_main_window();
  app_event_loop();
  hide_main_window();
}