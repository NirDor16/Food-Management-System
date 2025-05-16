#ifndef MYMACROS_H
#define MYMACROS_H

#define CHECK_RETURN_0(p) if (!(p)) return 0;

#define CHECK_MSG_RETURN_0(p){ if(!(p)){ printf("Error in open file\n"); return 0;}}

#define FREE_CLOSE_FILE_RETURN_0(ptr,p){ free(ptr); fclose(p); return 0; }

#define CLOSE_RETURN_0(p){	fclose(p);	return 0;}

#endif


