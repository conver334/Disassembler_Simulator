#include<iostream>
#include<cstdio>
#include<zlib.h>
#include<cstring>
#include<string>
#include<cstdlib>
#include<fstream>
using namespace std;
const int INS_SIZE=10000;
const int INPUT = 10000;
int gps[32]; //保存寄存器值
int pc=64;
int data_num = 0,data_begin; //data数量,data开始的位置
struct INSTRCUTION{ //指令
    int instype;
    int rs,rt,rd,imm,offset,sa;
    string raw[6],whole;
}ins[INS_SIZE];

int isword = 0; // isword = true 表示在读数据部分

int BinaryToDecimal(const string& para){ // 无符号 二进制转十进制
    int slen = para.length();
    int num = 0;   
    for(int i=0;i<slen;i++)num = (num<<1)+para[i]-'0';
    return num;
}

int SignedBinaryToDecimal(string para){ // 有符号 二进制转十进制
    int slen = para.length();
    int num = 0,flag=1;   
    if(para[0]=='1'){
        flag=-1;
        int jin = 1,now;
        for(int i=slen-1;i>0;i--){
            now = '1'-para[i];
            if(jin==1 && now==1)para[i]='0';
            else if(jin==1)para[i]='1',jin=0;
            else para[i]='0'+now;
        }
    }
    for(int i=1;i<slen;i++)num = (num<<1)+para[i]-'0';
    return num*flag;
}

void addOvf(int* result, int a, int b) {//有符号加法溢出
    *result = a + b;     
    if(a > 0 && b > 0 && *result < 0)throw "Integer Overflow";     
    if(a < 0 && b < 0 && *result > 0)throw "Integer Overflow";     
}
void fun_BEQ(){//2
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16))<<2;
    printf("BEQ R%d, R%d, #%d\n",ins[ins_num].rs,ins[ins_num].rt,ins[ins_num].offset);
}

void fun_BLTZ(){//0
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16))<<2;
    printf("BLTZ R%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
}
void fun_BGTZ(){//3
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16))<<2;
    printf("BGTZ R%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
}

void fun_LW(){//5
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16));
    printf("LW R%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
}
void fun_SW(){//6
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16));
    printf("SW R%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
}

