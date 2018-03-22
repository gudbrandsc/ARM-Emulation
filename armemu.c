#include <stdbool.h>
#include <stdio.h>


#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int add_s(int a, int b);
int sum_array_s(int *array, int i);

struct arm_state {
    unsigned int regs[NREGS];
    unsigned int cpsr;
    unsigned char stack[STACK_SIZE];
};

void arm_state_init(struct arm_state *as, unsigned int *func,
                    unsigned int arg0, unsigned int arg1,
                    unsigned int arg2, unsigned int arg3)
{
    int i;

    /* zero out all arm state */
    for (i = 0; i < NREGS; i++) {
        as->regs[i] = 0;
    }

    as->cpsr = 0;

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) &as->stack[STACK_SIZE];
    as->regs[LR] = 0;
    char array[] = {'a','b','c'};
    as->regs[0] = array;
    as->regs[1] = arg1;
    as->regs[2] = arg2;//arg2;
    as->regs[3] = arg3;
}

void arm_state_print(struct arm_state *as)
{
  int i;
  
    for (i = 0; i < NREGS; i++) {
      printf("reg[%d] = %d\n", i, as->regs[i]);
    }
    printf("cpsr = %X\n", as->cpsr);
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
  unsigned int load, byte,rn,rd,offset, iw, dest, sp, u;
  
  iw = *((unsigned int *) state->regs[PC]);  
  load = (iw >> 20) & 0b1;
  byte = (iw >> 22) & 0b1;
  rn = (iw >> 16) & 0xF;
  rd = (iw >> 12) & 0xF;
  offset = iw & 0xFF;
  u = (iw >>23) & 0b1;
  sp = state->regs[13];
  //TODO Add immitiate
  printf("sp %d\n",sp);
  if(load == 0 && byte == 0){ // STR
    if(u == 1){
          printf("str %d\n", state->regs[rn] + offset);
      *((unsigned int *)state->regs[rn] + offset) = state->regs[rd];
      
    }else{
      *((unsigned int *)state->regs[rn] - offset) = state->regs[rd];
    }
  }else if(load == 0 && byte == 1){
    printf("STRB");
  }else if(load == 1 && byte == 0){
    printf("ldr\n");
    if(u == 1){
      printf("before %d\n",rd);
      state->regs[rd] = (sp + offset);
      printf("after :::: %c\n", state->regs[rd]);
      printf("%d asdasdsadsad\n", sp + offset);  
    }else{
      printf("u0\n");
      *(unsigned int *) state->regs[rn] = state->stack[13] - offset;

    }

  }else if(load == 1 && byte == 1){

    //    state->regs[rd] = *(unsigned int*) state->regs[rn] + rm;
    
    printf("LDRB \n");
    
  }
  state->regs[PC] = state->regs[PC] + 4;
  //  op = (iw >> 26) & 0b11;
  //  opcode = (iw >> 21) & 0b1111; 
 }

//Handle branch
void armemu_branch(struct arm_state *state){
  unsigned int imm24, iw;
  iw = *((unsigned int *) state->regs[PC]);
  imm24 = (iw & 0xFFFFFF);
  state->regs[PC] += 8;
  state->regs[PC] += imm24 << 2;
}

unsigned int rightRotate(int n, unsigned int d)
{
  /* In n<<d, last d bits are 0. To put first 3 bits of n at 
     last, do bitwise or of n<<d with n >>(INT_BITS - d) */
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
    printf("Add R%d, R%d, #%d\n",rd, rn, rm);
    state->regs[rd] = state->regs[rn] + rm;
  }else{
    rm = iw & 0xF;
    printf("Add R%d, R%d, R%d\n", rd, rn, rm);
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
    
    printf("Sub R%d, R%d,  #%d\n", rd, rn, rm);
    state->regs[rd] = state->regs[rn] - rm;
    }else{
      rm = iw & 0xF;
      printf("Sub R%d, R%d,  R%d\n", rd, rn, rm);
      state->regs[rd] = state->regs[rn] - state->regs[rm];
    }
    
    if (rd != PC) {
      state->regs[PC] = state->regs[PC] + 4;
    }
}

void armemu_mov(struct arm_state *state)
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
      printf("MOV R%d, #%d\n", rd, rm);
      state->regs[rd] = rm;
    }else{
      rm = iw & 0xF;
      printf("MOV R%d, R%d\n", rd, rm);
      state->regs[rd] = state->regs[rm];
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
	res = state->regs[rn] - rm;
    }
    
    if(res < 0){
      state->cpsr = 1;
    }else{
      state->cpsr = 0;
    }
    
    printf("cprs is now : %d\n",state->cpsr);
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
    printf("bx lr\n");
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
    }
  }
}

int get_instruction_type(struct arm_state *state){
  unsigned int op, iw, cond;
  iw = *((unsigned int *) state->regs[PC]);
  op = (iw >> 26) & 0b0011;
  cond = (iw >> 28) & 0xF;
  if((cond == 0 || cond == 10 || cond == 12) && (state->cpsr == 0)){
    if(op == 0){
      armemu_data_process(state);
    }else if(op == 1){
      get_memory_inst(state);
    }else if(op == 2){
      armemu_branch (state);
    }
  }else if((cond == 1 || cond == 1) && (state->cpsr == 1)){
    if(op == 0){
      armemu_data_process(state);
    }else if(op == 1){
      get_memory_inst(state);
    }else if(op == 2){
      armemu_branch (state);
    }
  }else if(cond == 14){
  if(op == 0){
      armemu_data_process(state);
    }else if(op == 1){
      get_memory_inst(state);
    }else if(op == 2){
      armemu_branch (state);
    }
  }else{
    printf("invalid cond cpsr:%d and cond = %d\n",state->cpsr, cond);
    state->regs[PC] = state->regs[PC] + 4;
  }
}

unsigned int armemu(struct arm_state *state)
{
  int num_instructions = 0;

  while (state->regs[PC] != 0) {
    arm_state_print(state);
    get_instruction_type(state);
    num_instructions += 1;
  }
  
  printf("Num instructions executed: %d\n", num_instructions);
  return state->regs[0];
}
                  
    
 int main(int argc, char **argv)
 {
   struct arm_state state;
   unsigned int r;
   arm_state_init(&state, (unsigned int *) add_s, 1, 2, 0, 0);
   r = armemu(&state);
   
   printf("r = %d\n", r);
   
   return 0;
 }
 
