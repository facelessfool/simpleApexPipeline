/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1
int zeroFlag=1;
int ptr=-1;
int temp=0;


/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);



  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
   if (strcmp(stage->opcode, "ADD") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

   if (strcmp(stage->opcode, "ADDL") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

   if (strcmp(stage->opcode, "SUB") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "SUBL") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

   if (strcmp(stage->opcode, "AND") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
   if (strcmp(stage->opcode, "EX-OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

   if (strcmp(stage->opcode, "MUL") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "BZ") == 0) {
    printf("%s,#%d", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "NOP") == 0) {
    printf("NOP");
  }

  if (strcmp(stage->opcode, "HALT\n") == 0) {
    printf("HALT");
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu,const char* command)
{
  CPU_Stage* stage = &cpu->stage[F];
/*  if(jflag==1){
    cpu->pc=nextAddress;
  }
  */

  if (!stage->busy && !stage->stalled) {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    if(cpu->stage[DRF].stalled==0)
    {
    /* Update PC for next instruction */
    cpu->pc += 4;
    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];
  }

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
   
  else{
       CPU_Stage nop;
          memset(&nop, 0, sizeof(nop));
          memcpy(&nop.opcode, "NOP", 3);
          cpu->stage[DRF] = nop;

      //strcpy(cpu->stage[DRF].opcode, "NOP");

     
    }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu, const char* command)
{
  CPU_Stage* stage = &cpu->stage[DRF];

   // printf("opcode print %s", cpu->stage[EX1].opcode);

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {

      if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
    }
    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];
      //cpu->stage[EX1]=cpu->stage[DRF];
    }



    }

    if (strcmp(stage->opcode, "LOAD") == 0) {

      if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
    }
    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];

      //cpu->stage[EX1]=cpu->stage[DRF];
    }



    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) 
    {

    }

    if (strcmp(stage->opcode, "BZ") == 0) 
    {

   // printf("aba\n");


      if(strcmp(cpu->stage[EX1].opcode, "SUB")==0 || strcmp(cpu->stage[EX1].opcode, "MUL")==0 || strcmp(cpu->stage[EX1].opcode, "ADD")==0  ){
       ptr=0;
       //printf("inside conditin");
      }

      if(ptr!=5 && ptr>=0){
        stage->stalled=1;
        ptr +=1;
      }
      if(ptr==5){
        stage->stalled=0;
      }

    }



    

    if (strcmp(stage->opcode, "HALT\n") == 0) 
    {
        cpu->stage[F].busy=1;
    }

    if (strcmp(stage->opcode, "ADD") == 0){
      //printf("regs valid %d\n",cpu->regs_valid[stage->rs1]);

    if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
      //printf("Drf for add stall %d",  stage->stalled);

    }

   
      else {
      //  printf("going inside else");
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];
      //cpu->stage[EX1]=cpu->stage[DRF];
    }
   }

    if (strcmp(stage->opcode, "SUB") == 0){
    if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
      

    }

    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];
      }
   }

   if (strcmp(stage->opcode, "OR") == 0) {
  stage->rs1_value=cpu->regs[stage->rs1];
  stage->rs2_value=cpu->regs[stage->rs2];
 // cpu->regs_valid[stage->rd] = 0;
    }
  if (strcmp(stage->opcode, "EX-OR") == 0) {
  stage->rs1_value=cpu->regs[stage->rs1];
  stage->rs2_value=cpu->regs[stage->rs2];
 // cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "ADDL") == 0){
    if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
      

    }

    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->imm];
      }
   }

    if (strcmp(stage->opcode, "SUBL") == 0){
    if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
      

    }

    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->imm];
      }
   }


   if (strcmp(stage->opcode, "MUL") == 0){
    if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
      

    }

    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];
      }
   }

    if (strcmp(stage->opcode, "AND") == 0){
    if(cpu->regs_valid[stage->rs1]==0||cpu->regs_valid[stage->rs2]==0){
      stage->stalled=1;
      

    }

    else {
      stage->stalled=0;
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];
      }
   }






    /* Copy data from decode latch to execute latch*/
    if(stage->stalled==0 && stage->busy==0){
      cpu->stage[EX1]=cpu->stage[DRF];
    }
    else{ CPU_Stage nop;
          memset(&nop, 0, sizeof(nop));
          memcpy(&nop.opcode, "NOP", 3);
      cpu->stage[EX1]=nop;

     
    }

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute1(APEX_CPU* cpu,const char* command)
{
  CPU_Stage* stage = &cpu->stage[EX1];
  if(cpu->stage[DRF].stalled==1)
  {
    stage->stalled=1;
  }

  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
       cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "SUBL") == 0) {
   
    /* Making it Invalid */
    cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "ADDL") == 0) {
   
    /* Making it Invalid */
    cpu->regs_valid[stage->rd] = 0;
    }

     if (strcmp(stage->opcode, "ADD") == 0) {
   
    /* Making it Invalid */
    cpu->regs_valid[stage->rd] = 0;
    }


    if (strcmp(stage->opcode, "AND") == 0) {
    cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "OR") == 0) {
    cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "EX-OR") == 0) {
    cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "MUL") == 0) {
    cpu->regs_valid[stage->rd] = 0;
    }


    if (strcmp(stage->opcode, "SUB") == 0) {
    
    /* Making Invalid */
    cpu->regs_valid[stage->rd] = 0;
    }



    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute1", stage);
    }
  }

  else{
     CPU_Stage nop;
          memset(&nop, 0, sizeof(nop));
          memcpy(&nop.opcode, "NOP", 3);
          cpu->stage[EX2] = nop;
   // strcpy(cpu->stage[EX2].opcode, "NOP");

     if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute1", stage);
    }
  }
  return 0;
}

