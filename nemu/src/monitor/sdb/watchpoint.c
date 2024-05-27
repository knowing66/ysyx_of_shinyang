/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

// typedef struct watchpoint {
//   int NO;
//   struct watchpoint *next;
//   char expr_of_wp[32];
//   int enb;
//   char *type;
//   word_t address;
//   word_t oldval;
//   word_t newval;

//   /* TODO: Add more members if necessary */

// } WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    strcpy(wp_pool[i].expr_of_wp,"");
    wp_pool[i].oldval=0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

//move a wp from free_ to head's tail
WP* new_wp(){
  if(free_==NULL){
    return NULL;//mei you duo yv de kong jian le
  }
  WP *head_tail=head;
  //WP *free_first=free_;

  WP *returnWP=free_;
  free_=free_->next;
  returnWP->next=NULL;

  if(head_tail==NULL){
    head=returnWP;
  }
  else{
    while(head_tail->next!=NULL){
      head_tail=head_tail->next;
    }
    head_tail->next=returnWP;
  }
  
  return returnWP;
}

void free_wp(WP *wp){
  WP *h=head;
  //WP *f=free_;

  if(head==wp){
    head=wp->next;
  }
  else{
    while(h!=NULL && h->next!=wp){//xun zhao wp jie dian wei zhi
      h=h->next;
    }
    h->next=h->next->next;//shan chu jie dian
  }

  wp->next=free_;
  free_=wp;
  strcpy(free_->expr_of_wp,"");
  free_->oldval=0;
}

bool wpval_compare(){
  WP *h=head;
  bool wether_return_true=false;

  while(h!=NULL){
    bool expr_flag;
    expr_flag=false;

    word_t newval=expr(h->expr_of_wp,&expr_flag);

    if(expr_flag==false){
      printf("wp value compare failed");
      assert(0);
    }

    if(newval!=h->oldval){
      printf("Hardware Watchpoint  %d : %s\n",h->NO,h->expr_of_wp);
      printf("Oldvalue = 0x%x\n",h->oldval);
      printf("Newvalue = 0x%x\n",newval);
      h->oldval=newval;
      wether_return_true=true;
    }

    h=h->next;
  }
  if(wether_return_true){
    return true;

  }
  //printf("not change");
  return false;
}
// trying to git

void wp_print(){
  WP *ptr=head;
  if(ptr==NULL){
    printf("NO WATCHPOINT\n");
  }
  else{
    printf("|NO|      |OLDVALUE|       |EXPR|\n");
    while(ptr!=NULL){
      printf("%d         0x%08x       %s\n",ptr->NO,ptr->oldval,ptr->expr_of_wp);
      ptr=ptr->next;
    }
  }
}