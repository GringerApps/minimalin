#include "text_block.h"
#include "geometry.h"
#include "pebble.h"
#include "quadrant.h"

#define QUADRANT_COUNT 4
#define POSTIONS_COUNT 4
#define BLOCK_SIZE GSize(32, 18)
#define QUADRANT(blck, prio, pos) (Quadrant) { .block = block, .priority = prio, .position = pos }
#define BLOCK(quadrants, index) quadrants->quadrants[index]->block
#define PRIORITY(quadrants, index) quadrants->quadrants[index]->priority
#define POS(quadrants, index) quadrants->quadrants[index]->position

#ifdef PBL_PLATFORM_EMERY
  #define SOUTH_INFO_CENTER GPoint(100, 165)
  #define NORTH_INFO_CENTER GPoint(100, 65)
  #define EAST_INFO_CENTER GPoint(150, 115)
  #define WEST_INFO_CENTER GPoint(50, 115)
#elif PBL_ROUND
  #define SOUTH_INFO_CENTER GPoint(90, 122)
  #define NORTH_INFO_CENTER GPoint(90, 52)
  #define EAST_INFO_CENTER GPoint(130, 86)
  #define WEST_INFO_CENTER GPoint(50, 86)
#else
  #define SOUTH_INFO_CENTER GPoint(72, 118)
  #define NORTH_INFO_CENTER GPoint(72, 42)
  #define EAST_INFO_CENTER GPoint(108, 82)
  #define WEST_INFO_CENTER GPoint(36, 82)
#endif

#define SOUTH_BLOCK grect_from_center_and_size(SOUTH_INFO_CENTER, BLOCK_SIZE)
#define NORTH_BLOCK grect_from_center_and_size(NORTH_INFO_CENTER, BLOCK_SIZE)
#define EAST_BLOCK grect_from_center_and_size(EAST_INFO_CENTER, BLOCK_SIZE)
#define WEST_BLOCK grect_from_center_and_size(WEST_INFO_CENTER, BLOCK_SIZE)

static GRect rect_translate(const GRect rect, const int x, const int y){
  const GPoint origin = rect.origin;
  return (GRect) { .origin = GPoint(origin.x + x, origin.y + y), .size = rect.size };
}

static bool segment_intersect_with_position(const Segment segment, const Position position){
  const GRect blocks[4] = {
    [North] = NORTH_BLOCK,
    [South] = SOUTH_BLOCK,
    [West] = WEST_BLOCK,
    [East] = EAST_BLOCK
  };
  const GRect rect = blocks[position];
  return intersect(segment, rect_translate(rect, 0, 4));
}

static void quadrants_move_quadrant(Quadrants * const quadrants, const Index index, const Position position){
  const GPoint centers[POSTIONS_COUNT] = {
    [North] = NORTH_INFO_CENTER,
    [South] = SOUTH_INFO_CENTER,
    [West] = WEST_INFO_CENTER,
    [East] = EAST_INFO_CENTER
  };
  if(index >= POSTIONS_COUNT){
    return;
  }
  Quadrant * const quadrant = quadrants->quadrants[index];
  quadrant->position = position;
  text_block_move(quadrant->block, centers[position]);
}

static void quadrants_swap(Quadrants * quadrants, const Index first, const Index second){
  Quadrant * const first_quadrant = quadrants->quadrants[first];
  Quadrant * const second_quadrant = quadrants->quadrants[second];
  const Position first_position = first_quadrant->position;
  quadrants_move_quadrant(quadrants, first, second_quadrant->position);
  quadrants_move_quadrant(quadrants, second, first_position);
}

static bool quadrants_takeover_quadrant(Quadrants * const quadrants, const Index index, const Position position){
  if(index >= QUADRANT_COUNT){
    return false;
  }
  for(int index_to_takeover=0; index_to_takeover<quadrants->size; index_to_takeover++){
    if(index_to_takeover != index && POS(quadrants, index_to_takeover) == position){
      if( PRIORITY(quadrants, index_to_takeover) >= PRIORITY(quadrants, index) &&
        text_block_get_visible(BLOCK(quadrants, index_to_takeover))
        ){
        return false;
    }
    quadrants_swap(quadrants, index_to_takeover, index);
    return true;
  }
}
quadrants_move_quadrant(quadrants, index, position);
return true;
}

Quadrants * quadrants_create(const GPoint center, const int hour_hand_radius, const int minute_hand_radius){
  Quadrants * const quadrants = (Quadrants *) malloc(sizeof(Quadrants));
  quadrants->center = center;
  quadrants->size = 0;
  quadrants->hour_hand_radius = hour_hand_radius;
  quadrants->minute_hand_radius = minute_hand_radius;
  for(int i=0; i<QUADRANT_COUNT; i++){
    quadrants->quadrants[i] = NULL;
  }
  return quadrants;
}

