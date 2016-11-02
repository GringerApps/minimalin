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

#define SOUTH_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(90, 72), PBL_IF_ROUND_ELSE(122, 118))
#define NORTH_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(90, 72), PBL_IF_ROUND_ELSE(52, 42))
#define EAST_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(130, 108), PBL_IF_ROUND_ELSE(86, 82))
#define WEST_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(50, 36), PBL_IF_ROUND_ELSE(86, 82))
#define SOUTH_BLOCK grect_from_center_and_size(SOUTH_INFO_CENTER, BLOCK_SIZE)
#define NORTH_BLOCK grect_from_center_and_size(NORTH_INFO_CENTER, BLOCK_SIZE)
#define EAST_BLOCK grect_from_center_and_size(EAST_INFO_CENTER, BLOCK_SIZE)
#define WEST_BLOCK grect_from_center_and_size(WEST_INFO_CENTER, BLOCK_SIZE)

static bool intersect_with_position(Segment segment, Position position){
  GRect blocks[4] = {
    [North] = NORTH_BLOCK,
    [South] = SOUTH_BLOCK,
    [West] = WEST_BLOCK,
    [East] = EAST_BLOCK
  };
  GRect rect = blocks[position];
  rect.origin.y += 4;
  return intersect(segment, rect);
}

static void quadrants_move_quadrant(Quadrants * quadrants, Index index, Position position){
  GPoint centers[POSTIONS_COUNT] = {
    [North] = NORTH_INFO_CENTER,
    [South] = SOUTH_INFO_CENTER,
    [West] = WEST_INFO_CENTER,
    [East] = EAST_INFO_CENTER
  };
  if(index >= POSTIONS_COUNT){
    return;
  }
  Quadrant * quadrant = quadrants->quadrants[index];
  quadrant->position = position;
  text_block_move(quadrant->block, centers[position]);
}

static void quadrants_swap(Quadrants * quadrants, Index first, Index second){
  Quadrant * first_quadrant = quadrants->quadrants[first];
  Quadrant * second_quadrant = quadrants->quadrants[second];
  Position first_position = first_quadrant->position;
  quadrants_move_quadrant(quadrants, first, second_quadrant->position);
  quadrants_move_quadrant(quadrants, second, first_position);
}

static bool quadrants_takeover_quadrant(Quadrants * quadrants, Index index, Position position){
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

Quadrants * quadrants_create(GPoint center, int hour_hand_radius, int minute_hand_radius){
  Quadrants * quadrants = (Quadrants *) malloc(sizeof(Quadrants));
  quadrants->center = center;
  quadrants->size = 0;
  quadrants->hour_hand_radius = hour_hand_radius;
  quadrants->minute_hand_radius = minute_hand_radius;
  for(int i=0; i<QUADRANT_COUNT; i++){
    quadrants->quadrants[i] = NULL;
  }
  return quadrants;
}

Quadrants * quadrants_destroy(Quadrants * quadrants){
  for(int i=0; i<quadrants->size; i++){
    Quadrant * quadrant = quadrants->quadrants[i];
    if(quadrant != NULL){
      free(quadrant);
    }
  }
  free(quadrants);
  return NULL;
}

TextBlock * quadrants_add_text_block(Quadrants * quadrants, Layer * root_layer, GFont font, Priority priority, tm * time){
  static bool free_positions[POSTIONS_COUNT] = { true, true, true, true };
  Position position = North;
  for(int pos=0;pos<POSTIONS_COUNT;pos++){
    if(free_positions[pos]){
      free_positions[pos] = false;
      position = pos;
      break;
    }
  }
   GPoint centers[POSTIONS_COUNT] = {
    [North] = NORTH_INFO_CENTER,
    [South] = SOUTH_INFO_CENTER,
    [West] = WEST_INFO_CENTER,
    [East] = EAST_INFO_CENTER
  };
  TextBlock * block = text_block_create(root_layer, centers[position], font);
  text_block_set_ready(block, false);
  int size = quadrants->size;
  if(size >= QUADRANT_COUNT){
    return NULL;
  }
  Quadrant * quadrant = (Quadrant *) malloc(sizeof(Quadrant));
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

static void quadrants_try_takeover_quadrant(Quadrants * quadrants, Index index, tm * time){
  GPoint center = quadrants->center;
  int angle = angle_hour(time, true);
  Segment hour_hand = SEGMENT(quadrants->center, gpoint_on_circle(center, angle, quadrants->hour_hand_radius));
  angle = angle_minute(time);
  Segment minute_hand = SEGMENT(quadrants->center, gpoint_on_circle(center, angle, quadrants->minute_hand_radius));
  for(int pos=0; pos<POSTIONS_COUNT; pos++){
    if(!intersect_with_position(hour_hand, pos) && !intersect_with_position(minute_hand, pos)){
      if(quadrants_takeover_quadrant(quadrants, index, pos)){
        return;
      }
    }
  }
  Position positions[POSTIONS_COUNT] = { North, South, East, West };
  for(int index_pos = 0; index_pos < POSTIONS_COUNT; index_pos++){
    if(quadrants_takeover_quadrant(quadrants, index, positions[index_pos])){
      return;
    }
  }
}

void quadrants_ready(Quadrants * quadrants){
  for(int index = 0; index < quadrants->size; index++){
    text_block_set_ready(BLOCK(quadrants, index), true);
  }
}

void quadrants_update(Quadrants * quadrants, tm * time){
  for(int index = 0; index < quadrants->size; index++){
    TextBlock * block = BLOCK(quadrants, index);
    if(text_block_get_visible(block)){
      quadrants_try_takeover_quadrant(quadrants, index, time);
    }
  }
}
