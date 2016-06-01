#include "text_block.h"
#include "geometry.h"
#include "pebble.h"
#include "quadrant.h"

#define FOUR 4
#define BLOCK_SIZE GSize(30, 18)
#define QUADRANT(blck, prio, pos) (Quadrant) { .block = block, .priority = prio, .position = pos }

#define SOUTH_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(90, 72), PBL_IF_ROUND_ELSE(122, 118))
#define NORTH_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(90, 72), PBL_IF_ROUND_ELSE(52, 42))
#define EAST_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(130, 108), PBL_IF_ROUND_ELSE(86, 82))
#define WEST_INFO_CENTER GPoint(PBL_IF_ROUND_ELSE(50, 36), PBL_IF_ROUND_ELSE(86, 82))
#define SOUTH_BLOCK grect_from_center_and_size(SOUTH_INFO_CENTER, BLOCK_SIZE)
#define NORTH_BLOCK grect_from_center_and_size(NORTH_INFO_CENTER, BLOCK_SIZE)
#define EAST_BLOCK grect_from_center_and_size(EAST_INFO_CENTER, BLOCK_SIZE)
#define WEST_BLOCK grect_from_center_and_size(WEST_INFO_CENTER, BLOCK_SIZE)

static GRect block(Position position){
  GRect blocks[4] = {
    [North] = NORTH_BLOCK,
    [South] = SOUTH_BLOCK,
    [West] = WEST_BLOCK,
    [East] = EAST_BLOCK
  };
  GRect rect = blocks[position];
  rect.origin.y += 4;
  return rect;
}

static void quadrants_move_quadrant(Quadrants * quadrants, Index index, Position position){
  GPoint centers[4] = {
    [North] = NORTH_INFO_CENTER,
    [South] = SOUTH_INFO_CENTER,
    [West] = WEST_INFO_CENTER,
    [East] = EAST_INFO_CENTER
  };
  if(index >= FOUR){
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
  if(index >= FOUR){
    return false;
  }
  for(int i=0; i<quadrants->size; i++){
    if(i != index && quadrants->quadrants[i]->position == position){
      if( quadrants->quadrants[i]->priority >= quadrants->quadrants[index]->priority &&
        text_block_get_visible(quadrants->quadrants[i]->block)
        ){
        return false;
    }
    quadrants_swap(quadrants, i, index);
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
  for(int i=0; i<FOUR; i++){
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
  Position position = North;
  for(int j=0;j<FOUR;j++){
    bool skip = false;
    for(int i=0;i<quadrants->size;i++){
      if(quadrants->quadrants[i]->position == j){
        skip = true;
      }
    }
    if(!skip){
      position = j;
      break;
    }
  }
  TextBlock * block = text_block_create(root_layer, GPoint(-100,-100), font);
  text_block_set_enable(block, false);
  int size = quadrants->size;
  if(size >= FOUR){
    return NULL;
  }
  Quadrant * quadrant = (Quadrant *) malloc(sizeof(Quadrant));
  *quadrant = QUADRANT(block, priority, position);
  int i = size;
  while(i > 0 && quadrants->quadrants[i-1] != NULL && quadrants->quadrants[i-1]->priority < priority){
    quadrants->quadrants[i] = quadrants->quadrants[i-1];
    i--;
  }
  quadrants->quadrants[i] = quadrant;
  quadrants->size++;
  return block;
}

static void quadrants_try_takeover_quadrant(Quadrants * quadrants, Index i, tm * time){
  GPoint center = quadrants->center;
  int angle = angle_hour(time, true);
  Segment hour_hand = SEGMENT(quadrants->center, gpoint_on_circle(center, angle, quadrants->hour_hand_radius));
  angle = angle_minute(time);
  Segment minute_hand = SEGMENT(quadrants->center, gpoint_on_circle(center, angle, quadrants->minute_hand_radius));
  for(int j=0; j<FOUR; j++){
    if(!( intersect(hour_hand, block(j)) || intersect(minute_hand, block(j)))){
      if(quadrants_takeover_quadrant(quadrants, i, j)){
        return;
      }
    }
  }
  for(int j=0; j<FOUR; j++){
    if(quadrants_takeover_quadrant(quadrants, i, j)){
      return;
    }
  }
}

void quadrants_update(Quadrants * quadrants, tm * time){
  for(int i=0; i<quadrants->size; i++){
    TextBlock * block = quadrants->quadrants[i]->block;
    text_block_set_enable(block, true);
    if(text_block_get_visible(block)){
      quadrants_try_takeover_quadrant(quadrants, i, time);
    }
  }
}
