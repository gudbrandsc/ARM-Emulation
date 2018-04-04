#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>

#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

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
    unsigned int regs_write[NREGS];
    unsigned int regs_read[NREGS];
    int num_instructions_ex;
    int data_processing;
    int memory;
    int branches_taken;
    int branches_not_taken;

};

void set_flag(int z, int n, int p, struct arm_state *state) {
    state->z = z;
    state->n = n;
    state->p = p;

}

void emu_analysis_init(struct emu_analysis_struct *analysis) {
    int i;

    analysis->num_instructions_ex = 0;
    analysis->data_processing = 0;
    analysis->memory = 0;
    analysis->branches_taken = 0;
    analysis->branches_not_taken = 0;

    for (i = 0; i < NREGS; i++) {
        analysis->regs_write[i] = 0;
        analysis->regs_read[i] = 0;
    }
}

void arm_state_init(struct arm_state *as, unsigned int *func) {
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
    set_flag(0, 0, 0, as);
}

int get_process_inst(unsigned int iw) {

    unsigned int cmd;

    cmd = (iw >> 21) & 0b1111;

    if (cmd == 0b0100) { // if its a add
        return 1;
    } else if (cmd == 0b0010) { // if its a sub
        return 2;
    } else if (cmd == 0b1101) { // if its a mov
        return 3;
    } else if (cmd == 0b1010) {
        return 4;
    }
    return 0;
}
void set_memory_offset(struct arm_state *state, struct emu_analysis_struct *analysis, unsigned int* offset, unsigned int iw){
    immediate = (iw >> 25) & 0b1;
    if (immediate == 0) {
        offset = iw & 0xFFF;
    } else {
        sh = (iw >> 5) & 0b11;
        shamt5 = (iw >> 7) & 0b111111;
        if (sh == 0) {
            offset = state->regs[iw & 0xF << shamt5];
            analysis->regs_read[iw & 0xF << shamt5] = 1;
        } else {
            offset = state->regs[iw & 0xF];
            analysis->regs_read[iw & 0xF] = 1;
        }
    }
}

void execute_memory_inst(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int load, byte, rn, rd, offset, iw, u, immediate, sh, shamt5;

    iw = *((unsigned int *) state->regs[PC]);
    load = (iw >> 20) & 0b1;
    byte = (iw >> 22) & 0b1;
    rn = (iw >> 16) & 0xF;
    rd = (iw >> 12) & 0xF;
    u = (iw >> 23) & 0b1;
    set_memory_offset(state, analysis, &offset, iw);
    if (load == 0 && byte == 0) { //Store
        if (u == 1) {
            analysis->regs_write[rd] = 1;
            analysis->regs_read[rn] = 1;
            if (byte == 0) {
                *((unsigned int *) (state->regs[rn] + offset)) = state->regs[rd];
            } else {
                *((unsigned char *) (state->regs[rn] + offset)) = state->regs[rd];
            }
        } else {
            if (byte == 0) {
                *((unsigned int *) (state->regs[rn] - offset)) = state->regs[rd];
            } else {
                *((unsigned char *) (state->regs[rn] - offset)) = state->regs[rd];
            }
        }
    } else if (load == 1) {// load
        if (immediate == 0) {
            offset = iw & 0xFFF;
        } else {
            sh = (iw >> 5) & 0b11;
            shamt5 = (iw >> 7) & 0b11111;
            if (sh == 0) {
                offset = state->regs[iw & 0xF << shamt5];
                analysis->regs_read[iw & 0xF << shamt5] = 1;
            } else {
                offset = state->regs[iw & 0xF];
                analysis->regs_read[iw & 0xF] = 1;
            }
            analysis->regs_read[rn] = 1;
        }
        analysis->regs_write[rd] = 1;

        if (u == 1) {
            if (byte == 0) {
                state->regs[rd] = *((unsigned int *) (state->regs[rn] + offset));
            } else {
                state->regs[rd] = *((unsigned char *) (state->regs[rn] + offset));
            }
        } else {
            if (byte == 0) {
                state->regs[rd] = *((unsigned int *) (state->regs[rn] - offset));
            } else {
                state->regs[rd] = *((unsigned char *) (state->regs[rn] - offset));
            }
        }
    }

    state->regs[PC] = state->regs[PC] + 4;
}



