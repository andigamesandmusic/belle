/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

#ifndef PRIM_INCLUDE_TIME_H
#define PRIM_INCLUDE_TIME_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Stores a UTC timecode.
  class Time
  {
    public:

    ///Denotes the current year.
    count Year;

    ///Denotes the index of the month (1-12).
    count Month;

    ///Denotes the day (1-31).
    count Day;

    ///Denotes the hour (0-23).
    count Hour;

    ///Denotes the minute (0-59).
    count Minute;

    ///Denotes the second (0-59).
    count Second;

    ///Weekday enumeration.
    enum Weekdays {Sunday = 0, Monday, Tuesday, Wednesday, Thursday, Friday,
      Saturday};

    ///Days since January 1 (0-365).
    count Yearday;

    ///Denotes the absolute value number of hours ahead or behind GMT.
    count TimeZoneHoursOffset;

    /**Denotes the number of minutes in addition to offset hours. A few time
    zones use minute offsets such as India, Sri Lanka, Nepal, Australia, etc.*/
    count TimeZoneMinutesOffset;

    ///Denotes the weekday (Sunday-Saturday).
    Weekdays Weekday; PRIM_PAD(Weekdays)

    ///Denotes whether daylight savings is in effect.
    bool DaylightSavings; PRIM_PAD(bool)

    ///Denotes whether this a positive GMT offset.
    bool TimeZonePositiveGMTOffset; PRIM_PAD(bool)

    ///Creates the current time.
    Time() {SetToNow();}

    ///Gets the current time.
    void SetToNow()
    {
      SetSecondsSinceUnixEpoch(SecondsSinceUnixEpoch());
    }

    ///Gets the number of seconds since the Unix epoch.
    static integer SecondsSinceUnixEpoch();

    ///Sets the time based on the number of seconds since the Unix epoch.
    void SetSecondsSinceUnixEpoch(integer SecondsSinceUnixEpoch);

    ///Gets the month name in English.
    static String EnglishMonthName(count Month)
    {
      if(Month == 1) return "January";
      else if(Month == 2) return "February";
      else if(Month == 3) return "March";
      else if(Month == 4) return "April";
      else if(Month == 5) return "May";
      else if(Month == 6) return "June";
      else if(Month == 7) return "July";
      else if(Month == 8) return "August";
      else if(Month == 9) return "September";
      else if(Month == 10) return "October";
      else if(Month == 11) return "November";
      else if(Month == 12) return "December";
      return "";
    }

    ///Gets the short month name in English.
    static String EnglishMonthShortName(count Month)
    {
      if(Month == 1) return "Jan.";
      else if(Month == 2) return "Feb.";
      else if(Month == 3) return "Mar.";
      else if(Month == 4) return "Apr.";
      else if(Month == 5) return "May";
      else if(Month == 6) return "Jun.";
      else if(Month == 7) return "Jul.";
      else if(Month == 8) return "Aug.";
      else if(Month == 9) return "Sep.";
      else if(Month == 10) return "Oct.";
      else if(Month == 11) return "Nov.";
      else if(Month == 12) return "Dec.";
      return "";
    }

    ///Gets the weekday name in English.
    static String EnglishWeekdayName(Weekdays Weekday)
    {
      if(Weekday == Sunday) return "Sunday";
      else if(Weekday == Monday) return "Monday";
      else if(Weekday == Tuesday) return "Tuesday";
      else if(Weekday == Wednesday) return "Wednesday";
      else if(Weekday == Thursday) return "Thursday";
      else if(Weekday == Friday) return "Friday";
      else if(Weekday == Saturday) return "Saturday";
      return "";
    }

    ///Gets the weekday name in English.
    static String EnglishWeekdayShortName(Weekdays Weekday)
    {
      if(Weekday == Sunday) return "Sun";
      else if(Weekday == Monday) return "Mon";
      else if(Weekday == Tuesday) return "Tue";
      else if(Weekday == Wednesday) return "Wed";
      else if(Weekday == Thursday) return "Thu";
      else if(Weekday == Friday) return "Fri";
      else if(Weekday == Saturday) return "Sat";
      return "";
    }

    private:

    ///Prefixes zeros.
    static String PrefixZeroes(count i, count Length = 2)
    {
      String s(i);
      while(s.n() < Length)
        s.Prepend("0");
      return s;
    }

    public:

    ///Returns a verbose UTC date-time string.
    String Verbose() const
    {
      String s;
      s >> EnglishWeekdayName(Weekday) << ", " << EnglishMonthName(Month) <<
        " " << Day << ", " << Year << " " << PrefixZeroes(Hour) << ":" <<
        PrefixZeroes(Minute) << ":" << PrefixZeroes(Second) << " " <<
        (TimeZonePositiveGMTOffset ? "+" : "-") <<
        PrefixZeroes(Abs(TimeZoneHoursOffset)) <<
        ":" << PrefixZeroes(TimeZoneMinutesOffset) << (DaylightSavings ?
        " (Daylight Savings)" : "");
      return s;
    }

    ///Returns an ISO compatible UTC date-time string.
    String ISO() const
    {
      String s;
      s >> Year << "-" << PrefixZeroes(Month) << "-" << PrefixZeroes(Day) <<
        "T" << PrefixZeroes(Hour) << ":" <<  PrefixZeroes(Minute) << ":" <<
        PrefixZeroes(Second);
      if(TimeZoneHoursOffset == 0 and TimeZoneMinutesOffset == 0)
        s << "Z";
      else
        s << (TimeZonePositiveGMTOffset ? "+" : "-") <<
          PrefixZeroes(Abs(TimeZoneHoursOffset)) <<
          ":" << PrefixZeroes(TimeZoneMinutesOffset);
      return s;
    }

    ///Returns an ISO compatible UTC date-time string.
    String ISOCondensed() const
    {
      String s;
      s >> Year << PrefixZeroes(Month) << PrefixZeroes(Day) <<
        PrefixZeroes(Hour) << PrefixZeroes(Minute) << PrefixZeroes(Second);
      if(TimeZoneHoursOffset == 0 and TimeZoneMinutesOffset == 0)
        s << "Z";
      else
        s << (TimeZonePositiveGMTOffset ? "+" : "-") <<
          PrefixZeroes(Abs(TimeZoneHoursOffset)) <<
          "'" << PrefixZeroes(TimeZoneMinutesOffset) << "'";
      return s;
    }

    ///Returns the date as a string.
    operator String () const
    {
      return Verbose();
    }
  };
}

