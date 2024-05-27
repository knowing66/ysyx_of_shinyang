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

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state=NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args);

static int cmd_help(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_pinx(char *args);

static int cmd_w(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si","Execute the program for N step",cmd_si},
  {"info","Print the val of regesiters or watchpoint",cmd_info},
  {"x","Examine Ram",cmd_x},
  {"p","Evaluate expression",cmd_p},
  {"p/x","Evaluate expression",cmd_pinx},
  {"w","Watch Point",cmd_w},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  if(args!=NULL){
    uint64_t N=*args-'0';
    cpu_exec(N);

  }
  else{
    cpu_exec(1);
  }
  return 0;
}

static int cmd_info(char *args){
  if(*args=='r'){
    isa_reg_display();
  }
  else if(*args=='w'){
    wp_print();
  }
  return 0;
}

static int cmd_x(char *args){
    //unsigned int address;
    int num;
    char *nums4bits= strtok(args, " ");
    char *args2 = nums4bits + strlen(nums4bits) + 1;
    //sscanf(args2,"%x",&address);
    sscanf(nums4bits,"%x",&num);

    bool success;
    word_t expr_val=expr(args2,&success);
    if(success==false){
      printf("something wrong with expr");
      assert(0);
    }

    for(int i=0;i<num;i++){
      printf("0x%08x   =   0x%08x\n",expr_val,vaddr_read(expr_val,4));
      expr_val+=4;
    }
  return 0;
}

static int cmd_pinx(char *args){
    bool success;
    word_t res=expr(args,&success);
    if(success){
      printf("the result is 0x%08x\n",res) ;
    }
    else{
      printf("p/x expr failed");
      assert(0);
    }
    return 0;
}

static int cmd_p(char *args){
  bool *success=(bool *)malloc(sizeof(bool));
  expr(args,success);
  if(*success!=false){
    printf("the result is %u\n",expr(args,success)) ;
  }
  return 0;
  /*FILE *input_file = fopen("/home/shiyang/ics2023/nemu/tools/gen-expr/input", "r");
    if (input_file == NULL) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    // 读取每一行并处理
    char line[256]; // 假设每行不超过 256 个字符
    while (fgets(line, sizeof(line), input_file) != NULL) {
        // 从当前行中提取结果和表达式
        unsigned result;
        char expression[256];
        if (sscanf(line, "%u %[^\n]", &result, expression) != 2) {
            fprintf(stderr, "Error parsing line: %s\n", line);
            continue;
        }

        // 在这里执行你的测试用例逻辑
        // 例如，调用 expr() 函数处理表达式
        bool *success=NULL;
        if(result==expr(expression,success)){
          printf("Result: %u, Expression: %s,right\n", result, expression);;
        }
        else{
          printf("Result: %u, Expression: %s,wrong\n", result, expression);
        }

    }

    // 关闭文件
    fclose(input_file);*/
    //return 0;
}

static int cmd_w(char *args){
  bool success=false;
  word_t val=expr(args,&success);

  if(success==false){
    assert(0);
  }

  WP *wp = new_wp();
	if (wp == NULL) {
		printf("No space to add an extra watchpoint!\n");
		return 0;
	}
  strcpy(wp -> expr_of_wp, args);
	wp -> oldval = val;
	printf("Succefully add watchpoint NO.%d\n", wp -> NO);
	return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

