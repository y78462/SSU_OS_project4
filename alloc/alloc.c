#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "alloc.h"

const int flag1 = PROT_READ | PROT_WRITE;
const int flag2 = MAP_ANONYMOUS | MAP_PRIVATE;

struct chunk{
  int valid;
  int size;
  char *start;
};

int created_num; //생성된 청크 개수
int using_num; //쓰고있는 청크 개수
struct chunk chunks[512];//chunk 들의 시작 주소와 유효성 저장
char *page;
int usingsize;

// function declarations
int init_alloc()
{
  //mmap으로 페이지 할당
  page = mmap(0, PAGESIZE, flag1, flag2, -1, 0);
  for(int i=0;i<512;i++)
  {
	chunks[i].valid=0;
	chunks[i].size=0;
	chunks[i].start= NULL;
  }
  created_num = 0;
  using_num=0;
  return 0;
}
int cleanup()
{
  //가상 및 실제 메모리 사용량 측정
  system("ps aux | grep a.out");
  printf("\n");
 if( munmap(page,PAGESIZE) ==-1)
 {
   fprintf(stderr,"munmap error\n");
   exit(0);
 }
  system("ps aux | grep a.out");
  return 0;
}
char *alloc(int size)
{
  if(size %8 !=0)
  {
	fprintf(stderr,"8의 배수 사이즈만 가능\n");
	exit(0);
  }
  //size가 남았는지 확인
 
  if((PAGESIZE - usingsize) < size)
  {
	fprintf(stderr,"not enough size\n");
	return NULL;
  }
  //chunk_array에서 할당 가능한 청크가 있는지 확인 (first fit)
  for(int i=0; i<created_num;i++)
  {
	//사이즈가 같은 비어있는 (이미dealloc된) 청크가 있으면 주소 리턴
	if((chunks[i].size == size)&&(chunks[i].valid==0))
	{
	  chunks[i].valid =1;
	  //사용중인 청크개수 +1
	  using_num ++;
	  //사용중인 전체 사이즈 더함
	  usingsize +=size;
	  return chunks[i].start;
	}
	if((chunks[i].size > size)&&(chunks[i].valid==0))
	{
	  //쪼개서 뒷부분 조각은 chunks[created_num ]이 가리키게함
	  chunks[i].valid = 1;
	  using_num++;
	  usingsize += size;
	  chunks[created_num].valid= 0 ;
	  chunks[created_num].size = chunks[i].size - size;
	  chunks[i].size = size;
	  chunks[created_num].start = chunks[i].start + size;
	  created_num++;
	  
	  return chunks[i].start;
	}
  }
  //앞에서 사용이 끝난 같은 사이즈의 청크를 찾지 못함.
  //가장 마지막 에서부터 청크를 만들어야함
  if(created_num ==0)//first chunk make
  {
	created_num++;
	using_num++;
	char *newchunk = page;
	chunks[0].valid =1;
	chunks[0].size = size;
	chunks[0].start = page;
	usingsize+=size;
	return newchunk;

  }
  char *newchunk =chunks[created_num-1].start + chunks[created_num-1].size;
  chunks[created_num].valid = 1;
  chunks[created_num].start = newchunk;
  chunks[created_num].size = size;
  created_num++;
  using_num++;
  return newchunk;
}
void dealloc(char *chunk)
{

  for(int i=0;i<512;i++)
  {
	if(chunks[i].start == chunk)
	{
	  usingsize -= chunks[i].size;
	  chunks[i].valid=0;
	  for(int j=0; j<512; j++)
	  {
		if(chunks[j].start == chunks[i].start + chunks[i].size)
		{
		  chunks[i].size  += chunks[j].size;
		  chunks[j].size =0;
		  chunks[j].valid =0;
		  chunks[j].start = NULL;
		  created_num--;
		}
	  }
	}
  }

  using_num--;
  //사용한 것 모두 dealloc
  if(using_num ==0)
  {
	for(int i=0;i<512;i++)
	{
	  chunks[i].valid=0;
	  chunks[i].size=0;
	  created_num=0;
	}
  }
}

