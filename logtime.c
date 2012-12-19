#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <getopt.h>
#include <ctype.h>

/* Store the date, time, and associated count */
struct logtime {
    unsigned int datetime;
    unsigned long int count;
    struct logtime *next;
} *head = NULL, *tail = NULL;

/* options set and their pointers to argv */
int opt_S = 0, opt_M = 0, opt_H = 0, opt_D = 0; /* mutually exclusive */
int opt_v = 0, opt_i = 0, opt_x = 0, opt_t = 0;
int group_format = 0;
char *opt_i_p = NULL, *opt_x_p = NULL, *opt_t_p = NULL;

/* map short month names to int indexes+1 for integer conversion */
static char *short_month_to_int_map[] = {
    "jan", "feb", "mar", "apr", "may", "jun",
    "jul", "aug", "sep", "oct", "nov", "dec",
};
static const size_t short_month_to_int_map_count = sizeof(short_month_to_int_map)/sizeof(*short_month_to_int_map);

/* rbuf must be of min size 3 */
int short_month_to_int_string(const char *s, char *rbuf)
{
    int i = 0;
    for(; i < short_month_to_int_map_count; i++) {
        if(strncasecmp(s, short_month_to_int_map[i], 3) == 0) {
            i += 1; /* since months start at 1 */
            if(i < 10) {
                sprintf(rbuf, "0%d", i);
            } else {
                sprintf(rbuf, "%d", i);
            }
            return i;
        }
    }
    return 0;
}

void usage()
{
    printf("Usage: logtime [-M|-H|-D] [FILE <FILE...>]\n"
    "   <-h> <-v>\n"
    "   <-i REGEX> <-x REGEX>\n"
    "   <-t TIMEFORMAT>\n"
    "\n"
    "Parse the date and time from log files and print an ASCII graph of the\n"
    "occurrences.\n"
    "\n"
    "Required arguments:\n"
    "    -S, --second        Group by second\n"
    "    -M, --minute        Group by minute\n"
    "    -H, --hour          Group by hour\n"
    "    -D, --day           Group by day\n"
    "    FILE...             A list of files to read as input\n"
    "\n"
    "Optional arguments:\n"
    "    -h, --help          Show this help message and exit\n"
    "    -i REGEX, --include REGEX\n"
    "                        Include lines that match this pattern\n"
    "    -x REGEX, --exclude REGEX\n"
    "                        Exclude lines that match this pattern\n"
    "    -t TIMEFORMAT, --time-format TIMEFORMAT\n"
    "                        Describe the time format to match on\n"
    "    -v, --verbose       Print status messages while running\n");
    fflush(stderr);
    fflush(stdout);
    exit(1);
}

struct logtime *lt_new()
{
    struct logtime *lt = malloc(sizeof(*lt));
    if(!lt) {
        perror("malloc");
        exit(1);
    }
    lt->datetime = 0;
    lt->count = 0;
    lt->next = NULL;
    return lt;
}

/* takes a string in the format of YYYYMMDDhhiiss and converts it
 * to an integer based of of format (how much to truncate) 
 * S=all, M=month, H=hour, D=day
 */
long long strtime_to_ll(const char *s, int format)
{
    char *p = (char *)s;
    switch(format) {
        case 'S':
            break;
        case 'M':
            p[12] = '\0';
            break;
        case 'H':
            p[10] = '\0';
            break;
        case 'D':
            p[8] = '\0';
            break;
        default:
            fprintf(stderr, "error: invalid time format code -- this should not happen\n");
            exit(1);
    }
    return atoll(p);
}

int get_time_from_line(const char *line, char *rbuf)
{
    int i = 0, x = 0;
    int buf_pos = 0;

    for(i = 0; i < strlen(line); i++) {
        x = line[i];
        if(isdigit(x)) {
            rbuf[buf_pos] = x;
            buf_pos++;

        } else if(x == ':') {
            /* nop */
        } else {
            /* we gots it all*/
            if(buf_pos > 5)
                break;
            buf_pos = 0;
        }
    }
    rbuf[buf_pos] = '\0';
    return buf_pos;
}

