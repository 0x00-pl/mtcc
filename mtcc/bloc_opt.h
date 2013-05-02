#pragma once
#include<map>
#include<set>
using namespace std;
#include"bloc_graph.h"


vector<mid_code>&  
	const_bloc_opt(
	vector<mid_code>& block,
	vector<mid_block>& block_item,
	map<string,mid_code>& const_val
	);


mid_block const_opt(mid_block bloc, map<string,mid_code>& const_val){
	switch(bloc.type){
	case mid_block::seqence:
		const_bloc_opt(bloc.code_true, bloc.block_item, const_val);
		break;
	case mid_block::if_stmt:
		if(bloc.test_symbol.is_val){
			bloc.test_symbol.val= bloc.test_symbol.val!=0? 1: 0;
		}else if(const_val.find(bloc.test_symbol.name)!=const_val.end() &&
			const_val[bloc.test_symbol.name].op==mid_code::none &&
			const_val[bloc.test_symbol.name].s1.is_val){
				bloc.test_symbol= 
					symbol(const_val[bloc.test_symbol.name].s1.val!=0? 1: 0);
		}
		
		if(bloc.test_symbol.is_val){
			const_bloc_opt(
				bloc.test_symbol.val!=0? bloc.code_true:  bloc.code_false,
				bloc.block_item, const_val);
		}else{
			map<string,mid_code> br_true(const_val.begin(), const_val.end());
			map<string,mid_code> br_false(const_val.begin(), const_val.end());
			
			const_bloc_opt(bloc.code_true, bloc.block_item, br_true);
			const_bloc_opt(bloc.code_false,	bloc.block_item, br_false);

			//交集
			const_val.clear();
			const_val.insert(br_true.begin(), br_true.end());
			const_val.insert(br_false.begin(), br_false.end());
			for(auto i=const_val.begin(); i!=const_val.end();){
				auto j=i;
				++i;
				if(br_true.find(j->first)==br_true.end()){
					const_val.erase(j);
					continue;
				}
				if(br_true[j->first]!=j->second){
					const_val.erase(j);
					continue;
				}
			}
		}
		break;
	case mid_block::repeat_stmt:
		map<string,mid_code> temp_const_map= const_val;
		bool temp_const_test= false;
		for(int i=0; i<120; ++i){
			mid_block temp_bloc= bloc;
			const_bloc_opt(temp_bloc.code_true, temp_bloc.block_item, temp_const_map);
			if(temp_bloc.test_symbol.is_val){
			}else if(temp_const_map.find(temp_bloc.test_symbol.name)!=temp_const_map.end() &&
				temp_const_map[temp_bloc.test_symbol.name].op==mid_code::none &&
				temp_const_map[temp_bloc.test_symbol.name].s1.is_val){
				temp_bloc.test_symbol= symbol(temp_const_map[temp_bloc.test_symbol.name].s1.val);
			}else{
				temp_const_test= false;
				break;
			}

			//正常退出
			if(temp_bloc.test_symbol.is_val && temp_bloc.test_symbol.val==1){
				temp_const_test= true;
				break;
			}
		}
		//todo
		if(temp_const_test){
			/*常量化repeat为if-stmt*/
			bloc= mid_block();
			bloc.type= bloc.if_stmt;
			bloc.test_symbol= symbol(1);
			for(auto iter=temp_const_map.begin(); iter!=temp_const_map.end(); ++iter){
				if(const_val.find(iter->first)==const_val.end() || 
					const_val[iter->first]!=iter->second)
				{
					bloc.code_true.push_back(iter->second);
				}
			}
			const_val= temp_const_map;
		}else{
			const_bloc_opt(bloc.code_true, bloc.block_item, map<string,mid_code>());
			//todo 这里的判断不够精确 暂时先全删掉
			const_val.clear();
		}
		break;
	}
	return bloc;
}

