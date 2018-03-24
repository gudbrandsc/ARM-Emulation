#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int add_s(int a, int b);
int sum_array_s(int *array, int i);
int find_max_s(int *array, int i);
int fib_iter_s(int n);
int fib_rec_s(int n);
int find_str_s(char *string, char *substring);

struct arm_state {
  unsigned int regs[NREGS];
  unsigned char stack[STACK_SIZE];
  int z;
  int n;
  int p;
};


struct emu_analysis_struct {
  int num_instructions_ex;
  int computations;
  int memory;
  int branches;
};

void set_flag(int z, int n, int p, struct arm_state *state){
  state->z = z;
  state->n = n;
  state->p = p;
  
}

void emu_analysis_init(struct emu_analysis_struct *analysis){
  analysis->num_instructions_ex = 0;
  analysis->computations = 0;
  analysis->memory = 0;
  analysis->branches = 0;
}

void arm_state_init(struct arm_state *as, unsigned int *func)
{
    int i;

    /* zero out all arm state */
    for (i = 0; i < NREGS; i++) {
        as->regs[i] = 0;
    }

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) &as->stack[STACK_SIZE];
    as->regs[LR] = 0;
    set_flag(0,0,0,as);
}



void arm_state_print(struct arm_state *as)
{
  int i;
  
    for (i = 0; i < NREGS; i++) {
      printf("reg[%d] = %d\n", i, as->regs[i]);
    }
    printf("cpsr: n=%d z=%d p=%d\n", as->n, as->z, as->p);
}


int get_process_inst(unsigned int iw)
{
  
  unsigned int op;
  unsigned int cmd;
  
    op = (iw >> 26) & 0b11;
    cmd = (iw >> 21) & 0b1111;
    
    if(cmd == 0b0100){ // if its a add
      return 1;
    }else if(cmd == 0b0010){ // if its a sub
      return 2;
    } else if(cmd == 0b1101){ // if its a mov
      return 3;
    }else if(cmd == 0b1010){
      return 4;
    }else{
      return false;
    }
}
int get_memory_inst(struct arm_state *state)
{
  unsigned int op;
  unsigned int load, byte,rn,rd,offset, iw, target, sp, u, b, immediate, value;
  
  iw = *((unsigned int *) state->regs[PC]);  
  load = (iw >> 20) & 0b1;
  byte = (iw >> 22) & 0b1;
  immediate = (iw >> 25) &0b1;
  rn = (iw >> 16) & 0xF;
  rd = (iw >> 12) & 0xF;
  u = (iw >>23) & 0b1;
  sp = state->regs[13];
  
  if(load == 0 && byte == 0){ //STR
    if(immediate == 0){
      offset = iw & 0xFFF; //12 bits
    }else{
      offset = state->regs[iw & 0xF];
    }

    if(u == 1){ 
      if(byte == 0){
	*((unsigned int *)(state->regs[rn] + offset)) = state->regs[rd];    
      }else{
	*((unsigned char *)(state->regs[rn] + offset)) = state->regs[rd];
      }
    }else{
      if( byte == 0){
	*((unsigned int *)(state->regs[rn] - offset)) = state->regs[rd];    
      }else{
	*((unsigned char *)(state->regs[rn] - offset)) = state->regs[rd];
      }
    }
  }else if(load == 1){// LDR
    if(immediate == 0){
      offset = iw & 0xFFF; //12 bits
      value = state->regs[rn] + offset;
    }else{
      offset = iw & 0xF;
      value = state->regs[rn] + state->regs[offset];
    }
    
    if(u == 1){
      if(byte == 0){
	state->regs[rd] = *((unsigned int *)(value));
      }else{
	state->regs[rd] = *((unsigned char *)(value));
      }
    }else{
       if(byte == 0){
	state->regs[rd] = *((unsigned int *)(value));
      }else{
	state->regs[rd] = *((unsigned char *)(value));
      }
    }
    
  }
  state->regs[PC] = state->regs[PC] + 4;
}

int setBit(int value, int b, int index){
  return ( b << index) | value;
}

//Handle branch
void armemu_branch(struct arm_state *state){
  unsigned int iw, type, link_bit; 
  int imm24;

  iw = *((unsigned int *) state->regs[PC]);
  imm24 = iw & 0xFFFFFF;
  type = (iw >> 23) & 0b1;
  link_bit = (iw >> 24) & 0b1;
  
  if(link_bit == 1){
    state->regs[LR] = state->regs[PC] + 4;
  }
  
  imm24 = imm24 << 2;
  for(int i = 31; i >= 23; i--){
    imm24 = setBit(imm24, type, i);
  }
  
  
  if(type == 1){
    imm24 = ~(imm24) + 1;  
    state->regs[PC] += 8;
    state->regs[PC] -= imm24;
  }else{
    state->regs[PC] += 8;
    state->regs[PC] += imm24;
  }
}

