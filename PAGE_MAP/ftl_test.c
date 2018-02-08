// Copyright(c)2013 
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#include "common.h"

#ifdef DEBUG_MODE1
FILE* fp_dbg1_r;
FILE* fp_dbg1_w;
#endif

#ifdef DEBUG_MODE9
FILE* fp_dbg9_gc;
#endif

int g_init = 0;
extern double ssd_util;

void FTL_INIT(void)
{
	if(g_init == 0){
        	printf("[FTL_INIT] start\n");

		INIT_SSD_CONFIG();

#ifdef FTL_MAP_CACHE
		INIT_CACHE();
#endif
		INIT_MAPPING_TABLE();
		INIT_INVERSE_PAGE_MAPPING();
		INIT_INVERSE_BLOCK_MAPPING();
		INIT_VALID_ARRAY();
		INIT_EMPTY_BLOCK_LIST();
		INIT_VICTIM_BLOCK_LIST();
		INIT_PERF_CHECKER();
		g_init = 1;

#ifdef DEBUG_MODE1
		fp_dbg1_r = fopen("./data/p_dbg1_r.txt","a");
		fp_dbg1_w = fopen("./data/p_dbg1_w.txt","a");
#endif
#ifdef DEBUG_MODE9
		fp_dbg9_gc = fopen("./data/p_dbg9_gc.txt","a");
#endif

		printf("[FTL_INIT] complete\n");
	}
}

void FTL_TERM(void)
{
	printf("[FTL_TERM] start\n");

	TERM_MAPPING_TABLE();
	TERM_INVERSE_PAGE_MAPPING();
	TERM_VALID_ARRAY();
	TERM_INVERSE_BLOCK_MAPPING();
	TERM_EMPTY_BLOCK_LIST();
	TERM_VICTIM_BLOCK_LIST();
	TERM_PERF_CHECKER();

#ifdef DEBUG_MODE1
	fclose(fp_dbg1_r);
	fclose(fp_dbg1_w);
#endif
#ifdef DEBUG_MODE9
	fclose(fp_dbg9_gc);
#endif
	printf("[FTL_TERM] complete\n");
}

void FTL_READ(int32_t sector_nb, unsigned int length)
{
#ifdef FTL_DEBUG
	printf("[FTL_READ] Start\n");
#endif

	char szTemp[1024];
#ifdef MONITOR_ON
	int64_t read_start_time = get_usec();
#endif

#ifdef DEBUG_MODE1
	int64_t start_dbg1, end_dbg1;
#endif
	if(sector_nb + length > SECTOR_NB){
		printf("Error[FTL_READ] Exceed Sector number\n"); 
		return;	
	}

	int32_t lpn;
	int32_t ppn;
	int32_t lba = sector_nb;
	unsigned int remain = length;
	unsigned int left_skip = sector_nb % SECTORS_PER_PAGE;
	unsigned int right_skip;
	unsigned int read_sects;

	unsigned int ret;
	int read_page_nb = 0;

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
			return;
		}

		lba += read_sects;
		remain -= read_sects;
		left_skip = 0;
	}

	io_alloc_overhead = ALLOC_IO_REQUEST(sector_nb, length, READ);

	remain = length;
	lba = sector_nb;
	left_skip = sector_nb % SECTORS_PER_PAGE;

	while(remain > 0){
#ifdef DEBUG_MODE1
		start_dbg1 = get_usec();
#endif

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}
		read_sects = SECTORS_PER_PAGE - left_skip - right_skip;

		lpn = lba / (int32_t)SECTORS_PER_PAGE;

#ifdef FTL_MAP_CACHE
		ppn = CACHE_GET_PPN(lpn);
#else
		ppn = GET_MAPPING_INFO(lpn);
#endif

		if(ret == FAIL || ppn == -1){
#ifdef FTL_DEBUG
			printf("Error[FTL_READ] No Mapping info\n");
#endif
		}

		ret = CELL_READ(CALC_FLASH(ppn), CALC_BLOCK(ppn), CALC_PAGE(ppn), read_page_nb, READ);