void mid_code_constlizetion(mid_code& cd, map<string,mid_code>& db){
	if(cd.op==mid_code::none){
		//db[cd.s1.name].op==mid_code::none 此句不可去掉 因为
		//i=i+1 j=i ==> i=i+1 j=i+1  逻辑错误
		if(cd.s1.is_val==false && 
			db.find(cd.s1.name)!=db.end() && 
			db[cd.s1.name].op==mid_code::none){
			//a=x;b=a  ==> b=x
			symbol s0= cd.s0;
			cd= db[cd.s1.name];
			cd.s0= s0;
		}
	}else{
		if(!cd.s1.is_val){
			if(db.find(cd.s1.name)!=db.end()){
				if(db[cd.s1.name].op==mid_code::none){
					//a=x; b= a op s2  ==> b= x op s2
					cd.s1= db[cd.s1.name].s1;
				}
			}
		}
		if(!cd.s2.is_val){
			if(db.find(cd.s2.name)!=db.end()){
				if(db[cd.s2.name].op==mid_code::none){
					//a=x; b= s1 op a  ==> b= s2 op x
					cd.s2= db[cd.s2.name].s1;
				}
			}
		}
		if(cd.s1.is_val&&cd.s2.is_val){
			//b=1+2 ==> b=3
			int res= 0xffeeffee;
			int cs1= cd.s1.val;
			int cs2= cd.s2.val;

			switch(cd.op){
			case mid_code::lt:
				res= cs1<cs2? 1: 0;
				break;
			case mid_code::eq:
				res= cs1==cs2? 1: 0;
				break;
			case mid_code::plus:
				res= cs1+cs2;
				break;
			case mid_code::sub:
				res= cs1-cs2;
				break;
			case mid_code::mul:
				res= cs1*cs2;
				break;
			}
			cd.op= mid_code::none;
			cd.s1= symbol(res);
			cd.s2.bad= true;
		}
	}
	
	db[cd.s0.name]= cd;
}

vector<mid_code>& const_bloc_opt(
	vector<mid_code>& block,
	vector<mid_block>& block_item,
	 map<string,mid_code>& const_val
	){
		for(auto iter=block.begin();
			iter!=block.end();
			++iter
			){
				switch(iter->type){
				case mid_code::bloc:
					block_item[iter->s0.val]= 
						const_opt(block_item[iter->s0.val], const_val);
					break;
				case mid_code::set:
					mid_code_constlizetion(*iter,const_val);
					break;
				case mid_code::read:
					const_val.erase(iter->s0.name);
					break;
				case mid_code::write:
					break;
				}
		}
		return block;
}

bool required_bloc_opt(
	vector<mid_code>& block,
	vector<mid_block>& block_item,
	set<string>& required_symbol
	);

mid_block required_opt(mid_block bloc, set<string>& required_symbol){
	switch(bloc.type){
	case mid_block::seqence:
		bloc.required= required_bloc_opt(bloc.code_true, bloc.block_item, required_symbol);
		break;
	case mid_block::if_stmt:
		if(bloc.test_symbol.is_val){
			bloc.required= required_bloc_opt(
				bloc.test_symbol.val!=0? bloc.code_true: bloc.code_false,
				bloc.block_item, required_symbol);
		}else{
			set<string> required_symbol_true(required_symbol.begin(), required_symbol.end());
			set<string> required_symbol_false(required_symbol.begin(), required_symbol.end());

			bloc.required= required_bloc_opt(bloc.code_true, bloc.block_item, required_symbol_true);
			bloc.required|= required_bloc_opt(bloc.code_false, bloc.block_item, required_symbol_false);

			if(bloc.required){
				required_symbol= move(required_symbol_true);
				required_symbol.insert(required_symbol_false.begin(), required_symbol_false.end());
				required_symbol.insert(bloc.test_symbol.name);
			}
		}
		break;
	case mid_block::repeat_stmt:
		mid_block test_block= bloc;
		set<string> test_required_symbol(required_symbol.begin(), required_symbol.end());
		if(required_bloc_opt(test_block.code_true, test_block.block_item, test_required_symbol)){
			bloc.required= true;
			if(!bloc.test_symbol.is_val) 
				required_symbol.insert(bloc.test_symbol.name);
			required_bloc_opt(bloc.code_true, bloc.block_item, required_symbol);
		}
		break;
	}
	return move(bloc);
}

