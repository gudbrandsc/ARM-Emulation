#include <stdbool.h>
#include <stdio.h>


#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int add_s(int a, int b);

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

    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
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


int get_funct(unsigned int iw)
{
  unsigned int op;
  unsigned int opcode;
  
    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;
    if(op ==0){
      if(opcode == 0b0100){ // if its a add
	return 1;
      }else if(opcode == 0b0010){ // if its a sub
	return 2;
      } else if(opcode == 0b1101){ // if its a mov
	return 3;
      }else{
      return false;
      }
    }
}

void armemu_add(struct arm_state *state){
    unsigned int iw;
    unsigned int rd, rn, rm, i;

    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;
   
    if(i == 1){
      rm = iw & 0xFF;
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
    unsigned int iw;
    unsigned int rd, rn, rm, i;

    iw = *((unsigned int *) state->regs[PC]);

    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;


    if(i == 1){
      rm = iw & 0xF;
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
  unsigned int iw;
  unsigned int rd, rn, rm, i;
    
    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    //    rn = (iw >> 16) & 0xF;

    if(i == 1){
      rm = iw & 0xFF;
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

 void armemu_one(struct arm_state *state)
{
    unsigned int iw;
    int action_type;
    iw = *((unsigned int *) state->regs[PC]);
    
    if (is_bx_inst(iw)) {
      printf("bx lr\n");
      armemu_bx(state);
    }else{
      action_type = get_funct(iw);
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
      default :
	printf("default");
      }
    }
}


unsigned int armemu(struct arm_state *state)
{
  int num_instructions = 0;

  while (state->regs[PC] != 0) {
    arm_state_print(state);
    armemu_one(state);
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
 
