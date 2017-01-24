// Calculator.cpp : main project file.
/*		CHANGELOG

		Version 0.1 Alpha
-Test version implemented;
-Coding redability improved;
-Error handling dramatically improved;
-Exit button q now works properly;
-Remainder operation now supported;

		Version 0.2 Alpha
-Sine and cosine functions implemented;
-Factorial function implemented;
-Variables implemented, including constants like pi and e;
	
	hotfix 0.2.1
-Fixed a bug where exponencials of negative numbers would give a wrong result;
-Fixed a bug where factorials of negative numbers would give a wrong result;

		Version 0.3 Alpha
-Command's support added (\patchnotes and \help available);
-Visual noise at the start of the program dramatically reduced;

	version 0.3.1
-Preparing for beta!
-Newline now works properly as expression delimiter;
-Newline set as default expression delimiter;
-Quit character is now working (again xD);

		Version 0.4 Beta
-Parser now works with strings instead of standard input;
-End of expression assumed to be on Return;
-Lots of bug fixes;
-Code redability further improved;
-Calculator engine is now a stand alone fully functional module;
-History access implemented:
	Use ans, anss and ansss variables to access last three results;
	Use up and down arrow to access expressions you wrote;

	version 0.4.1
-You can now change the precision of the output with \precision n;

	Bugs Known
-overflows are not being dealt with;
-exponentials of negative numbers disabled;


	*/


#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctype.h>
using namespace std;

char end_line = '\n';
const char quit = 'q';
const char print = ' ';
const char prompt = '>';
const char result = '=';
const char sine = 's';
const char cosine = 'c';
const char tangent = 't';
const char var = 'v';
const char com = '\\';

const bool testmode = false;			//Enables feedback during data processing
bool qu = false;						//QUIT: Close the program when is true
string patch = "Patch Notes.txt";
string help = "Help.txt";

void operator >> (string& s, double& n){
	//Copies a number from a string to a double, deleting it in the string
	n = 0;
	while (s.size() > 0 && isdigit(s[0])){	// Stream integer part
		n *= 10;
		n += s[0] - 48;
		s.erase(s.begin(), s.begin() + 1);
	}

	if (s.size() > 0 && s[0] == '.'){		// When point is found, stream decimal part
		s.erase(s.begin(), s.begin() + 1);
		int times = 1;
		while (s.size() > 0 && isdigit(s[0])){
			//cout << s[0] << "->" << (s[0] - 48) << "*" << (10 ^ (-times)) << "\n";
			n += double(s[0] - 48) * pow(10, -times);
			times++;
			s.erase(s.begin(), s.begin() + 1);
		}
	}
	if (testmode)
		cout << "r: " << n << "\n";
}

enum ERRORS{	//Different error types for throw
	No_error,
	Invalid_character,
	Divide_by_zero,
	Brackets_missing,
	Unexpected_error,
	Not_an_integer,
	Quit_exc,
	Variable_not_found,
	Var_succ_aloc,
	Var_protected,
	Unknown_Command
};	

class Variable{
public:
	string name;
	double value;
	bool locked;
	Variable(double v, string s, bool locked = false) :
		name(s), value(v), locked(locked) {};
};

class Symboll{
public:
	Symboll(char key, double value = 0, string name = "")
		:key(key), value(value), name(name) {}
	char key;
	double value;
	string name;
	void show(){
		cout << "\nSym:" << key << " " << value << " " << name;
	}
};

class Symboll_stream{
	bool full;
	string expression;
	Symboll buffer;

	Symboll readexp(string& s) {		//Reads user input, returning it as tokens
		while (s.size() > 0 && s[0] == ' '){
			s.erase(s.begin(), s.begin() + 1);
		}
		char x = ' ';
		double y = 0;
		Symboll mem(';', 0);
		if (s.size() == 0)
			return mem;
		x = s[0];
		s.erase(s.begin(), s.begin() + 1);

		switch (x){
		case '0':case '1':case '2':case '3':case '4':case '5':		//Number found!
		case '6':case '7':case '8':case '9':case'.':{
			s = x + s;
			s >> y;
			mem = Symboll('n', y);
			break;
		}

		case '+':case'-':case'*':case'/':case'(':case')':
		case '=':case'%':case'^': case'!':{
			mem = Symboll(x);
			break;
		}
		case com:{								//Command found!
			string command = "";
			command = s;
			mem = Symboll('\\', 0, command);			//Save it!
			break;
		}

		default:{								//It's a string or invalid character
			string name = "";
			s += " ";
			while (isalpha(x) || x == '_'){
				name += x;
				if (s.size() > 0){
					x = s[0];
					s.erase(s.begin(), s.begin() + 1);
				}
				else
					break;
				if (testmode)cout << "+1";
			}
			string pas = ""; pas += quit;
			if (name == pas)throw Quit_exc;

			if (name == "sin"){					//Sine function found!
				mem = Symboll(sine);
				s = x + s;
				break;
			}
			if (name == "cos"){					//Cosine function found!
				mem = Symboll(cosine);
				s = x + s;
				break;
			}
			if (name == "tan"){
				mem = Symboll(tangent);
				s = x + s;
				break;
			}
			s = x + s;

			if (name != ""){					//Variable name found!
				mem = Symboll(var, 0, name);
				break;
			}
			throw Invalid_character;		//Input doesn't match with anything, let exception handle it.
		}
		}
		return mem;
	}

public:
	Symboll_stream() :
		full(false), buffer(0){};
	Symboll get(){
		if (full){
			full = false;
			return buffer;
		}
		return  readexp(expression);		//Reads a token
	}

