#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <stack>
#include <vector>
#include <fstream>
#include "LSA.h"


using namespace std;

// const string keyword[] = {"break", "case", "char", "continue", "do", "default", "double",
// 							  "else", "float", "for", "if", "int", "include", "long", "main", "return", "switch", "typedef", "void", "unsigned", "while", "iostream"};
// const char operators[] = {'+', '-', '*', '/', '=', '!', '%', '~', '&', '|', '^'};					 //运算符
// const char delimiter[] = {'(', ')', '{', '}', ';', '<', '>',  '$', '#',','};							//界符

TokenType op_index[256];
TokenType de_index[256];

string result[300][2] = {};
FILE* src_file = NULL;
ifstream lexical_rules;  //词法规则

TokenType identify(string str) //将关键字转化为枚举类型
{
	for (int i = 0; i < keywords_num; ++i)
		if (str == keyword[i])
			return (TokenType)i;
}
void init_special_char()
{
	for (int i = 0; i < delimiters_num; ++i)
		de_index[delimiter[i]] = (TokenType)(i + keywords_num);
	for (int i = 0; i < operators_num; ++i)
		op_index[operators[i]] = (TokenType)(i + keywords_num + delimiters_num);
}

char state[300];
int state_len;
char start;  //起始字符
char final[300];
int final_len;
bool is_final_array[450];

NFA_set nfa_tab[300][300];

NFA_set new_set[300];
int num_new_set = 0;
int dfa_tab[450][450];

bool is_int(char a)
{
	if (a >= '0' && a <= '9')
		return true;
	return false;
}

bool is_identifier(char a)
{
	if (a >= 'a' && a <= 'z')
		return true;
	if (a >= 'A' && a <= 'Z')
		return true;
	if (a == '_')
		return true;
	return false;
}

bool is_keyword(string str)
{
	for (int j = 0; j < keywords_num; ++j)
		if (keyword[j].length() == str.length())
			if (keyword[j] == str)
				return true;
	return false;
}

bool is_operator(char a)
{
	for (int i = 0; i < operators_num; ++i)
		if (operators[i] == a)
			return true;
	return false;
}

bool is_delimiter(char a)
{
	for (int i = 0; i < delimiters_num; ++i)
		if (delimiter[i] == a)
			return true;
	return false;
}

bool in_state(char a)
{
	for (int i = 0; i < state_len; ++i)
		if (a == state[i])
			return true;
	return false;
}

bool in_final(char a)
{
	for (int i = 0; i < final_len; ++i)
		if (a == final[i])
			return true;
	return false;
}

bool in_set(char a, NFA_set temp)
{
	for (int i = 0; i < temp.pos; ++i)
		if (a == temp.set[i])
			return true;
	return false;
}

void createNFA()
{
	int num;
	bool flag = true; //用于得到首个符号
	char ch, arrow;		  //读文法左部; 识别箭头
	string str;      //读文法右部的
	lexical_rules.open("lexical_rules.txt");
	lexical_rules >> num;
	while (num--)
	{
		lexical_rules >> ch >> arrow >> arrow >> str;
		if (flag)
		{
			start = ch;
			flag = false;
		}
		if (!in_state(ch))
			state[state_len++] = ch;
		if (!in_final(str[0]))
			final[final_len++] = str[0];
		if (str.length() > 1)
			nfa_tab[ch][str[0]].set[nfa_tab[ch][str[0]].pos++] = str[1];
		else
			nfa_tab[ch][str[0]].set[nfa_tab[ch][str[0]].pos++] = 'Y'; //终态
	}
}

void showNFA()
{
	for (int i = 0; i < 100; ++i)
		for (int j = 0; j < 100; ++j)
			for (int k = 0; k < 100; ++k)
				if (nfa_tab[i][j].set[k] != '#')
					cout << char(i) << " " << char(j) << " " << k << " " << nfa_tab[i][j].set[k] << endl;
}

