from __future__ import print_function, unicode_literals
import re
import time
import sys
from collections import Counter


def print_chart(counter):
    min_idx = 0
    min_val = 0
    max_idx = 0
    max_val = 0

    if not counter:
        print("No results found")
        sys.exit(1)

    sort_idx = sorted(counter)
    min_idx = sort_idx[0]
    min_val = counter[sort_idx[0]]

    print("")

    for i in sort_idx:
        c = counter[i]
        if c < min_val:
            min_idx = i
            min_val = c

        if c > max_val:
            max_idx = i
            max_val = c

    multiplier = 45.0 / max_val
    total = 0
    for i in sort_idx:
        c = counter[i]
        stars = multiplier * c
        print(i, '*' * (1+int(stars)), end=' ')
        print(c)
        total += c

    print('-' * 45)
    print('max: datetime=%s count=%s' % (max_idx, max_val))
    print('min: datetime=%s count=%s' % (min_idx, min_val))
    print("avg: %s" % (total/len(counter)))
    print("total:", total)
    print("")


def usage():
    print("Usage:")
    print("   logtime <grouping> <timepattern> <timespec> < FILENAME")
    print("")
    print("Example Usage:")
    print(" Apache logs grouped by hour")
    print("   grep FOO /var/log/apache2/access.log | \\")
    print("       logtime H '\[(.+?) ' '%d/%b/%Y:%H:%M:%S'")
    print("")
    print("   logtime H '\[(.+?) ' '%d/%b/%Y:%H:%M:%S' < /path/file")
    print("")
    print("Grouping spec:")
    print("   H - hour")
    print("   M - Minute")
    print("   S - Second")
    print("   y - year")
    print("   m - month")
    print("   d - day")
    print("")
    print("Timespecs:")
    print("   Directive   Meaning")
    print("   %a  Locale's abbreviated weekday name.")
    print("   %A  Locale's full weekday name.")
    print("   %b  Locale's abbreviated month name.")
    print("   %B  Locale's full month name.")
    print("   %c  Locale's appropriate date and time representation")
    print("   %d  Day of the month as a decimal number [01,31]")
    print("   %H  Hour (24-hour clock) as a decimal number [00,23]")
    print("   %I  Hour (12-hour clock) as a decimal number [01,12]")
    print("   %j  Day of the year as a decimal number [001,366]")
    print("   %m  Month as a decimal number [01,12]")
    print("   %M  Minute as a decimal number [00,59].")
    print("   %p  Locale's equivalent of either AM or PM.")
    print("   %S  Second as a decimal number [00,61].")
    print("   %U  Week number of the year (Sunday as the first day of the")
    print("       week) as a decimal number [00,53].")
    print("   %w  Weekday as a decimal number [0(Sunday),6]")
    print("   %W  Week number of the year (Monday as the first day of the")
    print("       week) as a decimal number [00,53].")
    print("   %x  Locale's appropriate date representation")
    print("   %X  Locale's appropriate time representation")
    print("   %y  Year without century as a decimal number [00,99]")
    print("   %Y  Year with century as a decimal number")
    print("   %Z  Time zone name (no characters if no time zone exists)")
    print("   %%  A literal '%' character.")
    print("")
    print("Important notes:")
    print("   Regex is Python regex re.findall()")
    print("   Timepsec is Python's time module")
    print("")
    sys.exit(1)


def main():
    counter = Counter()
    try:
        period = sys.argv[1]
        pattern = sys.argv[2]
        timespec = sys.argv[3]
    except:
        usage()

    try:
        pc = re.compile(pattern)
    except:
        print('ERROR: Unable to compile regex pattern')
        sys.exit(1)

    for line in sys.stdin:
        matches = pc.findall(line)
        if not matches: continue
        try:
            tstruct = time.strptime(matches[0], timespec)
            tstring = time.strftime("%Y%m%d%H%M%S", tstruct)
        except:
            print("Failed to convert match using timespec")
            print("Match is:", matches)
            print("Timespec is:", timespec)
            sys.exit(1)

        if period == 'H':
            tstring = tstring[:10]
        elif period == 'M':
            tstring = tstring[:12]
        elif period == 'S':
            tstring = tstring
        elif period == 'd':
            tstring = tstring[:8]
        elif period == 'm':
            tstring = tstring[:6]
        elif period == 'y':
            tstring = tstring[:4]
        else:
            print("ERROR: Unknown period type %s" % (period))
            sys.exit(1)

        counter[tstring] += 1

    print_chart(counter)

if __name__ == "__main__":
    main()

