
/* 

The project is developed as part of Computer Architecture class
Project Name: Functional Simulator for subset of ARM Processor

Developer's Name:
Developer's Email id:
Date: 

*/


/* myARMSim.cpp
   Purpose of this file: implementation file for myARMSim
*/

#include "myARMSim.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//Register file
static unsigned int R[16];
//flags
static int N,C,V,Z;
//memory
static unsigned char MEM[4000];

//intermediate datapath and control path signals
static unsigned int instruction_word;
static unsigned int operand1;
static unsigned int operand2;
unsigned int address, instruction;
unsigned int inst[32] , code=0, flag=0 ,write;
char opcode[20];


void run_armsim() {
  int h;
  while(1) {
    fetch();
    if(flag==1) break;
    decode();
    execute();
    mem();
    write_back();
    //for(h=0;h<16;h++)
    //printf("R[%d]%d ",h,R[h]);
    printf("\n");
    //printf("\nmem[100]%d mem[104]%d mem[108]%d mem[112]%d mem[116]\n",MEM[100],MEM[104],MEM[108],MEM[112],MEM[116]);
  }
}

// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc() {
  int j;
  for(j=0;j<16;j++)
    R[j]=0;
    flag=0;
    code=0;
}

//load_program_memory reads the input memory, and pupulates the instruction 
// memory
void load_program_memory(char *file_name) {
  FILE *fp;
  fp = fopen(file_name, "r");
  //printf("a ");
  if(fp == NULL) {
    printf("Error opening input mem file\n");
    exit(1);
  }
  while(fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    //printf("\n%x %x",address,instruction);
    write_word(MEM, address, instruction);
  }
  fclose(fp);
}

//writes the data memory in "data_out.mem" file
void write_data_memory() {
  FILE *fp;
  unsigned int i;
  fp = fopen("data_out.mem", "w");
  if(fp == NULL) {
    printf("Error opening dataout.mem file for writing\n");
    return;
  }
  
  for(i=0; i < R[15]; i = i+4){
    fprintf(fp, "%x %x\n", i, read_word(MEM, i));
  }
  fclose(fp);
}

//should be called when instruction is swi_exit
void swi_exit() {
  write_data_memory();
  exit(0);
}


