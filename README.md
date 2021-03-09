logtime
============

Parse the date and time from log files and print an ASCII graph of the
occurrences.  Time can be grouped by minute, hour or day.

matth-at-mtingers.com

Usage
-----
    Usage:
         logtime <grouping> <timepattern> <timespec> < FILENAME

    Example Usage:
       Apache logs grouped by hour
         grep FOO /var/log/apache2/access.log | \
             logtime H '\[(.+?) ' '%d/%b/%Y:%H:%M:%S'
  
         logtime H '\[(.+?) ' '%d/%b/%Y:%H:%M:%S' < /path/file

    Grouping spec:
         H - hour
         M - Minute
         S - Second
         y - year
         m - month
         d - day

    Timespecs:
         Directive   Meaning
         %a  Locale's abbreviated weekday name.
         %A  Locale's full weekday name.
         %b  Locale's abbreviated month name.
         %B  Locale's full month name.
         %c  Locale's appropriate date and time representation
         %d  Day of the month as a decimal number [01,31]
         %H  Hour (24-hour clock) as a decimal number [00,23]
         %I  Hour (12-hour clock) as a decimal number [01,12]
         %j  Day of the year as a decimal number [001,366]
         %m  Month as a decimal number [01,12]
         %M  Minute as a decimal number [00,59].
         %p  Locale's equivalent of either AM or PM.
         %S  Second as a decimal number [00,61].
         %U  Week number of the year (Sunday as the first day of the
             week) as a decimal number [00,53].
         %w  Weekday as a decimal number [0(Sunday),6]
         %W  Week number of the year (Monday as the first day of the
             week) as a decimal number [00,53].
         %x  Locale's appropriate date representation
         %X  Locale's appropriate time representation
         %y  Year without century as a decimal number [00,99]
         %Y  Year with century as a decimal number
         %Z  Time zone name (no characters if no time zone exists)
         %%  A literal '%' character.

    Important notes:
         Regex is Python regex re.findall()
         Timepsec is Python's time module
