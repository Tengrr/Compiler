#include <iostream>
#include<vector>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "LSA.h"

using namespace std;
using std::vector;

TokenType result_format(string);
string Parse_F();
string Parse_t(string f1);
string Parse_T();
string Parse_e(string t1);
string Parse_E();
string Parse_c(string e1);
string Parse_C();
void Parse_s();
void Parse_S();
void Parse_L();
void Parse_P();
void Parse_ALL();
string Fresh_t();
string Fresh_L();
void up(string str);

class Token {
public:
	TokenType type;
	string attribute;

	Token(TokenType type, string attribute) {
		this->type = type;
		this->attribute = attribute;
	}
};


//全局变量

vector<Token> tokens;

int index = -1;

int t = 0;

int jmp = -1;

string intermediateCode = "";

extern TokenType op_index[256];
extern TokenType de_index[256];
extern string result[300][2];

//全局变量

int main() {

	/*
	while (a3 + 15) > 0xa do
		if x2 = 07 then
			while y < z do
				y = x * y / z;
	c = b * c + d;
	*/
	LSA_func();

	int iter = 0;
	while (true) {
		string id_enum = result[iter][0];
		string id_data = result[iter][1];
		tokens.push_back(Token(result_format(id_enum), id_data));
		// cout << iter << ".	" << result_format(id_enum) << endl;
		iter++;
		if (result[iter][0] == "")
			break;
	}


	//在输入的末尾自动添加$
	tokens.push_back(Token(DOLLAR, "_"));


	Parse_ALL();

	cout << "Successfully compiled" << endl;
	cout << intermediateCode << endl;

	return 0;
}

TokenType result_format(string str)
{
	char ch = (str.c_str())[0];
	if (str == "IDN")
		return IDN;
	else if (str == "INT8")
		return INT8;
	else if (str == "INT10")
		return INT8;
	else if (str == "INT16")
		return INT16;
	else if (is_keyword(str))
		return identify(str);
	else if (is_operator(ch))
		return op_index[ch];
	else if (is_delimiter(ch))
		return de_index[ch];
	else
		cout << "Error!" << endl;
}


string Parse_F() {
	Token currentToken = tokens.at(++index);
	string f;
	switch (currentToken.type)
	{
	case LPAREN:
		f = Parse_E();
		currentToken = tokens.at(++index);
		if (currentToken.type == RPAREN) {
			return f;
			break;
		}
		cout << "error in parsing F,expect')'" << endl;
		exit(1);
		break;

	case IDN:
		return currentToken.attribute;
		break;

	case INT8:
		return currentToken.attribute;
		break;

	case INT10:
		return currentToken.attribute;
		break;

	case INT16:
		return currentToken.attribute;
		break;

	default:
		cout << "error in parsing F, expect a number or a variable" << endl;
		exit(1);
		break;
	}
}

string Parse_t(string f1) {
	Token currentToken = tokens.at(++index);
	string f2, f3;
	switch (currentToken.type)
	{
	case MULTI:
		f2 = Parse_F();
		f3 = Fresh_t();
		up(f3 + "=" + f1 + "*" + f2 + "\n");
		f3 = Parse_t(f3);
		return f3;
		break;

	case DIVIDE:
		f2 = Parse_F();
		f3 = Fresh_t();
		up(f3 + "=" + f1 + "/" + f2 + "\n");
		f3 = Parse_t(f3);
		return f3;
		break;

	default:
		--index;
		return f1;
		break;
	}
}

string Parse_T() {
	string f1 = Parse_F();
	return Parse_t(f1);
}

string Parse_e(string t1) {
	Token currentToken = tokens.at(++index);
	string t2, t3;
	switch (currentToken.type)
	{
	case PLUS:
		t2 = Parse_T();
		t3 = Fresh_t();
		up(t3 + "=" + t1 + "+" + t2 + "\n");
		t3 = Parse_e(t3);
		return t3;
		break;

	case MINUS:
		t2 = Parse_T();
		t3 = Fresh_t();
		up(t3 + "=" + t1 + "-" + t2 + "\n");
		t3 = Parse_e(t3);
		return t3;
		break;

	default:
		--index;
		return t1;
		break;
	}
}