//reads from the instruction memory and updates the instruction register
void fetch() {
  //printf("b ");
  instruction_word=read_word(MEM,R[15]);
  if(instruction_word == 0xEF000011) flag=1;
  //printf("%x ",instruction_word);
  printf("FETCH: Fetch instruction 0x%x from address 0x%x\n",instruction_word,R[15]);
  R[15]+=4;
}
//reads the instruction register, reads operand1, operand2 fromo register file, decides the operation to be performed in execute stage
void decode() {
  code=0;
  operand1=0;
  operand2=0;
  //printf("a ");
  int j;
  for(j=0 ; j<=31 ; j++)
    inst[j]=(instruction_word >> j) & 0x1 ;
  //for(j=0 ; j<=31 ; j++)
    //printf("%u ",inst[j]);
  //printf("end\n");

  if(inst[27]==0) {
    for(j=16 ; j<=19; j++) {
      operand1 += inst[j] * pow(2,j-16);
    }
    //printf("operand1%x ",operand1);
    if(inst[26]==0) {
      for(j=24 ; j>=21 ; j--)
        code += inst[j] * pow(2,j-21);
      //printf("code%x ",code);
      if (code == 0x0)
        strcpy(opcode,"AND");
      else if (code == 0x2)     
        strcpy(opcode,"SUB");
      else if (code == 0x4)
        strcpy(opcode,"ADD");
      else if(code == 0xA)
        strcpy(opcode,"CMP");
      else if (code == 0xC)
        strcpy(opcode,"ORR");
      else if (code == 0xD)
        strcpy(opcode,"MOV");
      if(inst[25]==0)
        for( j=0 ; j<=3 ; j++)
          operand2 += inst[j] * pow(2,j);
      else
        for( j=0 ; j<=7 ; j++)
          operand2 += inst[j] * pow(2,j);
        //printf("operand2%x ",operand2);
      int rd=0;
      for(j=12;j<=15;j++)
        rd += pow(2,j-12)*inst[j];
      if(strcmp(opcode,"MOV")==0) {
        if(inst[25]==0) {
          printf("DECODE: Operation is %s, second operand is R%d, destination register is R%d\n",opcode,operand2,rd);
          printf("DECODE: Read registers R%d=%d\n",operand2,R[operand2]);
        }
        else {
          printf("DECODE: Operation is %s, second operand is %d, destination register is R%d\n",opcode,operand2,rd);
          printf("DECODE: Read immediate %d\n",operand2); 
        }
      }
      else if(strcmp(opcode,"CMP")==0) {
        if(inst[25]==0) {
          printf("DECODE: Operation is %s, first operand is R%d, second operand is R%d\n",opcode,operand1,operand2);
          printf("DECODE: Read registers R%d=%d, R%d=%d\n",operand1,R[operand1],operand2,R[operand2]); 
        }
        else {
          printf("DECODE: Operation is %s, first operand is R%d, second operand is %d\n",opcode,operand1,operand2);
          printf("DECODE: Read registers R%d=%d, immediate %d\n",operand1,R[operand1],operand2); 
        }
      }
      else {
        if(inst[25]==0) {
          printf("DECODE: Operation is %s, first operand is R%d, second operand is R%d, destination register is R%d\n",opcode,operand1,operand2,rd);
          printf("DECODE: Read registers R%d=%d, R%d=%d\n",operand1,R[operand1],operand2,R[operand2]);
        }
        else {
          printf("DECODE: Operation is %s, first operand is R%d, second operand is R%d, destination register is R%d\n",opcode,operand1,operand2,rd);
          printf("DECODE: Read registers R%d=%d, immediate %d\n",operand1,R[operand1],operand2); 
        }
      }
    }

    else {
      //printf("enter ");
      for(j=20 ; j<=24 ; j++) {
        //printf("s%x %xend ",inst[j],code);
        code += inst[j] * pow(2,j-20);
      }
      //printf("code%x ",code);
      if(code == 0x18)
        strcpy(opcode,"STR");
      else if(code == 0x19)
        strcpy(opcode,"LDR");
      if( inst[25] == 0 )
        for( j=0 ; j<=11 ; j++ )
          operand2 += inst[j] * pow(2,j);
      else
        for( j=0 ; j<=3 ; j++ )
          operand2 += inst[j] * pow(2,j);
      //printf("operand1%d operand2%d R[operand1]%d R[operand2]%d ",operand1,operand2,R[operand1],R[operand2]);
      int rd=0;
      for(j=12;j<=15;j++)
        rd += pow(2,j-12)*inst[j];
      printf("DECODE: Operation is %s, first operand is R%d, second operand is R%d, destination register is R%d\n",opcode,operand1,operand2,rd);
      printf("DECODE: Read registers R%d=%d, R%d=%d\n",operand1,R[operand1],operand2,R[operand2]);
    }
    /*int rd=0;
    for(j=12;j<=15;j++)
      rd += pow(2,j-12)*inst[j];
    printf("DECODE: Operation is %s, first operand is R%d, second operand isR%d, destination register is R%d\n",opcode,operand1,operand2,rd);
    printf("DECODE: Read registers R%d=%d, R%d=%d\n",operand1,R[operand1],operand2,R[operand2]);*/
  }

  else{
    for(j=28 ; j<=31 ; j++)
      code += inst[j] * pow(2,j-28);
    //printf("code%x ",code);
    for(j=16 ; j<=19 ; j++) 
      operand1 += inst[j]*pow(2,j);
    //printf("operand1%x ",operand1);
    for(j=0 ; j<=3; j++)
      operand2 += inst[j]*pow(2,j);
    //printf("operand2%x ",operand2);
    if(code == 0x0)
      strcpy(opcode,"BEQ");
    else if(code == 0x1)
      strcpy(opcode,"BNE");
    else if(code == 0x2)
      strcpy(opcode,"BHS");
    else if(code == 0x3)
      strcpy(opcode,"BLO");
    else if(code == 0x8)
      strcpy(opcode,"BHI");
    else if(code == 0x9)
      strcpy(opcode,"BLS");
    else if(code == 0xA)
      strcpy(opcode,"BGE");
    else if(code == 0XB)
      strcpy(opcode,"BLT");
    else if(code == 0xC)
      strcpy(opcode,"BGT");
    else if(code == 0xD)
      strcpy(opcode,"BLE");
    else if(code == 0xE)
      strcpy(opcode,"BAL");
    else if(code == 0xF)
      strcpy(opcode,"BNV");
  int rd=0;
  for(j=12;j<=15;j++)
    rd += pow(2,j-12)*inst[j];
  printf("DECODE: Operation is %s\n",opcode);
  }
  //printf("\n%s\n",opcode);

}
//executes the ALU operation based on ALUop
void execute() {
  int j;
int shift=0;
if(strcmp(opcode,"AND")==0)
{
  if(inst[25]==0 && inst[6]==0)
  {

    for(j=11;j>=7;j--)
      shift += pow(2,j-7)*inst[j];
    if(inst[5]==0)
      R[operand2]=R[operand2] << shift;
    else
      R[operand2]=R[operand2] >> shift;
    write=R[operand1] && R[operand2];
  }
  else if(inst[25]==1)
    write=R[operand1] & operand2;
  //printf("write%d\n",write);
}

else if(strcmp(opcode,"ORR")==0)
{
  if(inst[25]==0 && inst[6]==0)
  {
    for(j=11;j>=7;j--)
      shift += pow(2,j-7)*inst[j];
    if(inst[5]==0)
      R[operand2]=R[operand2] << shift;
    else
      R[operand2]=R[operand2] >> shift;
    write=R[operand1] || R[operand2];
  }
  else if(inst[25]==1)
    write=R[operand1] | operand2;
  //printf("write%x\n",write);
}

else if(strcmp(opcode,"SUB")==0)
{
  if(inst[25]==0 && inst[6]==0)
  {
  for(j=11;j>=7;j--)
    shift += pow(2,j)*inst[j];
  if(inst[5]==0)
    R[operand2]=R[operand2] << shift;
  else
    R[operand2]=R[operand2] >> shift;
  write=R[operand1] - R[operand2];

  }
  else if(inst[25]==1)
    write=R[operand1] - operand2;
  //printf("write%x\n",write);
}


else if(strcmp(opcode,"ADD")==0)
{
    if(inst[25]==0 && inst[6]==0)
    {
    for(j=11;j>=7;j--)
      shift += pow(2,j-7)*inst[j];
    if(inst[5]==0)
      R[operand2]=R[operand2] << shift;
    else
      R[operand2]=R[operand2] >> shift;
    write=R[operand1] + R[operand2];
    }
    else if(inst[25]==1) {
      //printf("reach%x %x %x\n",operand1,R[operand1],operand2);
      write=R[operand1] + operand2;
    }
    //printf("write%x\n",write);
}

else if(strcmp(opcode,"CMP")==0)
  {   
    int temp;
    if(inst[25]==0 && inst[6]==0)
    {
      for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2] << shift;
      else
        R[operand2]=R[operand2] >> shift;
      temp=R[operand1] - R[operand2];
     }
    else if(inst[25]==1)
       temp=R[operand1] - operand2;
      if(temp>0)
      {
          N=0;
          V=0;
          Z=0;
          C=0;
      }
      else if(temp==0)
      {
          Z=1;
          N=0;
          V=0;
          C=0;

      }
      else if(temp<0)
      {
          Z=0;
          N=1;
          V=0;
          C=0;

      }
      //printf("N%d C%d V%d Z%d ",N,C,V,Z);
}


