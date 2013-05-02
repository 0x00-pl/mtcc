#pragma once
#include<string>
#include<sstream>
#include<vector>
#include<map>
using namespace std;
#include"parser.h"

class symbol{
public:
	symbol(){
		bad= true;
	}
	symbol(string _name){
		bad= false;
		is_val= false;
		name= _name;
	}
	symbol(int _value){
		bad= false;
		is_val= true;
		val= _value;
	}
	bool bad;
	bool is_val;
	int val;
	string name;
	bool operator!=(const symbol& o)const{
		if(o.is_val!=is_val) return false;
		if(is_val){
			return o.val!=val;
		}else{
			return o.name!=name;
		}
	}
	string to_string(){
		if(bad) return "bad-symbol";
		if(is_val){
			stringstream ss;
			ss<<val;
			return ss.str();
		}
		return name;
	}
};

string gen_name(){
	static unsigned long long id=0;
	return string("__compare_inner_")+to_string(id++);
}
class mid_code{
public:
	mid_code(){required=false;}
	enum{
		set,//s0 := s1 op s2
		bloc,//block[s0]
		read,//read s0
		write,//write s0
	}type;
	enum {
		lt,
		eq,
		plus,
		sub,
		mul,
		none
	}op;
	bool required;
	symbol s0;
	symbol s1;
	symbol s2;
	bool operator!=(const mid_code& o)const{
		if(o.type!=type || o.op!=op) return true;
		if(o.s0!=s0) return true;
		if(o.s1!=s1) return true;
		if(op!=none){
			if(o.s2!=s2) return true;
		}
		return false;
	}
	string to_string(){
		string ret;
		switch(type){
		case set:
			ret+= s0.name;
			ret+= ":= ";
			if(s1.is_val){
				stringstream ss;
				ss<<s1.val;
				ret+= ss.str();
			}else{
				ret+= s1.name;
			}
			if(op!=none){
				switch(op){
				case lt: ret+= " < "; break;
				case eq: ret+= " = "; break;
				case plus: ret+= " + "; break;
				case sub: ret+= " - "; break;
				case mul: ret+= " * "; break;
				default: ret+= "???";
				}
				if(s2.is_val){
					stringstream ss;
					ss<<s2.val;
					ret+= ss.str();
				}else{
					ret+= s2.name;
				}
			}
			break;
		case read:
			ret+= "read ";
			ret+= s0.name;
			break;
		case write:
			ret+= "write ";	
			if(s0.is_val){
				stringstream ss;
				ss<<s0.val;
				ret+= ss.str();
			}else{
				ret+= s0.name;
			}
			break;
		default:
			ret+= "???";
		}
		return ret;
	}
};
class mid_block{
public:
	mid_block(){required= false;}
	enum{
		seqence,
		if_stmt,
		repeat_stmt
	}type;
	bool required;
	symbol test_symbol;
	vector<mid_code> code_true;
	vector<mid_code> code_false;
	vector<mid_block> block_item;
};

//
//gencode(type){
//	if(type==gen){
//		gen....
//	}
//	items[i].gencode(type);
//}

vector<mid_code> genoptexp(code_tree_node& bloc, symbol dest){
	vector<mid_code> ret;
	mid_code temp_code;
	if(bloc.leaf==true){
		temp_code.type= mid_code::set;
		temp_code.op= mid_code::none;
		temp_code.s0= dest;
		if(bloc.tkn.type==token::number)
			temp_code.s1= symbol(atoi(bloc.tkn.text.c_str()));
		else
			temp_code.s1= symbol(bloc.tkn.text);
		temp_code.s2.bad= true;
		ret.push_back(temp_code);
		return ret;
	}
	if(bloc.items.size()==1){
		return genoptexp(bloc.items[0], dest);
	}else if(bloc.items[0].leaf==true && bloc.items[0].tkn.text=="("){
		return genoptexp(bloc.items[1], dest);
	}else{
		symbol templ(gen_name());
		vector<mid_code>& tempcdl= genoptexp(bloc.items[0], templ);
		ret.insert(ret.end(), tempcdl.begin(), tempcdl.end());
		for(size_t i=2; i<bloc.items.size(); i+=2){
		symbol tempr(gen_name());
		symbol tempd;
		if(i+2<bloc.items.size()){
			tempd= symbol(gen_name());
		}else{
			tempd= dest;
		}
		vector<mid_code>& tempcdr= genoptexp(bloc.items[i], tempr);
		ret.insert(ret.end(), tempcdr.begin(), tempcdr.end());
		
		temp_code.type= mid_code::set;
		temp_code.s0= tempd;
		temp_code.s1= templ;
		temp_code.s2= tempr;
		if(bloc.items[i-1].tkn.text=="<") temp_code.op= mid_code::lt;
		if(bloc.items[i-1].tkn.text=="=") temp_code.op= mid_code::eq;
		if(bloc.items[i-1].tkn.text=="+") temp_code.op= mid_code::plus;
		if(bloc.items[i-1].tkn.text=="-") temp_code.op= mid_code::sub;
		if(bloc.items[i-1].tkn.text=="*") temp_code.op= mid_code::mul;
		ret.push_back(temp_code);
		templ=tempd;
		}
	}
	return ret;
}

