#include "common.h"
#include "ssd_io_manager.h"
#include "ftl_config_manager.h"
#include "ssd_util.h"
#include <sys/time.h>


int64_t* access_time_ch;
int* access_type_ch;
int64_t* access_time_reg;
int* access_type_reg;

int old_channel_nb;

/* Testing : git hub commit */
//////////////////////////
///   get_usec(void)   ///
//////////////////////////
int64_t get_usec(void)
{
  int64_t t = 0;
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);
  t = tv.tv_sec;
  t *= 1000000;
  t += tv.tv_usec;

  return t;
}


/////////////////////////////
///   SSD_IO_INIT(void)   ///
/////////////////////////////
int SSD_IO_INIT(void){
  int i= 0;

  old_channel_nb = CHANNEL_NB;
  

  /* 변수를 초기화 한다 */
  access_time_ch = (int64_t *)malloc(sizeof(int64_t) * CHANNEL_NB);
  for(i=0; i<CHANNEL_NB; i++){
    *(access_time_ch +i)= CHANNEL_IS_EMPTY;
  }

  access_type_ch = (int *)malloc(sizeof(int) * CHANNEL_NB);
  for(i=0; i<CHANNEL_NB; i++){
    *(access_type_ch +i)= CHANNEL_IS_EMPTY;
  }

  access_time_reg = (int64_t *)malloc(sizeof(int64_t) * FLASH_NB * PLANES_PER_FLASH);
  for(i=0; i< FLASH_NB*PLANES_PER_FLASH; i++){
    *(access_time_reg + i) = REG_IS_EMPTY;
  }
  
  access_type_reg = (int *)malloc(sizeof(int) * FLASH_NB * PLANES_PER_FLASH);
  for(i=0; i< FLASH_NB*PLANES_PER_FLASH; i++){
    *(access_type_reg + i) = REG_IS_EMPTY;
  }

  return 0;

}

//////////////////////////////////////////
///   CELL_WRITE(flash, block, page)   ///
//////////////////////////////////////////
int CELL_WRITE(unsigned int flash_nb,unsigned int block_nb,unsigned int page_nb)
{
  int channel, reg;

  if( page_nb < 0 || page_nb >= PAGE_NB ){
    printf("[err] page_nb is not valid\n");
    return FAIL;
  }

  channel = flash_nb % CHANNEL_NB;
  reg = flash_nb*PLANES_PER_FLASH + block_nb%PLANES_PER_FLASH;

  CHANNEL_ACCESS_W(channel);
  /* Channel Access Check 'Write'
      If 'can access' : Just return. (time stamp)
      else if 'can't access' : Wait(busy loop) and return. (time stamp)
      Using : "access_time_ch[?]" */
  

  REG_ACCESS(reg, WRITE);
  /* 'WRITE' means, register "WRITE" access
     'READ' means, register "READ" access 
     'ERASE' means, register "ERASE" access */
  

  return SUCCESS;
}


/////////////////////////////////////////
///   CELL_READ(flash, block, page)   ///
/////////////////////////////////////////
int CELL_READ(unsigned int flash_nb, unsigned int block_nb, unsigned int page_nb)
{
  int channel, reg;
  if( page_nb < 0 || page_nb >= PAGE_NB ){
    printf("[err] page_nb is not valid\n");
    return FAIL;
  }

  channel = flash_nb % CHANNEL_NB;
  reg = flash_nb*PLANES_PER_FLASH + block_nb%PLANES_PER_FLASH;

  CHANNEL_CHECK(channel);
  /* CHANNEL_CHECK is just check channel
     : because, read command is so fast.
       Just check "channel"and if channel is avariable, return. */
  
  REG_ACCESS(reg, READ);
  /* 'WRITE' means, register "WRITE" access
     'READ' means, register "READ" access 
     'ERASE' means, register "ERASE" access */

  CHANNEL_ACCESS_R(channel);
  /* channel check and get time stamp */
  
  return SUCCESS;
}


////////////////////////////////////
///  BLOCK_ERASE(flash, block)   ///
////////////////////////////////////
int BLOCK_ERASE(unsigned int flash_nb, unsigned int block_nb){
  int channel, reg;

  channel = flash_nb % CHANNEL_NB;
  reg = flash_nb*PLANES_PER_FLASH + block_nb%PLANES_PER_FLASH;


  CHANNEL_CHECK(channel);
  /* CHANNEL_CHECK is just check channel
     : because, read command is so fast.
       Just check "channel"and if channel is avariable, return. */

  REG_ACCESS(channel, ERASE);
  /* 'WRITE' means, register "WRITE" access
     'READ' means, register "READ" access 
     'ERASE' means, register "ERASE" access */

  return SUCCESS;

}