int setBit(int value, int b, int index) {
    return (b << index) | value;
}

void execute_branch_inst(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int iw, type, link_bit;
    int imm24;

    iw = *((unsigned int *) state->regs[PC]);
    imm24 = iw & 0xFFFFFF;
    type = (iw >> 23) & 0b1;
    link_bit = (iw >> 24) & 0b1;
    analysis->branches_taken += 1;

    if (link_bit == 1) {
        state->regs[LR] = state->regs[PC] + 4;
    }

    imm24 = imm24 << 2;
    for (int i = 31; i >= 25; i--) {
        imm24 = setBit(imm24, type, i);
    }

    if (type == 1) {
        imm24 = ~(imm24) + 1;
        state->regs[PC] += 8;
        state->regs[PC] -= imm24;
    } else {
        state->regs[PC] += 8;
        state->regs[PC] += imm24;
    }
}

int rightRotate(int n, unsigned int d) {
    return (n >> d) | (n << (32 - d));
}

void execute_add_inst(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int iw, rd, rn, rm, i, rot;

    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (i == 1) {
        rot = (iw >> 8) & 0xF;
        rm = iw & 0xFF;
        rot = rot * 2;
        rm = rightRotate(rm, rot);
        analysis->regs_read[rn] = 1;
        analysis->regs_write[rd] = 1;
        state->regs[rd] = state->regs[rn] + rm;
    } else {
        rm = iw & 0xF;
        analysis->regs_write[rd] = 1;
        analysis->regs_read[rn] = 1;
        analysis->regs_read[rm] = 1;
        state->regs[rd] = state->regs[rn] + state->regs[rm];
    }

    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

void execute_sub_inst(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int iw, rd, rn, rm, i, rot;

    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (i == 1) {
        rot = (iw >> 8) & 0xF;
        rm = iw & 0xFF;
        rot = rot * 2;
        rm = rightRotate(rm, rot);
        analysis->regs_read[rn] = 1;
        analysis->regs_write[rd] = 1;
        state->regs[rd] = state->regs[rn] - rm;
    } else {
        rm = iw & 0xF;
        analysis->regs_write[rd] = 1;
        analysis->regs_read[rn] = 1;
        analysis->regs_read[rm] = 1;
        state->regs[rd] = state->regs[rn] - state->regs[rm];
    }

    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

void execute_mov_inst(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int iw, rd, rm, i, rot, rsr, shamt5, sh;

    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;

    if (i == 1) {
        rot = (iw >> 8) & 0xF;
        rm = iw & 0xFF;
        rot = rot * 2;
        rm = rightRotate(rm, rot);
        analysis->regs_write[rd] = 1;
        state->regs[rd] = rm;
    } else {
        rsr = (iw >> 4) & 0b1;
        rm = iw & 0xF;
        if (rsr == 0) {
            shamt5 = (iw >> 7) & 0b11111;
            sh = (iw >> 5) & 0b11;
            analysis->regs_write[rd] = 1;
            analysis->regs_read[rm] = 1;
            if (sh == 0) {
                state->regs[rd] = state->regs[rm] << shamt5;
            }
            //Could add more sh commands if needed
        } else {
            state->regs[rd] = state->regs[rm];
        }
    }

    if (rd != PC) {
        analysis->regs_write[PC] = 1;
        analysis->regs_read[PC] = 1;
        state->regs[PC] = state->regs[PC] + 4;
    }
}

void execute_cmp_inst(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int iw, rd, rn, rm, i;
    int res;

    iw = *((unsigned int *) state->regs[PC]);
    i = (iw >> 25) & 1;
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (i == 1) {
        rm = iw & 0xFF;
        analysis->regs_read[rn] = 1;
        res = state->regs[rn] - rm;
    } else {
        rm = iw & 0xF;
        analysis->regs_read[rn] = 1;
        analysis->regs_read[rm] = 1;
        res = state->regs[rn] - state->regs[rm];
    }

    if (res < 0) {
        set_flag(0, 1, 0, state);
    } else if (res > 0) {
        set_flag(0, 0, 1, state);
    } else {
        set_flag(1, 0, 0, state);
    }

    if (rd != PC) {
        analysis->regs_write[PC] = 1;
        analysis->regs_read[PC] = 1;
        state->regs[PC] = state->regs[PC] + 4;
    }
}

bool is_bx_inst(unsigned int iw) {
    unsigned int bx_code;

    bx_code = (iw >> 4) & 0x00FFFFFF;

    return (bx_code == 0b000100101111111111110001);
}

void armemu_bx(struct arm_state *state) {
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) state->regs[PC]);
    rn = iw & 0b1111;

    state->regs[PC] = state->regs[rn];
}