bool required_bloc_opt(
	vector<mid_code>& block,
	vector<mid_block>& block_item,
	set<string>& required_symbol
	){
		bool required_flag= false;
		for(auto iter=block.rbegin();
			iter!=block.rend();
			++iter
			){
				switch(iter->type){
				case mid_code::bloc:
					block_item[iter->s0.val]= 
						required_opt(block_item[iter->s0.val], required_symbol);
					iter->required= block_item[iter->s0.val].required;
					required_flag|= iter->required;
					break;
				case mid_code::set:
					if(required_symbol.find(iter->s0.name)==required_symbol.end())
						break;
					if(!iter->s1.is_val && iter->s0.name==iter->s1.name){
						bool brk= false;
						switch(iter->op){
						case mid_code::plus:
						case mid_code::sub:
							if(iter->s2.is_val && iter->s2.val==0)
								brk= true;
							break;
						case mid_code::mul:
							if(iter->s2.is_val && iter->s2.val==1)
								brk= true;
							break;
						}
						if(brk) break;
					}
					if(!iter->s2.is_val && iter->s0.name==iter->s2.name){
						bool brk= false;
						switch(iter->op){
						case mid_code::plus:
						case mid_code::sub:
							if(iter->s1.is_val && iter->s1.val==0)
								brk= true;
							break;
						case mid_code::mul:
							if(iter->s1.is_val && iter->s1.val==1)
								brk= true;
							break;
						}
						if(brk) break;
					}
					required_flag= true;
					iter->required= true;
					required_symbol.erase(iter->s0.name);
					if(!iter->s1.is_val) required_symbol.insert(iter->s1.name);
					if(iter->op!=mid_code::none){
						if(!iter->s2.is_val) required_symbol.insert(iter->s2.name);
					}
					break;
				case mid_code::read:
					required_flag= true;
					iter->required= true;
					required_symbol.erase(iter->s0.name);
					break;
				case mid_code::write:
					required_flag= true;
					iter->required= true;
					if(!iter->s0.is_val) required_symbol.insert(iter->s0.name);
					break;
				}
		}
		return required_flag;
}

void flip_unrequired_opt_iter(
	vector<mid_code>& dest, vector<mid_code>& src, 
	vector<mid_block>& bloc_dest, vector<mid_block>& bloc_src
	);
mid_block _flip_unrequired(mid_block& bloc){
	mid_block ret;
	ret.type= bloc.type;
	ret.required= bloc.required;
	ret.test_symbol= bloc.test_symbol;
	flip_unrequired_opt_iter(ret.code_true, bloc.code_true, ret.block_item, bloc.block_item);
	flip_unrequired_opt_iter(ret.code_false, bloc.code_false, ret.block_item, bloc.block_item);
	return ret;
}

void flip_unrequired_opt_iter(
	vector<mid_code>& dest, vector<mid_code>& src, 
	vector<mid_block>& bloc_dest, vector<mid_block>& bloc_src
	){
	for(auto iter=src.begin(); iter!=src.end(); ++iter){
		if(iter->required){
			if(iter->type!= mid_code::bloc){
				dest.push_back(*iter);
			}else{
				mid_code temp_code= *iter;
				temp_code.s0.val= bloc_dest.size();
				dest.push_back(temp_code);
				mid_block& temp_bloc_ref= _flip_unrequired(bloc_src[iter->s0.val]);
				bloc_dest.push_back(temp_bloc_ref);
			}
		}
	}
}


mid_block flip_unrequired_opt(mid_block& bloc){
	mid_block& temp_bloc= required_opt(bloc, set<string>());
	return _flip_unrequired(temp_bloc);
}