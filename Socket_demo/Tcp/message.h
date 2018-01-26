#ifndef __MESSAGES_H
#define __MESSAGES_H

#define BUFSIZE 512

#if 0 //method one
typedef struct _msgstr {
	int number;
	char value[32];
}MSGSTR;
#endif

#if 0 //method two
typedef struct _msgstr {
	int number;
	char *value;
}MSGSTR;
#endif


#if 1 //method two
typedef struct _msgstr {
	unsigned int type;
	unsigned int len;
	char value[0];
}MSGSTR;
#endif


#endif

