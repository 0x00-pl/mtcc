#pragma once
#include<vector>
#include<string>
#include<iostream>
using namespace std;


struct token{
	enum token_type{
		symbol,
		number,
		keyword,
		op
	};
	token_type type;
	int line;
	string text;
};
token token_text(string text){
	token tok;
	tok.text= text;
	return tok;
}

vector<token> get_tokens(char* p){
	vector<token> ret;
	token cur_token;
	int line= 0;
	while(*p!='\0'){
		switch(*p){
		case '\n':
			++p;
			++line;
			break;
		case '{':
			while(*p!='}'){ 
				if(*p=='\n') ++line;
				++p;
			}
			++p;
			break;
		case '+':case '-':case '*':case '/':case '=':case '<':case '(':case ')': case ';':
		case ':':
			cur_token.type= token::op;
			if(*p==':'&&p[1]=='='){
				++p;
				cur_token.text=":=";
			}else{
				cur_token.text=*p;
			}
			++p;
			cur_token.line= line;
			ret.push_back(cur_token);
			break;

		case 'i':case 't':case 'e':case 'r':case 'u':case 'w':
#define __temp_decl_keyword(name) if(p[strlen(name)]==' '&&string(p,p+strlen(name))==name){\
				cur_token.type= token::keyword;\
				cur_token.text= name;\
				p+= strlen(name)+1;\
				cur_token.line= line;\
				ret.push_back(cur_token);\
				break;\
			}

			__temp_decl_keyword("if");
			__temp_decl_keyword("then");
			__temp_decl_keyword("else");
			__temp_decl_keyword("end");
			__temp_decl_keyword("repeat");
			__temp_decl_keyword("until");
			__temp_decl_keyword("read");
			__temp_decl_keyword("write");
#undef __temp_decl_keyword
		default:
			if(*p>='0'&&*p<='9'){
				char* q= p;
				while(*q>='0'&&*q<='9') ++q;
				cur_token.type= token::number;
				cur_token.text= string(p,q);
				p=q;
				cur_token.line= line;
				ret.push_back(cur_token);
				break;
			}
			if((*p>='a'&&*p<='z') || (*p>='A'&&*p<='Z') || *p=='_'){
				char* q= p;
				while((*q>='0'&&*q<='9') || (*q>='a'&&*q<='z') || (*q>='A'&&*q<='Z') || *q=='_') ++q;
				cur_token.type= token::symbol;
				cur_token.text= string(p,q);
				p=q;
				cur_token.line= line;
				ret.push_back(cur_token);
				break;
			}
			//un managed char
			++p;
		}
	}
	return ret;
}

typedef vector<token>::iterator token_ptr;

bool match_token(token_ptr& ptkn, token_ptr& end, token t){
	if(ptkn==end) return false;
	if(ptkn->text==t.text){
		++ptkn;
		return true;
	}else{
		return false;
	}
}

struct code_tree_node{
	enum node_type{
		symbol,
		number,
		op,
		factor,
		term,
		simple_exp,
		exp,
		stament_sequence,
		if_stament,
		repeat_stament,
		assign_stament,
		read_stament,
		write_stament
	};
	bool leaf;
	node_type type;
	token tkn;
	vector<code_tree_node> items;
	string to_string(){
		string ret;
		if(leaf){
			ret+= tkn.text;
		}else{
			for(auto iter=items.begin(); iter!=items.end(); ++iter){
				ret+= iter->to_string();
				ret+= " ";
			}
		}
		return ret;
	}
};



code_tree_node match_number(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.type= code_tree_node::number;
	if(p->type==token::number){
		ret.tkn= *p;
		ret.leaf= true;
		++p;
	}else{
		return ret;//error
	}
	return ret;
}
code_tree_node match_symbol(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.type= code_tree_node::symbol;
	if(p->type==token::symbol){
		ret.tkn= *p;
		ret.leaf= true;
		++p;
	}else{
		return ret;//error
	}
	return ret;
}
code_tree_node match_op(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.type= code_tree_node::op;
	if(p->type==token::op){
		ret.tkn= *p;
		ret.leaf= true;
		++p;
	}else{
		return ret;//error
	}
	return ret;
}

code_tree_node match_exp(token_ptr& p, token_ptr& end);
code_tree_node match_simple_exp(token_ptr& p, token_ptr& end);
code_tree_node match_term(token_ptr& p, token_ptr& end);
code_tree_node match_factor(token_ptr& p, token_ptr& end);


