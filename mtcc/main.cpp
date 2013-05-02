#include<Windows.h>
#pragma comment(lib, "winmm.lib")
#include"parser.h"
#include"bloc_graph.h"
#include"bloc_opt.h"
#include"show_mid_code.h"


char* testprog(){
	return
	"{ Sample program\n\
	in TINY language\n\
	}\n\
	read x; { input an integer }\n\
	if 0 < x then { don't compute if x <= 0 }\n\
	  fact := 1;\n\
	  repeat\n\
	    fact := fact * x; \n\
	    x := x - 1 \n\
	  until x = 0; \n\
	  write fact { output factorial of x } \n\
	end";
}

void main(){
	system("title PL");
	auto start_time= timeGetTime();
	auto toks= get_tokens(testprog());
	auto prog= match_stmt_sequence(toks.begin(), toks.end());
	auto prog_code= genopt(prog);
	map<string,mid_code> global_const;
	string tcd1= show_mid_code(prog_code);
	auto const_opt_ed= const_opt(prog_code, global_const);
	string tcd2= show_mid_code(const_opt_ed);
	auto required_opt_ed= flip_unrequired_opt(const_opt_ed);
	string tcd3= show_mid_code(required_opt_ed);

	auto end_time= timeGetTime();
	cout<<tcd3<<endl;
	cout<<"using time: "<<end_time-start_time<<" ms"<<endl;
	system("pause");
}