void execute_data_process(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int iw;
    int action_type;

    iw = *((unsigned int *) state->regs[PC]);

    if (is_bx_inst(iw)) {
        analysis->branches_taken += 1;
        armemu_bx(state);
    } else {
        action_type = get_process_inst(iw);
        analysis->data_processing += 1;
        switch (action_type) {
            case 1 :
                execute_add_inst(state, analysis);
                break;
            case 2 :
                execute_sub_inst(state, analysis);
                break;
            case 3 :
                execute_mov_inst(state, analysis);
                break;
            case 4:
                execute_cmp_inst(state, analysis);
                break;
            default :
                exit(1);
        }
    }
}

void execute_instruction(struct arm_state *state, struct emu_analysis_struct *analysis) {
    unsigned int op, iw, cond, run_command;

    iw = *((unsigned int *) state->regs[PC]);
    op = (iw >> 26) & 0b11;
    cond = (iw >> 28) & 0xF;

    //GE
    if ((cond == 10) && (state->z == 1 || state->p == 1)) {
        run_command = 1;
    } else if ((cond == 0) && (state->z == 1)) { //EQ
        run_command = 1;
    } else if ((cond == 1) && (state->n == 1 || state->p == 1)) { //NE
        run_command = 1;
    } else if ((cond == 9) && (state->n == 1 || state->z == 1)) { //LS
        run_command = 1;
    } else if (cond == 11 && state->n == 1) { //LT
        run_command = 1;
    } else if (cond == 14) {
        run_command = 1;
    } else {
        run_command = 0;
    }

    if (run_command == 1) {
        if (op == 0) {
            execute_data_process(state, analysis);
        } else if (op == 1) {
            analysis->memory += 1;
            execute_memory_inst(state, analysis);
        } else if (op == 2) {
            execute_branch_inst(state, analysis);
        }
    } else {
        if (op == 0) {
            analysis->data_processing += 1;
        } else if (op == 1) {
            analysis->memory += 1;
        } else if (op == 2) {
            analysis->branches_not_taken += 1;
        }
        state->regs[PC] = state->regs[PC] + 4;
    }
    analysis->num_instructions_ex += 1;
}

int get_procentage(int val, int total) {
    float value = (val / (float) total) * 100;
    return (int) value;
}

