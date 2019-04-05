#include <stdio.h>
#include "adopt.h"

int adopt_parse(
    const char           **argv,
    const adopt_option *options,
    adopt_state          *state,
    adopt_result        *result);

int main(int argc, const char *argv[])
{
    argc--; argv++; // Skip progname

    // I'm very split about this.
    // Here's some considerations I've made:
    // Move adopt_result.type out of the struct and make it the return type instead
    // Merge adopt_state and adopt_result (both are strictly read-only on the user side)
    // Not sure what I'd call that though.
    // adopt_result.position is in "sync" with adopt_state.idx - 1
    adopt_state   _s = {0};
    adopt_result res = {0};

    adopt_option opts[] =
    {
    //  { id,  optchar, name         , description, has_arg                 },
        {  1,  0      , "ctest"      , 0          , ADOPT_ARGUMENT_NONE     },
        {  2, 'c'     , "create"     , 0          , ADOPT_ARGUMENT_NONE     },
        {  3, 'f'     , "create-file", 0          , ADOPT_ARGUMENT_NONE     },
        {  4, 'd'     , "delete"     , 0          , ADOPT_ARGUMENT_NONE     },
        {  5, 'x'     , "cut"        , 0          , ADOPT_ARGUMENT_NONE     },
        {  6,  0      , "xfile-debug", 0          , ADOPT_ARGUMENT_NONE     },
        {  7, 'X'     , "Xhello"     , 0          , ADOPT_ARGUMENT_OPTIONAL },
        {  8, 'D'     , "define"     , 0          , ADOPT_ARGUMENT_OPTIONAL },
        {  9, 'g'     ,  NULL        , 0          , ADOPT_ARGUMENT_OPTIONAL },
        { 10, 'G'     , "global"     , 0          , ADOPT_ARGUMENT_NONE     },
        { 11,  0      , "file"       , 0          , ADOPT_ARGUMENT_REQUIRED },
        { 12,  0      , "help"       , 0          , ADOPT_ARGUMENT_NONE     },
    //  { 13, '-'     , "-lol"       , 0          , ADOPT_ARGUMENT_OPTIONAL },
        {  0                                                                },
    //  ADOPT_NULL_OPTION, alternatively.
    };

    // Returns 0 when it's done
    // TODO: Think about if to expose option in the error case.
    while (adopt_parse(argv, opts, &_s, &res))
    {
        switch (res.type)
        {
            case ADOPT_RESULT_ARGUMENT:
                printf("[%2i] Got argument %s\n", res.position, res.argument);
                break;

            case ADOPT_RESULT_OPTION:
                // TODO: Add a built-in help function which pretty prints the whole options array.
                if (res.option->id == 12 /* help */)
                {
                    for (const adopt_option *opt = opts; opt->optchar || opt->name; opt++)
                    {
                        if (opt->optchar)
                        {
                            printf("-%c", opt->optchar);
                        }
                        else
                        {
                            printf("  ");
                        }

                        if (opt->name)
                        {
                            printf("%s-%-15s", opt->optchar ? ", " : "  ", opt->name);
                        }
                        else
                        {
                            printf("%-17s", "");
                        }

                        printf("%s\n", opt->description ? opt->description : "");
                    }

                    return 0;
                }

                printf("[%2i] Got option %s with argument %s.\n", res.position, res.option->name, res.argument);
                break;

            case ADOPT_RESULT_ERROR:
            {
                switch (res.error)
                {
                    case ADOPT_ERROR_INVALID_OPTION:
                        printf("[%2i] Got invalid option %s.\n", res.position, res.argument);
                        break;

                    case ADOPT_ERROR_AMBIGUOUS_OPTION:
                        printf("[%2i] option %s is ambiguous.\n", res.position, res.argument);
                        break;

                    case ADOPT_ERROR_MISSING_ARGUMENT:
                        printf("[%2i] option %s is missing an argument.\n", res.position, res.argument);
                        break;

                    case ADOPT_ERROR_EXCESS_ARGUMENT:
                        printf("[%2i] excess argument for option %s.\n", res.position, res.argument);
                        break;
                }
            }
        }
    }

    printf("\nScan ended at position %i.\nRemaining arguments:\n", res.position);
    for (int i = res.position; i < argc; i++)
    {
        printf("[%2i] %s\n", i, argv[i]);
    }

    return 0;
}
