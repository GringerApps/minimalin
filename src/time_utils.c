#include "time_utils.h"

static Time s_current_time;
static bool s_current_time_set = false;

void update_current_time() {
  const time_t temp = time(NULL); 
  const struct tm *tick_time = localtime(&temp);
  int hour = tick_time->tm_hour;
  if(hour > 12){
    hour -= 12;
  }else if(hour == 0){
    hour = 12;
  }
  s_current_time.hour   = hour;
  s_current_time.minute = tick_time->tm_min;
  s_current_time.day    = tick_time->tm_mday;
}

Time get_current_time(Time * current_time){
  if(!s_current_time_set){
    update_current_time();
    s_current_time_set = true;
  }
  return s_current_time;
}
