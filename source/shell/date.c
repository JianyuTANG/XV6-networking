#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "date.h"

rtcdate* timestampToDate(uint stamp)
{
  uint ret = 946684800; //utc+0 2000/1/1 0:0:0 946684800
  uint YEAR_SEC = 31536000;
  uint DAY_SEC = 86400;
  stamp -= ret;
  int years = 0;
  //get years
  while(1)
  {
    if(years % 4 == 0)
    {
      if(stamp >= YEAR_SEC + DAY_SEC)
      {
        stamp = stamp - (YEAR_SEC + DAY_SEC);
        years++;
      }
      else
        break;
    }
    else
    {
      if(stamp >= YEAR_SEC)
      {
        stamp -= YEAR_SEC;
        years++;
      }
      else
        break;
    }
  }
  //get month
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if(years != 0 && years % 4 == 0)
    days[1] = 29;
  int month = 1;
  while(1)
  {
    int temp = stamp - days[month-1] * DAY_SEC;
    if(temp >= 0)
    {
      stamp = temp;
      month++;
    }  
    else
      break;
  }
  //get days
  int day = 1;
  while(stamp >= DAY_SEC)
  {
    stamp -= DAY_SEC;
    day++;
  }
  //get hours
  int hour = 0;
  while(stamp >= 3600)
  {
    stamp -= 3600;
    hour++;
  }
  //get minutes
  int minute = 0;
  while(stamp >= 60)
  {
    stamp -= 60;
    minute++;
  }
  int second = stamp;
  rtcdate * date = malloc(sizeof(rtcdate));
  date->year = years + 2000;
  date->month = month;
  date->day = day;
  date->hour = hour;
  date->minute = minute;
  date->second = second;
  return date;
}

int main()
{
	int now = gettimestamp();//当前时间戳
	rtcdate* nowdate = timestampToDate(now);//查找当前时间
	printf(1, "timeNow: %d/%d/%d %d:%d:%d\n",nowdate->year,nowdate->month,nowdate->day,nowdate->hour,nowdate->minute,nowdate->second);
	free(nowdate);
	exit();
}