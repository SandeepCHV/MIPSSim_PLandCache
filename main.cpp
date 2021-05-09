
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

class Block
{
public:
    int64_t data;
    int64_t tag;
    bool dirty;
    bool valid;
    int lru;
    Block()
    {
        this->tag=-1;
        this->dirty=false;
        this->valid=0;
        this->lru=0;
    }
};

class Set
{
public:
    Block* blk;
    int noOfBlks;
    int getTag(int64_t address)
    {
        int temp=0;
        //Calculate the tag and store in temp
        return temp;
    }
    Block* findBlock(int64_t addr)
    {
        //Find whether the address is present in the set and return the block
        for(int i=0;i<noOfBlks;i++)
        {
            if(getTag(addr)==blk[i].tag&&blk[i].valid==1)
            {
                return &blk[i];
            }
        }
        return NULL;
    }
    void updateLRU(Block* blck)
    {
        int lru = blck->lru;
        for(int i=0;i<noOfBlks;i++)
        {
            if(blk[i].lru<lru)
                blk[i].lru++;
        }
        blck->lru=0;
    }
    void insertBlock(Block blck)
    {
        
    }
};

class Cache
{
public:
    Set* sets;
    int cacheSize;
    int blockSize;
    int associativity;
    int accessLatency;
    int noOfBlocks;
    int noOfSets;
    int extractIndex(int64_t addr)
    {
        int temp=0;
        //get the index from address and store in temo
        return temp;
    }
    int access(int64_t addr)
    {
        int set = extractIndex(addr);
        Block* blk = sets[set].findBlock(addr);
        if(blk!=NULL)
        {
            sets[set].updateLRU(blk);
            return blk->data;
        }
        return NULL;
    }
};

struct IFIDPipe
{
    int pc;
    string curr_instr;
    bool input;
};

struct IDEXPipe
{
    int idpc;
    string op;
    string idexsignal;
    bool input;
    int readdata1;
    int readdata2;
    int rs;
    int rt;
    int rd;
};

struct EXMEMPipe
{
    bool input;
    int aluout;
    int writedata;
    int exert;
    int exepc;
    int temp;
    int temp2;
    string exesignal;
    string aluop;
};

struct MEMWBPipe
{
    bool input;
    bool membranch;
    int memreaddata;
    int memalu;
    int memrt;
    string memsignal;
};

//structure for storing a label and its address
struct LabelTable
{
    string label;
    int32_t address;
};

//structure for storing a memory element's label and value
struct MemoryElement
{
    string label;
    int32_t value;
};

//class for the MIPS Simulator
class MIPSSimulator
{
    private:
        string Registers[32]; //array to store names of registers
        int32_t RegisterValues[32]; //array to store values of registers
        string InstructionSet[18]; //array to store names of instructions
        int32_t Mode; //to store the Mode of execution
        vector<string> InputProgram; //to store the input program
        int32_t NumberOfInstructions; //to store the number of lines in the program
        //string current_instruction; //to store the current instruction being worked with
        int32_t ProgramCounter; //to store the line number being worked with
        int32_t MaxLength; //to store the maximum length of the input program
        bool halt_value; //flag to check if program halted
        int32_t r[3]; //to store register names, values, etc. for the instruction
        vector<struct LabelTable> TableOfLabels; //to store all the labels and addresses
        vector<struct MemoryElement> Memory; //to store all the memory elements
        int32_t Stack[2000]; //stack array
        struct IFIDPipe if_id;
        struct IDEXPipe id_ex;
        struct EXMEMPipe ex_mem;
        struct MEMWBPipe mem_wb;
        bool enableDataForwarding;
        int noOfStalls;
        int clockcycle;
        bool loadHaz;
        bool branchHaz;
        bool flu;
        int wbrd;
        void binaryToDecimal();
        void add();
        void addi();
        void sub();
        void mul();
        void andi();
        void ori();
        void sll();
        void slt();
        void lw();
        void la();
        void sw();
        void bne();
        void beq();
        void j();
        void jr();
        void jal();
        void halt();
        void syscall();
        void preprocess();
        string ReadInstruction(int32_t line);
        void ReadInstructionPL(int32_t line);
        int32_t ParseInstruction();
        void ReportError();
        void ExecuteInstruction(int32_t instruction);
        void OnlySpaces(unsigned long int lower, unsigned long int upper, string str);
        void RemoveSpaces(string &str);
        void checkNumberConversion(string str);
        void findRegister(string current_instruction);
        string findLabel();
        void removeComma(string &current_instruction);
        void checkStackBounds(int32_t index);
        void checkLabelAllowed(string str);
        void inFetch();
        void inDecRF();
        void exec();
        void mem();
        void wb(string signal, int rt, int alu, int readdata);
        void squash();
        void detect();
    public:
        MIPSSimulator(int32_t mode, string fileName);//Constructor
        void Execute();
        bool ExecutePL();
        void displayState();
};
int sorttable(LabelTable a, LabelTable b);
//int sortmemory(MemoryElement a, MemoryElement b);

