
#define GENERIC_ERROR_HELPER(cond, errCode, msg)     do {           	\
        if (cond) {                                                 	\
            fprintf(stderr, "%s: %d\n", msg, GetLastError());	\
            exit(EXIT_FAILURE);                                     	\
        }                                                           	\
    } while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
