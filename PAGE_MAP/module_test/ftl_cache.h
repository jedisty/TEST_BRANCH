#ifndef _FTL_CACHE_H
#define _FTL_CACHE_H

typedef enum {false, true} bool;

extern int max_cache_entry_nb;
extern int cur_cache_entry_nb;

typedef struct cache_table_info{
	bool cache_state;
	unsigned changed_entry_nb;
}cache_table_info;

typedef struct cache_queue{
	int block_nb;
	struct cache_queue *next;
}cache_queue;


extern cache_table_info *ct_info;
extern cache_queue* g_pHead; 
extern cache_queue* g_pTail; 

void InsertQueue( int bn );
void DeleteQueue( int bn );
void FreeQueue( void );
int SelectVictim(void);
void PrintQueue(void);

void INIT_CACHE_TABLE(void);

#endif
