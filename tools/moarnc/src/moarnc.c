#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "moarApi.h"
#include <getopt.h>

int main (int argc, char **argv)  {
    /* Flag set by ‘--verbose’. */
    static int verbose = 0;

    static struct option long_options[] = {
            {"verbose", no_argument,      &verbose, 1}, // verbosity mode
            {"socket",  required_argument, 0, 's'}, //custom socket
            {"appid",   required_argument, 0, 'p'}, //Remote AppId to send data to
            {"remote",  required_argument, 0, 'h'}, //Remore Address
            {"listen",  required_argument, 0, 'l'},  // Listening mode; argument is own AppId
            {0, 0, 0, 0}
    };
    int c;
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long (argc, argv, "s:p:h:l:v", long_options, &option_index);
        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'v':
                verbose = true;
                puts ("option -v\n");
                break;
            case 's':
                printf ("option -s with value `%s'\n", optarg);
                break;
            case 'p':
                printf ("option -p with value `%s'\n", optarg);
                break;
            case 'h':
                printf ("option -h with value `%s'\n", optarg);
                break;
            case 'l':
                printf ("option -l with value `%s'\n", optarg);
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort ();
        }
    }

    if (verbose)
        puts ("verbose flag is set");

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

    return EXIT_SUCCESS;
}