void parse_log(const char *log_path)
{
    printf("OPEN: %s\n", log_path);
    FILE *f = fopen(log_path, "r");
    char *b = malloc(sizeof(*b)*1048576+1); /* 1MB buffer is large for a single log line */
    char *p;
    regex_t regxs[12];
    char buf[15];
    regmatch_t matches[1];
    char year_buf[3];
    int counter = 0, rc;
    char *sp1, *token;

    if(!f) {
        perror("fopen");
        exit(1);
    }
    if(!b) {
        perror("malloc failed to allocated 1MiB line buffer");
        fclose(f);
        exit(1);

    }

    /* Compile some common time formats to look for
     * most common: Dec 1 04:25:01
     * apache common: 12/Dec/2012:23:59:56
     * apache common: Thu Dec 13 23:43:10 2012 */
    if(regcomp(&regxs[0],
        "^[A-Za-z][a-z][a-z] {1,2}[0-9]{1,2} [0-9][0-9]:[0-5][0-9]:[0-5][0-9]:? ", REG_EXTENDED)) {
        fprintf(stderr, "error: failed to compile regex\n");
        exit(1);
    }
    if(regcomp(&regxs[1],
        "[0-9]{1,2}/[A-Za-z][a-z][a-z]/[0-9][0-9][0-9][0-9]:[0-9][0-9]:[0-5][0-9]:[0-5][0-9]", REG_EXTENDED)) {
        fprintf(stderr, "error: failed to compile regex\n");
        exit(1);
    }
    if(regcomp(&regxs[2],
        "[A-Za-z][a-z][a-z] [0-9]{1,2} [0-9][0-9]:[0-5][0-9]:[0-5][0-9] [0-9][0-9][0-9][0-9] ", REG_EXTENDED)) {
        fprintf(stderr, "error: failed to compile regex\n");
        exit(1);
    }

    /* we are munging everything into a YYYYMMDDhhiiss string that later is converted to an int */
    memset(buf, '0', sizeof(buf));
    buf[14] = '\0';
    while((p = fgets(b, 1048576, f))) {
        /* do parse */
        
        //printf("%d -- ", get_time_from_line(p, buf));
        if(regexec(&regxs[0], p, 1, matches, 0) == 0) {
            /* sep=space, month(as word), day, time */
            p = p+matches[0].rm_so;
            p[matches[0].rm_eo] = '\0';
            counter = 1;
            printf("line: %s\n", p);
            while((token = strtok_r(p, " ", &sp1))) {
                switch(counter) {
                    /* month */
                    case 1:
                        rc = short_month_to_int_string(token, year_buf);        
                        //printf("set year: \"%d -- %s\"\n", rc, year_buf);
                        buf[4] = year_buf[0];
                        buf[5] = year_buf[1];
                        break;
                    /* day */
                    case 2:
                        if(strlen(token) < 2) {
                            buf[6] = '0';
                            buf[7] = token[0];
                        } else {
                            buf[6] = token[0];
                            buf[7] = token[1];
                        }
                        //printf("set day: %c%c\n", buf[6], buf[7]);
                        break;
                    /* time */
                    case 3:
                        buf[8] = token[0];
                        buf[9] = token[1];
                        buf[10] = token[3];
                        buf[11] = token[4];
                        buf[12] = token[6];
                        buf[13] = token[7];
                        //printf("set time: %s\n", buf+8);
                        //printf("set date/all: %s\n", buf);
                        break;
                }
                
                p = sp1;
                counter++;
            }
            long long t = strtime_to_ll(buf, 'S');
            printf("strtime_to_llS: %lld\n", strtime_to_ll(buf, 'S'));
            printf("strtime_to_llM: %lld\n", strtime_to_ll(buf, 'M'));
            printf("strtime_to_llH: %lld\n", strtime_to_ll(buf, 'H'));
            printf("strtime_to_llD: %lld\n", strtime_to_ll(buf, 'D'));
        } else if(regexec(&regxs[1], p, 1, matches, 0) == 0) {
            /* sep=space, month(as word), day, time */
            p = p+matches[0].rm_so;
            p[matches[0].rm_eo] = '\0';
            counter = 1;
            printf("line: %s\n", p);
            /* 12/Dec/2012:23:59:55 */
            while((token = strtok_r(p, "/", &sp1))) {
                switch(counter) {
                    /* month */
                    case 2:
                        rc = short_month_to_int_string(token, year_buf);        
                        //printf("set year: \"%d -- %s\"\n", rc, year_buf);
                        buf[4] = year_buf[0];
                        buf[5] = year_buf[1];
                        break;
                    /* day */
                    case 1:
                        if(strlen(token) < 2) {
                            buf[6] = '0';
                            buf[7] = token[0];
                        } else {
                            buf[6] = token[0];
                            buf[7] = token[1];
                        }
                        printf("set day: %c%c\n", buf[6], buf[7]);
                        break;
                    /* time */
                    case 3:
                        buf[0] = token[0];
                        buf[1] = token[1];
                        buf[2] = token[2];
                        buf[3] = token[3];
                        buf[8] = token[5];
                        buf[9] = token[6];
                        buf[10] = token[8];
                        buf[11] = token[9];
                        buf[12] = token[11];
                        buf[13] = token[12];
                        //printf("set time: %s\n", buf+8);
                        //printf("set date/all: %s\n", buf);
                        break;
                }
                
                p = sp1;
                counter++;
            }
            long long t = strtime_to_ll(buf, 'S');
            printf("strtime_to_llS: %lld\n", strtime_to_ll(buf, 'S'));
            printf("strtime_to_llM: %lld\n", strtime_to_ll(buf, 'M'));
            printf("strtime_to_llH: %lld\n", strtime_to_ll(buf, 'H'));
            printf("strtime_to_llD: %lld\n", strtime_to_ll(buf, 'D'));

        } else {
            printf("NOMATCH: %s", p);
        }
    }
    fclose(f);
}