void MIPSSimulator::Execute()
{
    getchar();
    preprocess();
    while(ProgramCounter<NumberOfInstructions&&!halt_value)
    {
        ReadInstruction(ProgramCounter);
        //RemoveSpaces(current_instruction);
        //if(current_instruction=="")
        {
            ProgramCounter++;
            continue;
        }
        int32_t instruction=ParseInstruction();
        ExecuteInstruction(instruction);
        if(instruction!=6)//if not jump
        {
            ProgramCounter++;
        }
        if(Mode==0&&halt_value)
        {
            displayState();
            getchar();
        }
    }
    displayState();//display state ay the end
    if(!halt_value)
    {
        cout<<"Error: Program ended without halt"<<endl;
        exit(1);
    }
}
bool MIPSSimulator::ExecutePL()
{
    wbrd=mem_wb.memrt;
    wb(mem_wb.memsignal,mem_wb.memrt,mem_wb.memalu,mem_wb.memreaddata);
    mem();
    exec();
    inDecRF();
    inFetch();
    clockcycle++;
    wbrd=mem_wb.memrt;
    //branch hazard
    if(flu==true)
    {
        flu=false;
        squash();
    }
    else
        displayState();
    detect();
    if(mem_wb.memalu==0&&mem_wb.memreaddata==0&&id_ex.idexsignal==""&&clockcycle!=1)
        return false;
    else
        return true;
}
void MIPSSimulator::inFetch()
{
    if_id.input=true;
    if(ReadInstruction(if_id.pc/4)=="")
    {
        if_id.curr_instr="0000000000000000000000";
        if_id.input=false;
    }
    else
        if_id.curr_instr=ReadInstruction(if_id.pc/4);
    if_id.pc=if_id.pc+4;
}
void MIPSSimulator::inDecRF()
{
    id_ex.input=if_id.input;
    if_id.input=false;
    id_ex.idpc=if_id.pc;
    
}
void MIPSSimulator::exec()
{
    ex_mem.input=id_ex.input;
    id_ex.input=false;
    ex_mem.aluop=id_ex.op;
    ex_mem.writedata=id_ex.readdata2;
}
void MIPSSimulator::mem()
{
    mem_wb.input=ex_mem.input;
    ex_mem.input=false;
    mem_wb.membranch=false;
    mem_wb.memrt=ex_mem.exert;
    mem_wb.memalu=ex_mem.aluout;
    mem_wb.memsignal=ex_mem.exesignal;
    //make branchHaz false if branch operation
    //make mem_wb.memreaddata=stack[mem_wb.memalu-40000/4] if it is lw
    //make stack[mem_wb.memalu-40000/4]=mem_wb.writedata if it is sw
}
void MIPSSimulator::wb(string wbsignal, int rt, int alu, int readdata)
{
    if(rt==0)
        return;
    //if wbsignal is r-type ot i-type reisterValues[rt]=alu;
    //if wbsignal is lw registerValues[rt]=readdata;
    //if wbsignal is sw or branch return
}
void MIPSSimulator::squash()
{
    if_id.input=true;
    if_id.pc=ex_mem.exepc+4;
    if(InputProgram[if_id.pc/4]=="")
        if_id.input=false;
    else
        if_id.curr_instr=InputProgram[if_id.pc/4];
    id_ex.input=true;
    id_ex.readdata1=0;
    id_ex.readdata2=0;
    id_ex.rt=0;
    id_ex.rs=0;
    id_ex.rd=0;
    id_ex.idpc=0;
    id_ex.idexsignal="";
}
void MIPSSimulator::detect()
{
    //detect data hazard
}
//constructor
MIPSSimulator::MIPSSimulator(int mode, string fileName)
{
    MaxLength=10000;//maximum length of input file
    NumberOfInstructions=0;
    ProgramCounter=0;
    halt_value=false;
    Memory.clear();//clearing the list and setting size to zero
    TableOfLabels.clear();
    string tempRegisters[] = {"zero","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6",
        "t7","s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","s8","ra"};//names of registers
    for(int32_t i=0;i<32;i++)
    {
        Registers[i]=tempRegisters[i];
    }
    for(int32_t i=0;i<32;i++)
    {
        RegisterValues[i]=0;
    }
    string tempInstructionSet[] = {"add","sub","mul","slt","addi","andi","ori","sll","lw","la","sw","beq","bne","j","jal","jr","halt","syscall"};
    for(int32_t i=0;i<18;i++)
    {
        InstructionSet[i]=tempInstructionSet[i];
    }
    for(int32_t i=0;i<2000;i++)
    {
        Stack[i]=0;//initialising stack elements to zero
    }
    RegisterValues[29]=47996;//Stack Pointer
    RegisterValues[28]=100000000;
    Mode=mode;
    ifstream Inputfile;
    Inputfile.open(fileName.c_str(),ios::in);
    if(!Inputfile)
    {
        cout<<"Error: File does not exist or could not be found"<<endl;
        exit(1);
    }
    string tempString;
    while(getline(Inputfile,tempString))
    {
        NumberOfInstructions++;
        if(NumberOfInstructions>MaxLength)
        {
            cout<<"Error: Number of lines in input is too large"<<endl;
            exit(1);
        }
        InputProgram.push_back(tempString);
    }
    Inputfile.close();
}