void print_analysis(struct emu_analysis_struct *analysis) {
    int i, count, dp, nie, bt, bnt, mem, tot_branches;
    nie = analysis->num_instructions_ex;
    dp = analysis->data_processing;
    mem = analysis->memory;
    bt = analysis->branches_taken;
    bnt = analysis->branches_not_taken;
    tot_branches = bt + bnt;
    printf(" ______________________________________________\n");
    printf("|  Dynamic analysis of the function execution  |\n");
    printf(" ----------------------------------------------\n");
    printf("|- Number of instructions executed: %d\n", nie);
    printf("|- Instruction counts :\n");
    printf("|\t- Data_processing : %d (%d%c)\n", dp, get_procentage(dp, nie), 37);
    printf("|\t- Memory : %d (%d%c)\n", mem, get_procentage(mem, nie), 37);
    printf("|\t- Branches : %d (%d%c)\n", tot_branches, get_procentage(tot_branches, nie), 37);
    printf("|\t- Branches taken: %d (%d%c)\n", bt, get_procentage(bt, nie), 37);
    printf("|\t- Branches not taken: %d (%d%c)\n", bnt, get_procentage(bnt, nie), 37);
    printf("|- Read registers:  ");

    count = 0;
    for (i = 0; i < NREGS; i++) {
        if (analysis->regs_read[i] == 1) {
            if ((count % 5 == 0) && count != 0) {
                printf("\n|\t\t    ");
                count = 0;
            }
            count += 1;
            if (i == 13) {
                printf("SP");
            } else if (i == 14) {
                printf("LR");
            } else if (i == 15) {
                printf("PC");
            } else {
                printf("r%d", i);
            }
            if (i + 1 != NREGS) {
                printf(", ");
            }
        }
    }

    count = 0;
    printf("\n|- Write registers: ");
    for (i = 0; i < NREGS; i++) {
        if (analysis->regs_write[i] == 1) {
            if ((count % 5 == 0) && count != 0) {
                printf("\n|\t\t    ");
                count = 0;
            }
            count += 1;
            if (i == 13) {
                printf("SP");
            } else if (i == 14) {
                printf("LR");
            } else if (i == 15) {
                printf("PC");
            } else {
                printf("r%d", i);
            }
            if (i + 1 != NREGS) {
                printf(", ");
            }
        }
    }
    printf("\n");
}

unsigned int emulate_arm_func(struct arm_state *state, struct emu_analysis_struct *analysis) {
    int num_instructions = 0;
    emu_analysis_init(analysis);

    while (state->regs[PC] != 0) {
        execute_instruction(state, analysis);
        num_instructions += 1;
    }
    return state->regs[0];
}

void
get_execution_time_analysis(struct arm_state *state, struct emu_analysis_struct *analysis, int *array, int array_size,
                            int fib_num, char *string, char *substring) {
    int res, emu_time, reg_time;
    int num = 500000;
    int num_small = 300;
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;

    //Get time analysis for sum array
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        arm_state_init(state, (unsigned int *) sum_array_s);
        state->regs[0] = array;
        state->regs[1] = array_size;
        res = emulate_arm_func(state, analysis);
    }

    en_time = times(&en_cpu);
    emu_time = (int) (en_time - st_time);
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        sum_array_s(array, array_size);
    }

    en_time = times(&en_cpu);
    reg_time = (int) (en_time - st_time);
    printf("- sum_array_s() with %d elements\n", array_size);
    printf("\t- %d executions:\n", num);
    printf("\t\t- Emulated: %f clockticks\n", emu_time / (float) num);
    printf("\t\t- Nativ: %f clockticks,\n", reg_time / (float) num);
    printf("\t\t- Ratio: %f clockticks\n\n", emu_time / (float) reg_time);

    //Get time analysis for find max
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        arm_state_init(state, (unsigned int *) find_max_s);
        state->regs[0] = array;
        state->regs[1] = array_size;
        res = emulate_arm_func(state, analysis);
    }

    en_time = times(&en_cpu);
    emu_time = (int) (en_time - st_time);
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        find_max_s(array, array_size);
    }

    en_time = times(&en_cpu);
    reg_time = (int) (en_time - st_time);
    printf("- find_max_s() with %d elements\n", array_size);
    printf("\t- %d executions:\n", num);
    printf("\t\t- Emulated: %f clockticks\n", emu_time / (float) num);
    printf("\t\t- Nativ: %f clockticks,\n", reg_time / (float) num);
    printf("\t\t- Ratio: %f clockticks\n\n", emu_time / (float) reg_time);

    //Get time analysis for iterative fib
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        arm_state_init(state, (unsigned int *) fib_iter_s);
        state->regs[0] = fib_num;
        res = emulate_arm_func(state, analysis);
    }

    en_time = times(&en_cpu);
    emu_time = (int) (en_time - st_time);
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        fib_iter_s(fib_num);
    }

    en_time = times(&en_cpu);
    reg_time = (int) (en_time - st_time);
    printf("- fib_iter_s() sequence number  %d \n", fib_num);
    printf("\t- %d executions:\n", num);
    printf("\t\t- Emulated: %f clockticks\n", emu_time / (float) num);
    printf("\t\t- Nativ: %f clockticks,\n", reg_time / (float) num);
    printf("\t\t- Ratio: %f clockticks\n\n", emu_time / (float) reg_time);

    //Get time analysis for recursive fib
    st_time = times(&st_cpu);

    for (int i = 0; i <= num_small; i++) {
        arm_state_init(state, (unsigned int *) fib_rec_s);
        state->regs[0] = fib_num;
        res = emulate_arm_func(state, analysis);
    }

    en_time = times(&en_cpu);
    emu_time = (int) (en_time - st_time);
    st_time = times(&st_cpu);

    for (int i = 0; i <= num_small; i++) {
        fib_rec_s(fib_num);
    }

    en_time = times(&en_cpu);
    reg_time = (int) (en_time - st_time);
    printf("- fib_rec_s() sequence number  %d \n", fib_num);
    printf("\t- %d executions:\n", num_small);
    printf("\t\t- Emulated: %f clockticks\n", emu_time / (float) num_small);
    printf("\t\t- Nativ: %f clockticks,\n", reg_time / (float) num_small);
    printf("\t\t- Ratio: %f clockticks\n\n", emu_time / (float) reg_time);

    //Get time analysis for find sub string
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        arm_state_init(state, (unsigned int *) find_str_s);
        state->regs[0] = string;
        state->regs[1] = substring;
        res = emulate_arm_func(state, analysis);
    }

    en_time = times(&en_cpu);
    emu_time = (int) (en_time - st_time);
    st_time = times(&st_cpu);

    for (int i = 0; i <= num; i++) {
        find_str_s(string, substring);
    }

    en_time = times(&en_cpu);
    reg_time = (int) (en_time - st_time);
    printf("- find_str_s() with string and substring \"%s\": \"%s\"\n", string, substring);
    printf("\t- %d executions:\n", num);
    printf("\t\t- Emulated: %f clockticks\n", emu_time / (float) num);
    printf("\t\t- Nativ: %f clockticks,\n", reg_time / (float) num);
    printf("\t\t- Ratio: %f clockticks\n\n", emu_time / (float) reg_time);
}

