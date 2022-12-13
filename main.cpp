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
const int INF = 1000000;
int gps[REG_SIZE]; //保存寄存器值
int pc=64,usejump=0;
int data_num = 0,data_begin; //data数量,data开始的位置
struct INSTRCUTION{ //指令
    int instype;
    int rs,rt,rd,imm,offset,sa;
    int des, src1, src2;
    string raw[6],whole;
}ins[INS_SIZE];
int wgps[REG_SIZE]; //功能单元中有没有人写他

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
void fun_BEQ(int index){//2
    if(gps[ins[index].rt] == gps[ins[index].rs])pc+=ins[index].offset;
}

void fun_BLTZ(int index){//0
    if(gps[ins[index].rs]<0) pc+=ins[index].offset;
}

void fun_BGTZ(int index){//3
    if(gps[ins[index].rs]>0) pc+=ins[index].offset;
}

void fun_LW(int index){//5
    try{
        int vaddr = ins[index].offset+gps[ins[index].rs];
        if(vaddr%4!=0)throw "TLB Refill TLB Invalid, Bus Error, Address Error, Watch";
        gps[ins[index].rt] = ins[vaddr].imm;
    }catch(char* str){
        cout<<str<<endl;
    }
}
void fun_SW(int index){//6
    try{
        int vaddr = ins[index].offset+gps[ins[index].rs];
        if(vaddr%4!=0)throw "TLB Refill TLB Invalid, Bus Error, Address Error, Watch";
        ins[vaddr].imm = gps[ins[index].rt];
    }catch(char* str){
        cout<<str<<endl;
    }
}

void fun_SLL(int index){//22
    gps[ins[index].rd] = gps[ins[index].rt]<<ins[index].sa;
}
void fun_SRL(int index){//15 逻辑右移
    gps[ins[index].rd] = ((unsigned)gps[ins[index].rt])>>ins[index].sa;
}
void fun_SRA(int index){//16 算数右移
    gps[ins[index].rd] = gps[ins[index].rt]>>ins[index].sa;
}

void fun_J(int index){//0
    index = ins[index].imm;
    usejump = 1;
}

void fun_JR(int index){//13
    index = gps[ins[index].rs];
    usejump = 1;
}
void fun_BREAK(int index){//14
}

