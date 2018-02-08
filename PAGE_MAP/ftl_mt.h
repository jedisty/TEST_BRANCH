// Copyright(c)2013 
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#ifndef _FTL_MT_H_
#define _FTL_MT_H_

#include "common.h"

void FTL_MT_INIT(void);
void FTL_MT_TERM(void);

void FTL_MT_READ(int32_t sector_nb, unsigned int length);
void FTL_MT_WRITE(int32_t sector_nb, unsigned int length);

int _FTL_MT_READ(int32_t sector_nb, unsigned int length);
int _FTL_MT_WRITE(int32_t sector_nb, unsigned int length);
#endif