int
execute2(APEX_CPU* cpu,const char* command)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if(cpu->stage[EX1].stalled==1)
  {
    stage->stalled=1;
  }

  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address= stage->rs2_value+stage->imm;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer = stage->imm+0;

    }

    if (strcmp(stage->opcode, "BZ") == 0){

     // printf("BF zf");
      if(zeroFlag==1){
       // printf("inside zflag ex2");
         temp=stage->pc+stage->imm;
        int temp1=temp%4;
        temp=temp-temp1;

      //  cpu->pc=temp;
        
      }

    }

    if (strcmp(stage->opcode, "ADD") == 0) {
  stage->buffer= stage->rs1_value + stage->rs2_value;
    }

    if (strcmp(stage->opcode, "ADDL") == 0) {
  stage->buffer= stage->rs1_value + stage->rs2_value;
    }
    if (strcmp(stage->opcode, "SUB") == 0) {
  stage->buffer= stage->rs1_value - stage->rs2_value;
    }

    if (strcmp(stage->opcode, "SUBL") == 0) {
  stage->buffer= stage->rs1_value - stage->rs2_value;
    }

    if (strcmp(stage->opcode, "AND") == 0) {
  stage->buffer=stage->rs1_value & stage->rs2_value;
    }

    if (strcmp(stage->opcode, "OR") == 0) {
  stage->buffer=stage->rs1_value | stage->rs2_value;
    }

    /* MUL */
  if (strcmp(stage->opcode, "MUL") == 0) {
    stage->buffer=stage->rs1_value*stage->rs2_value;
    }

     if (strcmp(stage->opcode, "EX-OR") == 0) {
    if (stage->rs1_value == stage->rs2_value){
      stage->buffer = 0;
    }else {
      stage->buffer = 1;
    } 
    }



    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute2", stage);
    }
  }
  else{
      CPU_Stage nop;
          memset(&nop, 0, sizeof(nop));
          memcpy(&nop.opcode, "NOP", 3);
          cpu->stage[MEM1] = nop;
   // strcpy(cpu->stage[MEM1].opcode, "NOP");
     if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute2", stage);
    }
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory1(APEX_CPU* cpu, const char* command)
{   
    
  CPU_Stage* stage = &cpu->stage[MEM1];
  if(cpu->stage[EX2].stalled==1)
  {
    stage->stalled=1;
  }

  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

      if (strcmp(stage->opcode, "BZ") == 0) {
          if(zeroFlag==1){
              cpu->pc=temp;
              strcpy(cpu->stage[DRF].opcode, "NOP");
                strcpy(cpu->stage[EX1].opcode, "NOP");
                  strcpy(cpu->stage[EX2].opcode, "NOP");
                    //strcpy(cpu->stage[DRF].opcode, "NOP");

          }
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory1", stage);
    }
  }
  else
  {        CPU_Stage nop;
          memset(&nop, 0, sizeof(nop));
          memcpy(&nop.opcode, "NOP", 3);
          cpu->stage[MEM2] = nop;
   // strcpy(cpu->stage[MEM2].opcode, "NOP");
     if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory1", stage);
    }

  }
  return 0;
}


