/*
     SYSTEM SOFTWARE MINI PROJECT
     Topic: TWO PASS ASSEMBLER FOR ARM PROCESSORS
     BY
     BIJIL ABRAHAM PHILIP (09GAEC3011)

*/

#include <stdio.h>
#include <string>
#include <conio.h>
#include <fstream>
#include <stdlib.h>
#include <iostream.h>
#include<math.h>
#define TRUE 1
#define FALSE 0
#define FORMAT1 1
#define FORMAT2 2
#define FORMAT3 3
#define FORMAT4 4
#define REGS 2
#define DUPLICATE_LABEL	 1
#define INVALID_LABEL	 2
#define INVALID_FILE	 3

char *fname=NULL,*lname=NULL,*oname=NULL,*dname=NULL,*rname=NULL;

struct optab
{
	char name[10];
   unsigned int opcode;
};

struct optab oper[]={{"and",0x00},{"eor",0x02},{"cmn",0x04},{"rsb",0x06},{"add",0x08},
{"adc",0x0A},{"sbc",0x0C},{"rsc",0x0E},{"tst",0x10},{"teq",0x12},{"cmp",0x14},{"cmn",0x16},
{"orr",0x18},{"mov",0x1A},{"bic",0x1C},{"mvn",0x1E}};        //general instructions

struct optab oper1[]={{"ands",0x01},{"eors",0x03},{"cmns",0x05},{"rsbs",0x07},{"adds",0x09},
{"adcs",0x0B},{"sbcs",0x0D},{"rscs",0x0F},{"tsts",0x11},{"teqs",0x13},{"cmps",0x15},{"cmns",0x17},
{"orrs",0x19},{"movs",0x1B},{"bics",0x1D},{"mvns",0x1F}};     //general instructions with s flags

struct optab oper2[]={{"mul",0x1},{"muls",0x2},{"mla",0x3},{"mla",0x4}};       //multiplication instruction

struct optab oper3[]={{"beq",0x0A},{"bleq",0x0B},{"bne",0x1A},{"blne",0x1B},{"bns",0x2A},
{"blns",0x2B},{"bcc",0x3A},{"blcc",0x3B},{"bmi",0x4A},{"blmi",0x4B},{"bpl",0x5A},{"blpl",0x5B},
{"bge",0xAA},{"blge",0xAB},{"blt",0xBA},{"bllt",0xBB},{"bgt",0xCA},{"blgt",0xCB},{"ble",0xDA},{"blle",0xDB}};      //branch instruction

struct optab oper4[]={{"ldr",0x58},{"ldrb",0x5C},{"str",0x59},{"strb",0x5D}};  //load and store instruction

struct optab shrot[]={{"lsl",0x0},{"lsr",0x02},{"asr",0x04},{"ror",0x06}};   //shift and rotate

struct optab regs[]={{"r0",0x0},{"r1",0x1},{"r2",0x2},{"r3",0x3},{"r4",0x4},
{"r5",0x5},{"r6",0x6},{"r7",0x7},{"r8",0x8},{"r10",0xA},{"r11",0xB},{"r12",0xC},
{"r13",0xD},{"r14",0xE},{"r15",0xF}};      //registers


struct optab cond[]={{"eq",0x0},{"ne",0x1},{"cs",0x2},{"cc",0x3},{"mi",0x4},{"pl",0x5},
{"vs",0x6},{"vc",0x7},{"hi",0x8},{"ls",0x9},{"ge",0xA},{"lt",0xB},{"gt",0xC},{"le",0xD},{"al",0xE}};

struct symtab
{
   char name[20];
   unsigned long int addr;
   char value[80];
   unsigned int size;
   int type;           //1: normal label 2:constant
};


