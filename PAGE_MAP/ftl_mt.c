// Copyright(c)2013 
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#include "common.h"

int g_init_mt = 0;
extern double ssd_util;

void FTL_MT_INIT(void)
{
	if(g_init_mt == 0){
        	printf("[FTL_MT_INIT] start\n");

		INIT_SSD_CONFIG();

		INIT_MAPPING_TABLE();
		INIT_INVERSE_PAGE_MAPPING();
		INIT_INVERSE_BLOCK_MAPPING();
		INIT_VALID_ARRAY();
		INIT_EMPTY_BLOCK_LIST();
		INIT_VICTIM_BLOCK_LIST();
		INIT_PERF_CHECKER();
		
		g_init_mt = 1;

		SSD_MT_IO_INIT();
		printf("[FTL_MT_INIT] complete\n");
	}
}

void FTL_MT_TERM(void)
{
	printf("[FTL_MT_TERM] start\n");

	TERM_MAPPING_TABLE();
	TERM_INVERSE_PAGE_MAPPING();
	TERM_VALID_ARRAY();
	TERM_INVERSE_BLOCK_MAPPING();
	TERM_EMPTY_BLOCK_LIST();
	TERM_VICTIM_BLOCK_LIST();
	TERM_PERF_CHECKER();

	printf("[FTL_MT_TERM] complete\n");
}

void FTL_MT_READ(int32_t sector_nb, unsigned int length)
{
	int ret;

	ret = _FTL_MT_READ(sector_nb, length);
}

void FTL_MT_WRITE(int32_t sector_nb, unsigned int length)
{
	int ret;

	ret = _FTL_MT_WRITE(sector_nb, length);
}

int _FTL_MT_READ(int32_t sector_nb, unsigned int length)
{
#ifdef FTL_DEBUG
	printf("[FTL_READ] Start\n");
#endif

	if(sector_nb + length > SECTOR_NB){
		printf("Error[FTL_READ] Exceed Sector number\n"); 
		return FAIL;	
	}

	int32_t lpn;
	int32_t ppn;
	int32_t lba = sector_nb;
	unsigned int remain = length;
	unsigned int left_skip = sector_nb % SECTORS_PER_PAGE;
	unsigned int right_skip;
	unsigned int read_sects;

	unsigned int ret = FAIL;
	int read_page_nb = 0;
	int io_page_nb;

	while(remain > 0){

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}
		read_sects = SECTORS_PER_PAGE - left_skip - right_skip;

		lpn = lba / (int32_t)SECTORS_PER_PAGE;
		ppn = GET_MAPPING_INFO(lpn);

		if(ppn == -1){
			return FAIL;
		}

		lba += read_sects;
		remain -= read_sects;
		left_skip = 0;
	}

	remain = length;
	lba = sector_nb;
	left_skip = sector_nb % SECTORS_PER_PAGE;

	while(remain > 0){

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}
		read_sects = SECTORS_PER_PAGE - left_skip - right_skip;

		lpn = lba / (int32_t)SECTORS_PER_PAGE;

		ppn = GET_MAPPING_INFO(lpn);

		if(ppn == -1){
#ifdef FTL_DEBUG
			printf("Error[FTL_READ] No Mapping info\n");
#endif
		}

		if(remain <= SECTORS_PER_PAGE){
			F_O_DIRECT_VSSIM_MT = 1;
		}

		ret = SSD_PAGE_READ_MT(CALC_FLASH(ppn), CALC_BLOCK(ppn), CALC_PAGE(ppn));

#ifdef FTL_DEBUG
		if(ret == SUCCESS){
			printf("\t read complete [%u]\n",ppn);
		}
		else if(ret == FAIL){
			printf("Error[FTL_READ] %u page read fail \n", ppn);
		}
#endif
		read_page_nb++;

		lba += read_sects;
		remain -= read_sects;
		left_skip = 0;
	}

	F_O_DIRECT_VSSIM_MT = -1;

#ifdef FTL_DEBUG
	printf("[FTL_READ] Complete\n");
#endif

	return ret;
}

int _FTL_MT_WRITE(int32_t sector_nb, unsigned int length)
{
#ifdef FTL_DEBUG
	printf("[FTL_WRITE] Start\n");
#endif

	int io_page_nb;

	if(sector_nb + length > SECTOR_NB){
		printf("Error[FTL_WRITE] Exceed Sector number\n");
                return FAIL;
        }

	int32_t lba = sector_nb;
	int32_t lpn;
	int32_t new_ppn;
	int32_t old_ppn;

	unsigned int remain = length;
	unsigned int left_skip = sector_nb % SECTORS_PER_PAGE;
	unsigned int right_skip;
	unsigned int write_sects;

	unsigned int ret = FAIL;
	int write_page_nb=0;

	while(remain > 0){

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}

		write_sects = SECTORS_PER_PAGE - left_skip - right_skip;


		ret = GET_NEW_PAGE(VICTIM_OVERALL, EMPTY_TABLE_ENTRY_NB, &new_ppn);
		if(ret == FAIL){
			printf("ERROR[FTL_WRITE] Get new page fail \n");
			return FAIL;
		}

		if(remain <= SECTORS_PER_PAGE){
			F_O_DIRECT_VSSIM_MT = 1;
		}

		lpn = lba / (int32_t)SECTORS_PER_PAGE;
		old_ppn = GET_MAPPING_INFO(lpn);

		ret = SSD_PAGE_WRITE_MT(CALC_FLASH(new_ppn), CALC_BLOCK(new_ppn), CALC_PAGE(new_ppn));
		
		write_page_nb++;

		UPDATE_OLD_PAGE_MAPPING(lpn);
		UPDATE_NEW_PAGE_MAPPING(lpn, new_ppn);

#ifdef FTL_DEBUG
                if(ret == SUCCESS){
                        printf("\twrite complete [%d, %d, %d]\n",CALC_FLASH(new_ppn), CALC_BLOCK(new_ppn),CALC_PAGE(new_ppn));
                }
                else if(ret == FAIL){
                        printf("Error[FTL_WRITE] %d page write fail \n", new_ppn);
                }
#endif
		lba += write_sects;
		remain -= write_sects;
		left_skip = 0;

	}

	F_O_DIRECT_VSSIM_MT = -1;
#ifdef GC_ON
	GC_CHECK(CALC_FLASH(new_ppn), CALC_BLOCK(new_ppn));
#endif

#ifdef FTL_DEBUG
	printf("[FTL_WRITE] End\n");
#endif
	return ret;
}