unsigned int rightRotate(int n, unsigned int d)
{

  return (n >> d)|(n << (32 - d));
  
}

void armemu_add(struct arm_state *state){
  unsigned int iw,rd, rn, rm, i, rot;
  
  iw = *((unsigned int *) state->regs[PC]);
  i = (iw >> 25) & 1;
  rd = (iw >> 12) & 0xF;
  rn = (iw >> 16) & 0xF;

  if(i == 1){
    rot = (iw >> 8) & 0xF;
    rm = iw & 0xFF;
    rot = rot * 2;
    rm = rightRotate(rm, rot);
    state->regs[rd] = state->regs[rn] + rm;
  }else{
    rm = iw & 0xF;
    state->regs[rd] = state->regs[rn] + state->regs[rm];
    
  }
  
  if (rd != PC) {
    state->regs[PC] = state->regs[PC] + 4;
  }
}

void armemu_sub(struct arm_state *state)
{
  unsigned int iw, rd, rn, rm, i, rot;
  
  iw = *((unsigned int *) state->regs[PC]);
  i = (iw >> 25) & 1;
  rd = (iw >> 12) & 0xF;
  rn = (iw >> 16) & 0xF;
  
    
  if(i == 1){
    rot = (iw >> 8) & 0xF;
    rm = iw & 0xFF;
    rot = rot * 2;
    rm = rightRotate(rm, rot);
    
    state->regs[rd] = state->regs[rn] - rm;
    }else{
      rm = iw & 0xF;
      state->regs[rd] = state->regs[rn] - state->regs[rm];
    }
    
    if (rd != PC) {
      state->regs[PC] = state->regs[PC] + 4;
    }
}

void armemu_mov(struct arm_state *state)
{
  unsigned int iw, rd, rn, rm, i, rot, rsr, shamt5, sh;
    
    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if(i == 1){
      rot = (iw >> 8) & 0xF;
      rm = iw & 0xFF;
      rot = rot * 2;
      rm = rightRotate(rm, rot);
      state->regs[rd] = rm;
    }else{
      rsr = (iw >> 4) & 0b1;
      rm = iw & 0xF;
      if(rsr == 0 ){
	shamt5 = (iw >> 7) & 0b11111;
	sh = (iw >> 5) & 0b11;
	if(sh == 0){
	  state->regs[rd]  = state->regs[rm] << shamt5;
	}else if(sh == 1){
	}
      }else{
      state->regs[rd] = state->regs[rm];
      }
    }
    
    
    if (rd != PC) {
      state->regs[PC] = state->regs[PC] + 4;
    }
}

void armemu_cmp(struct arm_state *state)
{
   unsigned int iw, rd, rn, rm, i;
   int res;
   
    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if(i == 1){
        rm = iw & 0xFF;
	res = state->regs[rn] - rm;
    }else{
        rm = iw & 0xF;
	res = state->regs[rn] - state->regs[rm];
    }
    
    if(res < 0){
      set_flag(0,1,0,state);
    }else if(res > 0){
      set_flag(0,0,1,state);
    }else{
      set_flag(1,0,0,state);
    }
    
    if (rd != PC) {
      state->regs[PC] = state->regs[PC] + 4;
    }
}

bool is_bx_inst(unsigned int iw)
{
  unsigned int bx_code;

  bx_code = (iw >> 4) & 0x00FFFFFF;

  return (bx_code == 0b000100101111111111110001);
}

void armemu_bx(struct arm_state *state)
{
  unsigned int iw;
    unsigned int rn;
    
    iw = *((unsigned int *) state->regs[PC]);
    rn = iw & 0b1111;
    
    state->regs[PC] = state->regs[rn];
}

void armemu_data_process(struct arm_state *state)
{
  unsigned int iw;
  int action_type;
  iw = *((unsigned int *) state->regs[PC]);
  
  if (is_bx_inst(iw)) {
    armemu_bx(state);
  }else{
    action_type = get_process_inst(iw);
    switch(action_type){
    case 1 :
      armemu_add(state);
      break;
    case 2 :
      armemu_sub(state);
      break;
    case 3 :
      armemu_mov(state);
      break;
    case 4:
      armemu_cmp(state);
      break;
    default :
      printf("default");
      exit(1);
    }
  }
}

