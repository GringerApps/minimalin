#include "time_utils.h"

static Time s_current_time;
static bool s_current_time_set = false;

void update_current_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  int hours = tick_time->tm_hour;
  if(hours > 12){
    hours -= 12;
  }else if(hours == 0){
    hours = 12;
  }
  s_current_time.hours   = hours;
  s_current_time.minutes = tick_time->tm_min;
  s_current_time.day     = tick_time->tm_mday;
}

void set_current_time(Time * current_time){
  if(!s_current_time_set){
    update_current_time();
    s_current_time_set = true;
  }
  *current_time = s_current_time;
}
