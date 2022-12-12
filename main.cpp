#include<iostream>
#include<cstdio>
#include<zlib.h>
#include<cstring>
#include<string>
#include<cstdlib>
#include<fstream>
using namespace std;
const int MAX_EXE = 10000;
const int INS_SIZE=10000;
const int INPUT = 10000;
const int REG_SIZE=32;
int gps[REG_SIZE]; //保存寄存器值
int pc=64;
int data_num = 0,data_begin; //data数量,data开始的位置
struct INSTRCUTION{ //指令
    int instype;
    int rs,rt,rd,imm,offset,sa;
    string raw[6],whole;
}ins[INS_SIZE];
int wgps[REG_SIZE],rgps[REG_SIZE],reg_status[REG_SIZE];

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
}=-

void addOvf(int& result, int a, int b) {//有符号加法溢出
    result = a + b;     
    if(a > 0 && b > 0 && result < 0)throw "Integer Overflow";     
    if(a < 0 && b < 0 && result > 0)throw "Integer Overflow";     
}
void subOvf(int& result, int a, int b) {//有符号加法溢出
    result = a - b;     
    if(a > 0 && b < 0 && result < 0)throw "Integer Overflow";     
    if(a < 0 && b > 0 && result > 0)throw "Integer Overflow";     
}
void fun_BEQ(int& index){//2
    if(gps[ins[index].rt] == gps[ins[index].rs])index+=ins[index].offset;
}

void fun_BLTZ(int& index){//0
    if(gps[ins[index].rs]<0) index+=ins[index].offset;
}

void fun_BGTZ(int& index){//3
    if(gps[ins[index].rs]>0) index+=ins[index].offset;
}

void fun_LW(int& index){//5
    try{
        int vaddr = ins[index].offset+gps[ins[index].rs];
        if(vaddr%4!=0)throw "TLB Refill TLB Invalid, Bus Error, Address Error, Watch";
        gps[ins[index].rt] = ins[vaddr].imm;
    }catch(char* str){
        cout<<str<<endl;
    }
}
void fun_SW(int& index){//6
    try{
        int vaddr = ins[index].offset+gps[ins[index].rs];
        if(vaddr%4!=0)throw "TLB Refill TLB Invalid, Bus Error, Address Error, Watch";
        ins[vaddr].imm = gps[ins[index].rt];
    }catch(char* str){
        cout<<str<<endl;
    }
}

void fun_SLL(int& index){//22
    gps[ins[index].rd] = gps[ins[index].rt]<<ins[index].sa;
}
void fun_SRL(int& index){//15 逻辑右移
    gps[ins[index].rd] = ((unsigned)gps[ins[index].rt])>>ins[index].sa;
}
void fun_SRA(int& index){//16 算数右移
    gps[ins[index].rd] = gps[ins[index].rt]>>ins[index].sa;
}

void fun_J(int& index){//0
    index = ins[index].imm;
}

void fun_JR(int& index){//13
    index = gps[ins[index].rs];
}
void fun_BREAK(int& index){//14
}