//{code,needed-env} genop(bloc)
mid_block genopt(code_tree_node& bloc, int block_item_base=0){
	mid_block ret;
	mid_code temp_code;
	switch(bloc.type){
	case code_tree_node::stament_sequence:
		ret.type= mid_block::seqence;
		for(auto iter=bloc.items.begin(); iter!=bloc.items.end(); ++iter){
			switch(iter->type){
			case code_tree_node::if_stament:
				{
					symbol temp_for_test(gen_name());
					vector<mid_code>& temp_code_vec= 
						genoptexp(iter->items[0], temp_for_test);
					ret.code_true.insert(
						ret.code_true.end(),
						temp_code_vec.begin(), 
						temp_code_vec.end()
						);

					temp_code.type= mid_code::bloc;
					temp_code.s0= symbol(ret.block_item.size()+block_item_base);
					temp_code.s1.bad= true;
					temp_code.s2.bad= true;
					ret.code_true.push_back(temp_code);

					ret.block_item.push_back(genopt(*iter));
					ret.block_item.back().test_symbol= temp_for_test;
				}
				break;
			case code_tree_node::repeat_stament:
				temp_code.type= mid_code::bloc;
				temp_code.s0= symbol(ret.block_item.size()+block_item_base);
				temp_code.s1.bad= true;
				temp_code.s2.bad= true;
				ret.code_true.push_back(temp_code);
				ret.block_item.push_back(genopt(*iter));
				break;
			case code_tree_node::assign_stament:
				{
					vector<mid_code>& temp_code_vec= 
						genoptexp(iter->items[1], symbol(iter->items[0].tkn.text));

					ret.code_true.insert(
						ret.code_true.end(),
						temp_code_vec.begin(), 
						temp_code_vec.end()
						);
				}
				break;
			case code_tree_node::read_stament:
				temp_code.type= mid_code::read;
				temp_code.s0= symbol(iter->items[0].tkn.text);
				temp_code.s1.bad= true;
				temp_code.s2.bad= true;
				ret.code_true.push_back(temp_code);
				break;
			case code_tree_node::write_stament:
				temp_code.type= mid_code::write;
				if(iter->items[0].leaf){
					if(iter->items[0].type==code_tree_node::number){
						temp_code.s0= symbol(atoi(iter->items[0].tkn.text.c_str()));
					}else{
						temp_code.s0= symbol(iter->items[0].tkn.text);
					}
					temp_code.s1.bad= true;
					temp_code.s2.bad= true;
					ret.code_true.push_back(temp_code);
				}else{
					symbol exp_to_write(gen_name());
					vector<mid_code>& temp_code_vec= 
						genoptexp(iter->items[0], exp_to_write);

					ret.code_true.insert(
						ret.code_true.end(),
						temp_code_vec.begin(), 
						temp_code_vec.end()
						);
					
					temp_code.s0= exp_to_write;
					temp_code.s1.bad= true;
					temp_code.s2.bad= true;
					ret.code_true.push_back(temp_code);
				}
				break;
			}
		}
		break;
	case code_tree_node::if_stament:
		{
			ret.type= mid_block::if_stmt;
			mid_block& temp_bloc_then= genopt(bloc.items[1]);
			ret.code_true= move(temp_bloc_then.code_true);
			ret.block_item= move(temp_bloc_then.block_item);

			if(bloc.items.size()>2){
				mid_block& temp_bloc_else= genopt(bloc.items[2], ret.block_item.size());
				ret.code_true= move(temp_bloc_else.code_true);
				ret.block_item.insert(
					ret.block_item.end(),
					temp_bloc_else.block_item.begin(),
					temp_bloc_else.block_item.end()
					);
			}
			// fix ret.test_symbol in 
			//  case code_tree_node::stament_sequence:
			//   case code_tree_node::if_stament:
		}
		break;
	case code_tree_node::repeat_stament:
		{
			ret.type= mid_block::repeat_stmt;
			mid_block& temp_bloc= genopt(bloc.items[0]);
			ret.code_true= move(temp_bloc.code_true);

			symbol exp_for_test(gen_name());
			vector<mid_code>& temp_code_vec= 
				genoptexp(bloc.items[1], exp_for_test);
			ret.code_true.insert(
				ret.code_true.end(),
				temp_code_vec.begin(), 
				temp_code_vec.end()
				);
			ret.test_symbol= exp_for_test;
		}
		break;
	}
	return ret;
}