	void start(string s){
		expression = s;
		full = false;
	}

	void putback(Symboll t){
		buffer = t;
		full = true;
	}
	/*void stream(char c, char q){			//When is the arg[1] being found the function prepares the program to exit
	if (full && c == buffer.key) {
	full = false;
	return;
	}
	full = false;
	char ch = 0;
	while (true){
	cin.get(ch);
	if (ch == c || ch == q) {
	if (ch == q) qu = true;
	return; }
	else if (testmode) cout << ": " << ch << "\n";
	}
	}*/
};

class Calculate{
	vector<Variable> var_table;
	Symboll_stream sym;

	double compute(){			//Calculates the value of the expression
		if (testmode)cout << "Comp\n";
		double left = term();
		Symboll S = sym.get();
		if (testmode)cout << "c: " << S.key << "\n";

		while (true) {				//While there are terms to sum
			switch (S.key) {
			case '+':{
				left += term();		// Evaluate Term and add
				S = sym.get();
				break; }
			case '-':{
				left -= term();
				S = sym.get();
				break; }
			default:{
				sym.putback(S);
				return left;		// I finally: no more + or - ; return the answer
			}
			}
		}
	}

	double term(){
		if (testmode)cout << "term\n";
		double left = secondary();
		Symboll S = sym.get();		 //Get the next Token from the Token stream
		if (testmode)cout << "t: " << S.key << "\n";
		while (true) {
			switch (S.key) {
			case '*':
				left *= secondary();
				S = sym.get();
				break;
			case '/':{
				double d = secondary();
				if (d == 0){
					throw Divide_by_zero;		//We can't divide by 0
					break;
				}

				left /= d;
				S = sym.get();
				break; }

			case '%':{				//REMAINDER: only with integers
				double d = secondary();
				if (double(int(left)) == left && double(int(d)) == d){
					left = int(left) % int(d);
					S = sym.get();
				}
				else throw Not_an_integer;
				break;
			}
				/*case var:{		//Not working
				sym.putback(S);
				left *= secondary();
				break;
				}*/
			default:{
				sym.putback(S); }		//Put S back into the Token stream
				return left;
			}
		}
	}

	double secondary(){
		if (testmode)cout << "sec\n";
		double left = primary();
		Symboll S = sym.get();		 //Get the next Token from the Token stream
		if (testmode)cout << "s: " << S.key << "\n";

		while (true) {
			switch (S.key) {
			case '^':{
				if (left < 0)throw Unexpected_error;
				left = pow(left, primary());
				S = sym.get();
				break;
			}
			case '!':{									//FATORIAL: we can only compute integer, positive numbers
				if (left < 0)throw Unexpected_error;
				if (left == double(int(left))){
					left = factorial(int(left));
				}
				else throw Not_an_integer;
				S = sym.get();
				break;
			}
			case 'v': case 'n': case '(':{
				sym.putback(S);
				left *= primary();
				S = sym.get();
				if (testmode)cout << "s: " << S.key << "\n";
				break;
			}
			default:{
				sym.putback(S); }		//Put S back into the Token stream
					  return left;
			}
		}

	}

	double primary(){
		if (testmode)cout << "prim\n";
		Symboll S = sym.get();
		if (testmode)cout << "p: " << S.key << "\n";
		switch (S.key) {
		case '(':	{
			double d = compute();
			S = sym.get();
			if (S.key != ')'){					//After a compute() call, there is ) or ; in the buffer. If it's ; here, a bracket is missing
				throw Brackets_missing;
			}
			return d;
		}
		case 'n': // I use 'n' to represent a number
			return S.value; // return the number's value

		case sine:						//SINE function
			return sin(primary());

		case cosine:
			return cos(primary());		//COSINE function

		case tangent:
			return tan(primary());		//TANGENT function

		case var:{						//Variable found. Assign it if followed by =, else return it's value
			string name = S.name;
			S = sym.get();
			if (S.key == '='){
				double x = compute();
				set_var(name, x);
				return x;
			}
			else{
				sym.putback(S);
				return find_var(name);
			}


		}
		case '+':
			return primary();

		case'-':
			return 0 - primary();

		case quit: throw Quit_exc;

		case print:{
			sym.putback(S);		//We need it to move on
			return 0; }

		default:				//If the codes ends here, there is an input error
			sym.putback(S);
			throw Unexpected_error;		//"Unexpected error C2";
		}
	}

