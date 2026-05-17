#include <stdio.h>
#include <time.h>
#include "logger.h"


static log_level CURRENT_LEVEL = LOG_DEBUG;
// use macros to use this function...
void log_msg(log_level level, const char * msg){
    if(level < CURRENT_LEVEL) return;
    static int is_first_call = 1;
    FILE * file = fopen("log.log", "a");

    // write date and time only the first time...
    if(is_first_call){
	time_t rawtime;
	time(&rawtime); 
	fprintf(file, "Date and Time : %s\n", ctime(&rawtime));
	is_first_call = 0;
    }

    switch(level){
	case LOG_BREAK:
	    fprintf(file, "%s", msg);
	    break;
	case LOG_DEBUG:
	    fprintf(file, "[DEBUG]: %s\n", msg);
	    break;
	case LOG_INFO:
	    fprintf(file, "[INFO]: %s\n", msg);
	    break;
	case LOG_WARN:
	    fprintf(file, "[WARN]: %s\n", msg);
	    break;
	case LOG_ERROR:
	    fprintf(file, "[ERROR]: %s\n", msg);
	    break;
	case LOG_FATAL:
	    fprintf(file, "[FATAL]: %s\n", msg);
	    break;
	default:
	    fprintf(file, "<Unidentified Logging Level>\n");
	    //invalid logging level
    }

    fclose(file);
    return;
}
