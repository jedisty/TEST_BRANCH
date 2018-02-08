#include "common.h"
#include "ftl_config_manager.h"
#include "ftl_mapping_manager.h"
#include "ftl_cache.h"


int max_cache_entry_nb;
int cur_cache_entry_nb;

cache_table_info* ct_info = NULL;

/* for LRU queue */
cache_queue* g_pcHead; 
cache_queue* g_pcTail; 

void INIT_CACHE_TABLE(){
	int i;

	max_cache_entry_nb = (MAPPING_CACHE_SIZE*1024*1024) /( sizeof(mapping_table_entry) * PAGE_NB) ;
	// MAPPING_CACHE_SIZE * 1024 * 1024 : MAPPING_CACHE_SIZE는 ssd.conf에 정의되어 있는 M단위의 cache 크기
	// 	so, MAPPING_CACHE_SIZE*1024*1024 : byte단위로 cache 크기를 변환.
	// sizeof(mapping_table_entry) : 하나의 page를 mapping하는 자료구조의 크기
	// PAGE_NB : 한개의 block내의 page의 수
	// 	so, sizeof(mapping_table_entry)*PAGE_NB : 하나의 block을 caching할 수 있는 mapping table의 크기
	// 
	// max_cache_entry_nb : Cahing할 수 있는 block의 수.

	ct_info = (cache_table_info *)malloc(sizeof(cache_table_info) * max_cache_entry_nb);
#ifdef FTL_MAP_CACHE_DEBUG
	printf("\n\t[CACHE] INIT_CACHE_TABLE() start\n");
	printf("\t[CACHE] MAPPING CACHE SIZE is  %d\n",MAPPING_CACHE_SIZE);
	printf("\t[CACHE] Whole block entry number is %ld\n",BLOCK_MAPPING_ENTRY_NB);
	printf("\t[CACHE] max_cache_entry_nb is %d\n", max_cache_entry_nb);
#endif

	for(i=0; i<max_cache_entry_nb; i++)
	{
		ct_info[i].cache_state = false;
		ct_info[i].changed_entry_nb = 0;
	}

	cur_cache_entry_nb = 0;

}

void TERM_CACHE_TABLE(){
	free(ct_info);
}

int CHECK_CACHE_ENTRY(int64_t lba){
	int ret = 1;
	int64_t log_page_nb = (int64_t)lba / (int64_t)SECTORS_PER_PAGE;
	int64_t log_block_nb = log_page_nb / (int64_t)PAGE_NB; // block address

#ifdef FTL_MAP_CACHE_DEBUG
	printf("\n\t[CACHE] CHECK_CACHE_ENTRY() is called\n");
#endif
	if( cur_cache_entry_nb == max_cache_entry_nb ){ // cache is full
		if( ct_info[log_block_nb].cache_state == true ) // cache hit
		{
			/* Do nothing. : cache hit */
#ifdef FTL_MAP_CACHE_DEBUG
			printf("\t[CACHE] [CACHE HIT][full] [%ld]\n", log_block_nb);
			PrintQueue();
#endif
			return SUCCESS;
		}else // cache miss
		{
			ret = SelectVictim();
#ifdef FTL_MAP_CACHE_DEBUG
			printf("\t[CACHE] [CACHE MISS][full] [%ld]\n", log_block_nb);
			printf("\t[CACHE] Selected victim is [ %d ]\n", ret);
#endif
			DeleteQueue(ret); // Flush victim block(ret) in the cache queue.
			InsertQueue(log_block_nb);
#ifdef FTL_MAP_CACHE_DEBUG
			PrintQueue();
#endif
			ct_info[log_block_nb].cache_state = true;
		}
	}else if ( cur_cache_entry_nb < max_cache_entry_nb )// cache is not full
	{
		ct_info[log_block_nb].cache_state == true;
		cur_cache_entry_nb++;

		/* Insert LRU queue */
#ifdef FTL_MAP_CACHE_DEBUG
		printf("\t[CACHE] I/O page is in the log block [ %ld ]\n", log_block_nb);
#endif
		DeleteQueue(log_block_nb); // delete : if same block nb is exist
		InsertQueue(log_block_nb);
#ifdef FTL_MAP_CACHE_DEBUG
		PrintQueue();
		printf("\t[CACHE] [CACHE][not full] [%ld]\n", log_block_nb);
		printf("\t[CACHE] [Insert cache] block_nb : [%ld] cur_cache_entry_nb[%d]\n",log_block_nb, cur_cache_entry_nb);
#endif
		/******************************/
		/* Block read delay is needed */
		/******************************/


	}else
	{
#ifdef FTL_MAP_CACHE_DEBUG
		printf("[CACHE] ERROR[CHECK_CACHE_ENTRY] cur_cache_entry_nb is overflowed\n");
#endif
		return FAIL;
	}
	
}


