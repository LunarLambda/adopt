#include <string.h>
#include <stdio.h>
#include "adopt.h"

enum
{
    ARG_TYPE_EMPTY,
    ARG_TYPE_NORMAL,
    ARG_TYPE_OPTION,
    ARG_TYPE_END
};

int get_arg_type(const char *arg)
{
    // NULL terminates
    if (!arg)
    {
        return ARG_TYPE_END;
    }

    // Empty gets skipped
    if (!arg[0])
    {
        return ARG_TYPE_EMPTY;
    }

    if (arg[0] == '-' && arg[1])
    {
        // -- terminates
        if (arg[1] == '-' && !arg[2])
        {
            return ARG_TYPE_END;
        }

        // -X
        return ARG_TYPE_OPTION;
    }

    // "-" or anything not starting with '-'
    return ARG_TYPE_NORMAL;
}

// Nonstandard so we provide our own implementation
// Update 02-04-2019: functions starting with str are reserved by the standard library
// and using them is UB. So add a adopt_ prefix.
const char *adopt_strchrnul(const char *str, int ch)
{
    for(; *str && *str != ch; str++);
    return str;
}

// Compares too strings up to at most count characters
// Returns -1 if both strings are equal
// Otherwise, returns number of matching characters at the beginning of lhs and rhs
// Update 02-04-2019: functions starting with str are reserved by the standard library
// and using them is UB. So add a adopt_ prefix.
int adopt_strnmatch(const char *lhs, const char *rhs, int count)
{
    int i = 0;
    for (; lhs[i] == rhs[i] && lhs[i] && i < count; i++);
    return lhs[i] || rhs[i] ? i : -1;
}

// A string        -Xabc matches, in order:
// A long  option  -Xabc exactly
// A long  option  -Xabcd partially, if unambiguous
// A short option  -X with argument abc
//   short options -X, -a, -b, -c
//
// You can disambiguate by using -X=abc or -X abc
const adopt_option *adopt_find_option(const char *name, size_t len, const adopt_option *options, int *_short)
{
    const adopt_option *short_match = NULL;
    const adopt_option * long_match = NULL;
    int amb = 0;

    // Iterate over options[], nullopt has no optchar or name
    for (const adopt_option *opt = options; opt->optchar || opt->name; opt++)
    {
        // Short match.
        if (name[0] == opt->optchar)
        {
            short_match = opt;
        }

        if (opt->name)
        {
            int match = adopt_strnmatch(name, opt->name, len);

            if (match == -1)
            {
                // Exact match
                // Set amb 0 just in case we had an ambiguous match beforehand.
                amb = 0;
                long_match = opt;
                break;
            }
            else if (match == len)
            {
                // Set long_match the first time, otherwise we have an ambiguous match
                if (!long_match)
                {
                    long_match = opt;
                }
                else
                {
                    amb = 1;
                }
            }
        }
    }

    if (len == 1 && short_match)
    {
        *_short = 1;
        return short_match;
    }
    else if (!amb && long_match)
    {
        return long_match;
    }
    else
    {
        // excess argument for short option = return NULL
        if (short_match && !short_match->has_arg && len > 1)
        {
            short_match = NULL;
        }

        *_short = 1; /* This is also the null case, no problem though since short won't be checked then */
        return short_match;
    }
}

#define ADOPT_RETURN(var, _type, _option, _argument, _position, _ret) \
    var->type = _type; \
    var->option = _option; \
    var->argument = _argument; \
    var->position = _position; \
    return (_ret);


#define ADOPT_RETURN_ERROR(var, _error, _argument, _position) \
    var->type = ADOPT_RESULT_ERROR; \
    var->error = _error; \
    var->argument = _argument; \
    var->position = _position; \
    return 1;

int adopt_parse(
    const char           **argv,
    const adopt_option *options,
    adopt_state          *state,
    adopt_result        *result)
{
    // Grab the argument
    const char *a = argv[state->idx];

    if (!state->subidx)
    {
        int     atype = 0;
        while ((atype = get_arg_type(a)) == ARG_TYPE_EMPTY)
        {
            a = argv[++state->idx];
        }

        switch (atype)
        {
            case ARG_TYPE_END:
            {
                ADOPT_RETURN(result, ADOPT_RESULT_FINISHED, NULL, NULL, a ? ++state->idx : state->idx, 0);
            }

            case ARG_TYPE_NORMAL:
            {
                ADOPT_RETURN(result, ADOPT_RESULT_ARGUMENT, NULL, a, state->idx++, 1);
            }

            case ARG_TYPE_OPTION:
            {
                a++; // Skip -
                int _short = 0; // Short option match?

                const         char *end = adopt_strchrnul(a, '=');
                const adopt_option *opt = adopt_find_option(a, end - a, options, &_short);

                if (!opt)
                {
                    ADOPT_RETURN_ERROR(result, ADOPT_ERROR_INVALID_OPTION, --a, state->idx++);
                }

                const char *oarg = NULL;

                if (_short && a[1])
                {
                    oarg = a + 1;   // Skip optchar
                }
                else if (*end)
                {
                    oarg = ++end; // Skip =
                }
                else if (opt->has_arg && get_arg_type(argv[state->idx + 1]) != ARG_TYPE_END)
                {
                    oarg = argv[++state->idx];
                }

                if (opt->has_arg == ADOPT_ARGUMENT_REQUIRED && !oarg)
                {
                    ADOPT_RETURN_ERROR(result, ADOPT_ERROR_MISSING_ARGUMENT, --a, state->idx++);
                }

                if (!opt->has_arg && oarg)
                {
                    ADOPT_RETURN_ERROR(result, ADOPT_ERROR_EXCESS_ARGUMENT, --a, state->idx++);
                }

                ADOPT_RETURN(result, ADOPT_RESULT_OPTION, opt, oarg, state->idx++, 1);
            }
        }
    }
    else
    {
        // TODO: Condensed Option Parsing
    }

    return 0;
}