	int factorial(int x){
		if (x <= 1)return 1;
		return x*factorial(x - 1);

	}

	double find_var(string s){
		for (unsigned int i = 0; i < var_table.size(); i++){
			if (var_table[i].name == s) 
				return var_table[i].value;
		}
		throw Variable_not_found;
	}

	void set_var(string s, double x){
		for (unsigned int i = 0; i < var_table.size(); i++){
			if (var_table[i].name == s) {
				if (var_table[i].locked) throw Var_protected;
				var_table[i].value = x;
				return;
			}
		}
		var_table.push_back(Variable(x, s));
	}

public:
	Calculate(){ 
		fill_table();
	}

	bool resolve(string s, double& val){
		Symboll S = Symboll(' ');
		sym.start(s);
		S = sym.get();
		if (S.key == quit) throw Quit_exc;
		sym.putback(S);			//we don't wanna lose that! It was read by mistake
		val = compute();

		// Store last three results
		set_var("ansss", find_var("anss"));
		set_var("anss", find_var("ans"));
		set_var("ans", val);
		return true;
	}

	void fill_table(){
		//adds math constants to a variable table, as locked variables 
		//adds variables to save last 3 results
		var_table.push_back(Variable(3.1415926535897932384626433832795, "pi", true));
		var_table.push_back(Variable(2.71828182846, "e", true));
		var_table.push_back(Variable(0, "ans"));
		var_table.push_back(Variable(0, "anss"));
		var_table.push_back(Variable(0, "ansss"));
	}
};

void error_handler(int e);
void command(string s);


int main ()
{		
	cout << "\t\tWelcome to Calculator Pro! (\\help)\n\n";

	double val;
	bool err = false;
	Calculate calculator;
	
	while (true){
		cout << prompt;
		try{
			string s = "";
			while (s.size() == 0){
				getline(cin, s);
				if (!cin){	//Something went wrong!
					if (cin.eof())
						throw ERRORS::Quit_exc;
					cin.clear();
				}
			}
			if (s[0] == '\\'){
				s.erase(s.begin(), s.begin() + 1);
				command(s);
				throw No_error;
			}
			calculator.resolve(s, val); 

			}
		catch (ERRORS e) {					//Error Ocurred! Show and bring the program back to a stable position!
			if (testmode) cout << "Cat ERR\n";
			error_handler(e);
			err = true;
			if (testmode) cout << "Solved\n";
		}
		if (err) 
			err = false;				
		else				//Everything ok, show result
			cout << result << val << "\n\n";
		if (qu)
			break;				
	}
    return 0;
}

void error_handler(int e){			//Show some feedback about the error
	switch (e){
	case (No_error) :
		break;
	case (Invalid_character) : {
		cout << "Invalid character"; 
		break;
	}
	case (Divide_by_zero) : {
		cout << "Division by zero";
		break;
	}
	case (Brackets_missing) : {
		cout << "Brackets missing";
		break;
	}
	case(Not_an_integer) : {
		cout << "Not an integer";
		break; }
	case(Variable_not_found) : {
		cout << "Variable not found";
		break;
	}
	case(Var_protected) : {
		cout << "Variable is protected";
		break;
	}
	case(Unknown_Command) : {
		cout << "Unknown Command";
		break;

	}
	case(Quit_exc) : {
		qu = true;
		break;
	}
	default : {
		cout << "Unexpected error occurred";
		break;

	}
	}
	cout << "\n\n";

}

void display_f(std::string f_name){
	ifstream ist(f_name);
	if (!(ist)){
		cout << f_name << " not found! Make sure there is a file with that name on the program folder.";
		return;
	}
	while (!(ist.eof())){
		string pass;
		getline(ist, pass);
		cout << pass << "\n";
	}
	return;
}

void command(string s){		//Interprets commands and run accordingly
	if (testmode) cout << "Com: " << s << "\n";
	if (s == "patchnotes"){						//Read PATCH NOTES from a file and display them
		display_f(patch);
		return;
	}
	
	if (s == "help"){							//Read HELP from a file and display it
		display_f(help);
		return;
	}

	//std::cout << "hey:" << double(s.find("precision"));
	if (int(s.find("precision")) == 0){
		s.erase(s.begin(), s.begin() + 10);
		double n = 0;
		s >> n;
		std::cout.precision(n + 1);
		std::cout << "Precision set to: " << int(n);
		return;

	}
	if (testmode) cout << "Com ERR\n";
	throw Unknown_Command;
}