////////////////////////////////////////
///  CHANNEL_ACCESS_W(int channel)   ///
////////////////////////////////////////
int CHANNEL_ACCESS_W(int channel){
  int64_t diff, time_flag, time_stamp;

#ifdef SSD_CH_SWITCH_DELAY_W
  if( channel != old_channel_nb ){
	time_flag = get_usec();
	diff = time_flag - access_time_ch[old_channel_nb];
    while( diff < CHANNEL_SWITCH_DELAY_W ){
      time_stamp = get_usec();
      diff = time_stamp - time_flag;
    }
  }
  old_channel_nb = channel;
#endif

  diff = get_usec() - access_time_ch[channel];
  switch(access_type_ch[channel]){
    case CHANNEL_IS_WRITE:
      while( diff < REG_WRITE_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_ch[channel];
      }
	  access_time_ch[channel] = get_usec();
	  access_type_ch[channel] = CHANNEL_IS_WRITE;
      break;
    case CHANNEL_IS_READ:
      while( diff < REG_READ_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_ch[channel];
      }
	  access_time_ch[channel] = get_usec();
	  access_type_ch[channel] = CHANNEL_IS_WRITE;
      break;
    case CHANNEL_IS_EMPTY:
	  access_time_ch[channel] = get_usec();
	  access_type_ch[channel] = CHANNEL_IS_WRITE;
      break;
  }

  return SUCCESS;

}

////////////////////////////////////////
///   CHANNEL_CHECK_R(int channel)   ///
////////////////////////////////////////
int CHANNEL_CHECK(int channel){
  int64_t s_time, diff, time_flag, time_stamp;

#ifdef SSD_CH_SWITCH_DELAY_R
  if( channel != old_channel_nb ){
	time_flag = get_usec();
	diff = time_flag - access_time_ch[old_channel_nb];
    while( diff < CHANNEL_SWITCH_DELAY_R ){
      time_stamp = get_usec();
      diff = time_stamp - time_flag;
    }
  }
  old_channel_nb = channel;
#endif

  s_time = get_usec();
  diff = s_time - access_time_ch[channel];
  switch(access_type_ch[channel]){
    case CHANNEL_IS_WRITE:
      if( diff <= REG_WRITE_DELAY ){
        while( diff < REG_WRITE_DELAY ){
  	      time_stamp = get_usec();
	        diff = time_stamp - access_time_ch[channel];
        }
      }
      break;
    case CHANNEL_IS_READ:
      if( diff <= REG_READ_DELAY ){
        while( diff < REG_READ_DELAY ){
          time_stamp = get_usec();
          diff = time_stamp - access_time_ch[channel];
        }
      }
      break;
    case CHANNEL_IS_EMPTY:
      break;
  }

  return SUCCESS;
}


/////////////////////////////////////////
///   CHANNEL_ACCESS_R(int channel)   ///
/////////////////////////////////////////
int CHANNEL_ACCESS_R(int channel){
  int64_t s_time, diff, time_stamp;
  int64_t start_delay, delay_time;

  s_time = get_usec();
  diff = s_time - access_time_ch[channel];
  switch(access_type_ch[channel]){
    case CHANNEL_IS_WRITE:
      if( diff > REG_WRITE_DELAY ){ // 즉, 기다릴 필요가 없다면,
       	access_time_ch[channel] = s_time + CELL_READ_DELAY;
	      access_type_ch[channel] = CHANNEL_IS_READ;
        break;
      }

      start_delay = get_usec();
      while( diff < REG_WRITE_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_ch[channel];
      }
      delay_time = get_usec() - start_delay;

      if( delay_time <= 0 ){
    /* 채널 이용을 기다린 시간이 reg read보다 길었을 경우를 나타낸다.
       즉, 다른 delay없이 그 시점에서 바로 reg에 읽은 데이터를 ftl로
       전송하는 channel 접근이 발생한다. */
        access_time_ch[channel] = get_usec();
        access_type_ch[channel] = CHANNEL_IS_READ;
      }else{
    /* 채널 이용 시간을 기다렸으나, cell read가 끝나지 않은 경우다.
       이 경우에는, reg read시간에서 channel access를 기다린 시간을 
       빼준 만큼의 시간 뒤에 channel access가 수행되기 때문에
       현재 시간에서 더 기다려야 하는 시간 즉, cell read의 남은 시간
       만큼을 더해준 뒤 channel accee가 수행된다. */
        access_time_ch[channel] = get_usec() + CELL_READ_DELAY - delay_time;
        access_type_ch[channel] = CHANNEL_IS_READ;
      }

      break;
    case CHANNEL_IS_READ:
      if( diff > REG_READ_DELAY ){ // 즉, 기다릴 필요가 없다면,
        access_time_ch[channel] = s_time + CELL_READ_DELAY;
	      access_type_ch[channel] = CHANNEL_IS_READ;
        break;
      }

      start_delay = get_usec();
      while( diff < REG_READ_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_ch[channel];
      }
      delay_time = get_usec() - start_delay;

      if( delay_time <= 0 ){
        access_time_ch[channel] = get_usec();
        access_type_ch[channel] = CHANNEL_IS_READ;
      }else{
        access_time_ch[channel] = get_usec() + CELL_READ_DELAY - delay_time;
        access_type_ch[channel] = CHANNEL_IS_READ;
      }
      break;

    case CHANNEL_IS_EMPTY:
      access_time_ch[channel] = s_time + CELL_READ_DELAY;
      access_type_ch[channel] = CHANNEL_IS_READ;
      break;
  }
  return SUCCESS;
  
}