#ifdef FTL_DEBUG
		if(ret == SUCCESS){
			printf("[FTL_READ] read complete [%u]\n",ppn);
		}
		else if(ret == FAIL){
			printf("Error[FTL_READ] %u page read fail \n", ppn);
		}
#endif
		read_page_nb++;

		lba += read_sects;
		remain -= read_sects;
		left_skip = 0;

#ifdef DEBUG_MODE1
		end_dbg1 = get_usec();
		fprintf(fp_dbg1_r,"%ld\t%d\t%lf\n",end_dbg1-start_dbg1, read_sects, ssd_util);
#endif
	}

	INCREASE_IO_REQUEST_SEQ_NB();

#ifdef MONITOR_ON
	int64_t read_end_time = get_usec();
	sprintf(szTemp, "READ %ld %d %d %ld ", read_end_time - read_start_time, sector_nb, length, read_start_time);
//	WRITE_LOG(szTemp);
#endif

}

void FTL_WRITE(int32_t sector_nb, unsigned int length)
{
#ifdef FTL_DEBUG
	printf("[FTL_WRITE] Start\n");
#endif

	char szTemp[1024];
#ifdef MONITOR_ON
	int64_t write_start_time = get_usec();
#endif

#ifdef DEBUG_MODE1
	int64_t start_dbg1, end_dbg1;
#endif

	if(sector_nb + length > SECTOR_NB){
		printf("Error[FTL_WRITE] Exceed Sector number\n");
                return;
        }
	else{
		io_alloc_overhead = ALLOC_IO_REQUEST(sector_nb, length, WRITE);
	}

	int32_t lba = sector_nb;
	int32_t lpn;
	int32_t new_ppn;

	unsigned int remain = length;
	unsigned int left_skip = sector_nb % SECTORS_PER_PAGE;
	unsigned int right_skip;
	unsigned int write_sects;

	unsigned int ret;
	int write_page_nb=0;

	while(remain > 0){

#ifdef DEBUG_MODE1
		start_dbg1 = get_usec();
#endif
		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}

		write_sects = SECTORS_PER_PAGE - left_skip - right_skip;

#ifdef WRITE_NOPARAL
		ret = GET_NEW_PAGE(VICTIM_NOPARAL, empty_block_table_index, &new_ppn);
#else
		ret = GET_NEW_PAGE(VICTIM_OVERALL, EMPTY_TABLE_ENTRY_NB, &new_ppn);
#endif
		if(ret == FAIL){
			printf("ERROR[FTL_WRITE] Get new page fail \n");
			return;
		}

		ret = CELL_WRITE(CALC_FLASH(new_ppn), CALC_BLOCK(new_ppn), CALC_PAGE(new_ppn), write_page_nb, WRITE);

		write_page_nb++;

		lpn = lba / (int32_t)SECTORS_PER_PAGE;
		UPDATE_OLD_PAGE_MAPPING(lpn);
		UPDATE_NEW_PAGE_MAPPING(lpn, new_ppn);

#ifdef FTL_DEBUG
                if(ret == SUCCESS){
                        printf("[FTL_WRITE] write complete [%d, %d, %d]\n",new_phy_flash_nb, new_phy_block_nb,new_phy_page_nb);
                }
                else if(ret == FAIL){
                        printf("Error[FTL_WRITE] %d page write fail \n", new_phy_page_nb);
                }
#endif
		lba += write_sects;
		remain -= write_sects;
		left_skip = 0;

#ifdef DEBUG_MODE1
		end_dbg1 = get_usec();
		fprintf(fp_dbg1_w,"%ld\t%d\t%lf\n",end_dbg1-start_dbg1,write_sects,ssd_util);
#endif
	}

	INCREASE_IO_REQUEST_SEQ_NB();

#ifdef GC_ON
	GC_CHECK(CALC_FLASH(new_ppn), CALC_BLOCK(new_ppn));
#endif

#ifdef MONITOR_ON
	int64_t write_end_time = get_usec();
	sprintf(szTemp, "WRITE %ld %d %d %ld", write_end_time - write_start_time, sector_nb, length, write_start_time);
//	WRITE_LOG(szTemp);

	char szWrite[1024];
	sprintf(szWrite, "WB CORRECT %d", write_page_nb);
	WRITE_LOG(szWrite);
#endif
}
