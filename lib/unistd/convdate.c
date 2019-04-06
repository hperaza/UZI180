/*      UZI180 time format (very similar to MSDOS)
 *
 *      time_t.data:
 *
 *      |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *      | | | | | | | | | | | | | | | | |
 *      \   7 bits    /\4 bits/\ 5 bits /
 *         year+1900     month     day
 *
 *      time_t.time:
 *
 *      |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *      | | | | | | | | | | | | | | | | |
 *      \  5 bits /\  6 bits  /\ 5 bits /
 *         hour      minutes     sec*2
 */

/*
 * Convert a UZI180 time & date to the Unix time() format.
 *  (adapted from mtools convdate)
 */

/* uzi180 time format */

#ifdef time_t
#undef time_t
#endif

typedef struct time_t {
    unsigned t_time;
    unsigned t_date;
};

long convuzitime(time_field)
struct time_t *time_field;
{
    extern int time_zone;
    int year, mon, mday, hour, min, sec, old_leaps;
    long answer, sec_year, sec_mon, sec_mday, sec_hour, sec_min, sec_leap;
    static int month[] = {   0,  31,  59,  90, 120, 151,
                           181, 212, 243, 273, 304, 334 };

    /* disect the parts */
    year = ((time_field->t_date & 0xfe00) >> 9) + 1900;
    if (year < 1972)
	year += 100;
    mon = (time_field->t_date & 0x01e0) >> 5;
    mday = time_field->t_date & 0x1f;
    hour = (time_field->t_time & 0xf800) >> 11;
    min = (time_field->t_time & 0x07e0) >> 5;
    sec = (time_field->t_time & 0x001f) * 2;

    /* how many previous leap years */
    old_leaps = (year - 1972) / 4;
    sec_leap = old_leaps * 24L * 60L * 60L;

    /* back off 1 day if before 29 Feb */
    if (!(year % 4) && mon < 3)
	sec_leap -= 24L * 60L * 60L;
    sec_year = (year - 1970) * 365L * 24L * 60L * 60L;
    sec_mon = month[mon - 1] * 24L * 60L * 60L;
    sec_mday = mday * 24L * 60L * 60L;
    sec_hour = hour * 60L * 60L;
    sec_min = min * 60L;

    answer = sec_leap + sec_year + sec_mon + sec_mday 
           + sec_hour + sec_min + sec + (long) time_zone;

    return answer;
}