#ifdef PRIM_COMPILE_INLINE
#if 1 and defined(PRIM_ENVIRONMENT_UNIX_LIKE)
  #include <sys/time.h>
#endif
namespace PRIM_NAMESPACE
{
  integer Time::SecondsSinceUnixEpoch()
  {
    time_t RawTime;
    time(&RawTime);
    return integer(RawTime);
  }

  void Time::SetSecondsSinceUnixEpoch(integer SecondsSinceUnixEpoch)
  {
    //First figure out the time zone if possible.
    TimeZonePositiveGMTOffset = true;
    TimeZoneHoursOffset = 0; //Currently no way to calculate portably.
    TimeZoneMinutesOffset = 0;
    DaylightSavings = false;

    //Get the current time.
    time_t RawTime = time_t(SecondsSinceUnixEpoch);
    tm* ptm;

#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    //Get the local time.
    ptm = localtime(&RawTime);

    //Calculate the time zone offset.
    {
      struct timeval Ignore;
      struct timezone TimeZone;
      gettimeofday(&Ignore, &TimeZone);
      count MinutesEast = -count(TimeZone.tz_minuteswest);
      count Hours = MinutesEast / 60;
      count Minutes = Abs(MinutesEast - (Hours * 60));
      TimeZonePositiveGMTOffset = (TimeZone.tz_minuteswest <= 0);
      TimeZoneHoursOffset = Abs(Hours);
      TimeZoneMinutesOffset = Minutes;
    }

    //Account for daylight savings.
    if((DaylightSavings = ptm->tm_isdst > 0))
    {
      if(TimeZonePositiveGMTOffset)
        TimeZoneHoursOffset++;
      else
        TimeZoneHoursOffset--;
      if(TimeZoneHoursOffset < 0)
      {
        TimeZoneHoursOffset = -TimeZoneHoursOffset;
        TimeZonePositiveGMTOffset = not TimeZonePositiveGMTOffset;
      }
    }
#elif defined(PRIM_ENVIRONMENT_WINDOWS)
    struct tm ptm_struct;
    ptm = &ptm_struct;
    gmtime_s(ptm, &RawTime);
#else
    //Fallback: get UTC time with no time zone or daylight savings information.
    ptm = gmtime(&RawTime);
#endif

    //Save the information in the time structure.
    Year = count(ptm->tm_year + 1900);
    Month = count(ptm->tm_mon + 1);
    Day = count(ptm->tm_mday);
    Hour = count(ptm->tm_hour);
    Minute = count(ptm->tm_min);
    Second = count(ptm->tm_sec);
    Weekday = Weekdays(ptm->tm_wday);
    Yearday = count(ptm->tm_yday);
  }
}
#endif
#endif