//function to store label names and addresses and memory names and values from the whole program
void MIPSSimulator::preprocess()
{
    int32_t i=0,j=0;
    int32_t current_section=-1; //current_section=0 - data section, current_section=1 - text section/
    bool flag=false; //whether ".data" found once
    string tempString="";
    int    isLabel=0;
    int    doneFlag=0;
    int32_t dataStart=0; //line number for start of data section
    int32_t textStart=0;
    for(i=0;i<NumberOfInstructions;i++)
    {
        ReadInstruction(i);
        bool index=false;
        RemoveSpaces(InputProgram[i]);
        if(InputProgram[i]=="")
        {
            continue;
        }
        if(InputProgram[i].substr(0,5)==".data")
            index=true;
        if(!index) //not found
        {
            continue;
        }
        else if(!flag)
        {
            flag=true; //set as found
            current_section=0; //set section
            dataStart=i; //set location where section starts
        }
        else if(index&&flag) //for multiple findings of ".data"
        {
            cout<<"Error: Multiple instances of .data"<<endl;
            ReportError();
        }
    }
    unsigned long int LabelIndex; //to store index of ":"
    if(current_section==0) //in data section
    {
        for(i=dataStart+1;i<NumberOfInstructions;i++)
        {
            ReadInstruction(i);
            RemoveSpaces(InputProgram[i]);
            if(InputProgram[i]=="")
            {
                continue;
            }
            /*for(int k=0;k<current_instruction.size();k++)
            {
                if(current_instruction[k]==':')
                {
                    LabelIndex=k;
                    break;
                }
            }*/
            LabelIndex=InputProgram[i].find(":");
            if(LabelIndex==string::npos) //if ":" not found
            {
                if(InputProgram[i].find(".text")==string::npos) //if text section has not started
                {
                    cout<<"Error: Unexpected symbol in data section"<<endl;
                    ReportError();
                }
                else
                {
                    break;
                }
            }
            if(LabelIndex==0) //if found at first place
            {
                cout<<"Error: Label name expected"<<endl;
                ReportError();
            }
            j=LabelIndex-1; //ignore spaces before ":" till label
            while(j>=0 && (InputProgram[i][j]==' ' || InputProgram[i][j]=='\t'))
            {
                j--;
            }
            tempString=""; //to store label name
            int32_t doneFlag=0; //label name complete
            for(;j>=0;j--)
            {
                if(InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t' && doneFlag==0) //till label name characters are being found
                {
                    tempString=InputProgram[i][j]+tempString;
                }
                else if(InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t' && doneFlag==1) //if something found after name ends
                {
                    cout<<"Error: Unexpected text before label name"<<endl;
                    ReportError();
                }
                else //name ended
                {
                    doneFlag=1;
                }
            }
            checkLabelAllowed(tempString); //check validity of name
            MemoryElement tempMemory;
            tempMemory.label=tempString; //create memory and store memory element
            int32_t wordIndex=InputProgram[i].find(".word"); //search for ".word" in the same way
            if(wordIndex==string::npos)
            {
                cout<<"Error: .word not found"<<endl;
                ReportError();
            }
            OnlySpaces(LabelIndex+1,wordIndex,InputProgram[i]);
            int32_t foundValue=0;
            int32_t doneFinding=0;
            tempString="";
            for(j=wordIndex+5;j<InputProgram[i].size();j++)
            {
                if(foundValue==1 && (InputProgram[i][j]==' ' || InputProgram[i][j]=='\t') && doneFinding==0)
                {
                    doneFinding=1;
                }
                else if(foundValue==1 && InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t' && doneFinding==1)
                {
                    cout<<"Error: Unexpected text after value"<<endl;
                    ReportError();
                }
                else if(foundValue==0 && InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t')
                {
                    foundValue=1;
                    tempString=tempString+InputProgram[i][j];
                }
                else if(foundValue==1 && InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t')
                {
                    tempString=tempString+InputProgram[i][j];
                }
            }
            checkNumberConversion(tempString); //check that number found is a valid integer
            tempMemory.value=stoi(tempString); //change type and store
            Memory.push_back(tempMemory); //add to list
        }
    }
    //sort(Memory.begin(),Memory.end(),sortmemory); //sort for easy comparison
    /*for(i=0;Memory.size()>0 && i<Memory.size()-1;i++) //check for duplicates
    {
        if(Memory[i].label==Memory[i+1].label)
        {
            cout<<"Error: One or more labels are repeated"<<endl;
            exit(1);
        }
    }*/
    bool textFlag=false;
    for(i=ProgramCounter;i<NumberOfInstructions;i++)
    {
        ReadInstruction(i);
        RemoveSpaces(InputProgram[i]);
        bool textIndex=false;
        if(InputProgram[i]=="")
        {
            continue;
        }
        if(InputProgram[i].substr(0,5)==".text")
            textIndex=true; //find text section similar as above
        if(!textIndex)
        {
            continue;
        }
        else if(!textFlag)
        {
            textFlag=true;
            current_section=1;
            textStart=i;
        }
        else if(textIndex&&textFlag)
        {
            cout<<"Error: Multiple instances of .text"<<endl;
            ReportError();
        }
    }
    if(current_section!=1) //if text section not found
    {
        cout<<"Error: Text section does not exist or found unknown string"<<endl;
        exit(1);
    }
    int32_t MainIndex=0; //location of main label
    int32_t foundMain=0; //whether main label found
    LabelIndex=0;
    for(i=textStart+1;i<NumberOfInstructions;i++)
    {
        ReadInstruction(i);
        if(InputProgram[i]=="")
        {
            continue;
        }
        LabelIndex=InputProgram[i].find(":"); //find labels similar as above
        if(LabelIndex==0)
        {
            cout<<"Error: Label name expected"<<endl;
            ReportError();
        }
        if(LabelIndex==-1)
        {
            continue;
        }
        j=LabelIndex-1;
        while(j>=0 && (InputProgram[i][j]==' ' || InputProgram[i][j]=='\t'))
        {
            j--;
        }
        tempString="";
        isLabel=0;
        doneFlag=0;
        for(;j>=0;j--)
        {
            if(InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t' && doneFlag==0)
            {
                isLabel=1;
                tempString=InputProgram[i][j]+tempString;
            }
            else if(InputProgram[i][j]!=' ' && InputProgram[i][j]!='\t' && doneFlag==1)
            {
                cout<<"Error: Unexpected text before label name"<<endl;
                ReportError();
            }
            else if(isLabel==0)
            {
                cout<<"Error: Label name expected"<<endl;
                ReportError();
            }
            else
            {
                doneFlag=1;
            }
        }
        checkLabelAllowed(tempString);
        //OnlySpaces(LabelIndex+1,current_instruction.size(),current_instruction); //check that nothing is after label
        if(tempString=="main") //for main, set variables as needed
        {
            foundMain=1;
            MainIndex=ProgramCounter+1;
        }
        else
        {
            LabelTable tempLabel;
            tempLabel.address=ProgramCounter;
            tempLabel.label=tempString;
            TableOfLabels.push_back(tempLabel); //store labels
        }
    }
    //sort(TableOfLabels.begin(),TableOfLabels.end(),sorttable); //sort labels
    for(i=0;TableOfLabels.size()>0 && i<(TableOfLabels.size()-1);i++) //check for duplicates
    {
        if(TableOfLabels[i].label==TableOfLabels[i+1].label)
        {
            cout<<"Error: One or more labels are repeated"<<endl;
            exit(1);
        }
    }
    if(foundMain==0) //if main label not found
    {
        cout<<"Error: Could not find main"<<endl;
        exit(1);
    }
    ProgramCounter=MainIndex; //set ProgramCounter
    cout<<"Initialized and ready to execute. Current state is as follows:"<<endl;
    displayState();
    cout<<endl<<"Starting execution"<<endl<<endl;
}

//function to display line number and instruction at which error occurred and exit the program
void MIPSSimulator::ReportError()
{
    cout<<"Error found in line: "<<(ProgramCounter+1)<<": "<<InputProgram[ProgramCounter]<<endl;
    displayState();
    exit(1);
}
//function to read an instruction, crop out comments and set the ProgramCounter value
string MIPSSimulator::ReadInstruction(int32_t lineNo)
{
    string current_instruction=InputProgram[lineNo]; //set current_instruction
    RemoveSpaces(current_instruction);
    if(current_instruction.find("#")!=-1) //remove comments
    {
        current_instruction=current_instruction.substr(0,current_instruction.find("#"));
    }
    if_id.pc=lineNo*4; //set ProgramCounter
    return current_instruction;
}
void MIPSSimulator::ReadInstructionPL(int32_t lineNo)
{
    if_id.curr_instr=InputProgram[lineNo];
    if(if_id.curr_instr.find("#")!=-1)
    {
        if_id.curr_instr=if_id.curr_instr.substr(0,if_id.curr_instr.find("#"));
    }
    ProgramCounter=lineNo;
    if_id.pc=4*lineNo;
}
//function to decode the instruction and get the register and put them in r[]
int32_t MIPSSimulator::ParseInstruction()
{
    int32_t i=0,j=0;
    string current_instruction = if_id.curr_instr;
    if(current_instruction.find(":")!=-1)
    {
        return -2;
    }
    if(current_instruction.size()<4)
    {
        cout<<"Error: Unknown operation"<<endl;
        ReportError();
    }
    for(j=0;j<7;j++)
    {
        if(current_instruction[j]==' ' || current_instruction[j]=='\t')
        {
            break;
        }
    }
    string operation=current_instruction.substr(0,j);
    if(current_instruction.size()>0&&j<current_instruction.size()-1)
    {
        current_instruction=current_instruction.substr(j+1);
    }
    //int32_t foundOp=0;
    int32_t OperationID=-1;
    for(i=0;i<18;i++)
    {
        if(operation==InstructionSet[i])
        {
            OperationID=i;
            break;
        }
    }
    if(OperationID==-1)
    {
        cout<<"Error: Unknown operation"<<endl;
        ReportError();
    }
    if(OperationID<4)//R-type
    {
        findRegister(current_instruction);
        id_ex.readdata1=RegisterValues[id_ex.rs];
        id_ex.readdata2=RegisterValues[id_ex.rt];
        if((id_ex.rs==0&&id_ex.rt==0&&id_ex.rd==0)||loadHaz)
        {
            id_ex.idexsignal="00000000000";
            loadHaz=false;
        }
    }
    else if(OperationID<8)//I-type
    {
        for(int32_t count=0;count<2;count++)
        {
            RemoveSpaces(current_instruction);
            findRegister(current_instruction);
            RemoveSpaces(current_instruction);
            //removeComma();
        }
        RemoveSpaces(current_instruction);
        string tempString = "";
        for(int s=0;s<current_instruction.size();s++)
        {
            if(current_instruction[s]!=' '||current_instruction[s]!='\t')
                tempString = tempString + current_instruction[s];
            else
                break;
        }
        //string tempString=findLabel();
        checkNumberConversion(tempString);
        r[2]=stoi(tempString);
    }
    else if(OperationID<11)//lw,sw
    {
        string tempString="";
        int32_t offset;
        RemoveSpaces(current_instruction);
        findRegister(0);//find source or destination register
        RemoveSpaces(current_instruction);
        //removeComma();
        RemoveSpaces(current_instruction);
        //offset type or label type will be present
        if((current_instruction[0]>47&&current_instruction[0]<58) || current_instruction[0]=='-')//offset type
        {
            j=0;
            while(j<current_instruction.size()&&current_instruction[j]!=' '&&current_instruction[j]!='\t'&&current_instruction[j]!='(')
            {
                tempString=tempString+current_instruction[j];
                j++;
            }
            if(j==current_instruction.size())
            {
                cout<<"Error: '(' expected"<<endl;
                ReportError();
            }
            checkNumberConversion(tempString);
            offset=stoi(tempString);
            current_instruction=current_instruction.substr(j);
            RemoveSpaces(current_instruction);
            if(current_instruction=="" || current_instruction[0]!='(' || current_instruction.size()<2)
            {
                cout<<"Error: '(' expected"<<endl;
                ReportError();
            }
            current_instruction=current_instruction.substr(1);
            RemoveSpaces(current_instruction);
            //findRegister(1);
            RemoveSpaces(current_instruction);
            if(current_instruction=="" || current_instruction[0]!=')')
            {
                cout<<"Error: ')' expected"<<endl;
                ReportError();
            }
            current_instruction=current_instruction.substr(1);
            OnlySpaces(0, current_instruction.size(),current_instruction);
            r[2]=offset;
            if(r[2]==-1)
            {
                cout<<"Error: Invalid offset"<<endl;
                ReportError();
            }
        }
        else//label type
        {
            tempString=findLabel();
            int32_t foundLocation=0;
            for(j=0;j<Memory.size();j++)
            {
                if(tempString==Memory[j].label)
                {
                    foundLocation=1;
                    if(OperationID==8)
                    {
                        r[1]=Memory[j].value;//for lw, send value
                    }
                    else
                    {
                        r[1]=j;//for sw,la send index in memory
                    }
                    break;
                }
            }
            if(foundLocation==0)//label not found
            {
                cout<<"Error: Invalid label"<<endl;
                ReportError();
            }
            r[2]=-1;//just to indicate it is not offset type
        }
    }
    else if(OperationID<13)//beq, bne
    {
        for(int32_t count=0;count<2;count++)
        {
            RemoveSpaces(current_instruction);
            findRegister(current_instruction);
            RemoveSpaces(current_instruction);
            //removeComma();
        }
        RemoveSpaces(current_instruction);
        string tempString=findLabel();
        int32_t foundAddress=0;
        for(j=0;j<TableOfLabels.size();j++)
        {
            if(tempString==TableOfLabels[j].label)
            {
                foundAddress=1;
                r[2]=TableOfLabels[j].address;
                break;
            }
        }
        if(foundAddress==0)
        {
            cout<<"Error: Invalid label"<<endl;
            ReportError();
        }
    }
    else if(OperationID<15)//j
    {
        RemoveSpaces(current_instruction);
        int32_t foundAddress=0;
        string tempString=findLabel();
        for(j=0;j<TableOfLabels.size();j++)
        {
            if(tempString==TableOfLabels[j].label)
            {
                foundAddress=1;
                r[0]=TableOfLabels[j].address;
            }
        }
        if(foundAddress==0)
        {
            cout<<"Error: Invalid label"<<endl;
            ReportError();
        }
    }
    else if(OperationID==15)
    {
        RemoveSpaces(current_instruction);
        findRegister(0);
        RemoveSpaces(current_instruction);
    }
    else if(OperationID==16)//halt
    {
        RemoveSpaces(current_instruction);
    }
    else if(OperationID==17)//syscall
    {
        RemoveSpaces(current_instruction);
    }
    return OperationID;
}
void MIPSSimulator::OnlySpaces(unsigned long int lower, unsigned long int upper, string str)
{
    for(unsigned long int i=lower;i<upper;i++)
        {
            if(str[i]!=' ' && str[i]!='\t') //check that only ' ' and '\t' characters exist
            {
                cout<<"Error: Unexpected character"<<endl;
                ReportError();
            }
        }
}
void MIPSSimulator::ExecuteInstruction(int32_t instruction)
{
    switch(instruction)
    {
        case 0:
                    add();
                    break;
        case 1:
                    sub();
                    break;
        case 2:
                    mul();
                    break;
        case 3:
                    slt();
                    break;
        case 4:
                    addi();
                    break;
        case 5:
                    andi();
                    break;
        case 6:
                    ori();
                    break;
        case 7:
                    sll();
                    break;
        case 8:
                    lw();
                    break;
        case 9:
                    la();
                    break;
        case 10:
                    sw();
                    break;
        case 11:
                    beq();
                    break;
        case 12:
                    bne();
                    break;
        case 13:
                    j();
                    break;
        case 14:
                    jal();
                    break;
        case 15:
                    jr();
                    break;
        case 16:
                    halt();
                    break;
        case 17:
                    syscall();
                    break;
        case -2: //if instruction containing label, ignore
                    break;
        default:
                    cout<<"Error: Invalid instruction received"<<endl;
                    ReportError();
    }
}

//function to remove spaces starting from first elements till they exist continuously in str
void MIPSSimulator::RemoveSpaces(string &str)
{
    int32_t j=0;
    while(j<str.size() && (str[j]==' ' || str[j]=='\t')) //till only ' ' or '\t' found
    {
        j++;
    }
    str=str.substr(j); //remove all of those
}
void MIPSSimulator::add()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]+RegisterValues[r[2]]);
    }
    if(r[0]!=0&&r[0]!=1&&r[1]!=1&&r[2]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]+RegisterValues[r[2]];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
    }
}
void MIPSSimulator::addi()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]+r[2]);
    }
    if(r[0]!=0 && r[0]!=1 && r[1]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]+r[2];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::sub()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]-RegisterValues[r[2]]);
    }
    if(r[0]!=0&&r[0]!=1&&r[1]!=1&&r[2]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]-RegisterValues[r[2]];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
    }
}
void MIPSSimulator::mul()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]*RegisterValues[r[2]]);
    }
    if(r[0]!=0&&r[0]!=1&&r[1]!=1&&r[2]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]*RegisterValues[r[2]];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
    }
}
void MIPSSimulator::andi()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]&r[2]);
    }
    if(r[0]!=0 && r[0]!=1 && r[1]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]&r[2];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::ori()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]|r[2]);
    }
    if(r[0]!=0 && r[0]!=1 && r[1]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]|r[2];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::sll()
{
    if(r[0]==29)
    {
        checkStackBounds(RegisterValues[r[1]]<<r[2]);
    }
    if(r[0]!=0 && r[0]!=1 && r[1]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]<<r[2];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::slt()
{
    if(r[0]!=0 && r[0]!=1 && r[1]!=1 && r[2]!=1)
    {
        RegisterValues[r[0]]=RegisterValues[r[1]]<RegisterValues[r[2]];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::lw()
{
    if(r[0]==29)
        checkStackBounds(r[1]);
    if(r[0]!=0&&r[0]!=1&&r[2]==-1)//label type
        RegisterValues[r[0]]=r[1];
    else if(r[0]!=0&&r[0]!=1)//offset type
    {
        if(RegisterValues[r[1]]+r[2]>48000)
            RegisterValues[r[0]]=Memory[(RegisterValues[r[1]]+r[2]-48000)/4].value;
        else
            RegisterValues[r[0]]=Stack[(RegisterValues[r[1]]+r[2]-40000)/4];
    }
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::la()
{
    RegisterValues[r[0]]=48000+4*r[1];
}
void MIPSSimulator::sw()
{
    if(r[0]!=1&&r[2]==-1)//label type
        Memory[r[1]].value=RegisterValues[r[0]];
    else if(r[0]!=1)//offset type
    {
        if(RegisterValues[r[1]]+r[2]>48000)
            Memory[(RegisterValues[r[1]]+r[2]-48000)/4].value=RegisterValues[r[0]];
        else
            Stack[(RegisterValues[r[1]]+r[2]-40000)/4]=RegisterValues[r[0]];
    }
    /*else if(r[0]==31)
    {
        checkStackBounds(RegisterValues[r[1]]+r[2]);
        Stack[(RegisterValues[r[1]]+r[2]-40000)/4]=ProgramCounter;
    }*/
    else
    {
        cout<<"Error: Invalid usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::beq()
{
    if(r[0]!=1&&r[1]!=1)
    {
        if(RegisterValues[r[0]]==RegisterValues[r[1]])
            ProgramCounter=r[2];
        else
            ProgramCounter++;
    }
    else
    {
        cout<<"Error: Invalis usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::bne()
{
    if(r[0]!=1&&r[1]!=1)
    {
        if(RegisterValues[r[0]]!=RegisterValues[r[1]])
            ProgramCounter=r[2];
        else
            ProgramCounter++;
    }
    else
    {
        cout<<"Error: Invalis usage of registers"<<endl;
        ReportError();
    }
}
void MIPSSimulator::j()
{
    ProgramCounter=r[0];
}
void MIPSSimulator::halt()
{
    halt_value=true;
}
void MIPSSimulator::syscall()
{
    if(RegisterValues[2]==10)
        halt();
    if(RegisterValues[2]==1)
        cout<<RegisterValues[4]<<endl;
}
void MIPSSimulator::jal()
{
    RegisterValues[31]=ProgramCounter+1;
    ProgramCounter=r[0];
}
void MIPSSimulator::jr()
{
    if(r[0]==31)
        ProgramCounter=RegisterValues[31];
    if(ProgramCounter==0)
        halt();
}
void MIPSSimulator::displayState()
{
    int32_t current_address = 40000; // starting address
    if(ProgramCounter<NumberOfInstructions)
        cout<<"Executing Instruction: "<<InputProgram[ProgramCounter]<<endl;
    else
        cout<<endl<<"Executing instruction: "<<InputProgram[ProgramCounter-1]<<endl; //to display at the end, where ProgramCounter==NumberOfInstructions and is out of bounds
    cout<<endl<<"PC:"<<(4*ProgramCounter)<<endl;
    cout<<"Registers:"<<endl;
    cout<<"Register\tNumber\tValues"<<endl;
    for(int32_t i=0;i<32;i++)
        cout<<Registers[i].c_str()<<"\t\t"<<i<<"\t\t"<<RegisterValues[i]<<endl;
    cout<<endl<<"Memory:"<<endl;
    cout<<"Stack Address\tValue"<<endl;
    for(int32_t i=0;i<2000;i++)
        cout<<current_address+4*i<<"\t"<<Stack[i]<<endl;
    current_address+=8000;
    cout<<endl<<"Address\tLabel\tValue"<<endl;
    for(int32_t i=0;i<Memory.size();i++)
        cout<<48000+4*i<<"\t"<<Memory[i].label.c_str()<<"\t"<<Memory[i].value<<endl;
    cout<<endl;
}
void MIPSSimulator::checkNumberConversion(string str)
{
    for(int32_t j=0;j<str.size();j++) //check that each character is a digit
    {
        if(j==0 && str[j]=='-') //ignore minus sign
        {
            continue;
        }
        if(str[j]<48 || str[j]>57)
        {
            cout<<"Error: Specified value is not a number"<<endl;
            ReportError();
        }
    }
    if(str[0]!='-' && (str.size()>10 || (str.size()==10 && str>"2147483647"))) //check against maximum allowed for 32 bit integer
    {
        cout<<"Error: Number out of range"<<endl;
        ReportError();
    }
    else if(str[0]=='-' && (str.size()>11 || (str.size()==11 && str>"-2147483648"))) //same check as above for negative integers
    {
        cout<<"Error: Number out of range"<<endl;
        ReportError();
    }
}
void MIPSSimulator::findRegister(string current_instruction)
{
    bool foundRegister=false;
    if(current_instruction[0]!='$'||current_instruction.size()<2)
    {
        cout<<"Error: Register expected"<<endl;
        ReportError();
    }
    current_instruction=current_instruction.substr(1);
    string registerID=current_instruction.substr(0,2);
    if(registerID=="ze"&&current_instruction.size()>=4)
    {
        registerID+=current_instruction.substr(2,2);
    }
    else if(registerID=="ze")
    {
        cout<<"Error: Register expected"<<endl;
        ReportError();
    }
    for(int32_t i=0;i<32;i++)
    {
        if(registerID==Registers[i])
        {
            id_ex.rd=i;
            foundRegister=true;
            if(i!=0)
                current_instruction=current_instruction.substr(2);
            else
                current_instruction=current_instruction.substr(4);
        }
    }
    if(!foundRegister)
    {
        cout<<"Error: Invalid Register"<<endl;
        ReportError();
    }
    RemoveSpaces(current_instruction);
    removeComma(current_instruction);
    RemoveSpaces(current_instruction);
    foundRegister=false;
    if(current_instruction[0]!='$'||current_instruction.size()<2)
    {
        cout<<"Error: Register expected"<<endl;
        ReportError();
    }
    current_instruction=current_instruction.substr(1);
    registerID=current_instruction.substr(0,2);
    if(registerID=="ze"&&current_instruction.size()>=4)
    {
        registerID+=current_instruction.substr(2,2);
    }
    else if(registerID=="ze")
    {
        cout<<"Error: Register expected"<<endl;
        ReportError();
    }
    for(int32_t i=0;i<32;i++)
    {
        if(registerID==Registers[i])
        {
            id_ex.rs=i;
            foundRegister=true;
            if(i!=0)
                current_instruction=current_instruction.substr(2);
            else
                current_instruction=current_instruction.substr(4);
        }
    }
    if(!foundRegister)
    {
        cout<<"Error: Invalid Register"<<endl;
        ReportError();
    }
    RemoveSpaces(current_instruction);
    removeComma(current_instruction);
    RemoveSpaces(current_instruction);
    foundRegister=false;
    if(current_instruction[0]!='$'||current_instruction.size()<2)
    {
        cout<<"Error: Register expected"<<endl;
        ReportError();
    }
    current_instruction=current_instruction.substr(1);
    registerID=current_instruction.substr(0,2);
    if(registerID=="ze"&&current_instruction.size()>=4)
    {
        registerID+=current_instruction.substr(2,2);
    }
    else if(registerID=="ze")
    {
        cout<<"Error: Register expected"<<endl;
        ReportError();
    }
    for(int32_t i=0;i<32;i++)
    {
        if(registerID==Registers[i])
        {
            id_ex.rt=i;
            foundRegister=true;
            if(i!=0)
                current_instruction=current_instruction.substr(2);
            else
                current_instruction=current_instruction.substr(4);
        }
    }
    if(!foundRegister)
    {
        cout<<"Error: Invalid Register"<<endl;
        ReportError();
    }
}
string MIPSSimulator::findLabel()
{
    RemoveSpaces(current_instruction);
    string tempString="";
    bool foundValue=false;
    bool doneFinding=false;
    for(int32_t j=0;j<current_instruction.size();j++)
    {
        if(foundValue&&(current_instruction[j]==' '||current_instruction[j]=='\t')&&!doneFinding)
            doneFinding=true;
        else if(foundValue&&current_instruction[j]!=' '&&current_instruction[j]!='\t'&&doneFinding)
        {
            cout<<"Error: Unexpected text after value"<<endl;
            ReportError();
        }
        else if(!foundValue&& current_instruction[j]!=' ' && current_instruction[j]!='\t')
        {
            foundValue=true; //when first non space found, finding starts
            tempString=tempString+current_instruction[j];
        }
        else if(foundValue&& current_instruction[j]!=' ' && current_instruction[j]!='\t')
        {
            tempString=tempString+current_instruction[j]; //continue finding
        }
    }
    return tempString;
}
void MIPSSimulator::removeComma(string current_instruction)
{
    if(current_instruction.size()<2||current_instruction[0]!=',')
    {
        cout<<"Error: Comma expected"<<endl;
    }
    current_instruction=current_instruction.substr(1);
}

void MIPSSimulator::checkStackBounds(int32_t index)
{
    if(index>47996||index<40000||index%4!=0)
    {
        cout<<"Error: Invalid address for stack pointer"<<endl;
        ReportError();
    }
}

//function to check that the label name does not start with a number and does not contain special characters
void MIPSSimulator::checkLabelAllowed(string str)
{
    if(str.size()==0 || (str[0]>47 && str[0]<58)) //check that label size is at least one and the first value is not an integer
    {
        cout<<"Error: Invalid label"<<endl;
        ReportError();
    }
    for(int32_t i=0;i<str.size();i++)
    {
        if(!((str[i]>47 && str[i]<58) || (str[i]>=65 && str[i]<=90) || (str[i]>=97 && str[i]<=122))) //check that only numbers and letters are used
        {
            cout<<"Error: Invalid label"<<endl;
            ReportError();
        }
    }
}
/*int32_t sortmemory(MemoryElement a, MemoryElement b)
{
    return a.label<b.label; //sort by label
}*/
int32_t sorttable(LabelTable a, LabelTable b)
{
    return a.label<b.label; //sort by label
}
void ReadFile(Cache l1, Cache l2, string file, string file_, int32_t mode)
{
    ifstream Inputfile;
    Inputfile.open(file.c_str(),ios::in);
    if(!Inputfile)
    {
        cout<<"Error: File does not exist or could not be found"<<endl;
        exit(1);
    }
    string tempString;
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l1.cacheSize=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l2.cacheSize=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l1.blockSize=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l2.blockSize=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l1.associativity=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l2.associativity=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l1.accessLatency=temp;
        }
    }
    getline(Inputfile,tempString);
    for(int i=0;i<tempString.length();i++)
    {
        if(tempString[i]=='=')
        {
            int temp;
            for(int j=i+1;j<tempString.length();j++)
            {
                temp = 10*temp + tempString[i] - '0'
            }
            l2.accessLatency=temp;
        }
    }
    l1.noOfBlocks=l1.cacheSize/l1.blockSize;
    l2.noOfBlocks=l2.cacheSize/l2.blockSize;
    l1.noOfSets=l1.noOfBlocks/l1.associativity;
    l2.noOfSets=l2.noOfBlocks/l2.associativity;
    l1.sets = new Set[l1.noOfSets];
    l2.sets = new Set[l2.noOfSets];
    for(int i=0;i<l1.noOfSets;i++)
    {
        l1.sets[i].noOfBlks = l1.noOfBlocks;
        l1.sets[i].blk = new Block[l1.noOfBlocks];
    }
    for(int i=0;i<l2.noOfSets;i++)
    {
        l2.sets[i].noOfBlks = l2.noOfBlocks;
        l2.sets[i].blk = new Block[l2.noOfBlocks];
    }
    MIPSSimulator simulate(mode-1,file_);
    while(simulate.ExecutePL()==true)
    {
    }
}
int main()
{
    string file;
    int32_t mode;
    cout<<"MIPS Simulator"<<endl;
    cout<<"Modes available:"<<endl<<"1.Step-by-step"<<endl<<"2.Complete Execution"<<endl;
    cout<<"Enter the file and the mode:"<<endl;
    cin>>file>>mode;
    if(mode!=1&&mode!=2)
    {
        cout<<"Error: Invalid mode"<<endl;
        return 1;
    }
    Cache L1Cache;
    Cache L2Cache;
    string file2;
    cout<<"Enter the file with details of cache"<<endl;
    cin>>file2;
    ReadFile(L1Cache,L2Cache,file2,file,mode);
    return 0;
}
