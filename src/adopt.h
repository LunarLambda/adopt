#pragma once
#define ADOPT_VERSION "0.1"
#define ADOPT_MESSAGE "Adopt " ADOPT_VERSION " made with <3 by LunarLambda"

typedef enum
{
    ADOPT_ARGUMENT_NONE,
    ADOPT_ARGUMENT_REQUIRED,
    ADOPT_ARGUMENT_OPTIONAL,
} adopt_argument;

typedef enum
{
    ADOPT_RESULT_FINISHED,
    ADOPT_RESULT_OPTION,
    ADOPT_RESULT_ARGUMENT,
    ADOPT_RESULT_ERROR,
} adopt_result_type;

typedef enum
{
    ADOPT_ERROR_INVALID_OPTION   = -1,
    ADOPT_ERROR_AMBIGUOUS_OPTION = -2,
    ADOPT_ERROR_MISSING_ARGUMENT = -3,
    ADOPT_ERROR_EXCESS_ARGUMENT  = -4,
} adopt_error;

typedef struct
{
    int                  id;
    char            optchar;
    const char        *name;
    const char *description;
    adopt_argument  has_arg;
} adopt_option;

#define ADOPT_NULL_OPTION (adopt_option){ 0, 0, 0, 0, 0 }

typedef struct
{
    adopt_result_type type;

    union
    {
        const adopt_option *option;
        adopt_error          error;
    };

    int         position;   // Position in argv[]
    const char *argument;   // Argument to option or non-option argument
} adopt_result;

typedef struct
{
    int idx, subidx;
} adopt_state;