void sum_array_test(struct arm_state *state, int *array, int size, struct emu_analysis_struct *analysis) {
    unsigned int res, res_emu, i;

    state->regs[0] = array;
    state->regs[1] = size;
    printf(" sum_array_s({");
    if (size > 10) {
        printf("%d,%d,%d...%d", array[0], array[1], array[2], array[size - 1]);
    } else {
        for (i = 0; i < size; i++) {
            if (i + 1 == size) {
                printf("%d", array[i]);
            } else {
                printf("%d, ", array[i]);
            }
        }
    }
    res = sum_array_s(array, size);

    printf("}, %d) = %d\n", size, res);
    printf(" sum_array_s({");
    if (size > 10) {
        printf("%d,%d,%d...%d", array[0], array[1], array[2], array[size - 1]);
    } else {
        for (i = 0; i < size; i++) {
            if (i + 1 == size) {
                printf("%d", array[i]);
            } else {
                printf("%d, ", array[i]);
            }
        }
    }
    res_emu = emulate_arm_func(state, analysis);
    printf("}, %d) = %d (Emulator)\n", size, res_emu);
}

void find_max_test(struct arm_state *state, int *array, int size, struct emu_analysis_struct *analysis) {
    unsigned int res, res_emu, i;
    state->regs[0] = array;
    state->regs[1] = size;

    printf(" find_max_s({");
    if (size > 10) {
        printf("%d,%d,%d...%d", array[0], array[1], array[2], array[size - 1]);
    } else {
        for (i = 0; i < size; i++) {
            if (i + 1 == size) {
                printf("%d", array[i]);
            } else {
                printf("%d, ", array[i]);
            }
        }
    }
    res = find_max_s(array, size);

    printf("}, %d) = %d\n", size, res);
    printf(" find_max_s({");
    if (size > 10) {
        printf("%d,%d,%d...%d", array[0], array[1], array[2], array[size - 1]);
    } else {
        for (i = 0; i < size; i++) {
            if (i + 1 == size) {
                printf("%d", array[i]);
            } else {
                printf("%d, ", array[i]);
            }
        }
    }
    res_emu = emulate_arm_func(state, analysis);
    printf("}, %d) = %d (Emulator)\n", size, res_emu);
}