void fun_MUL(int& index){//4 9
    if(ins[index].raw[0][0]=='0')gps[ins[index].rd]=gps[ins[index].rs]*gps[ins[index].rt];  
    else gps[ins[index].rt]=gps[ins[index].rs]*ins[index].imm; 
}
void fun_ADD(int& index){//7
    try{
        if(ins[index].raw[0][0]=='0')addOvf(gps[ins[index].rd],gps[ins[index].rs],gps[ins[index].rt]);  
        else addOvf(gps[ins[index].rt],gps[ins[index].rs],ins[index].imm); 
    }
    catch(char *str){
        cout<<str<<endl;
    }
}
void fun_SUB(int& index){//8
    try{
        if(ins[index].raw[0][0]=='0')subOvf(gps[ins[index].rd],gps[ins[index].rs],gps[ins[index].rt]);  
        else subOvf(gps[ins[index].rt],gps[ins[index].rs],ins[index].imm); 
    }
    catch(char *str){
        cout<<str<<endl;
    }
}
void fun_AND(int& index){//10
    if(ins[index].raw[0][0]=='0') gps[ins[index].rd] = gps[ins[index].rs] & gps[ins[index].rt];  
    else gps[ins[index].rt] = gps[ins[index].rs] & ins[index].imm;
}
void fun_NOR(int& index){
    if(ins[index].raw[0][0]=='0') gps[ins[index].rd] = ~(gps[ins[index].rs] | gps[ins[index].rt]);  
    else gps[ins[index].rt] = ~(gps[ins[index].rs] | ins[index].imm);
}
void fun_SLT(int& index){
    if(ins[index].raw[0][0]=='0') gps[ins[index].rd] = gps[ins[index].rs]<gps[ins[index].rt]?1:0; 
    else gps[ins[index].rt] = gps[ins[index].rs]<ins[index].imm?1:0;
}
void fun_NOP(int& index){//23
}
void (*func[])(int &) = {
    fun_BLTZ,fun_J,fun_BEQ,fun_BGTZ,fun_MUL,fun_LW,fun_SW, \
    fun_ADD,fun_SUB,fun_MUL,fun_AND,fun_NOR,fun_SLT,\
    fun_JR,fun_BREAK,fun_SRL,fun_SRA,\
    fun_ADD,fun_SUB,fun_AND,fun_NOR,fun_SLT,\
    fun_SLL,fun_NOP
};
//指令所属的功能单元
int belong_func = {0,0,0,0,2,3,3,1,1,2,1,1,1,   0,0,2,2,1,1,1,1,1,2}
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

