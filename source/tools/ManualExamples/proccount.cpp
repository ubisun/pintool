/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2018 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
//
// This tool counts the number of times a routine is executed and 
// the number of instructions executed in a routine
//

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include "pin.H"
#include "Profile.h"

ofstream outFile;
ofstream myFile;

typedef std::map<std::string, UINT64> OP_t;
OP_t g_op;

#define VERSION 1.05
#define START_PROFILE_FNAME    "_Z12startProfilei"
#define END_PROFILE_FNAME      "_Z10endProfilei"
#define DO_NOT_PROFILE  0

int idx;
unsigned long insCount[MAX_PROFILE_FUNC] = {0,};
int curStep[MAX_PROFILE_FUNC];
int profiling = DO_NOT_PROFILE;
int originTargetFunc;

VOID printLegacyResult();
VOID printGroupCount();

// Holds instruction count for a single procedure
typedef struct RtnCount
{
    string _name;
    string _image;
    ADDRINT _address;
    RTN _rtn;
    UINT64 _rtnCount;
    UINT64 _icount;
    OP_t _op;
    struct RtnCount * _next;
} RTN_COUNT;

// Linked list of instruction counts for each routine
RTN_COUNT * RtnList = 0;

// This function is called before every instruction is executed
VOID docount(UINT64 * counter)
{
    if (profiling > DO_NOT_PROFILE)
        (*counter)++;
}

// This function is called before every instruction is executed
VOID opcount(UINT64 * counter)
{
    if (profiling > DO_NOT_PROFILE)
        (*counter)++;
}

VOID groupcount(string &image)
{
    int want = 0;
    if (profiling > DO_NOT_PROFILE) {
        size_t m = image.find("libm.so");
        size_t gmp = image.find("libgmp.so");
        size_t gcc_s =image.find("libgcc_s.so");
        size_t heaan = image.find("TestHEAAN");
        if (m != std::string::npos ||
            gmp != std::string::npos ||
            gcc_s != std::string::npos ||
            heaan != std::string::npos) {
            want = 1;
        }

        if (want == 1) insCount[profiling]++;
    }
}


VOID profile_ctl(INT32 ctl)
{
    //cout << "profile ctl : " << ctl << endl;
    if (ctl > 0) {
        if (profiling == -1) {
            if (originTargetFunc != ctl)
                return; // Do not start until target func                
        }
        //cout << "profile ctl : " << ctl << endl;
        profiling = ctl;
        curStep[++idx] = ctl;        
    } else {
        if ((ctl * -1) == originTargetFunc) {
            printLegacyResult();
            printGroupCount();
            cout << "Group count file written Done." << endl;
            profiling = DO_NOT_PROFILE;
            if (idx != 0) {
                cout << "Something wrong idx : " << idx << endl;
                cout << "Maybe the count result is wrong" << endl;
                exit(1); // End program
            }
            idx = -1;
        }
        
        if (idx > 0) {
            profiling = curStep[--idx];
        }
    }
}
    
const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

// Pin calls this function every time a new rtn is executed
VOID Routine(RTN rtn, VOID *v)
{
    // Allocate a counter for this routine
    RTN_COUNT * rc = new RTN_COUNT;

    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
    rc->_name = RTN_Name(rtn);
    if (rc->_name.find("startProfile") != std::string::npos) {
        cout << rc->_name << endl;
    }
    if (rc->_name.find("endProfile") != std::string::npos) {
        cout << rc->_name << endl;
    }
    rc->_image = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
    rc->_address = RTN_Address(rtn);
    rc->_icount = 0;
    rc->_rtnCount = 0;

    // Add to list of routines
    rc->_next = RtnList;
    RtnList = rc;
            
    RTN_Open(rtn);
            
    // Insert a call at the entry point of a routine to increment the call count
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_rtnCount), IARG_END);
    
    // For each instruction of the routine
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        // get dissassemble code
        // return value is string
        // ex) jmp %ebp-0x10
        //std::string disassemble = INS_Disassemble(ins);

        // get only operand
        // ex) jmp %ebp-0x10
        // =====> jmp
        //std::string op = disassemble.substr(0, disassemble.find(' '));

        // for column of Fini table
        //g_op[op] += 1;

        // for callback
        //rc->_op[op] += 0;

        // Insert a call to docount to increment the instruction counter for this rtn
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_icount), IARG_END);
        //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)opcount, IARG_PTR, &(rc->_op[op]), IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)groupcount, IARG_PTR, rc->_image, IARG_END);
    }

    
    RTN_Close(rtn);
}

VOID Image(IMG img, VOID *v)
{
    RTN startProfileRtn = RTN_FindByName(img, START_PROFILE_FNAME);
    RTN endProfileRtn = RTN_FindByName(img, END_PROFILE_FNAME);
    
    if (RTN_Valid(startProfileRtn))
    {
        cout << "Found start Profile in Image" << endl;
        RTN_Open(startProfileRtn);
        RTN_InsertCall(startProfileRtn, IPOINT_BEFORE, (AFUNPTR)profile_ctl,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);

        RTN_Close(startProfileRtn);
    }

    if (RTN_Valid(endProfileRtn))
    {
        cout << "Found end Profile in Image" << endl;
        RTN_Open(endProfileRtn);
        RTN_InsertCall(endProfileRtn, IPOINT_BEFORE, (AFUNPTR)profile_ctl,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);

        RTN_Close(endProfileRtn);
    }
}


