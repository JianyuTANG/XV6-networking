//Shell history

#ifndef HISTORY_H
#define HISTORY_H


#define H_NUMBER 11
#define H_LENGTH 128

struct history
{
    int curcmd;
    int lastcmd; // 指最新的cmd应该放的位置 该位置是空
    int length;
    char record[H_NUMBER][H_LENGTH];
};


#endif