int is_in(NFA_set temp) //和已有的newset有没有重复的，有就返回重复的编号
{
	bool flag[300];
	bool flag1;
	for (int i = 0; i < temp.pos; ++i)
	{
		flag[i] = false;
	}
	for (int i = 0; i < num_new_set; ++i)
	{
		for (int k = 0; k < temp.pos; ++k)
		{
			for (int j = 0; j < new_set[i].pos; ++j)
			{
				if (temp.set[k] == new_set[i].set[j])
				{
					flag[k] = true;
				}
			}
		}
		flag1 = true;
		for (int m = 0; m < temp.pos; ++m)
		{
			if (flag[m] == false)
			{
				flag1 = false;
				break;
			}
		}
		if (flag1 == true)
			return i;
		for (int m = 0; m < temp.pos; ++m)
			flag[m] = false;
	}
	return -1;
}

void eps_closure(NFA_set& temp) //得到一个完整的子集
{
	for (int i = 0; i < temp.pos; ++i)
		for (int j = 0; j < nfa_tab[temp.set[i]]['@'].pos; ++j)
			if (!in_set(nfa_tab[temp.set[i]]['@'].set[j], temp))
				temp.set[temp.pos++] = nfa_tab[temp.set[i]]['@'].set[j];
}

bool is_final_state(NFA_set temp) //判断是否是终态
{
	for (int i = 0; i < temp.pos; ++i)
	{
		if (temp.set[i] == 'Y')
			return true;
	}
	return false;
}

void NFA_to_DFA()
{
	num_new_set = 0;
	NFA_set work_set;
	NFA_set worked_set;
	work_set.set[work_set.pos++] = start;
	worked_set.pos = 0;
	stack<NFA_set> s;
	eps_closure(work_set);
	s.push(work_set);
	new_set[num_new_set++] = work_set;
	memset(dfa_tab, '-1', sizeof(dfa_tab));
	memset(is_final_array, 0, sizeof(is_final_array));
	if (is_final_state(work_set))
		is_final_array[num_new_set - 1] = true;
	while (!s.empty())
	{
		work_set = s.top();
		s.pop();
		for (int i = 0; i < final_len; ++i) {
			for (int j = 0; j < work_set.pos; ++j) {
				for (int k = 0; k < nfa_tab[work_set.set[j]][final[i]].pos; ++k) {
					if (nfa_tab[work_set.set[j]][final[i]].set[k] != '#' && nfa_tab[work_set.set[j]][final[i]].set[k] != 'Y' && !in_set(nfa_tab[work_set.set[j]][final[i]].set[k], worked_set))
						worked_set.set[worked_set.pos++] = nfa_tab[work_set.set[j]][final[i]].set[k];
					if (nfa_tab[work_set.set[j]][final[i]].set[k] == 'Y' && !in_set(nfa_tab[work_set.set[j]][final[i]].set[k], worked_set))
						worked_set.set[worked_set.pos++] = 'Y'; //用Y表示终态
				}
			}
			eps_closure(worked_set);
			if (worked_set.pos > 0 && is_in(worked_set) == -1)
			{
				dfa_tab[num_new_set - 1][final[i]] = num_new_set;
				s.push(worked_set);
				new_set[num_new_set++] = worked_set;
				if (is_final_state(worked_set))
					is_final_array[num_new_set - 1] = true;
			}
			if (worked_set.pos > 0 && is_in(worked_set) > -1 && final[i] != '@')
				dfa_tab[is_in(work_set)][final[i]] = is_in(worked_set);
			worked_set.pos = 0;
		}
	}
}

bool DFA(string str)
{
	char now_state = 0;
	for (int i = 0; i < str.length(); ++i)
	{
		now_state = dfa_tab[now_state][str[i]];
		if (now_state == -1)
			return false;
	}
	if (is_final_array[now_state] == true)
		return true;
	return false;
}