///////////////////////////////////////
///    REG_ACCESS_R(int channel)    ///
///   				                      /// 
///     - flag 0 : write reg access ///
///     - flag 1 : read reg access  ///
///     - flag 2 : erage reg access ///
///////////////////////////////////////
int REG_ACCESS( int reg, int flag){
  int64_t s_time, diff, time_stamp;

  s_time = get_usec();
  diff = s_time + REG_WRITE_DELAY - access_time_reg[reg];
  switch(access_type_reg[reg]){
    case REG_IS_WRITE:
      if( diff > CELL_PROGRAM_DELAY ){ // 즉, 기다릴 필요가 없다면,
	// if reg access is 'write'
	if(flag == WRITE){
          access_time_reg[reg] = s_time + REG_WRITE_DELAY;
          access_type_reg[reg] = REG_IS_WRITE;

	// if reg access is 'read'
	}else if(flag == READ){
          access_time_reg[reg] = s_time;
          access_type_reg[reg] = REG_IS_READ;

	// if reg access is 'erase'
	}else if(flag == ERASE){
          access_time_reg[reg] = s_time;
          access_type_reg[reg] = REG_IS_ERASE;
	}
    return SUCCESS;
  }

      while( diff < CELL_PROGRAM_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_reg[reg];
      }

      /* Get stamp : access_time_reg & access_type_reg */
      access_time_reg[reg] = get_usec();
      if( flag == WRITE){ //if access is write
      	access_type_reg[reg] = REG_IS_WRITE;
      }else if(flag == READ){ // if access is read
      	access_type_reg[reg] = REG_IS_READ;
      }else if(flag == ERASE){
      	access_type_reg[reg] = REG_IS_ERASE;
      }
      return SUCCESS;

    case REG_IS_READ:
      if( diff > CELL_READ_DELAY ){ // 즉, 기다릴 필요가 없다면,
        if( flag == WRITE ){ // if access is write
          access_time_reg[reg] = s_time + REG_WRITE_DELAY;
          access_type_reg[reg] = REG_IS_WRITE;
	}else if (flag == READ){ // if access is read
          access_time_reg[reg] = s_time;
          access_type_reg[reg] = REG_IS_READ;
        }else if (flag == ERASE){
          access_time_reg[reg] = s_time;
          access_type_reg[reg] = REG_IS_ERASE;
        }
        return SUCCESS;

      }

      while( diff < CELL_READ_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_reg[reg];
      }

      access_time_reg[reg] = get_usec();
      if( flag == WRITE){ //if access is write
      	access_type_reg[reg] = REG_IS_WRITE;
      }else if(flag == READ){ // if access is read
      	access_type_reg[reg] = REG_IS_READ;
      }else if(flag == ERASE){
	access_type_reg[reg] = REG_IS_ERASE;
      }
      return SUCCESS;

#ifdef SSD_ERASE_ON
    case REG_IS_ERASE:
      if(diff > BLOCK_ERASE_DELAY ){
	if( flag == WRITE){
          access_time_reg[reg] = s_time + REG_WRITE_DELAY;
          access_type_reg[reg] = REG_IS_WRITE;
	}else if( flag == READ){
          access_time_reg[reg] = s_time;
          access_type_reg[reg] = REG_IS_READ;
        }else if(flag == ERASE){
          access_time_reg[reg] = s_time;
          access_type_reg[reg] = REG_IS_ERASE;
	}
        return SUCCESS;
      }

      while( diff < BLOCK_ERASE_DELAY ){
        time_stamp = get_usec();
        diff = time_stamp - access_time_reg[reg];
      }

      access_time_reg[reg] = get_usec();
      if( flag == WRITE){ //if access is write
      	access_type_reg[reg] = REG_IS_WRITE;
      }else if(flag == READ){ // if access is read
      	access_type_reg[reg] = REG_IS_READ;
      }else if(flag == ERASE){
      	access_type_reg[reg] = REG_IS_ERASE;
      }
      return SUCCESS;
#endif

    case REG_IS_EMPTY:
      if( flag == WRITE ){
        access_time_reg[reg] = s_time + REG_WRITE_DELAY;
        access_type_reg[reg] = REG_IS_WRITE;
      }else if( flag == READ){
        access_time_reg[reg] = s_time;
        access_type_reg[reg] = REG_IS_READ;
      }else if(flag == ERASE){
        access_time_reg[reg] = s_time;
        access_type_reg[reg] = REG_IS_ERASE;
      }
      return SUCCESS;
  }
  return SUCCESS;
}