void ins_print(int ins_num){
    index = ins[ins_num].instype,
    switch(index){
    case 0:
        printf("BLTZ\tR%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
        break;
    case 1:
        printf("J\t#%d\n",ins[ins_num].imm);
        break;
    case 2:
        printf("BEQ\tR%d, R%d, #%d\n",ins[ins_num].rs,ins[ins_num].rt,ins[ins_num].offset);
        break;
    case 3:
        printf("BGTZ\tR%d, #%d\n",ins[ins_num].rs,ins[ins_num].offset);
        break;
    case 4: case 9:
        if(ins[ins_num].raw[0][0]=='0')printf("MUL\tR%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("MUL\tR%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 5:
        printf("LW\tR%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
        break;
    case 6:
        printf("SW\tR%d, %d(R%d)\n",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
        break;
    case 7: case 17:
        if(ins[ins_num].raw[0][0]=='0')printf("ADD\tR%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("ADD\tR%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 8: case 18:
        if(ins[ins_num].raw[0][0]=='0')printf("SUB\tR%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("SUB\tR%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 10: case 19:
        if(ins[ins_num].raw[0][0]=='0')printf("AND\tR%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("AND\tR%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 11: case 20:
        if(ins[ins_num].raw[0][0]=='0')printf("NOR\tR%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("NOR\tR%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 12: case 21:
        if(ins[ins_num].raw[0][0]=='0')printf("SLT\tR%d, R%d, R%d\n",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("SLT\tR%d, R%d, #%d\n",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 13:
        printf("JR\tR%d\n",ins[ins_num].rs);
        break;
    case 14:
        printf("BREAK\n");
        break;
    case 15:
        printf("SRL\tR%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
        break;
    case 16:
        printf("SRA\tR%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);  
        break;
    case 22:
        printf("SLL\tR%d, R%d, #%d\n",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
        break;
    case 23:
        printf("NOP\n");
    }
}

int circle = 1;
void simulation_print(int ins_num){
    printf("--------------------\nCycle:%d\t%d\t",circle,ins_num);
    ins_print(ins[ins_num].instype,ins_num);
    printf("\nRegisters\n");
    int r_num=0;
    while(r_num<32){
        printf("R%02d:",r_num);
        for(int i=0;i<16;i++)printf("\t%d",gps[i+r_num]);
        printf("\n");
        r_num+=16;
    }
    printf("\nData\n");
    r_num=isword;
    while(r_num<data_num){
        printf("%d:",r_num);
        for(int i=0;i<8;i++)printf("\t%d",ins[r_num+(i<<2)].imm);
        printf("\n");
        r_num+=4*8;
    }
    printf("\n");
}
void simulation_run(){
    int oldnow,now = 64;
    while(1){
        oldnow = now;
        func[ins[now].instype](now);
        simulation_print(oldnow);
        circle+=1;
        if(ins[now].instype == 14) break;
        if(ins[oldnow].instype == 1 ||ins[oldnow].instype== 13)continue;
        now += 4;
    }
}
struct Queue{
    int queue[MAX_EXE],head,tail;
    int pop(){return queue[head++];}
    void push(int x){queue[tail++]=x;}
}que[4];
struct Column{
    int index, inpre, inexe, inwrite;
    int instype;
    int des, src1, src2, srcd1, srcd2;
};
struct Table{
    Column list[MAX_EXE];
    int tnum;
    int push(int x){
        list[tnum].index = x;
        list[tnum].inpre = circle;
        list[tnum].instype = ins[x].instype;
        switch (ins[x].instype){
        case /* constant-expression */:
            /* code */
            break;
        
        default:
            break;
        }
    }
}table1;
struct Buffer{
    int ins_num,ti; //指令，取的时间
}if_unit[1],pre_issue[4],pre_fun[4][2],post_fun[4];

int pre_issue_num=0,pre_fun_num[4],delay[3]={1,2,1};
int write_io = 0;
bool Fetch(){
    for(int cou=0;pre_issue_num<4,cou<2;cou++,pc+=4){
        switch (ins[pc].instype){
        case 14:
            return true; //遇到break停止执行
            break;
        case 23:
            continue;//nop 不用做任何事
        case 1: case 13: case 2: case 0: case 3: //J JR BEQ BLTZ BGTZ
            if_unit[0].ins_num = pc;
            if_unit[0].ti = circle;
            pc+=4
            break;
        default: //其他指令
            pre_issue[pre_issue_num].ins_num = pc;
            pre_issue[pre_issue_num].ti = circle;
        }
    }
    return false;
}
void Issue_success(int pos){
    int ins_num = pre_issue[pos].ins_num;
    int ins_type = ins[ins_num].instype;
    int ins_func = belong_func[ins_type];
    //放到对应功能区
    pre_fun[ins_func][pre_fun_num[ins_func]].ins_num = pre_issue[pos].ins_num;
    pre_fun[ins_func][pre_fun_num[ins_func]].ti = circle;
    pre_fun_num[ins_func]++;
    //把issue的指令删掉
    for(int i=pos;i<pre_issue_num-1;i++){ 
        pre_issue[i].ins_num = pre_issue[i+1].num;
        pre_issue[i].ti = pre_issue[i+1].ti;
    }
    pre_issue_num--;
}
/*
不能issue的情况
1. 功能没空
2. 写目标是之前的写
*/
bool Issue(){
    //done 已经发射的指令
    for(int i=0,done=0;i<pre_issue_num,done<2;i++){
        int now = pre_issue[i].ins_num;
        int ins_num = pre_issue[pos].ins_num;
        int ins_type = ins[ins_num].instype;
        int ins_func = belong_func[ins_type];
        if(pre_fun_num[ins_func]>=2 )continue;//功能有空
        switch (ins[now].instype){
        case 6://SW
            bool ok = true;
            for(int j=0;j<i;j++){
                int check_now = pre_issue[j].ins_num;
                if(pre_issue[check_now].ins_num == 6){
                    ok = false;//sw指令前有sw没有issue
                    break;
                }
            }
            if(write_io==0 && ok && !wgps[ins[ins_num].rt]){ //写端口没有被占用, 资源都准备好了
                Issue_success(i);
                done++;
                i--;
            }
            break;
        case 5://LW
            bool ok = true;
            for(int j=0;j<i;j++){
                int check_now = pre_issue[j].ins_num;
                if(pre_issue[check_now].ins_num == 6){
                    ok = false;//lw指令前有sw没有issue
                    break;
                }
            }
            //不确定这里要不要检查写端口占用
            if(ok && !wgps[ins[ins_num].rt] && !wgps[ins[index].rs]){//资源准备好了, 没出现waw
                Issue_success(i);
                done++;
                i--;
            }
            break;
        
        default:
            break;
        }
    }
}
void pipeline_run(){
    int pc = 64;
    while(1){
        if(Fetch())break;
        if()
        for(int i=0;i<4;i++){
            if
        }
        oldnow = now;
        func[ins[now].instype](now);
        simulation_print(oldnow);
        circle+=1;
        if(ins[now].instype == 14) break;
        if(ins[oldnow].instype == 1 ||ins[oldnow].instype== 13)continue;
        now += 4;
    }
}
void pipeline_print(int ins_num){
    printf("--------------------\nCycle:%d\n\n",circle);
    printf("IF Unit:\n");
    printf("\t>Waiting Instruction:\n"); //!!没处理
    printf("\t>Executed Instruction:\n"); //!!没处理
    printf("Pre-Issue Buffer:\n");
    for(int i=0;i<4;i++){
        printf("Entry %d:",i);
        if(pre_issue[i].ins_num>=0){
            printf("[")
            ins_print(pre_issue[i].ins_num);
            printf("]");
        }
        printf("\n");
    }
    printf("Pre-ALU Queue\n");
    for(int i=0;i<2;i++){
        printf("Entry %d:",i);
        if(pre_alu[i].ins_num>=0){
            printf("[")
            ins_print(pre_alu[i].ins_num);
            printf("]");
        }
        printf("\n");
    }
    printf("Post-ALU Buffer:");
    if(post_alu[i].ins_num>=0){
        printf("[")
        ins_print(post_alu[i].ins_num);
        printf("]");
    }
    printf("\n");
    printf("Pre-ALUB Queue\n");
    for(int i=0;i<2;i++){
        printf("Entry %d:",i);
        if(pre_aluB[i].ins_num>=0){
            printf("[")
            ins_print(pre_aluB[i].ins_num);
            printf("]");
        }
        printf("\n");
    }
    printf("Post-ALUB Buffer:");
    if(post_aluB[i].ins_num>=0){
        printf("[")
        ins_print(post_aluB[i].ins_num);
        printf("]");
    }
    printf("\n");
    

    printf("\nRegisters\n");
    int r_num=0;
    while(r_num<32){
        printf("R%02d:",r_num);
        for(int i=0;i<16;i++)printf("\t%d",gps[i+r_num]);
        printf("\n");
        r_num+=16;
    }
    printf("\nData\n");
    r_num=isword;
    while(r_num<data_num){
        printf("%d:",r_num);
        for(int i=0;i<8;i++)printf("\t%d",ins[r_num+(i<<2)].imm);
        printf("\n");
        r_num+=4*8;
    }
    printf("\n");
}



void disassembler_print(){
    for(int i=64;i<isword;i+=4){
        for(int j=0;j<6;j++){
            cout<<ins[i].raw[j];
            if(j!=5)cout<<' ';
        }
        printf("\t%d\t",i);
        ins_print(ins[i].instype,i);
    }
    for(int i=isword;i<data_num;i+=4){
        cout<<ins[i].whole<<'\t'<<i<<'\t'<<ins[i].imm<<endl;
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
            ins[ins_num].offset = SignedBinaryToDecimal(buf.substr(16,16))<<2;//注意lw,sw不需要左移两位
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
    freopen("simulation.txt","w",stdout);
    simulation_run();
    fclose(stdout);
    fclose(stdin);
    return 0;
}