else if(strcmp(opcode,"MOV")==0)
{
    //printf("execute mov\n");
    if(inst[25]==0 && inst[6]==0)
    {
      for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      //printf("shift%x ",shift);
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
      write=R[operand2];
    }
    else if(inst[25]==1)
      write=operand2;
    //printf("write%d\n",write);


}


else if(strcmp(opcode,"LDR")==0 || strcmp(opcode,"STR")==0)
{
  if (inst[23]==0 && inst[24]==0 && inst[21]==0)
  { 
      for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
      write=R[operand1];
      //R[operand1]=R[operand1]-R[operand2];
  }

  else if(inst[23]==0 && inst[24]==1 && inst[21]==0)
  {
    for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else 
        R[operand2]=R[operand2]>>shift;
      write=R[operand1]-R[operand2];
  }

  else if(inst[23]==0 && inst[24]==0 && inst[21]==1)
  {
    for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
      write=R[operand1];
      //R[operand1]=R[operand1]-R[operand2];
  }

  else if(inst[23]==0 && inst[24]==1 && inst[21]==1)
  {
     for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
      //R[operand1]=R[operand1]-R[operand2];
      //write=R[operand1];
      write=R[operand1]-R[operand2];
  }

  else if (inst[23]==1 && inst[24]==0 && inst[21]==0)
  {
     for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
      write=R[operand1];
      //R[operand1]=R[operand1]+R[operand2];
  }
  
  else if(inst[23]==1 && inst[24]==1 && inst[21]==0)
  {
      //printf("alind\n");
     for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      //printf("\nshift %d\n",shift);
      if(inst[5]==0)
        write=R[operand1]+(R[operand2]<<shift);
        //R[operand2]=R[operand2]<<shift;
      else
        write=R[operand1]+(R[operand2]>>shift);
        //R[operand2]=R[operand2]>>shift;sss
      //write=R[operand1]+R[operand2];
      //printf("\nwrite %d\n",write);
  }

  else if(inst[23]==1 && inst[24]==0 && inst[21]==1)
  {
      for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
        write=R[operand1];
        //R[operand1]=R[operand1]+R[operand2];
  }
  else if(inst[23]==1 && inst[24]==1 && inst[21]==1)
  {
      for(j=11;j>=7;j--)
        shift += pow(2,j-7)*inst[j];
      if(inst[5]==0)
        R[operand2]=R[operand2]<<shift;
      else
        R[operand2]=R[operand2]>>shift;
      //R[operand1]=R[operand1]+R[operand2];
      //write=R[operand1];
      write=R[operand1]+R[operand2];
  }
  //printf("writeldr%d\n",write);
}

