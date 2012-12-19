logtime
============

Parse the date and time from log files and print an ASCII graph of the
occurrences.  Time can be grouped by minute, hour or day.

matth-at-mtingers.com

Usage
-----

    Usage: logtime [-M|-H|-D] [FILE1 <FILE2...>]
                           <-h> <-i REGEX> <-x REGEX>

    Parse the date and time from log files and print an ASCII graph of the
    occurrences.

    Optional arguments:
        -h, --help          Show this help message and exit

        -i REGEX, --include REGEX
                            Include lines that match this pattern
        
        -x REGEX, --exclude REGEX
                            Exclude lines that match this pattern
        
        -t TIMEFORMAT, --exclude TIMEFORMAT
                            Describe the time format to match on

        -v, --verbose       Print status messages while running

Example:

Calculate the amount of POST operations per hour in an a Apache log file.

`logtime -H -p ' POST ' /var/log/apache2/access_log`


Time Format
-----------
Time formats are very primitive in this version.
    Y = year part
    M = month part
    D = day part
    h = hour part
    i = minute part
    s = second part

An example format to match on "20121212 01:22:13":
    `YYYYMMDD hh:ii:ss`