void fun_SLL(){//22
    ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
    printf("SLL R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
}
void fun_SRL(){//15
    ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
    printf("SRL R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
}
void fun_SRA(){//16
    ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
    printf("SRA R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
}

void fun_J(){//0
    ins[ins_num].imm = BinaryToDecimal(ins[ins_num].whole.substr(6,26))<<2;
    printf("J #%d\n",ins[ins_num].imm);
}
void fun_JR(){//13
    printf("JR R%d\n",ins[ins_num].rs);
}
void fun_BREAK(){//14
    printf("BREAK\n");
}

void fun_MUL(){//4 9
    if(ins[ins_num].raw[0][0]=='0'){
        printf("MUL R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("MUL R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_ADD(){//7
    try{
        int tmp;
        if(ins[ins_num].raw[0][0]=='0'){
            // addOvf(&tmp, gpr[rs], gpr[rt]);
            printf("ADD R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        }
        else{
            // addOvf(&tmp, gpr[rs], gpr[rt]);
            printf("ADD R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        }
    }catch(const char* msg){
        cerr << msg << endl;
    }
}
void fun_SUB(){//8
    if(ins[ins_num].raw[0][0]=='0'){
        printf("SUB R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("SUB R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_AND(){//10
    // printf("AND");
    if(ins[ins_num].raw[0][0]=='0'){
        printf("AND R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("AND R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_NOR(){
    // printf("NOR");
    if(ins[ins_num].raw[0][0]=='0'){
        printf("NOR R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("NOR R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_SLT(){
    // printf("SLT");
    if(ins[ins_num].raw[0][0]=='0'){
        printf("SLT R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("SLT R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_NOP(){//23
    printf("NOP\n");
}
void (*func[])() = {
    fun_BLTZ,fun_J,fun_BEQ,fun_BGTZ,fun_MUL,fun_LW,fun_SW, \
    fun_ADD,fun_SUB,fun_MUL,fun_AND,fun_NOR,fun_SLT,\
    fun_JR,fun_BREAK,fun_SRL,fun_SRA,\
    fun_ADD,fun_SUB,fun_AND,fun_NOR,fun_SLT,\
    fun_SLL,fun_NOP
};
string SPECIAL[13]={
    "000001", //BLTZ
    "000010", //J
    "000100", //BEQ
    "000111", //BGTZ
    "011100", //MUL
    "100011", //LW
    "101011", //SW
    
    "110000", //ADD
    "110001", //SUB
    "100001", //MUL
    "110010", //AND
    "110011", //NOR
    "110101" //SLT
    // "000000", //JR,BREAK,SW,SLL,SRL,SRA,NOP
};
string SPECIAL0[10]={
    "001000", //JR
    "001101", //BREAK
    "000010", //SRL
    "000011", //SRA

    "100000", //AND
    "100010", //SUB
    "100100", //AND
    "100111", //NOR
    "101010", //SLT
    // "000000", //NOP 全是0
    "000000" //SLL 
};

int circle = 1;
void simulation_print(int ins_num){
    printf("--------------------\nCycle:%d\t%d\t",circle,ins_num);
    ins_print(ins[ins_num].instype,ins_num);
    printf("\nRegisters\n");
    int r_num=0;
    while(r_num<32){
        printf("R%02d:",r_num);
        for(int i=0;i<16;i++)printf("\t%d",gps[i]);
        printf("\n");
        r_num+=16;
    }
    printf("\nData\n");
    r_num=isword;
    while(r_num<data_num){
        printf("%d:",r_num);
        for(int i=0;i<8;i++)printf("\t%d",data[i]);
        printf("\n");
        r_num+=8;
    }
    printf("\n");
}
void disassembler_print(){
    for(int i=64;i<isword;i+=4){
        for(int j=0;j<6;j++)cout<<ins[ins_num].raw[i]<<" ";
        printf("\t%d\t",i);
        ins_print(ins[i].instype,i);
    }
    for(int i=isword;i<data_num;i+=4){
        cout<<ins[i].whole<<'\t'<<i<<'\t'<<ins[i].imm<<endl;
    }
}
void ins_print(int index,int ins_num){
    switch(index){
    case 0:
        printf("BLTZ R%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
        break;
    case 1:
        printf("J #%d\n",ins[ins_num].imm);
    case 2:
        printf("BEQ R%d, R%d, #%d\n",ins[ins_num].rs,ins[ins_num].rt,ins[ins_num].offset);
        break;
    case 3:
        printf("BGTZ R%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
        break;
    case 4: case 9:
        if(ins[ins_num].raw[0][0]=='0')printf("MUL R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("MUL R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 5:
        printf("LW R%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
        break;
    case 6:
        printf("SW R%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
        break;
    case 7: case 17:
        if(ins[ins_num].raw[0][0]=='0')printf("ADD R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("ADD R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 8: case 18:
        if(ins[ins_num].raw[0][0]=='0')printf("SUB R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("SUB R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 10: case 19:
        if(ins[ins_num].raw[0][0]=='0')printf("AND R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("AND R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 11: case 20:
        if(ins[ins_num].raw[0][0]=='0')printf("NOR R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("NOR R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 12: case 21:
        if(ins[ins_num].raw[0][0]=='0')printf("SLT R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("SLT R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 13:
        printf("JR R%d\n",ins[ins_num].rs);
        break;
    case 14:
        printf("BREAK\n");
        break;
    case 15:
        printf("SRL R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
        break;
    case 16:
        printf("SRA R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);  
        break;
    case 22:
        printf("SLL R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
        break;
    case 23:
        printf("NOP\n");
    }
}
int main(int argc, char** argv){
    if(argc == 1){
        printf("reading from input file\n");
    }
    freopen("disassembly.txt","w",stdout);
    freopen(argv[1],"r",stdin);
    
    string buf;
    int ins_len[6] = {6,5,5,5,5,6};
    int ins_num = 64; //当前扫描到的指令
    
    while(cin>>buf){
        ins[ins_num].whole = buf;
        if(isword)ins[ins_num].imm = SignedBinaryToDecimal(buf);
        else{
            for(int i=0,now_index=0;i<6;now_index+=ins_len[i],i++)ins[ins_num].raw[i]=buf.substr(now_index,ins_len[i]);
            ins[ins_num].rs = BinaryToDecimal(ins[ins_num].raw[1]);
            ins[ins_num].rt = BinaryToDecimal(ins[ins_num].raw[2]);
            ins[ins_num].rd = BinaryToDecimal(ins[ins_num].raw[3]);
            ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
            ins[ins_num].offset = BinaryToDecimal(buf.substr(16,16))<<2;//注意lw,sw不需要左移两位
            ins[ins_num].imm = SignedBinaryToDecimal(buf.substr(16,16));//注意jump的offset是6，26
            
            bool gettar = false;
            if(buf.compare("00000000000000000000000000000000")==0)ins[ins_num].instype = 24;
            for(int i=0;i<13;i++){
                if(ins[ins_num].raw[0].compare(SPECIAL[i])==0){
                    if(i==1)ins[ins_num].imm = BinaryToDecimal(buf.substr(6,26))<<2;
                    if(i==5||i==6)ins[ins_num].offset = ins[ins_num].offset>>2;
                    ins[ins_num].instype = i;
                    gettar = true;
                    break;
                }
            }
            if(!gettar && ins[ins_num].raw[0].compare("000000")==0){
                for(int i=0;i<10;i++){
                    if(ins[ins_num].raw[5].compare(SPECIAL0[i])==0){
                        if(i==1)isword = ins_num+4;
                        ins[ins_num].instype = 13+i;
                        break;
                    }
                }
            }
        }
        ins_num+=4;
    }
    data_num=ins_num;
    disassembler_print();
    fclose(stdout);
    fclose(stdin);
    return 0;
}