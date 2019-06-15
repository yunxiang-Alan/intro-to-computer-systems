/*
 * myshell.c: a basic unix shell simulator
 *
 * Author: Qizheng "Alex" Zhang (qizhengz)
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>

void display_error(){
  char error_message[30] = "An error has occurred\n";
  write(STDOUT_FILENO, error_message, strlen(error_message));
}

void myPrint(char *msg)
{
  write(STDOUT_FILENO, msg, strlen(msg));
}

int blank_line(char *str){
  int i = 0;
  while(str[i] != '\0'){
    if(str[i] != ' ' && str[i] != '\n' && str[i] != '\t')
      return 0;
    i++;
  }
  return 1;
}

// decides if a command requires (advanced) redirection or not
int redirect_or_not(char *cmd){
  char *redirection = ">";
  char *ad_redirection = ">+";
  char *ret;
  ret = strstr(cmd,redirection);
  if(ret == NULL){
    return 0; // no redirection
  }
  ret = strstr(cmd,ad_redirection);
  if(ret == NULL){
    return 1; // basic rediection
  }
  else{
    return 2; // advanced redirection
  }
}

// checks if a redirection command contains more than one > or >+
// return 0 if contains more than one, 1 if not
int if_re_legal_one(char *cmd){
  int i = 0, coun = 0;
  while(cmd[i] != '\0'){
    if(cmd[i] == '>')
      coun++;
    i++;
  }
  if(coun > 1){
    return 0;
  }
  else{
    return 1;
  }
}

// checks if a redirection command has more than two args after > or >+
int if_re_legal_two(char *cmd){
  int i = 0;
  if(cmd[0] == '\0') // empty string after no_blanks is also illegal
    return 0;
  while(cmd[i] != '\0'){
    if(cmd[i] == ' ')
      return 0;
    i++;
  }
  return 1;
}

// checks if the contents before the redirection symbol is NULL(after trimming)
// e.g. for commands like "ls > ", return 0
int if_re_legal_three(char *before,char *after){
  if(before == NULL || after == NULL)
    return 0;
  return 1;
}

// checks if the user requires redirection on built-in command
// e.g. for commands like "cd > n.txt", return 0
int if_re_legal_four(char *cmd){
  char token[512];
  int i = 0;
  while(1){
    if(cmd[i] == '\0' || cmd[i] == ' ' || cmd[i] == '\t')
      break;
    token[i] = cmd[i];
    i++;
  }
  token[i] = '\0';
  if(strcmp(token,"pwd") == 0 ||
     strcmp(token,"exit") == 0 ||
     strcmp(token,"cd") == 0){
    return 0;
  }
  return 1;
}

// cover the content of ori with new
void cover_str(char *ori, char *new){
  int i = 0;
  while(new[i] != '\0'){
    ori[i] = new[i];
    i++;
  }
  ori[i] = '\0';
}

// remove frontmost and backmost ' ' \n \t of a given string
char *no_blanks(char *instr, char *outstr){
  int i = 0, j = 0, k = 0;
  if(instr[0] == '\0')
    return "\0";
  while(instr[i] == ' ' || instr[i] == '\n' || instr[i] == '\t'){
    i++;
  }
  while(instr[i] != '\0'){
    outstr[j] = instr[i];
    i++;
    j++;
  }
  outstr[j] = '\0';
  k = j;
  while(outstr[k-1] == ' ' || outstr[k-1] == '\n' || outstr[k-1] == '\t'){
    k--;
  }
  outstr[k] = '\0';
  //printf("outstr is:%s(the end)\n",outstr);
  return outstr;
}

// return the number of arguments in a single command
// e.g. "pwd" gives 1, "cd Desktop" gives 2
int num_arg(char *cmd){
  char *cmd_dup = strdup(cmd);
  int i = 0, num = 0;
  while(cmd_dup[i+1] != '\0'){
    if(cmd_dup[i] == ' ' && cmd_dup[i+1] != ' '){
      num++;
    }
    i++;
  }
  free(cmd_dup);
  return num+1;
}

// generate an array composed of elements of a single command
// note: the array is ended with NULL
void generate_array(int num_argr, char *cmd, char *arr[]){
  int i = 0;
  char *tok_pt;
  char *token = strtok_r(cmd," ",&tok_pt);
  while(token != NULL) {
    arr[i] = token;
    i++;
    token = strtok_r(NULL," ",&tok_pt);
  }
  arr[i] = NULL;
}

int main(int argc, char *argv[])
{
  char cmd_buff[512];
  char *pinput;
  int empty_flag;
  pid_t forkret;
  FILE *batch_file;
  char *str_ptt;
  int origin_stdout_add; // used to point STDOUT back to the screen
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // used in creat()
  
  if(argc == 2){
    batch_file = fopen(argv[1],"r");
    if(batch_file == NULL){
      display_error();
      exit(0);
    } // if batch file doesn't exist, exit
  }
  if(argc > 2){
    display_error();
    exit(0);
  } // if there're two or more input files to the shell program, exit
    
  while (1) {
    empty_flag = 1;
    if(argc == 1){
      myPrint("myshell> ");
      pinput = fgets(cmd_buff, 514, stdin);
      if(!pinput) {
	exit(0);
      }
    }
    else if(argc == 2){
      while(empty_flag){
	pinput = fgets(cmd_buff, 514, batch_file);
	if(!pinput) {
	  exit(0);
	}
	empty_flag = blank_line(pinput);
      } // if there's an empty line, ignore it and fgets the next line
    }
    if(argc == 2) // part of batch mode
      write(STDOUT_FILENO, pinput, strlen(pinput));
    
    if(strlen(pinput) > 512 && pinput[512] != '\n'){
      char curr;
      if(argc == 1)
	curr = getc(stdin);
      else if(argc == 2)
	curr = getc(batch_file);
      while(curr != '\n'){
	putc(curr,stdout);
	if(argc == 1)
	  curr = getc(stdin);
	else if(argc == 2)
	  curr = getc(batch_file);
      }
      putc('\n',stdout);
      fflush(stdout);
      display_error();
      continue;
    } // if single command exceeds 512 bytes, gives error
    
    // remove unnecessary ' ' \n \t
    pinput = no_blanks(cmd_buff,pinput);
    
    char *raw_curr_cmd = malloc(514);
    char *curr_cmd = malloc(514);
    raw_curr_cmd = strtok_r(pinput,";",&str_ptt);
    
    while (raw_curr_cmd != NULL){
      curr_cmd = no_blanks(raw_curr_cmd,curr_cmd); // command w/o unnec spaces
      
      if(curr_cmd[0] == '\0'){
	raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	continue;
      } // if there's an empty command, ignore it and get the next one
      // e.g. pwd ; ; pwd -> ignore the command between the two semicolons

      // redirection starts

      int redirect_flag = redirect_or_not(curr_cmd);
      char *re_pt_one;
      char *re_pt_two;
      int exist_flag;
      int output_fd;

      // only for advanced redirection
      int tmp_fd;
      //int target_fd;
      FILE *target_fp;
      
      char *before_re_tmp;
      char *after_re_tmp;
      char *before_re;
      char *after_re;
      
      if(redirect_flag){ // general entrance for redirection (basic & advanced)
	if(!if_re_legal_one(curr_cmd)){
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	} // gives error if command contains more than one > or >+

	//char *before_re_tmp;
	//char *after_re_tmp;

	if(redirect_flag == 1){
	  before_re_tmp = strtok_r(curr_cmd,">",&re_pt_one);
	  after_re_tmp = strtok_r(NULL,">",&re_pt_one);
	}
	else if(redirect_flag == 2){
	  before_re_tmp = strtok_r(curr_cmd,">+",&re_pt_two);
	  after_re_tmp = strtok_r(NULL,">+",&re_pt_two);
	}

	before_re = before_re_tmp;
	after_re = after_re_tmp;

	if(!if_re_legal_three(before_re,after_re)){
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	} // gives error if contents before or after > or >+
	// is NULL(after trimming)
	// e.g. "ls > ", " > res.txt" -> error
        
	before_re = no_blanks(before_re_tmp,before_re);
	after_re = no_blanks(after_re_tmp,after_re);
	//printf("before_re is:%s\n",before_re);
	//printf("after_re is:%s\n",after_re);
        
	if(!if_re_legal_two(after_re)){
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	} // gives error if there're two or more input files after > or >+
	
	if(!if_re_legal_four(before_re)){
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	} // gives error if the user requires redirection on built-in command

	if(redirect_flag == 1){ // part only for basic redirection
	  // changing file descriptor related with stdout
	  exist_flag = open(after_re,O_WRONLY | O_APPEND);

	  if(exist_flag != -1){
	    display_error();
	    raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	    continue;
	  } // trying to > to an existing file, give an error
	  // note: this check is only for basic redirection
          
	  output_fd = creat(after_re, mode);

	  //printf("output_fd is: %d\n",output_fd);
	  
	  if(output_fd < 0){ // file creation failed
	    display_error();
	    raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	    continue;
	  }
	  
	  origin_stdout_add = dup(1);
	  dup2(output_fd, STDOUT_FILENO);
	}
	
	else if(redirect_flag == 2){ // part only for advanced redirection
	  exist_flag = open(after_re,O_WRONLY | O_APPEND);
	  if(exist_flag == -1){ // a non-existing file
	    output_fd = creat(after_re, mode);
	    origin_stdout_add = dup(1);
	    dup2(output_fd, STDOUT_FILENO);

	    if(output_fd < 0){ // file creation failed
	      display_error();
	      raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	      continue;
	    }
	    
	  }
	  else{
	    // exist_flag != -1, an existing file

	    tmp_fd = creat("tmp_file", mode);
	    target_fp = fopen(after_re,"r");
	    origin_stdout_add = dup(1);
	    dup2(tmp_fd, STDOUT_FILENO); // print to tmp_file
	  }
	  
	}
        
	// now change curr_cmd to the content before > or >+
	// (real command to execvp)
	cover_str(curr_cmd,before_re);
      }
      
      // redirection ends
            
      int num_argr = num_arg(curr_cmd);
      char **cmd_element = malloc((num_argr+1)*sizeof(char *));
      generate_array(num_argr,curr_cmd,cmd_element);
      
      if(strcmp(cmd_element[0],"exit") == 0){
	if(num_argr > 1){
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	}
	else{
	  exit(0);
	}
      }
      else if(strcmp(cmd_element[0],"pwd") == 0){
	if(num_argr > 1){
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	}
	else{
	  char cwd[200];
	  myPrint(getcwd(cwd,200));
	  myPrint("\n");
	}
      }
      else if(strcmp(cmd_element[0],"cd") == 0){
	if(num_argr == 1){
	  chdir(getenv("HOME"));
	}
	else if(num_argr == 2){
	  int error_flag = chdir(cmd_element[1]);
	  if(error_flag){
	    display_error(); // displays an error when cd fails
	    raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	    continue;
	  }
	}
	else{
	  display_error();
	  raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
	  continue;
	}
      }
      else{
	forkret = fork();
	if(forkret == 0){
	  int not_success = 0;
	  not_success = execvp(cmd_element[0],cmd_element);
	  if(not_success){
	    display_error();
	  }
	  exit(0);
	  //printf("exeflag is: %d\n",exeflag);
	}else{
	  int status;
	  waitpid(forkret,&status,0);
	  if(exist_flag == -1){ // change STDOUT_FILENO back to the screen
	    dup2(origin_stdout_add,STDOUT_FILENO);
	  }
	  else if(redirect_flag == 2){
	    char *if_end = NULL;
	    char curr_line[514];

	    close(tmp_fd);
	    if_end = fgets(curr_line,514,target_fp);
	    // append target file & tmp file
	    while(if_end != NULL){
	      myPrint(curr_line);
	      if_end = fgets(curr_line,514,target_fp);
	    }

	    /*
	    // write the tmp file back to the target file
	    while((if_end = read(tmp_fd,curr_line,514)) != 0){
	      write(target_fd,curr_line,514);
	    }
	    */

	    fclose(target_fp);
	    
	    // rename the tmp_file to the target file
	    remove(after_re);
	    rename("tmp_file",after_re);
	    
	    // change STDOUT_FILENO back to the screen
	    dup2(origin_stdout_add,STDOUT_FILENO);

	  }
          
	}
      }
            
      free(cmd_element);
      raw_curr_cmd = strtok_r(NULL,";",&str_ptt);
    } // inner while loop ends
    
    free(raw_curr_cmd);
    free(curr_cmd);
        
  } // while loop ends
  if(argc == 2)
    fclose(batch_file);
  
}