void InsertQueue( int bn )
{
    cache_queue* pNewQueue;
	         
    // 새로운 Node를 저장할 공간을 만들어서 pNewNode 포인터가 가르키게 함 
    pNewQueue = (cache_queue*) malloc(sizeof(cache_queue));
		         
    pNewQueue->block_nb = bn;           // 화살표 연산자를 통해서도 값 변경 가능
				     
    if ( g_pcHead == NULL )          // 리스트가 비어 있을 경우 
    {
           g_pcHead = pNewQueue;         // pHead 포인터가 pNewNode가 가르키고 있는 곳을 가르키게 함
           g_pcTail = pNewQueue;         // pTail 포인터가 pNewNode가 가르키고 있는 곳을 가르키게 함
    }
    else                            // 리스트가 비어 있지 않을 경우 
    {
           g_pcTail->next = pNewQueue;    // 추가하기 전 마지막 Node의 다음 Node를 새로 만들어진 Node를 가르키게 함
	   g_pcTail = pNewQueue;         // 새로 추가된 노드를 pTail이 가르키게 함 
	   g_pcTail->next = NULL; // add rec
    }
}

void DeleteQueue(int bn)
{
	int count = 0;
	cache_queue* pPrevQueue = g_pcHead;
	cache_queue* pCurrentQueue= g_pcHead;

	while( pCurrentQueue != NULL )
	{
		if( pCurrentQueue->block_nb == bn )
		{
			if(count == 0){
				g_pcHead = pCurrentQueue->next;
				free(pCurrentQueue);
				return ;
			}else{
				pPrevQueue->next = pCurrentQueue->next;
				free(pCurrentQueue);
				return ;
			}
		}
		pPrevQueue = pCurrentQueue;
		pCurrentQueue = pCurrentQueue->next;
		count++;
	}
	return ;
}

int SelectVictim(void){
	int block_nb;
	cache_queue* pCurrentQueue= g_pcHead;

	block_nb = g_pcHead->block_nb;
	g_pcHead = pCurrentQueue->next;
	free(pCurrentQueue);

	return block_nb;
}
 
void FreeQueue(void)
{
	cache_queue* pTempQueue;

	while( g_pcHead != NULL)
	{
		pTempQueue = g_pcHead;
		g_pcHead = pTempQueue->next;
		free(pTempQueue);

	}
}

void PrintQueue(void)
{
	int nCount = 0;
	cache_queue* pCurrentQueue = g_pcHead;
#ifdef FTL_MAP_CACHE_DEBUG
	printf("\t[CACHE] *****************************\n");
	printf("\t[CACHE] QUEUE  ");
#endif
	while( pCurrentQueue != NULL ){
#ifdef FTL_MAP_CACHE_DEBUG
		printf("[%d] ",pCurrentQueue->block_nb);
#endif
		pCurrentQueue = pCurrentQueue->next;
	}
#ifdef FTL_MAP_CACHE_DEBUG
	printf("\n");
	printf("\t[CACHE] *****************************\n");
#endif
}

