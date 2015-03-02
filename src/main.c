#include <pebble.h>
#define KEY_BLOCK_HEIGHT 0

// DECLARATIONS ****************************************************
// Background LAYER and Actual BITMAP
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;  

// Textlayers These go atop BACKGROUND and/or Windows
static TextLayer *s_time_layer;
static TextLayer *s_block_layer;

// Fonts
static GFont s_time_font;
static GFont s_block_font;

static Window *s_main_window;

// UPDATE TIME FUNCTION ****************************************************
static void update_time() {

  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

// MAIN WINDOW LOAD FUNCTION ****************************************************
static void main_window_load(Window *window) {

  // Create GBitmap, then set to created Bitmaplayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  
  // Create time TextLayer for TIME
  s_time_layer = text_layer_create(GRect(0, 20, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  // Create the TextLayer for BLOCK
  s_block_layer = text_layer_create(GRect(0,115,144,168));
  text_layer_set_background_color(s_block_layer, GColorClear);
  text_layer_set_text_color(s_block_layer, GColorBlack);
  text_layer_set_text_alignment(s_block_layer, GTextAlignmentCenter);
  text_layer_set_text(s_block_layer, "Loading");

  // Create declare font, set font, set alignment, apply font to layer, apply layer to window for TIME
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Ubuntu_R_24));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create declare font, set font, set alignment, apply font to layer, apply layer to window for BLOCK
  s_block_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Ubuntu_R_18));
  text_layer_set_font(s_block_layer, s_block_font);
  text_layer_set_text_alignment(s_block_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_block_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

// MAIN WINDOW UN LOAD FUNCTION ****************************************************
static void main_window_unload(Window *window) {

  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_block_layer);
  
  // Destroy Custom Fonts
  fonts_unload_custom_font(s_block_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

// TICK HANDLER FUNCTION ****************************************************
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

  update_time();
  
  if(tick_time->tm_min % 5 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    dict_write_uint8(iter, 0, 0);
    
    app_message_outbox_send();
  }
}

// CALLBACK FUNCTIONS ****************************************************
// CALLBACKS:  INBOX RECEIVED, INBOX DROPPED, OUTBOX FAILED, OUTBOX SENT

static void inbox_received_callback(DictionaryIterator *iterator, void *context){
  static char blockheight_buffer[32];
  static char blockheight_layer_buffer[32];
    
  Tuple *t = dict_read_first(iterator);
  
  while(t != NULL) {
    switch(t->key) {
      case KEY_BLOCK_HEIGHT:
        snprintf(blockheight_buffer, sizeof(blockheight_buffer), "%u", (int)t->value->int32);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized",  (int)t->key);
        break;
    }
    t = dict_read_next(iterator);
  }
  
  // Now Display this information
  snprintf(blockheight_layer_buffer, sizeof(blockheight_layer_buffer), "%s", blockheight_buffer);
  text_layer_set_text(s_block_layer, blockheight_layer_buffer);
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context){
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Message Dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context){
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success");
}
  
// INIT FUNCTION ****************************************************
static void init() {

  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback); 
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

// DE INIT FUNCTION ****************************************************
static void deinit() {

  // Destroy Window
  window_destroy(s_main_window);
}

// M A I N FUNCTION ****************************************************
int main(void) {
  init();
  app_event_loop();
  deinit();
}
