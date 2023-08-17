#include "scanner.hh"
#include "parser.tab.hh"
#include <fstream>
#include<cstdio>
#include<cstring>
#include <vector>
using namespace std;

SymbTab gst, gstfun, gststruct; 
string filename;
extern std::map<string,abstract_astnode*> ast;
extern int getlocalsize(SymbTab gst, string name);
extern int getparamsize(SymbTab gst, string name);
extern std::array<std::string,5> registers;
extern int reg;
std::map<std::string, datatype> predefined {
            {"printf", createtype(VOID_TYPE)},
            {"scanf", createtype(VOID_TYPE)},
            {"mod", createtype(INT_TYPE)}
        };
extern std::vector<std::pair<int, std::string>> printfl;
extern int ljump;
// void printLCInfoList(const std::vector<LCinfo>& lcInfoList)
// {
//     for (const auto& lcInfo : lcInfoList) 
//     {
//         std::cout << ".LC" << lcInfo.labellc << ":" << std::endl;
//         std::cout << "\t.string " << lcInfo.stringlc << std::endl;
//     }
// }

int main(int argc, char **argv)
{
	fstream in_file, out_file;
	

	in_file.open(argv[1], ios::in);

	IPL::Scanner scanner(in_file);

	IPL::Parser parser(scanner);

#ifdef YYDEBUG
	parser.set_debug_level(1);
#endif
parser.parse();
// create gstfun with function entries only

for (const auto &entry : gst.Entries)
{
	if (entry.second.varfun == "fun")
	gstfun.Entries.insert({entry.first, entry.second});
}
// create gststruct with struct entries only

// for (const auto &entry : gst.Entries)
// {
// 	if (entry.second.varfun == "struct")
// 	gststruct.Entries.insert({entry.first, entry.second});
// }
// start the JSON printing


    cout << "\t.section\t.rodata\n";
    // cout << ".LC0:\n";

    // cout << "\t.string \"";
    // cout << "%";
    // cout << "d ";
    // cout << "\\n";
    // cout << "\"\n";

	
    // for(int j=0;j<printfl.size();j++)
	// {
	// 	cout<<".LC"<<printfl[j].first<<":"<<endl;
	// 	cout << "\t.string " << printfl[j].second << endl;
	// }

for (auto it = gstfun.Entries.begin(); it != gstfun.Entries.end(); ++it)
{   
    //generate the label of the function and the prelude
    // for(int j=0;j<printfl.size();j++)
	// {
	// 	cout<<".LC"<<printfl[j].first<<":"<<endl;
	// 	cout << "\t.string " << printfl[j].second << endl;
	// }

    //f:
    cout << "\t.text\n";
    cout<<"\t.globl\t"<<it->first<<endl;
	cout<<"\t.type\t"<<it->first<<",\t@function"<<endl;
    cout<<it->first<<":\n";
    //     pushl  %ebp 
    //     movl   %esp,%ebp
    cout<<"\tpushl\t%ebp\n";
    
    cout <<"\tmovl\t%esp, %ebp\n";
    // int cnt=reg;
    // for(int s=0;s<(getparamsize(gst,it->first)/8);s++){
    //     cout<<"\tpushl\t%"<<registers[cnt]<<endl;
    //     cnt++;
    // }
    cout<<"\tsubl\t$"<<getlocalsize(gst,it->first)<<", %esp"<<endl;
    //generate the code for statements of the function

	ast[it->first]->gencode(); //the code for the statements of the function would have been generated
    

    //generate the stack cleanup code
    //addl $8,%esp
    //
    //basically number of pushl in function
    
    //generate the postlude
    cout<<".L"<<ljump+1<<":\n";
    ljump++;
    cout<<"\tnop\n";
    if(it->first=="main"){
        cout<<"\tleave\n";
    }
    else{
        // cout<<"\tleave\n";
        if(it->second.type.type!=0){
            cout<<"\tleave\n";
        }
        else{
            // int cnt=reg;
            // for(int s=0;s<(getparamsize(gst,it->first)/8);s++){
                
            //     cnt--;
            //     cout<<"\tpopl\t%"<<registers[cnt]<<endl;
            // }
            cout<<"\tpopl\t%ebp\n";
        }
        // cout<<it->second.type.type<<endl;
        
    }	
	cout<<"\tret\n";
    //      nop
    //      leave
    //      ret
	
}
    for(int j=0;j<printfl.size();j++)
	{
		cout<<".LC"<<printfl[j].first<<":"<<endl;
		cout << "\t.string " << printfl[j].second << endl;
	}
    

	fclose(stdout);
}