void find_str_test(struct arm_state *state, char *string, char *substring, struct emu_analysis_struct *analysis) {
    state->regs[0] = string;
    state->regs[1] = substring;

    printf(" find_str_s(%s,%s) = %d\n", string, substring, find_str_s(string, substring));
    printf(" find_str_s(%s,%s) = %d (Emulator)\n", string, substring, emulate_arm_func(state, analysis));
}

void populate_large_array(int *array, int size) {
    int i;

    for (i = 1; i < size + 1; i++) {
        array[i - 1] = i;
    }
}

void run_sum_array_tests(struct arm_state *state, struct emu_analysis_struct *analysis,
                         int *array1, int *array2, int *array3, int *array4, int size) {

    printf(" \n--------------- Sum Array Tests ----------------\n");
    arm_state_init(state, (unsigned int *) sum_array_s);
    sum_array_test(state, array1, size, analysis);
    arm_state_init(state, (unsigned int *) sum_array_s);
    sum_array_test(state, array2, size, analysis);
    arm_state_init(state, (unsigned int *) sum_array_s);
    sum_array_test(state, array3, size, analysis);
    arm_state_init(state, (unsigned int *) sum_array_s);
    sum_array_test(state, array4, 2000, analysis);
}

void run_find_max_tests(struct arm_state *state, struct emu_analysis_struct *analysis,
                        int *array1, int *array2, int *array3, int *array4, int size) {
    printf(" \n--------------- Find Max Tests ----------------\n");
    arm_state_init(state, (unsigned int *) find_max_s);
    find_max_test(state, array1, size, analysis);
    arm_state_init(state, (unsigned int *) find_max_s);
    find_max_test(state, array2, size, analysis);
    arm_state_init(state, (unsigned int *) find_max_s);
    find_max_test(state, array3, size, analysis);
    arm_state_init(state, (unsigned int *) find_max_s);
    find_max_test(state, array4, 2000, analysis);
}

void run_fib_iter_tests(struct arm_state *state, struct emu_analysis_struct *analysis, int n) {
    unsigned int i;
    printf(" \n--------------- Testing Iterative Fibonacci Sequence To: %d ----------------\n", n);

    printf(" fib_iter_s: ");
    for (i = 0; i <= n; i++) {
        if (i == n) {
            printf("%d\n", fib_iter_s(i));
        } else {
            printf("%d, ", fib_iter_s(i));
        }
    }

    printf(" fib_iter_s: ");
    for (i = 0; i <= n; i++) {
        arm_state_init(state, (unsigned int *) fib_iter_s);
        state->regs[0] = i;
        if (i == n) {
            printf("%d", emulate_arm_func(state, analysis));
        } else {
            printf("%d, ", emulate_arm_func(state, analysis));
        }
    }
    printf(" (Emulator)\n");
}

void run_fib_rec_tests(struct arm_state *state, struct emu_analysis_struct *analysis, int n) {
    unsigned int i;
    printf(" \n--------------- Testing Recursive Fibonacci Sequence To: %d ----------------\n", n);
    printf(" fib_rec_s: ");
    for (i = 0; i <= n; i++) {
        if (i == n) {
            printf("%d\n", fib_rec_s(i));
        } else {
            printf("%d, ", fib_rec_s(i));
        }
    }
    printf(" fib_rec_s: ");
    for (i = 0; i <= n; i++) {
        arm_state_init(state, (unsigned int *) fib_rec_s);
        state->regs[0] = i;
        if (i == n) {
            printf("%d", emulate_arm_func(state, analysis));
        } else {
            printf("%d, ", emulate_arm_func(state, analysis));
        }
    }
    printf(" (Emulator)\n");
}