Quadrants * quadrants_destroy(Quadrants * const quadrants){
  for(int i=0; i<quadrants->size; i++){
    Quadrant * const quadrant = quadrants->quadrants[i];
    if(quadrant != NULL){
      free(quadrant);
    }
  }
  free(quadrants);
  return NULL;
}

TextBlock * quadrants_add_text_block(Quadrants * const quadrants, Layer * const root_layer, const GFont font, const Priority priority, const tm * const time){
  static bool free_positions[POSTIONS_COUNT] = { true, true, true, true };
  Position position = North;
  for(int pos=0;pos<POSTIONS_COUNT;pos++){
    if(free_positions[pos]){
      free_positions[pos] = false;
      position = pos;
      break;
    }
  }
  const GPoint centers[POSTIONS_COUNT] = {
    [North] = NORTH_INFO_CENTER,
    [South] = SOUTH_INFO_CENTER,
    [West] = WEST_INFO_CENTER,
    [East] = EAST_INFO_CENTER
  };
  TextBlock * const block = text_block_create(root_layer, centers[position], font);
  text_block_set_ready(block, false);
  const int size = quadrants->size;
  if(size >= QUADRANT_COUNT){
    return NULL;
  }
  Quadrant * const quadrant = (Quadrant *) malloc(sizeof(Quadrant));
  *quadrant = QUADRANT(block, priority, position);
  int i = size;
  while(i > 0 && quadrants->quadrants[i-1] != NULL && PRIORITY(quadrants, i-1) < priority){
    quadrants->quadrants[i] = quadrants->quadrants[i-1];
    i--;
  }
  quadrants->quadrants[i] = quadrant;
  quadrants->size++;
  return block;
}

static bool time_intersect_with_position(Quadrants * const quadrants, const tm * const time, const Position pos){
  const GPoint center = quadrants->center;
  const int hour_handle = angle_hour(time, true);
  const Segment hour_hand = SEGMENT(quadrants->center, gpoint_on_circle(center, hour_handle, quadrants->hour_hand_radius));
  const int minute_angle = angle_minute(time);
  const Segment minute_hand = SEGMENT(quadrants->center, gpoint_on_circle(center, minute_angle, quadrants->minute_hand_radius));
  return segment_intersect_with_position(hour_hand, pos) || segment_intersect_with_position(minute_hand, pos);
}

static bool quadrants_try_takeover_quadrant_in_order(Quadrants * const quadrants, const Index index, const tm * const time, const Position intersect_positions[POSTIONS_COUNT], const bool check_intersect){
  for(int index_pos=0; index_pos<POSTIONS_COUNT; index_pos++){
    const Position pos = intersect_positions[index_pos];
    if(check_intersect && time_intersect_with_position(quadrants, time, pos)){
      continue;
    }
    if(quadrants_takeover_quadrant(quadrants, index, pos)){
      return true;
    }
  }
  return false;
}

static void quadrants_try_takeover_quadrant(Quadrants * const quadrants, const Index index, const tm * const time){
  Position order[POSTIONS_COUNT] = { North, South, East, West };
  const int hour_mod_12 = time->tm_hour % 12;
  const int min_fifth = time->tm_min / 5;
  const bool hour_at_3 = hour_mod_12 == 3;
  const bool hour_at_9 = hour_mod_12 == 9;
  const bool min_at_3 = min_fifth == 3;
  const bool min_at_9 = min_fifth == 9;
  if(hour_at_3 || hour_at_9 || min_at_3 || min_at_9){
    if((hour_at_3 && !min_at_9) || (!hour_at_9 && min_at_3)){
      order[2] = West;
      order[3] = East;
    }else if(!((hour_at_9 && !min_at_3) || (!hour_at_3 && min_at_9))){
      quadrants_try_takeover_quadrant_in_order(quadrants, index, time, order, false);
      return;
    }
  }
  if(quadrants_try_takeover_quadrant_in_order(quadrants, index, time, order, true)){
    return;
  }
  quadrants_try_takeover_quadrant_in_order(quadrants, index, time, order, false);
}

void quadrants_ready(Quadrants * const quadrants){
  for(int index = 0; index < quadrants->size; index++){
    text_block_set_ready(BLOCK(quadrants, index), true);
  }
}

void quadrants_update(Quadrants * const quadrants, const tm * const time){
  for(int index = 0; index < quadrants->size; index++){
    TextBlock * const block = BLOCK(quadrants, index);
    if(text_block_get_visible(block)){
      quadrants_try_takeover_quadrant(quadrants, index, time);
    }
  }
}
