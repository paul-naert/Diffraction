#ifndef TEXT_FILE_H
#define TEXT_FILE_H

// textfile.h: interface for reading and writing text files
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//////////////////////////////////////////////////////////////////////

// Prototypes des fonctions
char* textFileRead(const char* fn);
int   textFileWrite(const char* fn, const char* s);

#endif
