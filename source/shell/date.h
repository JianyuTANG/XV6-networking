typedef struct rtcdate {
  uint second;
  uint minute;
  uint hour;
  uint day;
  uint month;
  uint year;
}rtcdate;

uint dateToTimestamp(rtcdate * date);