void fun_MUL(int index){//4 9
    if(ins[index].raw[0][0]=='0')gps[ins[index].rd]=gps[ins[index].rs]*gps[ins[index].rt];  
    else gps[ins[index].rt]=gps[ins[index].rs]*ins[index].imm; 
}
void fun_ADD(int index){//7 17
    try{
        if(ins[index].raw[0][0]=='0')addOvf(gps[ins[index].rd],gps[ins[index].rs],gps[ins[index].rt]);  
        else addOvf(gps[ins[index].rt],gps[ins[index].rs],ins[index].imm); 
    }
    catch(char *str){
        cout<<str<<endl;
    }
}
void fun_SUB(int index){//8 18
    try{
        if(ins[index].raw[0][0]=='0')subOvf(gps[ins[index].rd],gps[ins[index].rs],gps[ins[index].rt]);  
        else subOvf(gps[ins[index].rt],gps[ins[index].rs],ins[index].imm); 
    }
    catch(char *str){
        cout<<str<<endl;
    }
}
void fun_AND(int index){//10 19
    if(ins[index].raw[0][0]=='0') gps[ins[index].rd] = gps[ins[index].rs] & gps[ins[index].rt];  
    else gps[ins[index].rt] = gps[ins[index].rs] & ins[index].imm;
}
void fun_NOR(int index){//11 20 
    if(ins[index].raw[0][0]=='0') gps[ins[index].rd] = ~(gps[ins[index].rs] | gps[ins[index].rt]);  
    else gps[ins[index].rt] = ~(gps[ins[index].rs] | ins[index].imm);
}
void fun_SLT(int index){//12 21
    if(ins[index].raw[0][0]=='0') gps[ins[index].rd] = gps[ins[index].rs]<gps[ins[index].rt]?1:0; 
    else gps[ins[index].rt] = gps[ins[index].rs]<ins[index].imm?1:0;
}
void fun_NOP(int index){//23
}
// 7 6 4 5 2
void (*func[])(int) = {
    fun_BLTZ,fun_J,fun_BEQ,fun_BGTZ,fun_MUL,fun_LW,fun_SW, 
    fun_ADD,fun_SUB,fun_MUL,fun_AND,fun_NOR,fun_SLT,
    fun_JR,fun_BREAK,fun_SRL,fun_SRA,
    fun_ADD,fun_SUB,fun_AND,fun_NOR,fun_SLT,
    fun_SLL,fun_NOP 
};
//指令所属的功能单元
int belong_func[23] = {0,0,0,0,2,3,3,1,1,2,1,1,1,   0,0,2,2,1,1,1,1,1,2};
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
    int index = ins[ins_num].instype;
    switch(index){
    case 0:
        printf("BLTZ\tR%d, #%d",ins[ins_num].rs,ins[ins_num].offset);
        break;
    case 1:
        printf("J\t#%d",ins[ins_num].imm);
        break;
    case 2:
        printf("BEQ\tR%d, R%d, #%d",ins[ins_num].rs,ins[ins_num].rt,ins[ins_num].offset);
        break;
    case 3:
        printf("BGTZ\tR%d, #%d",ins[ins_num].rs,ins[ins_num].offset);
        break;
    case 4: case 9:
        if(ins[ins_num].raw[0][0]=='0')printf("MUL\tR%d, R%d, R%d",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("MUL\tR%d, R%d, #%d",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 5:
        printf("LW\tR%d, %d(R%d)",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
        break;
    case 6:
        printf("SW\tR%d, %d(R%d)",ins[ins_num].rt,ins[ins_num].offset,ins[ins_num].rs);
        break;
    case 7: case 17:
        if(ins[ins_num].raw[0][0]=='0')printf("ADD\tR%d, R%d, R%d",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("ADD\tR%d, R%d, #%d",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 8: case 18:
        if(ins[ins_num].raw[0][0]=='0')printf("SUB\tR%d, R%d, R%d",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("SUB\tR%d, R%d, #%d",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 10: case 19:
        if(ins[ins_num].raw[0][0]=='0')printf("AND\tR%d, R%d, R%d",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("AND\tR%d, R%d, #%d",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 11: case 20:
        if(ins[ins_num].raw[0][0]=='0')printf("NOR\tR%d, R%d, R%d",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("NOR\tR%d, R%d, #%d",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 12: case 21:
        if(ins[ins_num].raw[0][0]=='0')printf("SLT\tR%d, R%d, R%d",ins[ins_num].rd,ins[ins_num].rs,ins[ins_num].rt);
        else printf("SLT\tR%d, R%d, #%d",ins[ins_num].rt,ins[ins_num].rs,ins[ins_num].imm);
        break;
    case 13:
        printf("JR\tR%d",ins[ins_num].rs);
        break;
    case 14:
        printf("BREAK");
        break;
    case 15:
        printf("SRL\tR%d, R%d, #%d",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
        break;
    case 16:
        printf("SRA\tR%d, R%d, #%d",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);  
        break;
    case 22:
        printf("SLL\tR%d, R%d, #%d",ins[ins_num].rd,ins[ins_num].rt,ins[ins_num].sa);
        break;
    case 23:
        printf("NOP");
    }
}

int circle = 1;
struct Queue{
    int queue[MAX_EXE],head,tail;
    int pop(){return queue[head++];}
    void push(int x){queue[tail++]=x;}
    bool empty(){return head==tail;}
}que[4],post[4];
struct Column{
    int index, inpre, inexe, inwrite;
    int instype;
    int des, src[2],srcd[2];
};
struct Table{
    Column list[MAX_EXE];
    int tnum;
    int push(int index){
        list[tnum].index = index;
        list[tnum].inpre = circle;
        list[tnum].instype = ins[index].instype;
        list[tnum].inexe = list[tnum].inwrite = INF; //预设为无限大
        switch (ins[index].instype){
            case 5: case 7: case 8: case 9: case 10: case 11: case 12:{ //lw add~slt 用立即数的
                list[tnum].des = ins[index].rt;
                list[tnum].src[0] = ins[index].rs;
                list[tnum].src[1] = -1; //-1 为没有
                break;
            }
            case 6: case 2:{//sw beq
                list[tnum].des = -1;
                list[tnum].src[0] = ins[index].rs;
                list[tnum].src[1] = ins[index].rt;
                break;
            }
            case 22: case 15: case 16:{//sll srl sra
                list[tnum].des = ins[index].rd;
                list[tnum].src[0] = ins[index].rt;
                list[tnum].src[1] = -1;
                break;
            }
            case 13: case 0: case 3:{// jr bltz bgtz
                list[tnum].des = -1;
                list[tnum].src[0] = ins[index].rs;
                list[tnum].src[1] = -1;
                break;
            }
            case 17: case 18: case 19: case 20: case 21: case 4: {//add~slt 用寄存器的
                list[tnum].des = ins[index].rd;
                list[tnum].src[0] = ins[index].rs;
                list[tnum].src[1] = ins[index].rt;               
            }
        }
        return tnum++;
    }
    void exe(int num){
        list[num].inexe = circle;
    }
    void write(int num){
        list[num].inwrite = circle;
    }    
}table1;

int pre_issue[4]; //pre_issue buffer
int pre_issue_num=0,delay[4]={1,1,2,1};
string funprin[4]={"ALU","ALUB","MEM"};
int write_io = 0;

bool Fetch(){
    if(!que[0].empty())return false; 
    for(int cou=0;pre_issue_num<4&&cou<2;cou++,pc+=4){
        switch (ins[pc].instype){
            case 14:
                return true; //遇到break停止执行
            case 23:
                continue;//nop 不用做任何事  
            case 1: case 13: case 2: case 0: case 3: {//J JR BEQ BLTZ BGTZ
                int nowtnum = table1.push(pc); //注意检查 b开头的那些对不对！！
                que[0].push(nowtnum);
                pc+=4;
                return false;
            }
            default:{ //其他指令
                int nowtnum = table1.push(pc);
                pre_issue[pre_issue_num++] = nowtnum;
            }
        }
    }
    return false;
}
void Execute(){
    for(int i=1;i<4;i++){ 
        int now = que[i].queue[que[i].head];//执行
        if(!que[i].empty() && circle-table1.list[now].inexe>=delay[i]){//不为空且满足了执行时间
            table1.write(now);
            int havepop = que[i].pop();
            if(table1.list[now].des>=0)post[i].push(havepop); //压入写等待
            else func[table1.list[now].instype](table1.list[now].index); //直接执行
        }
        now = post[i].queue[post[i].head];//写
        if(!post[i].empty() && circle-table1.list[now].inwrite>=1){
            func[table1.list[now].instype](table1.list[now].index);
            if(table1.list[now].des>0)wgps[table1.list[now].des]=-1; //该寄存器不再处于写等待
            post[i].pop();
        }
    }    
}
void Issue_success(int pos){
    int insnum = pre_issue[pos];
    int ins_func = belong_func[table1.list[insnum].instype];
    //放到对应功能区
    que[ins_func].push(insnum);
    //该指令状态为将要执行
    table1.exe(insnum);
    //该寄存器要被写
    if(table1.list[insnum].des>0)wgps[table1.list[insnum].des]=insnum;
    //把issue的指令删掉
    for(int i=pos;i<pre_issue_num-1;i++)pre_issue[i] = pre_issue[i+1];
    pre_issue_num--;
    for(int i=pre_issue_num;i<4;i++)pre_issue[i] = -1;
}
/*
不能issue的情况
1. 功能没空
2. 写目标是之前的写
*/
bool Issue_check(int i){
    //满足发射要求返回true
    int now = pre_issue[i];
    int instype = table1.list[now].instype;
    int insfunc = belong_func[instype];
    if(table1.list[now].inpre == circle)return false;//不能是刚刚取的
    if(que[insfunc].tail-que[insfunc].head>=2)return false;//功能单元没空
    for(int j=0;j<i;j++){
        for(int k=0;k<2;k++){
            if(table1.list[pre_issue[j]].src[k]==table1.list[now].des)return false;//WAR冲突
        }
        if(table1.list[pre_issue[j]].des == table1.list[now].des)return false; //WAW 冲突
    }
    if(table1.list[now].des>=0&&wgps[table1.list[now].des]>=0)return false; //有功能单元要写 WAW
    for(int j=0;j<2;j++){ //检查src有没有准备好
        if(table1.list[now].src[j]>=0){
            if(wgps[table1.list[now].src[j]]>=0)return false; //有功能单元要写
            for(int k=0;k<i;k++){ //有还没发射的语句要写
                if(table1.list[pre_issue[k]].des == table1.list[now].src[j])return false;
            }
        }
    }
    if(instype == 6){//sw
        for(int j=0;j<i;j++){if(table1.list[pre_issue[j]].instype == 6)return false;}
    }
    if(instype == 5){//lw 
        for(int j=0;j<i;j++){if(table1.list[pre_issue[j]].instype == 6)return false;}//lw指令前有sw没有issue
    }
    return true;//满足
        //不确定这里要不要检查写端口占用
        // if(ok && !wgps[ins[ins_num].rt] && !wgps[ins[index].rs]){//资源准备好了, 没出现waw
}
void init(){
    for(int i=0;i<REG_SIZE;i++)wgps[i]=-1;
    for(int i=0;i<4;i++)pre_issue[i]=-1;
}
void Branch_exe(){
    if(!que[0].empty()){
        int now = que[0].queue[que[0].head];//执行pc跳转
        if(circle-table1.list[now].inexe>=1){
            func[table1.list[now].instype](table1.list[now].index);
            que[0].pop();
        }
    }
}
void Branch(){
    if(!que[0].empty()){
        int now = que[0].queue[que[0].head];//执行pc跳转
        if(table1.list[now].inpre == circle)return;
        bool ok = true;
        for(int j=0;j<2;j++){
            if(table1.list[now].src[j]>=0){
                if(wgps[table1.list[now].src[j]]>=0)ok=false; //有功能单元要写
                for(int i=0;i<pre_issue_num;i++){ //有还没发射的语句要写
                    if(table1.list[pre_issue[i]].des == table1.list[now].src[j])ok = false;
                }
            }
        }
        if(ok){table1.list[now].inexe = circle;} //准备执行
    }
}

void pipeline_print(){
    printf("--------------------\nCycle:%d\n\n",circle);
    printf("IF Unit:\n");
    printf("\tWaiting Instruction:");
    if(!que[0].empty()){
        int now = que[0].queue[que[0].head];
        if(table1.list[now].inexe==INF)ins_print(table1.list[now].index);
    }
    printf("\n");
    printf("\tExecuted Instruction:");
    if(!que[0].empty()){
        int now = que[0].queue[que[0].head];
        if(table1.list[now].inexe!=INF)ins_print(table1.list[now].index);
    }
    printf("\n");
    printf("Pre-Issue Buffer:\n");
    for(int i=0;i<4;i++){
        printf("\tEntry %d:",i);
        if(pre_issue[i]>=0){
            printf("[");
            ins_print(table1.list[pre_issue[i]].index);
            printf("]");
        }
        printf("\n");
    }
    for(int i=1;i<4;i++){
        cout<<"Pre-"<<funprin[i-1]<<" Queue:"<<endl;
        for(int j=0;j<2;j++){
            printf("\tEntry %d:",j);
            if(que[i].head+j<que[i].tail){
                printf("[");
                ins_print(table1.list[que[i].queue[que[i].head+j]].index);
                printf("]");
            }
            printf("\n");
        }
        cout<<"Post-"<<funprin[i-1]<<" Buffer:";
        if(!post[i].empty()){
            printf("[");
            ins_print(table1.list[post[i].queue[post[i].head]].index);
            printf("]");
        }
        printf("\n");
    }
    printf("\nRegisters\n");
    int r_num=0;
    while(r_num<32){
        printf("R%02d:",r_num);
        for(int i=0;i<8;i++)printf("\t%d",gps[i+r_num]);
        printf("\n");
        r_num+=8;
    }
    printf("\nData\n");
    r_num=isword;
    while(r_num<data_num){
        printf("%d:",r_num);
        for(int i=0;i<8;i++)printf("\t%d",ins[r_num+(i<<2)].imm);
        printf("\n");
        r_num+=4*8;
    }
}
void pipeline_run(){
    pc = 64;
    circle = 1;
    init();
    while(1){
        Branch_exe();
        if(Fetch())break;
        for(int i=0,cou=0;i<pre_issue_num&&cou<2;){
            if(Issue_check(i)){//检查是否满足发射条件
                cou++; 
                Issue_success(i);//发射成功
            }
            else i++;
        }
        Branch();
        Execute();
        pipeline_print();
        if(usejump>0){//是否jump
            pc = usejump;
            usejump = 0;
        }
        circle+=1;
    }
}
void disassembler_print(){
    for(int i=64;i<isword;i+=4){
        for(int j=0;j<6;j++){
            cout<<ins[i].raw[j];
            if(j!=5)cout<<' ';
        }
        printf("\t%d\t",i);
        ins_print(i);
        printf("\n");
    }
    for(int i=isword;i<data_num;i+=4){
        cout<<ins[i].whole<<'\t'<<i<<'\t'<<ins[i].imm<<endl;
    }
}

int main(int argc, char** argv){
    if(argc == 1){
        printf("reading from input file\n");
    }
    // freopen("disassembly.txt","w",stdout);
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
    // fclose(stdout);
    // freopen("simulation.txt","w",stdout);
    pipeline_run();
    // fclose(stdout);
    fclose(stdin);
    return 0;
}
