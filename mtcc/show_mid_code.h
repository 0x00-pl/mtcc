#pragma once
#include"bloc_opt.h"

string show_mid_code_vector(vector<mid_code>& codes, vector<mid_block>& block_item);

string show_mid_code(mid_block& bloc){
	string ret;
	switch(bloc.type){
	case mid_block::seqence:
		ret+= show_mid_code_vector(bloc.code_true, bloc.block_item);
		break;
	case mid_block::if_stmt:
		if(bloc.test_symbol.is_val){
			if(bloc.test_symbol.val!=0){
				ret+= show_mid_code_vector(bloc.code_true, bloc.block_item);
			}else{
				ret+= show_mid_code_vector(bloc.code_false, bloc.block_item);
			}
		}else{
			ret+= "test "; ret+= bloc.test_symbol.to_string()+ "\n";
			ret+= "jz L-else \n";
			ret+= show_mid_code_vector(bloc.code_true, bloc.block_item);
			ret+= "jmp L-end \n";
			ret+= "L-esle: \n";
			ret+= show_mid_code_vector(bloc.code_false, bloc.block_item);
			ret+= "L-end: \n";
		}
		break;
	case mid_block::repeat_stmt:
		ret+= "L-repeat: \n";
		ret+= show_mid_code_vector(bloc.code_true, bloc.block_item);
		ret+= "test "; ret+= bloc.test_symbol.to_string()+ "\n";
		ret+= "jnz L-repeat \n";
		break;
	}
	return ret;
}

string show_mid_code_vector(vector<mid_code>& codes, vector<mid_block>& block_item){
	string ret;
	for(auto iter=codes.begin(); iter!=codes.end(); ++iter){
		switch(iter->type){
		case mid_code::set:
		case mid_code::read:
		case mid_code::write:
			ret+= iter->to_string()+ "\n";
			break;
		case mid_code::bloc:
			ret+= show_mid_code(block_item[iter->s0.val]);
			break;
		}
	}
	return ret;
}