void decToHex(char *value)
{
	char hextab[][2]={"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};
   char hex[8]="",temp[8];
	int dec=atoi(value),i;
   while(dec!=0)
   {
   	i=dec%16;
      strcat(hex,hextab[i]);
      dec=dec/16;
   }
   strrev(hex);
   strcpy(value,hex);
}

symtab labels[50];

int symcount=0;

struct record          //record that is written to lst in 1st pass and rec file in 2nd pass
{
	char address[5];
   char instr[80];
   char obcode[9];
};

class passone
{
	int locctr; //location counter
   int sf,ef; //start flag and end flag
   public:
   		 passone(int strtaddr)
          {
          	locctr=strtaddr;
            sf=FALSE;
            ef=FALSE;
          }
          void run();
          int search_optab(char * str);
          int search_symtab(char * str);
          void error(short int code, char *temp);
          void append_symtab(char*label);
};

int passone::search_optab(char * str)    //returns the type of format of a particular instructio
{
	int i;
   for(i=0;i<16;i++)
   {
   	if(strcmpi(oper[i].name,str)==0)
			return FORMAT1;
   }
   for(i=0;i<16;i++)
   {
   	if(strcmpi(oper1[i].name,str)==0)
			return FORMAT1;
   }
   for(i=0;i<4;i++)
   {
   	if(strcmpi(oper2[i].name,str)==0)
			return FORMAT2;
   }
   for(i=0;i<20;i++)
   {
   	if(strcmpi(oper3[i].name,str)==0)
			return FORMAT3;
   }
   for(i=0;i<4;i++)
   {
   	if(strcmpi(oper4[i].name,str)==0)
			return FORMAT4;
   }
   for(i=0;i<16;i++)
   {
   	if(strcmpi(regs[i].name,str)==0)
			return REGS;
   }
   return FALSE ;
}

int passone::search_symtab(char * str)
{
	int i;
   for(i=0;i<symcount;i++)
    {
   	if(strcmpi(labels[i].name,str)==0)
			return TRUE;
   }

   return FALSE;
}

void passone::append_symtab(char*label)
{
	strcpy(labels[symcount].name,label);
   strcpy(labels[symcount].value,"");
   labels[symcount].addr=locctr;
   labels[symcount].size=0;
   labels[symcount].type=1;
   symcount++;
}

void passone::run()
{
  ifstream fin;
  ofstream fout;
  char line[80],temp[80];
  char delim[]={' ',',','.'};
  char * pch=NULL;
  char * tch=NULL;
  int tkno; //token number, to verify if it is a refrence or a label
  int df; //checks if given element is a constant or a label
  int lf; // label flag, to identify a label and classify it
  int cf; //comment flag
  int prevctr;
  fin.open(fname);

  if(!fin)
  {
  		error(INVALID_FILE,fname);
  }

  fout.open(lname);
  while(!fin.eof())
  {
   while((sf!=TRUE) && (!fin.eof()))
   {
  		fin.getline(line,80);
  		pch = strtok (line,delim);
  		while (pch != NULL)
      {
         if(strcmpi(pch,"entry")==0)
         {
         	sf=TRUE;
            record r1;
				strcpy(r1.address," ");
   			strcpy(r1.instr,"entry");
            strcpy(r1.obcode," ");
            fout.write((char*)&r1,sizeof(record));
            break;
         }
    		pch = strtok (NULL, delim);
  		}
   }
   fin.getline(line,80);
   strcpy(temp,line);
   pch = strtok (line,delim);
   tkno=1;
   lf=0; df=0; cf=0;
   prevctr=locctr;
   while (pch != NULL)
   {
         if(strcmpi(pch,"end")==0)
         {
         	ef=TRUE;
            break;
         }

         string str(pch);
         size_t found;
      	if(str.find(";")!=string::npos)
         {
         	cf=1;
            break;
         }

         if(search_optab(pch)==FALSE)
         {
             if(search_symtab(pch)==TRUE && tkno==1)
             {
             	error(DUPLICATE_LABEL,temp);
             }
             else if(tkno==1)
             {  lf=1;
             	 append_symtab(pch);
             }

             if(tkno==2)
             {
             	if(strcmpi(pch,"dcb")==0||strcmpi(pch,"dcbu")==0)
               {
               	labels[symcount-1].size=0x1;
                  df=1;
               }
               else if(strcmpi(pch,"dcw")==0||strcmpi(pch,"dcwu")==0)
               {
               	labels[symcount-1].size=0x2;
                  df=1;
               }
               else if(strcmpi(pch,"dcd")==0||strcmpi(pch,"dcdu")==0)
               {
               	labels[symcount-1].size=0x4;
                  df=1;
               }
               else
               {
               	locctr+=0x4;
                  break;
               }
             }
             if(tkno>2 && df==1)
             {
               tch=pch;
               decToHex(tch);
               strcat(labels[symcount-1].value,tch);
               locctr+=(labels[symcount-1].size);
             }
         }
         else
         {
         	prevctr=locctr;
          	locctr+=0x4;
            break;
         }
         tkno++;
    		pch = strtok (NULL, delim);
   }
   if(cf==0)
   {
   	record r1;
		itoa(prevctr,r1.address,10);
   	strcpy(r1.instr,temp);
   	if(df==1)
      	strcpy(r1.obcode,labels[symcount-1].value);
   	else
   		strcpy(r1.obcode,"0");
   	fout.write((char*)&r1,sizeof(record));
   }
   if(ef==TRUE)
   {
      fin.close();
      fout.close();
   	break;
   }
  }
}

void passone::error(short int code, char *str)
{
	switch(code)
   {
   	case DUPLICATE_LABEL: cout<<"Error!!!\n"<<"Duplicate Label in:"<<str<<endl;
      							 getch();
      						    exit(0);
      case INVALID_FILE: cout<<"Error!!!\n"<<"Invalid file:"<<str<<endl;
      							 getch();
      						    exit(0);
   }
}

class passtwo
{
   long int obcode;
   char hexcode[9];
   int baseaddr;
	public:
   		 void run();
          int search_condition(char *pch); //search condition...in arm this, if present is the second argument
          int getopcode(char * str);   //get the opcode
          int getshiftrot(char * str);    //get the shift or rotation code
          void genldstobcd(char * pch, int tkno);     //function to generate object code for (load and store) instruction
          void genbranch(char * pch, int tkno);   //function to generate object code code for branch statements
          void multiply(char * pch, int tkno);
          long int search_symtab(char * str);
          void error(short int code, char * str);
} ;


void passtwo::error(short int code, char * str)
{
	switch(code)
   {
   	case INVALID_LABEL : cout<<"Error!!!"<<endl;
      			cout<<"Undefined label: "<<str<<endl;
               getch();
      			exit(0);
   }
}
long int passtwo::search_symtab(char * str)
{
	int i;
   for(i=0;i<=symcount;i++)
    {
   	if(strcmpi(labels[i].name,str)==0)
			return labels[i].addr;
   }

   return -1;
}
void passtwo::multiply(char * pch, int tkno)
{
	passone p4(0);
	long int code=getopcode(pch);
   int flag=p4.search_optab(pch);
	if(tkno==2 && flag==REGS)
   {
       obcode=obcode*0x100000+0x90;
       code=code*0x10000;
       obcode+=code;
   }
   if(tkno==3 && flag==REGS)
       obcode+=code;
   if(tkno==4 && flag==REGS)
       obcode+=(code*0x100);
   if(tkno==5 && flag==REGS)
       obcode+=(code*0x1000);

}
void passtwo::genldstobcd(char * pch, int tkno)         //load and store instructions
{
	passone p4(0);
	long int code=getopcode(pch);
   int flag=p4.search_optab(pch);
	if(tkno==2 && flag==REGS)
   {
       obcode=obcode*0x100000;
       code=code*0x1000;
       obcode+=code;
   }
   if(tkno==3 && flag==REGS)
   {
       code=code*0x10000;
       obcode+=code;
   }
   else if(tkno==3 && flag!=REGS)
   {
   	if(pch[0]=='#')
      {
       char *str1;
       str1=pch+1;
       code=atoi(str1);
       obcode=obcode+code+0x2000000;
      }
      else if(search_symtab(pch)!=0)
      {
      	code=search_symtab(pch);
      	obcode+=(code-baseaddr);
      }
   }
   if(tkno==4 && flag==REGS)
   {
       obcode+=code;
   }
   if(tkno==5 && flag!=REGS)
   {
   	if(pch[0]=='#')
      {
       char *str1;
       str1=pch+1;
       code=atoi(str1)* 0x10;
       obcode=obcode+code+0x2000000;
      }
   }
}

void passtwo::genbranch(char *pch,int tkno)
{
   if(tkno==2)
   {
   	obcode=obcode*0x1000000;
   	long int code=search_symtab(pch);
      if(code==-1)
      {
         error(INVALID_LABEL,pch);
      }
      else
      obcode+=(code-baseaddr);
   }
}

int passtwo::getshiftrot(char * str)
{
   int i;
   for(i=0;i<15;i++)
    {
   	if(strcmpi(shrot[i].name,str)==0)
			return shrot[i].opcode;
   }
   return -1;
}

int passtwo::search_condition(char * str)
{
	int i;
   for(i=0;i<15;i++)
    {
   	if(strcmpi(cond[i].name,str)==0)
			return TRUE;
   }

   return -1;
}

int passtwo::getopcode(char * str)
{
	int i;
   for(i=0;i<16;i++)
   {
   	if(strcmpi(oper[i].name,str)==0)
			return oper[i].opcode;
   }
   for(i=0;i<16;i++)
   {
   	if(strcmpi(oper1[i].name,str)==0)
			return oper1[i].opcode;
   }
   for(i=0;i<4;i++)
   {
   	if(strcmpi(oper2[i].name,str)==0)
			return oper2[i].opcode;
   }
   for(i=0;i<20;i++)
   {
   	if(strcmpi(oper3[i].name,str)==0)
			return oper3[i].opcode;
   }
   for(i=0;i<4;i++)
   {
   	if(strcmpi(oper4[i].name,str)==0)
			return oper4[i].opcode;
   }
   for(i=0;i<16;i++)
   {
   	if(strcmpi(regs[i].name,str)==0)
			return regs[i].opcode;
   }
   return -1;
}

void passtwo::run()
{
  ifstream fin;
  ofstream fout;
  char line[80],temp[80];
  char delim[]={' ',',','.'};
  char * pch=NULL;
  char * tch=NULL,*tmp1,*tmp2;
  record r1;
  int tkno; //token number, to verify if it is a refrence or a label
  int lf,ef,sf,df; // label flag, to identify a label and classify it
  int tf,ocode; //instruction format type flag
  fin.open(lname,ios::in|ios::binary);
  fout.open(rname);
  passone p3(0);
  while(!fin.eof())
  {
   while((sf!=TRUE) && (!fin.eof()))
   {
      fin.read((char*)&r1,sizeof(record));
  		strcpy(line, r1.instr);
  		pch = strtok (line,delim);
  		while (pch != NULL)
      {
         if(strcmpi(pch,"entry")==0)
         {
         	sf=TRUE;
            break;
         }
    		pch = strtok (NULL, delim);
  		}
   }
   fin.read((char*)&r1,sizeof(record));
   strcpy(line, r1.instr);
   pch = strtok (line,delim);
   tkno=1;
   lf=FALSE;
   tf=-1;
   df=FALSE;
   while (pch != NULL)
   {
         if(strcmpi(pch,"end")==0)
         {
         	ef=TRUE;
            break;
         }
         if(p3.search_symtab(pch)==TRUE && tkno==1)  //if 1st token is a label
         {
         	tkno=1;
            lf=TRUE;
            pch = strtok (NULL, delim);
            continue;
         }
         if(p3.search_optab(pch)!=FALSE && tkno==1)  // 1st token is a mnemonic
         {
           tf=p3.search_optab(pch);
           ocode=getopcode(pch);
           obcode=ocode;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }
         if(search_condition(pch)!=-1 && tkno==2)  // if 2nd token is a condition
         {
               ocode=getopcode(pch);
               obcode=(ocode*0x10)+obcode;
               continue;
         }
         if(tkno==1)    //search for assembler directives
         {
        		 	if(strcmpi(pch,"dcb")==0||strcmpi(pch,"dcbu")==0)
                {  df=0x1; obcode=0;
                   tkno++;
                }

               else if(strcmpi(pch,"dcw")==0||strcmpi(pch,"dcwu")==0)
                {  df=0x2; obcode=0;
                   tkno++;
                }

               else if(strcmpi(pch,"dcd")==0||strcmpi(pch,"dcdu")==0)
                 {  df=0x4; obcode=0;
                    tkno++;
                 }

               if(df!=0)
              	 {
                     pch = strtok (NULL, delim);
                     int tadr;
                     if(pch!=NULL)
                     {
                       record r2;
                       tadr=atoi(r1.address);
                       itoa(tadr,r2.address,16);
                       strcpy(r2.instr,r1.instr);
                       obcode=atoi(pch);
                       itoa(obcode,r2.obcode,16);
                       fout.write((char*)&r2,sizeof(record));
                       pch = strtok (NULL, delim);
                     }
                     while (pch != NULL)
                     {
   							record r2;
   							strcpy(r2.address," ");
   							strcpy(r2.instr," ");
                        obcode=atoi(pch);
                        itoa(obcode,r2.obcode,16);
   							fout.write((char*)&r2,sizeof(record));
                     	pch = strtok (NULL, delim);
                     }
                     break;
                }
         }
         if(tf==FORMAT2)      //multiplication statements
         {
           tch=strdup(pch);
           baseaddr=atoi(r1.address);
           multiply(tch,tkno);
           free(tch);
           tch=NULL;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }
         if(tf==FORMAT3)      //branching statements
         {
           tch=strdup(pch);
           baseaddr=atoi(r1.address);
           genbranch(tch,tkno);
           free(tch);
           tch=NULL;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }
         if(tf==FORMAT4)   //load and store instructions
         {
           tch=strdup(pch);
           baseaddr=atoi(r1.address);
           genldstobcd(tch,tkno);
           free(tch);
           tch=NULL;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }
         if(p3.search_optab(pch)==REGS && tkno==2)   //check if the token is a register
         {
           ocode=getopcode(pch);
           obcode=(obcode*0x10)+ocode;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }

         if(p3.search_optab(pch)==REGS && tkno==3)
         {
           ocode=getopcode(pch);
           obcode=(obcode*0x10)+ocode;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }

         if(getshiftrot(pch)!=-1 && tkno==4)    //checks if shift/rotate field is specified
         {
           ocode=getshiftrot(pch);
           obcode=(obcode * 0x1000)+ocode;
           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }
         if(tkno==5)
         {
            if(p3.search_optab(pch)==REGS)
         	{
            	ocode=getopcode(pch);
               obcode=(obcode*0x10)+ocode+0x10;
            }
           else if(pch[0]=='#')   //check if the token is a immediate operand
            {
               char *str1;
               str1=pch+1;
               ocode=atoi(str1)* 0x100;
               obcode=obcode+ocode;
            }

           tkno++;
           pch = strtok (NULL, delim);
           continue;
         }
         tkno++;
    		pch = strtok (NULL, delim);
   }
   if(tkno==4 && tf!=FORMAT4 )
   {
   	obcode=(obcode)*(0x1000);
   }
   if(df==FALSE)      //given statement is not a data element
   {
   	itoa(obcode,temp,16);
   	record r2;
      if(ef==TRUE)
      {
      strcpy(r2.address,"");
   	strcpy(r2.instr,r1.instr);
   	strcpy(r2.obcode,"");
      }
      else
      {
      int taddr=atoi(r1.address);
   	itoa(taddr,r2.address,16);
   	strcpy(r2.instr,r1.instr);
   	itoa(obcode,r2.obcode,16);
      }
   	fout.write((char*)&r2,sizeof(record));
   }
   if(ef==TRUE)     //end flag is set
   {
      record r2;
      strcpy(r2.address,"");
      strcpy(r2.instr,"");
      strcpy(r2.obcode,"");
      fout.write((char*)&r2,sizeof(record));
      fin.close();
      fout.close();
   	break;
   }
  }

}

void genObjectFile(char *fname,char *oname,char *rname)     //generate object file
{
   char *rec="",*tmp="";
   char delim='^';
   int i=0,j,lim;
   record r1;
   ifstream fin;
   ofstream fout;
   fin.open(rname,ios::in|ios::binary);
   fout.open(oname);
   fout<<"H^"<<fname<<"^1^28^000000^6"<<endl;                //header record
   fin.read((char*)&r1,sizeof(record));
   for(j=strlen(r1.address);j<6;j++)
   	strcat(tmp,"0");
   fout<<"T^"<<tmp<<r1.address;
   while(!fin.eof())                     //generate text record
   {
   	if(i>79)
      {
      	fout<<endl;
      	i=0;
         for(j=strlen(r1.address)+1;j<6;j++)
   			strcat(tmp,"0");
   		fout<<"T^"<<tmp<<r1.address;
         fin.read((char*)&r1,sizeof(record));
         continue;
      }
      tmp=strdup("");
      string str(r1.instr);
      size_t found;
      if(str.find(" dcb ")!=string::npos || str.find(" DCB ")!=string::npos)
      	lim=2;
      else if(str.find(" dcbu ")!=string::npos || str.find(" DCBU ")!=string::npos)
      	lim=3;
      else if(str.find(" dcw ")!=string::npos || str.find(" DCW ")!=string::npos)
      	lim=4;
      else if(str.find(" dcwu ")!=string::npos || str.find(" DCWB ")!=string::npos)
      	lim=5;
      else if(str.find(" dcd ")!=string::npos || str.find(" DCD ")!=string::npos)
      	lim=8;
      else if(str.find(" dcdu ")!=string::npos || str.find(" DCDU ")!=string::npos)
      	lim=9;
      else if(strcmpi(r1.instr," ")==0)
      	lim=lim;
      else if(strcmpi(r1.instr,"end")==0)
      	break;
      else
      	lim=8;
      for(j=strlen(r1.obcode);j<lim;j++)
   		strcat(tmp,"0");
      fout<<delim<<tmp<<r1.obcode;
      i+=lim;
      fin.read((char*)&r1,sizeof(record));

   }
   fout<<endl;
   fout<<"E"<<"^000000"<<endl;
   fout.close();
   fin.close();
}

void genDemoFile(char *rname, char *dname)
{
	int lim,j;
   char *tmp;
 	ifstream fin;
   fin.open(rname,ios::in|ios::binary);
   ofstream fout;
   fout.open(dname);
   fout.width (11);
   fout<<"Address";
   fout.width (65);
   fout<<"ARM Instruction";
   fout.width (25);
   fout<<"Object Code"<<endl;

   while(!fin.eof())
   {
   	record r1;
      fin.read((char*)&r1,sizeof(record));
      fout.width (11);
      fout<<r1.address;
      fout.width (65);
      fout<<r1.instr;

      tmp=strdup("");
      string str(r1.instr);
      if(str.find(" dcb ")!=string::npos || str.find(" DCB ")!=string::npos)
      	lim=2;
      else if(str.find(" dcbu ")!=string::npos || str.find(" DCBU ")!=string::npos)
      	lim=2;
      else if(str.find(" dcw ")!=string::npos || str.find(" DCW ")!=string::npos)
      	lim=4;
      else if(str.find(" dcwu ")!=string::npos || str.find(" DCWB ")!=string::npos)
      	lim=4;
      else if(str.find(" dcd ")!=string::npos || str.find(" DCD ")!=string::npos)
      	lim=8;
      else if(str.find(" dcdu ")!=string::npos || str.find(" DCDU ")!=string::npos)
      	lim=8;
      else if(strcmpi(r1.instr," ")==0)
      	lim=lim;
      else if(strcmpi(r1.instr,"end")==0)
      	break;
      else
      	lim=8;
      for(j=strlen(r1.obcode);j<lim;j++)
   		strcat(tmp,"0");
         strcat(tmp,r1.obcode);
      fout.width (25);
      fout<<tmp<<endl;
   }
   fin.close();
}

void main()
{
	cout<<"\t ***** TWO PASS ASSEMBLER FOR ARM PROCESSORS *****"<<endl;
   cout<<"\t\tBijil Abraham Philip (09GAEC3011)"<<endl;
   cout<<"\t\t  VI Sem CSE, UVCE, Bangalore"<<endl<<endl;
   cout<<"\t-------------------------------------------------"<<endl;
	passone p1(0x000);
   cout<<" Enter file name: ";
   fname=new char[12];
   cin>>fname;
   int len=strlen(fname);
   lname=strdup(fname);
   oname=strdup(fname);
   dname=strdup(fname);
   rname=strdup(fname);
   lname[len-3]='l'; lname[len-2]='s'; lname[len-1]='t';
   oname[len-3]='e'; oname[len-2]='l'; oname[len-1]='f';
   rname[len-3]='r'; rname[len-2]='e'; rname[len-1]='c';
   dname[len-3]='t'; dname[len-2]='x'; dname[len-1]='t';
   cout<<"File being assembled:\t"<<fname<<endl;

   p1.run();
   cout<<endl<<" Pass one complete........."<<endl;
   cout<<"  ->Intermediate  file created:\t"<<lname<<endl;
   //getch();
   passtwo p2;
   p2.run();
   cout<<endl<<" Pass two complete........."<<endl;
   genDemoFile(rname,dname);
   genObjectFile(fname,oname,rname);
   cout<<"  ->Object file created: "<<oname<<endl;
   cout<<"  ->Assignment of addresses and object code can be seen in: "<<dname<<endl;
   getch();
}
