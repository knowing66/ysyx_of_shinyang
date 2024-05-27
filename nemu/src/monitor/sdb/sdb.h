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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>
//#include "expr.c"

enum{
  y=1,n,
};

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr_of_wp[32];
  word_t oldval;
  //int enb;
  //char *type;
  //word_t address;
  
  //word_t newval;

  /* TODO: Add more members if necessary */

} WP;

word_t expr(char *e, bool *success);
WP* new_wp();
void free_wp(WP *wp);
bool wpval_compare();
void wp_print();


#endif
