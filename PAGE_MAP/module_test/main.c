#include<stdio.h>
#include "ftl_cache.h"

void main(void){
	FTL_INIT();
	FTL_WRITE(0,1);
	FTL_WRITE(3000,1);
	FTL_WRITE(6000,1);
	FTL_WRITE(2000,1);
	FTL_WRITE(10000,1);
	FTL_WRITE(2000,1);

	PrintQueue();
	return ;
}