else if(strcmp(opcode,"BEQ")==0)
{
  if(Z==1 && N==0 && V==0 && C==0)
  {
    for(j=0;j<=23;j++)
      shift += pow(2,j)*inst[j];
    shift*=4;
    //sign extension
    for(j=24;j<=31;j++)
      shift += shift+pow(2,j)*inst[23];
    R[15]=R[15]-4;
    R[15]=R[15]+(shift)+8;
  }

}

else if(strcmp(opcode,"BNE")==0)
{
  if(Z!=1)
  {
    for(j=0;j<=22;j++)
      shift += pow(2,j)*inst[j];
    //shift-=pow(2,23)*inst[23];
    //sign extension
    for(j=23;j<=31;j++) {
      if(inst[23]==1 && j==31)
        shift=shift-pow(2,j);
      else 
        shift=shift+pow(2,j)*inst[23];
    }
    shift*=4;
    //printf("shift%d ",shift);
    R[15]=R[15]-4;
    R[15]=R[15]+(shift)+8;
  }
  //printf("R[15]bne %d\n",R[15]);
}


else if(strcmp(opcode,"BGT")==0)
{
  if(Z==0 && N==0 && V==0 && C==0)
  {
    for(j=0;j<=23;j++)
      shift=shift+pow(2,j)*inst[j];
    shift*=4;
    //sign extension
    for(j=24;j<=31;j++)
      shift=shift+pow(2,j)*inst[23];
    R[15]=R[15]-4;
    R[15]=R[15]+(shift)+8;
  }

}


else if(strcmp(opcode,"BGE")==0)
{
  if((Z==0 || Z==1) && N==0 && V==0 && C==0)
  {
    for(j=0;j<=23;j++)
      shift=shift+pow(2,j)*inst[j];
    shift*=4;
    //sign extension
    for(j=24;j<=31;j++)
      shift=shift+pow(2,j)*inst[23];
    R[15]=R[15]-4;
    R[15]=R[15]+(shift)+8;
  }
}

else if(strcmp(opcode,"BLT")==0)
{
  if(Z==0 && N==1 && V==0 && C==0)
  {
    for(j=0;j<=23;j++)
      shift=shift+pow(2,j)*inst[j];
    shift*=4;
    //sign extension
    for(j=24;j<=31;j++)
      shift=shift+pow(2,j)*inst[23];
    R[15]=R[15]-4;
    R[15]=R[15]+(shift)+8;
  }
}

else if(strcmp(opcode,"BLE")==0)
{
  if((Z==0 || Z==1) && (N==1 || N==0) && V==0 && C==0)
  {
    for(j=0;j<=23;j++)
      shift=shift+pow(2,j)*inst[j];
    //sign extension
    for(j=24;j<=31;j++)
      shift=shift+pow(2,j)*inst[23];
    R[15]=R[15]-4;
    R[15]=R[15]+(shift)+8;
  }
}