int
memory2(APEX_CPU* cpu,const char* command)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if(cpu->stage[MEM1].stalled==1)
  {
    stage->stalled=1;
  }

  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
       cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }
     if (strcmp(stage->opcode, "LOAD") == 0) {
     
    stage->buffer = cpu->data_memory[stage->mem_address];
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory2", stage);
    }
  }
   else
  {       CPU_Stage nop;
          memset(&nop, 0, sizeof(nop));
          memcpy(&nop.opcode, "NOP", 3);
          cpu->stage[WB] = nop;
    //strcpy(cpu->stage[WB].opcode, "NOP");
     if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory2", stage);
    }

  }
  return 0;
}
/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu, const char* command)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd] = 1;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
    cpu->regs[stage->rd] = stage->buffer;
    cpu->regs_valid[stage->rd] = 1;
    }

    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }
  
  /* EX-OR */
  if (strcmp(stage->opcode, "EX-OR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }

    if (strcmp(stage->opcode, "ADD") == 0) {
    
       cpu->regs[stage->rd] = stage->buffer;
      
      cpu->regs_valid[stage->rd] = 1;
    if(cpu->regs[stage->rd] == 0){
      zeroFlag=1;
      }
    else if(cpu->regs[stage->rd] != 0){
    zeroFlag=0;
    }
    // printf("\nregister valid in wb after updating it to 1 %d\n", cpu->regs_valid[stage->rd]);
    
    }


     if (strcmp(stage->opcode, "ADDL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    cpu->regs_valid[stage->rd] = 1;
    if(cpu->regs[stage->rd] == 0){
    zeroFlag=1;
     }
     else if(cpu->regs[stage->rd] != 0){
    zeroFlag=0;
     }
    }


    if (strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
   cpu->regs_valid[stage->rd] = 1;
    }

     if (strcmp(stage->opcode, "MUL") == 0) {
    cpu->regs[stage->rd] = stage->buffer;
    cpu->regs_valid[stage->rd] = 1;

    if(cpu->regs[stage->rd] == 0){
    zeroFlag=1;
    }else if(cpu->regs[stage->rd] != 0){
    zeroFlag=0;
     }
  }

   if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }


    if(cpu->stage[WB].pc !=0){
        cpu->ins_completed++;
    }
  

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

/*
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1) {
*/
    /* All the instructions committed, so exit */
   
  /*
    if (cpu->ins_completed == cpu->code_memory_size) {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
  }

  return 0;
}
*/

int display_mem(APEX_CPU* cpu){
  
    printf("\t**************  MEMORY  ************\n");
  for(int i=0; i< 100 ; i++){
    printf("\t |MEM[%d]| \t |Value=%d| \n",i,cpu->data_memory[i]);
  }

  return 0;
}

int display_reg(APEX_CPU* cpu){
  printf("\t**************  REGISTERS  ************\n");
  for(int i=0; i < 16 ; i++){
    if(cpu->regs_valid[i]==1 ){
      printf("\t |REG[%d]| \t |Value=%d| \t |Status='VALID'|\n",i,cpu->regs[i]);
    }else if(cpu->regs_valid[i]==0){
      printf("\t |REG[%d]| \t |Value=%d| \t |Status='INVALID'|\n",i,cpu->regs[i]);
    }
  }
  
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu, const char* command, const char* cycle)
{
  cpu->clock=1;
    //printf("clock bef: %d\n", cpu->clock);
  int numberOfCycles = atoi(cycle); 
  while (cpu->clock <=  numberOfCycles) {

    /* All the instructions committed, so exit */
    //printf("cpu mem size %d", cpu->code_memory_size);
   // printf("\n ins completed: %d",cpu->ins_completed);
    /*if (cpu->ins_completed == cpu->code_memory_size ) {
      printf("(apex) >> Cycles Complete\n");
      break;
    }*/

    if (ENABLE_DEBUG_MESSAGES && strcmp(command, "display")==0 ) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu, command);
    memory2(cpu, command);
    memory1(cpu, command);
    execute2(cpu,command);
    execute1(cpu, command);
    
  /*
  // Before Decode Logic to Un Stall the things if Data is Dependency has gone
  if(cpu->regs_valid[cpu->stage[DRF].rs1] == 1 && cpu->regs_valid[cpu->stage[DRF].rs2] == 1){
    cpu->stage[DRF].stalled = 0;
    cpu->stage[F].stalled = 0;
  }
  */

    //fetch(cpu, command);
    
    decode(cpu, command);
  
  //  After Decode Logic to Stall the things if Data Dependenent has gone
   /*
   if( cpu->regs_valid[cpu->stage[DRF].rs1] == 0 || cpu->regs_valid[cpu->stage[DRF].rs2] == 0){
    CPU_Stage nop;
    memset(&nop, 0, sizeof(nop));
    memcpy(&nop.opcode, "NOP", 3);
    cpu->stage[EX1] = nop;
  }
  */
  
    fetch(cpu, command);
    cpu->clock++;
    printf("Clock : %d \n", cpu->clock);
  }

  display_reg(cpu);
 // display_mem(cpu);
  return 0;
}