string Parse_E() {
	string t1 = Parse_T();
	return Parse_e(t1);
}

string Parse_c(string e1) {
	Token currentToken = tokens.at(++index);
	string e2;
	switch (currentToken.type)
	{
	case GREATER:
		e2 = Parse_E();
		return e1 + ">" + e2;
		break;

	case LESS:
		e2 = Parse_E();
		return e1 + "<" + e2;
		break;

	case EQUALS:
		e2 = Parse_E();
		return e1 + "=" + e2;
		break;

	default:
		cout << "error in parsing c, expect comparison" << endl;
		exit(1);
		break;
	}
}

string Parse_C() {
	string e1 = Parse_E();
	return Parse_c(e1);
}

void Parse_s() {
	Token currentToken = tokens.at(++index);
	switch (currentToken.type)
	{
	case ELSE:
		Parse_S();
		break;

	default:
		--index;
		break;
	}
}

void Parse_S() {
	Token currentToken = tokens.at(++index);
	string id;
	string e;
	string c;
	string L0, L1, L2;
	switch (currentToken.type)
	{
	case IDN:
		id = currentToken.attribute;
		currentToken = tokens.at(++index);
		if (currentToken.type == EQUALS) {
			string e = Parse_E();
			up(id + "=" + e + "\n");
			break;
		}
		cout << "error in parsing S, expect '='" << endl;
		exit(1);
		break;

	case IF:
		c = Parse_C();
		L1 = Fresh_L();
		L2 = Fresh_L();
		up("if " + c + " goto " + L1 + "\n");
		up("goto " + L2 + "\n");
		currentToken = tokens.at(++index);
		if (currentToken.type == THEN) {
			up(L1 + ": ");
			Parse_S();
			up(L2 + ": ");
			Parse_s();
			break;
		}
		cout << "error in parsing S, expect 'THEN'" << endl;
		exit(1);
		break;

	case WHILE:
		L0 = Fresh_L();
		up(L0 + ": ");
		c = Parse_C();
		L1 = Fresh_L();
		L2 = Fresh_L();
		up("if " + c + " goto " + L1 + "\n");
		up("goto " + L2 + "\n");
		currentToken = tokens.at(++index);
		if (currentToken.type == DO) {
			up(L1 + ": ");
			Parse_S();
			up("goto " + L0 + "\n");
			up(L2 + ": ");
			break;
		}
		cout << "error in parsing S, expect 'DO'" << endl;
		exit(1);
		break;

	case LBRACE:
		Parse_P();
		currentToken = tokens.at(++index);
		if (currentToken.type == RBRACE) {
			break;
		}
		cout << "error in parsing S, expect '}'" << endl;
		exit(1);
		break;

	default:
		cout << "error in parsing S, expect a sentence" << endl;
		exit(1);
		break;
	}
}

void Parse_L() {

	Parse_S();			//将当前S所在的分支传递给Parse_S函数
	Token currentToken = tokens.at(++index);
	if (currentToken.type == SEMICOLON) {
		return;
	}
	else {
		cout << "error in parsing L, expect ';'" << endl;
		exit(1);
	}
}

void Parse_P() {
	Parse_L();
	Token currentToken = tokens.at(++index);
	while (currentToken.type != DOLLAR && currentToken.type != RBRACE) {
		--index;
		Parse_L();
		currentToken = tokens.at(++index);
	}
	--index;
}

string Fresh_t() {
	string out;
	stringstream in;
	in << ++t;
	in >> out;
	return "t" + out;
}

string Fresh_L() {
	string out;
	stringstream in;
	in << ++jmp;
	in >> out;
	return "L" + out;
}

void up(string str) {
	intermediateCode += str;
}

void Parse_ALL() {
	Parse_P();
	Token currentToken = tokens.at(++index);
	if (currentToken.type != DOLLAR) {
		cout << "error in Parsing ALL, expect '$'" << endl;
		exit(1);
	}
}






