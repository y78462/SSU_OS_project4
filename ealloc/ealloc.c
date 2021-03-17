#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ealloc.h"

const int flag1 = PROT_READ | PROT_WRITE;
const int flag2 = MAP_ANONYMOUS | MAP_PRIVATE;

struct chunk{
  int valid;
  int size;
  char *start;
};
int created_page;
int created_num[4]; //생성된 청크 개수
int using_num[4]; //쓰고있는 청크 개수
int usingsize[4];//쓰고있는 배열 크기
struct chunk chunks[4][16];//chunk 들의 시작 주소와 유효성 저장
char *page[4];
int page_num;

// function declarations
void init_alloc()
{
  //mmap으로 페이지 할당
  //page = mmap(0, PAGESIZE, flag1, flag2, -1, 0);
  for(int i=0;i<4;i++)
  {
	for(int j=0;j<16;j++)
	{
	  chunks[i][j].valid=0;
	  chunks[i][j].size=0;
	  chunks[i][j].start= NULL;
	}
  }
  for (int i=0;i<4;i++)
  {
	created_num[i]=0;
	using_num[i]=0;
	usingsize[i]=0;
  }
  page_num = 0;
  created_page =0;
}
void cleanup()
{
  exit(1);
}
int count=0;
int n=0;int m=0;
char *alloc(int size)
{
  if(created_page ==0)
  {
    page[0] = mmap(0, PAGESIZE, flag1, flag2, -1, 0);
	created_page++; //1
  }
  //size 부족 ->새페이지 할당 가능한지 보기
  if((PAGESIZE - usingsize[page_num]) < size && page_num == 3)
  { 
	  fprintf(stderr,"all 4 pages created!\n");
	  exit(1);
  }
  if((PAGESIZE - usingsize[page_num]) < size)
  {
	if(created_page <4)
	{
	  page_num++;
      page[page_num] = mmap(0, PAGESIZE, flag1, flag2, -1, 0);
	  created_page++;
	}
	else
	{
	  if(page_num <4)
	  //다음 페이지로 이동
	  {
	    page_num++;
		created_num[page_num]=0;
		usingsize[page_num]=0;
		using_num[page_num]=0;
	  }
	}
  }

  //앞에서 사용이 끝난 같은 사이즈의 청크를 찾지 못함.
  //가장 마지막 에서부터 청크를 만들어야함
  if(created_num[page_num] ==0)//first chunk make
  {
	created_num[page_num]++;
	using_num[page_num]++;
	char *newchunk = page[page_num];
	chunks[page_num][0].valid =1;
	chunks[page_num][0].size = size;
	chunks[page_num][0].start = page[page_num];
	usingsize[page_num] += size;
	return newchunk;

  }
  char *newchunk =chunks[page_num][created_num[page_num]-1].start + chunks[page_num][created_num[page_num]-1].size;
  chunks[page_num][created_num[page_num]].valid = 1;
  chunks[page_num][created_num[page_num]].start = newchunk;
  chunks[page_num][created_num[page_num]].size = size;
  usingsize[page_num]+=size;
  created_num[page_num]++;
  using_num[page_num]++;
  return newchunk;
}
void dealloc(char *chunk)
{

  for(int j=0;j<4;j++)
  {
  for(int i=0;i<16;i++)
  {
	if(chunks[j][i].start == chunk)
	{
	  usingsize[j] = usingsize[j]- chunks[j][i].size;
	  chunks[j][i].valid=0;
	  for(int k=0; k<16; k++)
	  {
		if(chunks[j][k].start == chunks[j][k].start + chunks[j][k].size)
		{
		  chunks[j][k].size  += chunks[j][k].size;
		  chunks[j][k].size =0;
		  chunks[j][k].valid =0;
		  chunks[j][k].start = NULL;
		  created_num[j]--;
		}
	  }
	}
  }
  }

  using_num[page_num]--;
  //사용한 것 모두 dealloc
  if(using_num[page_num] ==0)
  {
	for(int i=0;i<16;i++)
	{
	  chunks[page_num][i].valid=0;
	  chunks[page_num][i].size=0;
	  created_num[page_num]=0;
	}
	if(page_num !=0)
	{
	  page_num--;
	}
  }
}

