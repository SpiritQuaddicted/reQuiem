// version.h

#define SERVER_VERSION		1.09
//#define SERVER_VERSION_PLUS 1.10

#ifdef HEXEN2_SUPPORT
  #define SERVER_VERSION_H2   1.12
#endif

#define REQUIEM_VERSION_SHORT               "0.95"
#define	REQUIEM_VERSION	REQUIEM_VERSION_SHORT"-b2"

int build_number (void);
void Host_Version_f (cmd_source_t src);
char *VersionString (void);
