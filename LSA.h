#pragma once
#include <iostream>
# define string_len(a) sizeof(a)/sizeof(a[0]);
using namespace std;

enum TokenType {
	IF, ELSE, WHILE, DO, THEN, LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, LESS, GREATER, DOLLAR, PLUS, MINUS, MULTI, DIVIDE, EQUALS, IDN, INT8, INT10, INT16
};

struct NFA_set
{
	char set[300];
	int pos = 0;
};

const string keyword[] = { "if", "else", "while", "do", "then" };
const char operators[] = { '+', '-', '*', '/', '=' };					 //运算符
const char delimiter[] = { '(', ')', '{', '}', ';', '<', '>', '$' };							//界符
const int keywords_num = string_len(keyword);
const int operators_num = string_len(operators);
const int delimiters_num = string_len(delimiter);




TokenType identify(string); //将关键字转化为枚举类型
void init_special_char();

void init();

bool is_int(char a);
bool is_identifier(char a);
bool is_keyword(string str);
bool is_operator(char a);
bool is_delimiter(char a);
bool in_state(char a);
bool in_final(char a);
bool in_set(char a, NFA_set temp);
bool is_final_state(NFA_set temp); //判断是否是终态
int is_in(NFA_set temp); //和已有的newset有没有重复的，有就返回重复的编号
void eps_closure(NFA_set& temp); //得到一个完整的子集

void createNFA();
void NFA_to_DFA();
bool DFA(string);
void scan();
void show_result();
int LSA_func();
