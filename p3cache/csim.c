/*
 * csim.c - a basic cache simulator
 *
 * Author: Qizheng "Alex" Zhang (qizhengz)
 *
 */

#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

struct parameter {
  int v;
  int s; // Number of set index bits (S = 2^s is the number of sets)
  int e; // Associativity (number of lines per set)
  int b; // Number of block bits (B = 2^b is the block size)
  FILE *trace;
};

struct line {
  int valid_bit;
  int order_visited;
  unsigned long long tag;
};

struct set {
  struct line *lines;
};

struct cache {
  struct set *sets;
};

struct stats {
  int hit;
  int miss;
  int evict;
};

typedef struct parameter parameter;
typedef struct line line;
typedef struct set set;
typedef struct cache cache;
typedef struct stats stats;

void cache_initializer(cache *new_cache,parameter *paras){
  int set_count = pow(2,(paras->s));
  int line_count = paras->e;

  new_cache->sets = malloc(set_count*sizeof(set));
  for(int i = 0; i < set_count; i++){
    new_cache->sets[i].lines = malloc(line_count*sizeof(line));
    for(int j = 0; j < line_count; j++){
      new_cache->sets[i].lines[j].valid_bit = 0;
      new_cache->sets[i].lines[j].order_visited = 0;
    }
  }
  return ;
}

void stats_initializer(stats *new_stats){
  new_stats->hit = 0;
  new_stats->miss = 0;
  new_stats->evict = 0;
}

void cache_modify(cache *curr_cache,stats *curr_stats,parameter *paras,unsigned long long addr,int *visit_order){
  unsigned long long all_one = (unsigned long long) -1;
  unsigned long long mask = all_one >> (64 - paras->s);
  unsigned long long curr_set = (addr >> paras->b) & mask;
  unsigned long long curr_tag = (addr >> (paras->s + paras->b)) & all_one;
  
  int i = 0;
  int hit_flag = 0;
  while(i < paras->e){
    if(curr_cache->sets[curr_set].lines[i].valid_bit == 1 &&
       curr_cache->sets[curr_set].lines[i].tag == curr_tag){
      // hit happens
      curr_cache->sets[curr_set].lines[i].order_visited = *visit_order;
      (*visit_order)++;
      curr_stats->hit ++;
      hit_flag = 1;
      if(paras->v)
	printf(" hit");
      break;
    }
    i++;
  }

  // dealing with a miss, including eviction
  if(!hit_flag){
    curr_stats->miss ++;
    if(paras->v)
      printf(" miss");
    
    int eviction_flag = 1;
    int write_to_index = 0;
    for(int i = 0; i < paras->e; i++){
      if(curr_cache->sets[curr_set].lines[i].valid_bit == 0){
	eviction_flag = 0;
	write_to_index = i;
  	break;
      }
    }

    // conduct eviction if necessary
    if(eviction_flag){
      int smallest_order = curr_cache->sets[curr_set].lines[0].order_visited;
      int smallest_order_index = 0;
      for(int i = 0; i < paras->e; i++){
	if(curr_cache->sets[curr_set].lines[i].order_visited < smallest_order){
	  smallest_order =
	    curr_cache->sets[curr_set].lines[i].order_visited;
	  smallest_order_index = i;
	}
      }
      write_to_index = smallest_order_index;
      if(paras->v)
	printf(" eviction");
      curr_stats->evict ++;
    }

    // write new content
    curr_cache->sets[curr_set].lines[write_to_index].valid_bit = 1;
    curr_cache->sets[curr_set].lines[write_to_index].tag = curr_tag;
    curr_cache->sets[curr_set].lines[write_to_index].order_visited =
      *visit_order;
    (*visit_order)++;
    
  }// end of if(!hit_flag)
}

void cache_free(cache *a_cache,parameter *paras){
  int num_set = pow(2,paras->s);
  for(int i = 0; i < num_set; i++){
    free(a_cache->sets[i].lines);
  }
  free(a_cache->sets);
  free(a_cache);
}

int main(int argc, char **argv)
{
  parameter *curr_para = malloc(sizeof(parameter));
  
  // part 1: reading in and storing command-line arguments
  
  char *tvalue = NULL;
  int c;

  while ((c = getopt (argc, argv, "vs:E:b:t:")) != -1){
    switch (c)
      {
      case 'v':
        curr_para->v = 1;
        break;
      case 's':
        curr_para->s = atoi(optarg);
        break;
      case 'E':
        curr_para->e = atoi(optarg);
        break;
      case 'b':
        curr_para->b = atoi(optarg);
        break;
      case 't':
	tvalue = optarg;
        curr_para->trace = fopen(tvalue,"r");
        break;
      case '?':
        if (optopt == 's' || optopt == 'E' || optopt == 'b' || optopt == 't')
          fprintf (stderr, "Option requires an argument.\n");
        else
          fprintf (stderr, "Unknown option character\n");
	return 1;
      default:
	abort ();
      }
  }

  // line used to test if the storage process is successful
  printf ("vflag = %d, svalue = %d, evalue = %d, bvalue = %d, tvalue = %s\n",
	  curr_para->v, curr_para->s, curr_para->e, curr_para->b, tvalue);

  // part 2: read in files & record hits and misses

  // initialize the cache struct we'll be using
  cache *curr_cache = malloc(sizeof(cache));
  cache_initializer(curr_cache,curr_para);
  stats *curr_stats = malloc(sizeof(stats));
  stats_initializer(curr_stats);

  // starting scanning and cache behavior analysis!
  char op_type;
  unsigned long long addres;
  int size;
  int counter = 0;
  int *visit_order = malloc(sizeof(int));
  *visit_order = 0;
  
  while(fscanf(curr_para->trace," %c %llx,%d",&op_type,&addres,&size) == 3){
    counter++;
    //printf("This is the %d th scan\n",counter);
    if(curr_para->v)
      printf("%c %llx,%d",op_type,addres,size);

    switch(op_type){
    case 'I':
      break;
    case 'L':
      cache_modify(curr_cache,curr_stats,curr_para,addres,visit_order);
      break;
    case 'S':
      cache_modify(curr_cache,curr_stats,curr_para,addres,visit_order);
      break;
    case 'M':
      cache_modify(curr_cache,curr_stats,curr_para,addres,visit_order);
      cache_modify(curr_cache,curr_stats,curr_para,addres,visit_order);
      break;
    default:
      fprintf(stderr,"invalid operation type\n");
    }

    if(curr_para->v)
      printf("\n");
  }
  
  printSummary(curr_stats->hit, curr_stats->miss, curr_stats->evict);

  cache_free(curr_cache,curr_para);
  free(curr_stats);
  fclose(curr_para->trace);
  free(curr_para);
  free(visit_order);
  return 0;
}
