#ifndef _INVERSE_MAPPING_MANAGER_H_
#define _INVERSE_MAPPING_MANAGER_H_

extern int64_t total_empty_block_nb;
extern int64_t victim_block_nb;

typedef struct inverse_page_mapping_entry
{
	unsigned int log_flash_nb;
	unsigned int log_block_nb;
	unsigned int log_page_nb;
	unsigned int valid;
}inverse_page_mapping_entry;

typedef struct inverse_block_mapping_entry
{
	int valid_page_nb;
	int type;
	char* valid_array;

}inverse_block_mapping_entry;

typedef struct empty_block_root
{
	struct empty_block_entry* next;
	struct empty_block_entry* tail;
	unsigned int empty_block_nb;
}empty_block_root;

typedef struct empty_block_entry
{
	unsigned int phy_flash_nb;
	unsigned int phy_block_nb;
	unsigned int curr_phy_page_nb;
	struct empty_block_entry* next;

}empty_block_entry;

void INIT_INVERSE_PAGE_MAPPING(void);
void INIT_INVERSE_BLOCK_MAPPING(void);
void INIT_EMPTY_BLOCK_LIST(void);
void INIT_VICTIM_BLOCK_LIST(void);
void INIT_VALID_ARRAY(void);

void TERM_INVERSE_PAGE_MAPPING(void);
void TERM_INVERSE_BLOCK_MAPPING(void);
void TERM_EMPTY_BLOCK_LIST(void);
void TERM_VICTIM_BLOCK_LIST(void);
void TERM_VALID_ARRAY(void);

empty_block_entry* GET_EMPTY_BLOCK(void);
int INSERT_EMPTY_BLOCK(unsigned int phy_flash_nb, unsigned int phy_block_nb);

inverse_block_mapping_entry* GET_INVERSE_BLOCK_MAPPING_ENTRY(unsigned int phy_flash_nb, unsigned int phy_block_nb);
inverse_page_mapping_entry* GET_INVERSE_PAGE_MAPPING_ENTRY(unsigned int phy_flash_nb, unsigned int phy_block_nb, unsigned int phy_page_nb);

int UPDATE_INVERSE_PAGE_MAPPING(unsigned int phy_flash_nb, unsigned int phy_block_nb, unsigned int phy_page_nb, unsigned int log_flash_nb, unsigned int log_block_nb, unsigned int log_page_nb, int type);
int UPDATE_INVERSE_BLOCK_MAPPING(unsigned int phy_flash_nb, unsigned int phy_block_nb, int type);
int UPDATE_INVERSE_BLOCK_VALIDITY(unsigned int phy_flash_nb, unsigned int phy_block_nb, unsigned int phy_page_nb, int valid);

void PRINT_VALID_ARRAY(unsigned int phy_flash_nb, unsigned int phy_block_nb);

#endif