VOID printGroupCount() {
    if (originTargetFunc == NORMAL_ADD) myFile.open("profile/NORMAL_ADD.group");
    else if (originTargetFunc == HE_ADD) myFile.open("profile/HE_ADD.group");
    else if (originTargetFunc == NORMAL_MULT) myFile.open("profile/NORMAL_MULT.group");
    else if (originTargetFunc == HE_MULT) myFile.open("profile/HE_MULT.group");
    else if (originTargetFunc == BOOT_STRAP) myFile.open("profile/BOOT_STRAP.group");
    else {
        cout << "Something wrong originTargetFunc value : " << originTargetFunc << endl;
        cout << "Save in profile/unknown.group" << endl;
        myFile.open("profile/unknown.group");
    }

    for (int i = 0; i < MAX_PROFILE_FUNC; i++) {
        if (insCount[i] == 0) {
            continue;
        }

        if (i == NORMAL_ADD) myFile << "NORMAL_ADD : " ;
        if (i == HE_ADD) myFile << "HE_ADD : ";
        if (i == RING_addAndEqual) myFile << "RING_addAndEqual : " ;
        if (i == NORMAL_MULT) myFile << "NORMAL_MULT : ";
        if (i == HE_MULT) myFile << "HE_MULT : ";
        if (i == RING_CRT) myFile << "RING_CRT : ";
        if (i == RING_multDNTT) myFile << "RING_multDNTT : ";
        if (i == RING_addNTTAndEqual) myFile << "RING_addNTTAndEqual : ";
        if (i == RING_rightShiftAndEqual) myFile << "RING_rightShiftAndEqual : ";
        if (i == RING_subAndEqual) myFile << "RING_subAndEqual : ";
        if (i == RING_leftRotate) myFile << "RING_leftRotate : ";
        if (i == RING_MULT_CRT) myFile << "RING_MULT_CRT : ";
        if (i == RING_MULT_NTT) myFile << "RING_MULT_NTT : ";
        if (i == RING_MULT_multDNTT) myFile << "RING_MULT_multDNTT : ";
        if (i == RING_MULT_INTT) myFile << "RING_MULT_INTT : ";
        if (i == RING_MULT_reconstruct) myFile << "RING_MULT_reconstruct : ";
        if (i == RING_MULT_addNTTAndEqual) myFile << "RING_MULT_addNTTAndEqual : ";
        if (i == RING_MULT_multNTT) myFile << "RING_MULT_multNTT : ";
        if (i == RING_MULT_squareNTT) myFile << "RING_MULT_squareNTT : ";
        if (i == BOOT_STRAP) myFile << "BOOT_STRAP : ";
        if (i == BOOT_STRAP_SUB_SUM) myFile << "BOOT_STRAP_SUB_SUM : ";
        if (i == BOOT_STRAP_CoeffToSlot) myFile << "BOOT_STRAP_CoeffToSlot : ";
        if (i == BOOT_STRAP_EvalExp) myFile << "BOOT_STRAP_EvalExp : ";
        if (i == BOOT_STRAP_SlotToCoeff) myFile << "BOOT_STRAP_SlotToCoeff : ";


        myFile << insCount[i] << endl;
    }

    myFile << "idx = " << idx << endl;
    myFile.close();
}

VOID printLegacyResult() {
    outFile.close();
    if (originTargetFunc == NORMAL_ADD) outFile.open("profile/NORMAL_ADD.out");
    else if (originTargetFunc == HE_ADD) outFile.open("profile/HE_ADD.out");
    else if (originTargetFunc == NORMAL_MULT) outFile.open("profile/NORMAL_MULT.out");
    else if (originTargetFunc == HE_MULT) outFile.open("profile/HE_MULT.out");
    else if (originTargetFunc == BOOT_STRAP) outFile.open("profile/BOOT_STRAP.out");
    else {
        cout << "Something wrong originTargetFunc value : " << originTargetFunc << endl;
        cout << "Save in profile/unknown.out" << endl;
        outFile.open("profile/unknown.out");
    }
    
    // Start of column
    outFile << setw(23) << "Procedure" << " "
          << setw(15) << "Image" << " "
          << setw(18) << "Address" << " "
          << setw(12) << "Calls" << " "
          << setw(12) << "Instructions";
    // OP_t::iterator it;
    // for (it = g_op.begin(); it != g_op.end(); it++)
    // {
    //         outFile << setw(12) << it->first;
    // }
    outFile << endl;
    // End of column

    for (RTN_COUNT * rc = RtnList; rc; rc = rc->_next)
    {
        if (rc->_icount > 0)
	{
            outFile << setw(23) << rc->_name << " "
                  << setw(15) << rc->_image << " "
                  << setw(18) << hex << rc->_address << dec <<" "
                  << setw(12) << rc->_rtnCount << " "
                  << setw(12) << rc->_icount;

	    // OP_t::iterator it;
	    // for (it = g_op.begin(); it != g_op.end(); it++)
	    // {
	    //         outFile << setw(12) << rc->_op[it->first];
	    // }
            rc->_rtnCount = 0;
            rc->_icount = 0;
            outFile << endl;

       }
    }
}


// This function is called when the application exits
// It prints the name and count for each procedure
VOID Fini(INT32 code, VOID *v)
{
    // printLegacyResult();
    // printGroupCount();
    cout << "Version : " << VERSION << endl;
    cout << "originTargetFunc : " << originTargetFunc << endl;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This Pintool counts the number of times a routine is executed" << endl;
    cerr << "and the number of instructions executed in a routine" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    originTargetFunc = atoi(argv[argc-1]);
    profiling = -1;
    idx = -1;

    cout << "Version : " << VERSION << endl;
    cout << "originTargetFunc : " << originTargetFunc << endl;

    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    outFile.open("proccount.out");

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Routine to be called to instrument rtn
    RTN_AddInstrumentFunction(Routine, 0);

    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