if(inst[27]==0) {
  if(inst[26]==0 && strcmp(opcode,"MOV")!=0 && strcmp(opcode,"CMP")!=0) {
    if(inst[25]==0)
      printf("EXECUTE: %s %d and %d\n",opcode,R[operand1],R[operand2]);
    else
      printf("EXECUTE: %s %d and %d\n",opcode,R[operand1],operand2);
    }
  else if (inst[26]==1)
    printf("EXECUTE: %s\n",opcode);
  else if(strcmp(opcode,"MOV")==0) {
    int rd=0;
    for(j=12;j<=15;j++)
    rd += pow(2,j-12)*inst[j];
    if(inst[25]==0)
      printf("EXECUTE: %s R%d and %d\n",opcode,rd,R[operand2]);
    else 
      printf("EXECUTE: %s R%d and %d\n",opcode,rd,operand2);
    }
  else if(strcmp(opcode,"CMP")==0) {
    int rd=0;
    for(j=12;j<=15;j++)
    rd += pow(2,j-12)*inst[j];
    if(inst[25]==0)
      printf("EXECUTE: %s R%d and %d\n",opcode,operand1,R[operand2]);
    else 
      printf("EXECUTE: %s R%d and %d\n",opcode,operand1,operand2);
  }
  }
else 
  printf("EXECUTE: %s\n",opcode);
}
//perform the memory operation
void mem() {
  int rd=0,j,data;
  if(strcmp(opcode,"BEQ")!=0 || strcmp(opcode,"BNE")!=0 || strcmp(opcode,"BGE")!=0 || strcmp(opcode,"BGT")!=0 || strcmp(opcode,"BLE")!=0 || strcmp(opcode,"BLT")!=0)
  {
  if(strcmp(opcode,"LDR")==0)
  {
    //printf("\n1write%d ",write);
    data=read_word(MEM,write);
    write=data;
    //data=(int *)(MEM + write);
    //printf("\n2data%d ",data);
    //write=*data;
    //printf("\n3write%d ",write);
  }    
  if(strcmp(opcode,"STR")==0)
  {
    //printf("\nwrite %d ",write);
    //int *data_p,j=0,rd=0;
    //data_p = (int*) (MEM + write);
    for(j=12;j<=15;j++)
      rd += pow(2,j-12)*inst[j];
    //printf("rd%d R[rd]%d ",rd,R[rd]);
    write_word(MEM,write,R[rd]);
  //data_p = R[rd];
  }
  }  
  if(strcmp(opcode,"STR")==0)
    printf("MEMORY: Storing %d in memory\n",R[rd]);
  else if(strcmp(opcode,"LDR")==0)
    printf("MEMORY: Loading %d from memory\n",data);
  else
    printf("MEMORY: No memory operation\n");
  //printf("memR[15]%d R[0]%d write %d ",R[15],R[0],write); 
}
//writes the results back to register file
void write_back() {
  int rd=0,j;
  //printf("s1R[15] %d ",R[15]);
  if( strcmp(opcode,"CMP")!=0 && strcmp(opcode,"STR")!=0 && strcmp(opcode,"BEQ")!=0 && strcmp(opcode,"BNE")!=0 && strcmp(opcode,"BGE")!=0 && strcmp(opcode,"BGT")!=0 && strcmp(opcode,"BLE")!=0 && strcmp(opcode,"BLT")!=0)
  {
    //printf("s2R[15] %d write %d ",R[15],write);
    for(j=12;j<=15;j++)
      rd += pow(2,j-12)*inst[j];
    R[rd]=write;
    printf("WRITEBACK: write %d to R%d\n",write,rd);
    //printf("s3R[15] %d rd %d R[rd] %d ",R[15],rd,R[rd]);
  }
  else if(strcmp(opcode,"BEQ")==0 || strcmp(opcode,"BNE")==0 || strcmp(opcode,"BGE")==0 || strcmp(opcode,"BGT")==0 || strcmp(opcode,"BLE")==0 || strcmp(opcode,"BLT")==0)
  {
    N=0;
    C=0;
    V=0;
    Z=0;
    printf("WRITEBACK: No writeback\n");
  }
  else
    printf("WRITEBACK: No writeback\n"); 
  
  //printf("wbR[15]%d R[0]%d ",R[15],R[0]);
}


int read_word(char *mem, unsigned int address) {
  int *data;
  data =  (int*) (mem + address);
  return *data;
}

void write_word(char *mem, unsigned int address, unsigned int data) {
  int *data_p;
  data_p = (int*) (mem + address);
  *data_p = data;
}