/** main sets options and calls the parsing function **/
int main(int argc, char **argv)
{
    int c, option_index;

    static struct option long_options[] = {
        {"verbose",     0, 0, 'v'},  
        {"help  ",      0, 0, 'h'},
        {"second",      0, 0, 'S'},
        {"minute",      0, 0, 'M'},
        {"hour",        0, 0, 'H'},
        {"day",         0, 0, 'D'},
        {"include",     1, 0, 'i'},
        {"exclude",     1, 0, 'e'},
        {"time-format", 1, 0, 't'},
    };
    while(1) {
        c = getopt_long(argc, argv, "hvSMHDi:e:t:", long_options, &option_index);
        /* end of opts */
        if(c == -1)
            break;
        switch(c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
               if (long_options[option_index].flag != 0)
                 break;
               printf ("option %s", long_options[option_index].name);
               if (optarg)
                 printf (" with arg %s", optarg);
               printf ("\n");
               break;
            case 'h': usage();
            case 'S': opt_S = 1;
                group_format = 'S';
                break;
            case 'M': opt_M = 1;
                group_format = 'M';
                break;
            case 'H': opt_H = 1;
                group_format = 'H';
                break;
            case 'D': opt_D = 1;
                group_format = 'D';
                break;
            case 'v': opt_v = 1;
                break;
        }
    }
    if(opt_S + opt_M + opt_H + opt_D != 1) {
        fprintf(stderr, "error: one of options S,M,H, or D must be set\n");
        usage();
    }
    if(optind < argc) {
        while(optind < argc) {
            if(opt_v)
                printf("parsing: %s\n", argv[optind]);
            parse_log(argv[optind]);
            optind++;
        }
    } else {
        fprintf(stderr, "error: no log file(s) specified\n");
        usage();
    }
    fflush(stderr);
    fflush(stdout);
    return 0;
}