//factor -> ( exp ) | number | symbol
code_tree_node match_factor(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf=false;
	ret.type= code_tree_node::factor;
	if(p==end) return ret;//error
	if(p->text=="("){
		match_token(p,end, token_text("("));
		ret.items.push_back(match_exp(p,end));
		match_token(p,end, token_text(")"));
	}else{
		if(p->type==token::number){
			return match_number(p,end);
		}
		if(p->type==token::symbol){
			return match_symbol(p,end);
		}
	}
	return ret;
}

//trem -> factor { * factor }
code_tree_node match_term(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::term;
	ret.items.push_back(match_factor(p,end));
	while(p!=end && p->text=="*"){
		ret.items.push_back(match_op(p,end));
		ret.items.push_back(match_factor(p,end));
	}
	if(ret.items.size()==1)
		return ret.items[0];
	else
		return ret;
}

//simple-exp -> term { + term }
code_tree_node match_simple_exp(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::simple_exp;
	ret.items.push_back(match_term(p,end));
	while(p!=end && (p->text=="+"||p->text=="-")){
		ret.items.push_back(match_op(p,end));
		ret.items.push_back(match_term(p,end));
	}
	if(ret.items.size()==1)
		return ret.items[0];
	else
		return ret;
}
//exp -> simple-exp [ < simple-exp ]
code_tree_node match_exp(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::exp;
	ret.items.push_back(match_simple_exp(p,end));
	if(p!=end && (p->text=="<"||p->text=="=")){
		ret.items.push_back(match_op(p,end));
		ret.items.push_back(match_simple_exp(p,end));
	}
	if(ret.items.size()==1)
		return ret.items[0];
	else
		return ret;
}
//
code_tree_node match_stmt_sequence(token_ptr& p, token_ptr& end);
code_tree_node match_stament(token_ptr& p, token_ptr& end);
code_tree_node match_if_stmt(token_ptr& p, token_ptr& end);
code_tree_node match_repeat_stmt(token_ptr& p, token_ptr& end);
code_tree_node match_assign_stmt(token_ptr& p, token_ptr& end);
code_tree_node match_read_stmt(token_ptr& p, token_ptr& end);
code_tree_node match_write_stmt(token_ptr& p, token_ptr& end);

//stmt-sequence -> stament { ; stament }
code_tree_node match_stmt_sequence(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::stament_sequence;
	ret.items.push_back(match_stament(p,end));
	while(p!=end && p->text==";"){
		++p;
		ret.items.push_back(match_stament(p,end));
	}
	return ret;
}
code_tree_node match_stament(token_ptr& p, token_ptr& end){
	if(p==end) return code_tree_node();//error
	if(p->text=="if") return match_if_stmt(p,end);
	if(p->text=="repeat") return match_repeat_stmt(p,end);
	if(p->type==token::symbol) return match_assign_stmt(p,end);
	if(p->text=="read") return match_read_stmt(p,end);
	if(p->text=="write") return match_write_stmt(p,end);
	return code_tree_node();//error
}
//if-stmt -> if exp then stmt-seqence [ else stmt-seqence ] end 
code_tree_node match_if_stmt(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::if_stament;
	match_token(p,end, token_text("if"));
	ret.items.push_back(match_exp(p,end));
	if(!match_token(p,end, token_text("then"))) return ret;//error
	ret.items.push_back(match_stmt_sequence(p,end));
	if(match_token(p,end, token_text("else"))){
		ret.items.push_back(match_stmt_sequence(p,end));
	}
	if(!match_token(p,end, token_text("end"))) return ret;//error
	return ret;
}
//repeat-stmt -> repeat stmt-seqence until exp
code_tree_node match_repeat_stmt(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::repeat_stament;
	match_token(p,end, token_text("repeat"));
	ret.items.push_back(match_stmt_sequence(p,end));
	if(!match_token(p,end, token_text("until"))) return ret;//error
	ret.items.push_back(match_exp(p,end));
	return ret;
}
//assign-stmt -> symbol := exp
code_tree_node match_assign_stmt(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::assign_stament;
	ret.items.push_back(match_symbol(p,end));
	match_token(p,end, token_text(":="));
	ret.items.push_back(match_exp(p,end));
	return ret;
}
//read-stmt -> read symbol
code_tree_node match_read_stmt(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::read_stament;
	match_token(p,end, token_text("read"));
	ret.items.push_back(match_symbol(p,end));
	return ret;
}
//write-stmt -> write exp
code_tree_node match_write_stmt(token_ptr& p, token_ptr& end){
	code_tree_node ret;
	ret.leaf= false;
	ret.type= code_tree_node::write_stament;
	match_token(p,end, token_text("write"));
	ret.items.push_back(match_exp(p,end));
	return ret;
}