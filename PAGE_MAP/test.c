#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SET_FLAG_BIT(idx, bit) 		(idx = (idx|(0x00000001 << bit)))
#define RESET_FLAG_BIT(idx, bit) 	(idx = (idx&(0x11111110 << bit)))
#define GET_FLAG_BIT(idx, bit)		(idx & (0x00000001 << bit))

int main(void){

	int i;
	unsigned int a = 0;
	unsigned int pre = 0;
	int cnt = 0;
/*
	for(i=0;i<31;i++){
		SET_FLAG_BIT(a,i);
	}
	printf("\n");
	printf("%x\n",a);

	a = a << 1;
*/
	
	printf("%lu\n",sizeof(a));
	while(cnt< 5){
		pre = a;
		a++;
		/*
		for(i=0;i<32;i++){
			if(GET_FLAG_BIT(a,31-i)==0)
				printf("0");
			else
				printf("1");
		}
		printf("\n");
		*/
		if(a == 0){
			printf("%x %u\n",pre,pre);
			cnt++;
			a = 0;
		}
	}
	printf("\n");

	a = 286331153 + 1000000000;
	printf("a = %d\n",a);
/*	RESET_FLAG_BIT(a, 31);
	RESET_FLAG_BIT(a, 30);

	for(i=0;i<31;i++){
		if(GET_FLAG_BIT(a,31-i)==0)
			printf("0");
		else
			printf("1");
	}
	printf("\n");
	printf("%x\n",a);
*/
	return 0;
}