void run_find_str_tests(struct arm_state *state, struct emu_analysis_struct *analysis,
                        char *string1, char *string2, char *substring1, char *substring2, char *substring3) {
    printf(" \n--------------- Find Max Tests ----------------\n");
    arm_state_init(state, (unsigned int *) find_str_s);
    find_str_test(state, string1, substring1, analysis);
    arm_state_init(state, (unsigned int *) find_str_s);
    find_str_test(state, string1, substring2, analysis);
    arm_state_init(state, (unsigned int *) find_str_s);
    find_str_test(state, string2, substring3, analysis);
    arm_state_init(state, (unsigned int *) find_str_s);
}

void sum_array_analysis(struct arm_state *state, int *array, int size, struct emu_analysis_struct *analysis) {
    printf(" \n--------------- Sum Array Analysis ----------------\n");
    arm_state_init(state, (unsigned int *) sum_array_s);
    sum_array_test(state, array, size, analysis);
    print_analysis(analysis);
}

void find_max_analysis(struct arm_state *state, int *array, int size, struct emu_analysis_struct *analysis) {
    printf(" \n--------------- Find Max Analysis ----------------\n");
    arm_state_init(state, (unsigned int *) find_max_s);
    find_max_test(state, array, size, analysis);
    print_analysis(analysis);
}

void fib_iter_analysis(struct arm_state *state, int fib_num, struct emu_analysis_struct *analysis) {
    int res;
    printf(" \n--------------- Fib Iterativ Analysis  ----------------\n");
    arm_state_init(state, (unsigned int *) fib_iter_s);
    state->regs[0] = fib_num;
    res = emulate_arm_func(state, analysis);
    printf("fib_rec_s(%d) = %d\n", fib_num, res);
    print_analysis(analysis);
}

void fib_rec_analysis(struct arm_state *state, int fib_num, struct emu_analysis_struct *analysis) {
    int res;
    printf(" \n--------------- Fib Recursive Analysis ----------------\n");
    arm_state_init(state, (unsigned int *) fib_rec_s);
    state->regs[0] = fib_num;
    res = emulate_arm_func(state, analysis);
    printf("fib_rec_s(%d) = %d\n", fib_num, res);
    print_analysis(analysis);
}

void find_str_analysis(struct arm_state *state, char *string, char *substring,
                       struct emu_analysis_struct *analysis) {
    printf(" \n--------------- Find substring Analysis ----------------\n");
    arm_state_init(state, (unsigned int *) find_str_s);
    find_str_test(state, string, substring, analysis);
    print_analysis(analysis);
}

void print_header(char *string) {
    printf(" ________________________________________________________________________________________\n");
    printf("|                             %s                                        |\n", string);
    printf(" ----------------------------------------------------------------------------------------\n");
}

int main(int argc, char **argv) {
    struct arm_state state;
    struct emu_analysis_struct analysis;
    int array1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[] = {-5, -8, -3, -4, -1, -6, -7, -2, -9, -10};
    int array3[] = {-1, 0, 8, 2, 0, 0, -2, 3, 7, -5};
    int array4[2000];
    int size = 10;
    int fib_num = 20;
    char string1[] = "abcddfghijdde";
    char substring1[] = {"dfgh"};
    char substring2[] = {"ddee"};
    char string2[] = {" "};
    char substring3[] = {" "};

    populate_large_array(array4, 2000);
    print_header("Functionality Tests");
    run_sum_array_tests(&state, &analysis, array1, array2, array3, array4, size);
    run_find_max_tests(&state, &analysis, array1, array2, array3, array4, size);
    run_fib_iter_tests(&state, &analysis, fib_num);
    run_fib_rec_tests(&state, &analysis, fib_num);
    run_find_str_tests(&state, &analysis, string1, string2, substring1, substring2, substring3);

    print_header("Arm Emulator Analysis");
    sum_array_analysis(&state, array1, size, &analysis);
    find_max_analysis(&state, array1, size, &analysis);
    fib_iter_analysis(&state, fib_num, &analysis);
    fib_rec_analysis(&state, fib_num, &analysis);
    find_str_analysis(&state, string1, substring1, &analysis);

    print_header("Performance Measurements");
    get_execution_time_analysis(&state, &analysis, array1, size, fib_num, string1, substring1);
    return 0;
}