void scan()
{
	string word_str = "";
	int position = 0;
	char ch;
	int i, j;
	bool flag = 0; // 用于辨别浮点数或整数
	ch = fgetc(src_file);
	while (1)
	{
		word_str = "";
		if (is_int(ch)) //多一个ch
		{
			word_str += ch;
			ch = fgetc(src_file);
			while (is_identifier(ch) || is_int(ch) || ch == '.') {
				if (ch == '.')
					flag = 1;
				word_str += ch;
				ch = fgetc(src_file);
			}

			if (DFA(word_str)) {
				if (word_str.substr(0, 2) == "0x" || word_str.substr(0, 2) == "0X")
					if (flag) {
						result[position][0] = "REAL16";
						result[position++][1] = word_str.substr(2);
						// cout << "REAL16\t" << word_str << endl;
					}
					else {
						result[position][0] = "INT16";
						result[position++][1] = word_str.substr(2);
						// cout << "INT16\t" << word_str << endl;
					}
				else if (word_str[0] == '0' && word_str.length() > 1)
					if (flag) {
						result[position][0] = "REAL8";
						result[position++][1] = word_str.substr(1);
						// cout << "REAL8\t" << word_str << endl;
					}
					else {
						result[position][0] = "INT8";
						result[position++][1] = word_str.substr(1);
						// cout << "INT8\t" << word_str << endl;
					}
				else
					if (flag) {
						result[position][0] = "REAL10";
						result[position++][1] = word_str;
						// cout << "REAL10\t" << word_str << endl;
					}
					else {
						result[position][0] = "INT10";
						result[position++][1] = word_str;
						// cout << "INT10\t" << word_str << endl;
					}
			}
			else {
				result[position][0] = "ERROR, CONST";
				result[position++][1] = word_str;
				// cout << "出错，不是常量\t" << word_str << endl;
			}
			flag = 0;
			continue;
		}

		if (is_identifier(ch))
		{
			word_str += ch;
			ch = fgetc(src_file);
			while (is_identifier(ch) || is_int(ch) || ch == '.') {
				word_str += ch;
				ch = fgetc(src_file);
			}
			if (is_keyword(word_str)) {
				result[position][0] = word_str;
				result[position++][1] = '_';
				// cout << word_str << "\t_" << endl;
			}
			else {
				if (DFA(word_str)) {
					result[position][0] = "IDN";
					result[position++][1] = word_str;
					// cout << "IDN\t" << word_str << endl;
				}
				else {
					result[position][0] = "ERROR,IDN";
					result[position++][1] = word_str;
					// cout << "出错，不是标识符\t" << word_str << endl;
				}

			}
			continue;
		}

		if (is_delimiter(ch))
		{
			result[position][0] = ch;
			result[position++][1] = '_';
			// cout << ch << "\t_" << endl;
			ch = fgetc(src_file);
			continue;
		}

		if (is_operator(ch))
		{
			result[position][0] = ch;
			result[position++][1] = '_';
			// cout << ch << "\t_" << endl;
			ch = fgetc(src_file);
			continue;
		}
		if (ch == ' ' || ch == '\n' || ch == '\t')
		{
			ch = fgetc(src_file);
			continue;
		}
		if (ch == EOF)
			break;
		result[position][0] = "ERROR";
		result[position++][1] = ch;
		// cout << "无法识别标识符\t" << ch << endl;
		ch = fgetc(src_file);
	}
}

void init()
{
	final_len = 0;
	state_len = 0;
	init_special_char();
	for (int i = 0; i < 100; ++i)
	{
		// is_final[i]=false;
		for (int j = 0; j < 100; ++j)
			for (int k = 0; k < 100; ++k)
				nfa_tab[i][j].set[k] = '#';
	}
}
void show()
{
	for (int i = 0; i < num_new_set; ++i)
	{
		cout << i << "的set" << endl;
		for (int j = 0; j < new_set[i].pos; ++j)
		{

			cout << new_set[i].set[j] << " ";
		}
		cout << endl;
	}
}

void show_result() {
	int iter = 0;
	//cout << result[iter][0] << "->->->" << result[iter][1] << endl;
	//iter += 1;
	//cout << result[iter][0] << "->->->" << result[iter][1] << endl;
	while (true) {
		cout << result[iter++][0] << "\t" << result[iter][1] << endl;
		// iter++;
		if (result[iter][0] == "") {
			break;
		}
	}
}

int LSA_func()
{
	init();
	final_len, state_len = 0;
	createNFA();
	NFA_to_DFA();
	src_file = fopen("src_file.txt", "r+");
	scan();
	show_result();
	fclose(src_file);
	return 0;
}
