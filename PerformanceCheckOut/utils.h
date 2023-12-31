#pragma once
#include <stdio.h>

#define CHECK(condition, ret, errorText, ...){												    \
	if (!(condition)){																		    \
		printf("Error at line %d: %s; Error code: %d \n", __LINE__, errorText, GetLastError());	\
        __VA_ARGS__;																		    \
        return (ret);																		    \
	}																						    \
}