int get_instruction_type(struct arm_state *state, struct emu_analysis_struct *analysis){
  unsigned int op, iw, cond, run_command;
  iw = *((unsigned int *) state->regs[PC]);
  op = (iw >> 26) & 0b11;
  cond = (iw >> 28) & 0xF;
 
  //GE
  if((cond == 10) && (state->z == 1 || state->p == 1)){
    run_command = 1;
  }else if((cond == 0) && (state->z == 1)){ //EQ
      run_command = 1;
  }else if((cond == 1) && (state->n == 1 || state->p == 1)){ //NE
    run_command = 1;
  }else if((cond == 9) && (state->n == 1 || state->z == 1)){ //LS
      run_command = 1;
  }else if(cond == 11 && state->n == 1){ //LT
      run_command = 1;
  }else if(cond == 14){
    run_command = 1;
  }else{
    run_command = 0;
  }

  if(run_command == 1){
    if(op == 0){
      armemu_data_process(state);
    }else if(op == 1){
      get_memory_inst(state);
    }else if(op == 2){
      armemu_branch (state);
    } 
  }else{
    //    printf("Skiped command\n");
    state->regs[PC] = state->regs[PC] + 4;
  }
  analysis->num_instructions_ex = analysis->num_instructions_ex + 1;
 
}
void print_analysis(struct arm_state *state, struct emu_analysis_struct *analysis){
  printf("------ARM emu analysis------\n");
  arm_state_print(state);
  printf("Number of instructions: %d\n", analysis->num_instructions_ex);
  //Stack   
}

unsigned int armemu(struct arm_state *state, struct emu_analysis_struct *analysis){
 int num_instructions = 0;

 emu_analysis_init(analysis);
 while (state->regs[PC] != 0) {
   get_instruction_type(state, analysis);
   num_instructions += 1;
 }

 //  printf("Num instructions executed: %d\n", num_instructions);
 return state->regs[0];
}



void sum_array_test(struct arm_state *state, int * array1, int size){

  unsigned int res, res_emu, i;
  struct emu_analysis_struct analysis;  
   state->regs[0] = array1;
   state->regs[1] = size; 
   printf("---------- Sum array test ----------\n");
   printf("sum_array_s(");
   for(i = 0; i < size; i++){
     if(i+1 == size){
       printf("%d",array1[i]);
     }else{
       printf("%d, ",array1[i]);
     }
   }
   res = sum_array_s(array1,size);
   printf(") = %d\n",res);
   
   printf("sum_array_s_emu(");
   for(i = 0; i < size; i++){
     if(i + 1 == size){
       printf("%d",array1[i]);
     }else{
       printf("%d, ",array1[i]);
     }
   }
   res_emu = armemu(state, &analysis);
   printf(") = %d\n", res);
   print_analysis(state, &analysis);
}
     
void find_max_test(struct arm_state *state){
  unsigned int res;
  int array[] = {1,2,8,5,4};

   state->regs[0] = array;
   state->regs[1] = 5; 
   printf("------find max test ------\n");
   //   res = armemu(state);
   printf("sum_array_s() = %d\n", res);

}



void find_str_test(struct arm_state *state){
  int res, i;
  char sub_string[] = {"e"};
  char string[] = {"abc"};
   
  state->regs[0] = string;
  state->regs[1] = sub_string; 
  printf("------find substring ------\n");
   
   
   // res = armemu(state);
   printf("find_str_s() = %d\n", res);

}

void fib_iter_test(struct arm_state *state, int n){
  unsigned int res;

  state->regs[0] = n;
   printf("------fib iter with value %d------\n", n);
   //   res = armemu(state);
   printf("fib_iter_s(%d) = %d\n", n, res);
}

void fib_rec_test(struct arm_state *state, int n){
  unsigned int res;
   state->regs[0] = n;
   printf("------fib rec with value %d------\n", n);
   //   res = armemu(state);
   printf("fib_rec_s(%d) = %d\n", n, res);
}



    
int main(int argc, char **argv)
{
  struct arm_state state;

  int int_array1[] = {1,2,3,4,5};
  int size = 5;
  arm_state_init(&state, (unsigned int *) sum_array_s);
  sum_array_test(&state, int_array1, size);

   /*arm_state_init(&state, (unsigned int *) find_max_s);
   find_max_test(&state);

   arm_state_init(&state, (unsigned int *) fib_iter_s);
   fib_iter_test(&state, 20);

   arm_state_init(&state, (unsigned int *) find_max_s);
   sum_array_test(&state);

   arm_state_init(&state, (unsigned int *) fib_rec_s);
   fib_rec_test(&state, 20);

   arm_state_init(&state, (unsigned int *) find_str_s);
   find_str_test(&state);*/
    //unsigned int r;
   //   arm_state_init(&state, (unsigned int *) fib_iter_s, 20, 0, 0, 0);
   // r = armemu(&state);

   
  
  return 0;
}
 
