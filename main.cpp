#include<iostream>
#include<cstdio>
#include<zlib.h>
#include<cstring>
#include<string>
#include<cstdlib>
#include<fstream>
using namespace std;
const int IMM_SIZE=100;
const int INS_SIZE=100;
const int INPUT = 100;
int gps[32]; //保存寄存器值
int data[IMM_SIZE]; //保存data
int pc=64;
int data_num = 0; //data数量
int ins_num = 0; //当前扫描到的指令
struct INSTRCUTION{ //指令
    int instype;
    int rs,rt,rd,imm,offset,sa;
    string raw[6],whole;
}ins[INS_SIZE];

bool isword = false; // isword = true 表示在读数据部分

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

bool eagerMatch(char* in, const char* str) {
    for (; *str != '\0'; ++str, ++in)
        if (*str != *in)
            return false;
    return true; 
}

void addOvf(int* result, int a, int b) {//有符号加法溢出
    *result = a + b;     
    if(a > 0 && b > 0 && *result < 0)throw "Integer Overflow";     
    if(a < 0 && b < 0 && *result > 0)throw "Integer Overflow";     
}
void fun_BEQ(){
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16))<<2;
    printf("BEQ R%d, R%d, #%d\n",ins[ins_num].rs,ins[ins_num].rt,ins[ins_num].offset);
}

void fun_BLTZ(){
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16))<<2;
    printf("BLTZ R%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
}
void fun_BGTZ(){
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16))<<2;
    printf("BGTZ R%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
}

void fun_LW(){
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16));
    printf("LW R%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
}
void fun_SW(){
    ins[ins_num].offset = BinaryToDecimal(ins[ins_num].whole.substr(16,16));
    printf("SW R%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
}

void fun_SLL(){
    ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
    printf("SLL R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
}
void fun_SRL(){
    ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
    printf("SRL R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
}
void fun_SRA(){
    ins[ins_num].sa = BinaryToDecimal(ins[ins_num].raw[4]);
    printf("SRA R%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
}

void fun_J(){
    ins[ins_num].imm = BinaryToDecimal(ins[ins_num].whole.substr(6,26))<<2;
    printf("J #%d\n",ins[ins_num].imm);
}
void fun_JR(){
    printf("JR R%d\n",ins[ins_num].rs);
}
void fun_BREAK(){
    printf("BREAK\n");
    isword = 1;
}

void fun_MUL(){
    if(ins[ins_num].raw[0][0]=='0'){
        printf("MUL R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("MUL R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_ADD(){
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
void fun_SUB(){
    if(ins[ins_num].raw[0][0]=='0'){
        printf("SUB R%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
    }
    else{
        printf("SUB R%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
    }
}
void fun_AND(){
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
void fun_NOP(){
    printf("NOP\n");
}
void (*func[])() = {fun_BLTZ,fun_J,fun_BEQ,fun_BGTZ,fun_MUL,fun_LW,fun_SW, \
    fun_ADD,fun_SUB,fun_MUL,fun_AND,fun_NOR,fun_SLT,\
    fun_JR,fun_BREAK,fun_SRL,fun_SRA,\
    fun_ADD,fun_SUB,fun_AND,fun_NOR,fun_SLT,fun_SLL,\
    fun_NOP
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
void simulation_print(){
    printf("Cycle:%d\t%d\t");
    
}
int main(int argc, char** argv){
    if(argc == 1){
        printf("reading from input file\n");
    }
    freopen("disassembly.txt","w",stdout);
    freopen(argv[1],"r",stdin);
    
    string buf;
    int ins_len[6] = {6,5,5,5,5,6};
    
    while(cin>>buf){
        if(isword){
            cout<<buf<<'\t'<<64+4*ins_num<<'\t';
            int num = SignedBinaryToDecimal(buf);
            cout<<num<<endl;
            data[data_num++]=num;
        }
        else{
            int now_index = 0;
            ins[ins_num].whole = buf;
            for(int i=0;i<6;i++){
                ins[ins_num].raw[i]=buf.substr(now_index,ins_len[i]);
                now_index += ins_len[i];
                cout<<ins[ins_num].raw[i]<<" ";
            }
            ins[ins_num].rs = BinaryToDecimal(ins[ins_num].raw[1]);
            ins[ins_num].rt = BinaryToDecimal(ins[ins_num].raw[2]);
            ins[ins_num].rd = BinaryToDecimal(ins[ins_num].raw[3]);
            cout<<'\t'<<64+4*ins_num<<'\t';
            bool gettar = false;
            if(buf.compare("00000000000000000000000000000000")==0){
                fun_SLL();
                gettar = true;
            }
            for(int i=0;i<13;i++){
                if(ins[ins_num].raw[0].compare(SPECIAL[i])==0){
                    if(i>=7) ins[ins_num].imm = SignedBinaryToDecimal(buf.substr(16,16));
                    func[i]();
                    gettar = true;
                    break;
                }
            }
            if(!gettar && ins[ins_num].raw[0].compare("000000")==0){
                for(int i=0;i<10;i++){
                    if(ins[ins_num].raw[5].compare(SPECIAL0[i])==0){
                        func[13+i]();
                        gettar = true;
                        break;
                    }
                }
            }
        }
        ins_num++;
    }
    fclose(stdout);
    fclose(stdin);
    return 0;
}