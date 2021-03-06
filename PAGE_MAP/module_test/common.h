#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

//typedef signed long long int64_t;

#define SUCCESS	1
#define FAIL	0

#define VALID	50
#define INVALID	51

#define EMPTY_BLOCK		33
#define DATA_BLOCK		34

#define CHANNEL_IS_EMPTY 700
#define CHANNEL_IS_WRITE 701
#define CHANNEL_IS_READ  702
#define CHANNEL_IS_ERASE 703

#define REG_IS_EMPTY 705
#define REG_IS_WRITE 706
#define REG_IS_READ  707
#define REG_IS_ERASE 708

#define WRITE 801
#define READ  802
#define ERASE 803

/* FTL Debugging */
#define FTL_DEBUG
//#define DEBUG_MODE1
#define MNT_DEBUG
#define MONITOR_ON
#define GC_ON

//#define REMAIN_WRITE_DELAY // Remaining delay

/* SSD Debugging */
//#define SSD_DEBUG
#define SSD_CH_SWITCH_DELAY_R
#define SSD_CH_SWITCH_DELAY_W

//#define SSD_ERASE_ON

//#define SSD_SYNC

#define SPO_MONITOR
//#define FTL_BUFFER_ON
//#define SSD_BUFFER_ON
#define FTL_MAP_CACHE
#define FTL_MAP_CACHE_DEBUG

#define FTL_BUFFER_DEBUG


#endif

