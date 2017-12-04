#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <assert.h>
#include <vector>
#include "Symbol.h"
#include "Rule.h"

using namespace std;

regex r_letter("[a-zA-Z]+");
regex r_digit("[0-9]+");

regex r_blank("\s");

regex r_op("(@|=|/=|>|:=|<|>=|<=|\\+|\\*|/|-)"); // operators
regex r_sep("(\\(|\\)|,|;|:|%%|\\{|\\}|\\[|\\])"); // separator
regex r_key("(boolean|else|false|fi|floating|if|integer|read|return|true|while|write)"); // keyword

#define PGM 0
#define SLIST 1
#define STMT 2
#define STMT_ 3 // can lead to "E" | "kwdinput"
#define S_OUT 4
#define ELIST 5 // can lead to "E Elist" | "eps"
#define ELIST2 6
#define E 7
#define E_ 8
#define T 9
#define T_ 10
#define F 11 // can lead to "Fatom | paren1 E paren2"
#define FATOM 12 // can lead to "id | int | string| float"
#define OPADD 13 // can lead to "plus | minus"
#define OPMUL 14 // can lead to "aster | slash | caret"
#define EPS 1000


FILE * file;
char nextChar;
char lexeme[100];
char lexemeLength = 0;
int lexCode;
int lineCount = 1;
/*Punctuation DFA Functions*/
void stateOnePuncDFA(char c);
void stateTwoPuncDFA(char c);

/*KeyWord DFA Functions*/
void stateOneKeyWordDFA(char c);
void stateTwoKeyWordDFA(char c);
void stateThreeKeyWordDFA(char c);
void stateFourKeyWordDFA(char c);
void stateFiveKeyWordDFA(char c);
void stateSixKeyWordDFA(char c);

/*RegEx DFA Functions*/
void stateOneRegex(char c);
void stateTwoRegex(char c);
void stateThreeRegex(char c);
void terminateStateRegex();
int getRegexCode(string lex);

/* General Functions */
void addChar(char c);
void terminateState();
int getCode(string s);

/* Regex Codes */
#define COMMENT 1

/* Key Words */
#define KWDMAIN 11      // "main"
#define KWDFCN 12       // "fcn"
#define KWDCLASS 13     // "class"
#define KWDFLOAT 15     // "float"
#define KWDINT 16       // "int"
#define KWDSTRING 17    // "string"
#define KWDIF 18        // "if"
#define KWDELSEIF 19    // "elseif"
#define KWDELSE 20      // "else"
#define KWDWHILE 21     // "while"
#define KWDINPUT 22     // "input"
#define KWDNEW 24       // "new"
#define KWDRETURN 25    // "return"

/* Paired Delimiters */
#define ANGLE1 31       // '<'
#define ANGLE2 32       // '>'
#define BRACKET1 35     // '['
#define BRACKET2 36     // ']'

/* Other Punctuation */
#define COLON 43        // ':'
#define DOT 44          // '.'

/* Multi-char Operators */
#define OPARROW 51      // '->'
#define OPEQ 52         // '=='
#define OPNE 53         // '!='
#define OPLE 54         // '<='
#define OPGE 55         // '>='
#define OPSHL 56        // '<<'
#define OPSHR 57        // '>>'

/* Miscellaeous */
#define ERROR 99 // Unknown token

vector<Symbol> input;
vector<Symbol> reverseInput(vector<Symbol> input) {
	vector<Symbol> revIn;

	while (input.size()) {

		Symbol tmp = input.back();
		input.pop_back();
		revIn.push_back(tmp);
	}
	return revIn;
}


typedef int fsm_state;
typedef char fsm_input;

bool recognize(string str);
bool is_final_state(fsm_state state);
fsm_state get_start_state(void);
fsm_state move(fsm_state state, fsm_input input);
void lexer();

////////////////////////////////////////////////////////////////////////////////


class Parser {
public:
	Rule rules[24];
	int mtx[17][20] = {
		{ pgm1 },
		{ 0,0,EPS,0,slist2,0,0,slist2 },
		{ 0,0,0,0,stmt3,0,0,stmt4 },
		{ 0,0,0,0,stmt_5,0,stmt_6,0,0,stmt_5,0,stmt_5,stmt_5,stmt_5 },
		{ 0,0,0,0,0,0,0,s_out7 },
		{ 0,0,0,0,elist8,0,0,0,0,elist8,EPS,elist8,elist8,elist8 },
		{ 0,0,0,0,0,0,0,0,elist2_9,0,EPS },
		{ 0,0,0,0,e10,0,0,0,0,e10,0,e10,e10,e10 },
		{ 0,0,0,EPS,0,0,0,0,EPS,0,EPS,0,0,0,e_11,e_11 },
		{ 0,0,0,0,t12,0,0,0,0,t12,0,t12,t12,t12 },
		{ 0,0,0,EPS,0,0,0,0,EPS,0,EPS,0,0,0,EPS,EPS,t_13,t_13,t_13 },
		{ 0,0,0,0,f14,0,0,0,0,f15,0,f14,f14,f14 },
		{ 0,0,0,0,fatom16,0,0,0,0,0,0,fatom17,fatom18,fatom19 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,opadd20,opadd21 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,opmul22,opmul23,opmul24 }
	};
	enum Lhs {
		pgm1, slist2, stmt3, stmt4, stmt_5, stmt_6,
		s_out7, elist8, elist2_9, e10, e_11, t12,
		t_13, f14, f15, fatom16, fatom17, fatom18,
		fatom19, opadd20, opadd21, opmul22, opmul23, opmul24
	};

	vector<Symbol> input;
	vector<Symbol> stack;

	Parser(vector<Symbol> input_) {
		input = input_;
		//reverse(input.begin(),input.end());
	}
	~Parser() {

	}
	void buildRules() {


		/* RULES BUILT BY HAND TO REDUCE ERRORS */
		/* RULE FORMAT: Rule: Symbol(LHS) => {Symbol(RHS), Symbol(RHS), Symbol(RHS)}*/

		/* RULE 1 */

		Symbol lhs1 = Symbol(0, "", "Pgm", false);
		Symbol rhs1[4] = {
			Symbol(KWDPROG, "prog", "", true),
			Symbol(BRACE1, "{", "", true),
			Symbol(SLIST, "", "Slist", false),
			Symbol(BRACE2, "}", "", true)
		};
		rules[0] = Rule(PGM, lhs1, rhs1, 4);


		/* RULE 2 */
		Symbol lhs2 = Symbol(1, "", "Slist", false);
		Symbol rhs2[3] = {
			Symbol(STMT,"","Stmt", false),
			Symbol(SEMI, ";", "", true),
			Symbol(SLIST,"","Slist", false)
		};

		rules[1] = Rule(SLIST, lhs2, rhs2, 3);

		/* RULE 3 */

		Symbol lhs3 = Symbol(2, "", "Stmt", false);
		Symbol rhs3[3] = {
			Symbol(ID, "id","", true),
			Symbol(EQUAL, "=","", true),
			Symbol(STMT_, "","Stmt_", false) };
		rules[2] = Rule(STMT, lhs3, rhs3, 3);

		/* RULE 4 */

		Symbol lhs4 = Symbol(3, "", "Stmt", false);
		Symbol rhs4[1] = {
			Symbol(S_OUT, "","S_Out", false)
		};
		rules[3] = Rule(STMT, lhs4, rhs4, 1);

		/* RULE 5 */

		Symbol lhs5 = Symbol(4, "", "Stmt_", false);
		Symbol rhs5[1] = {
			Symbol(E, "","E", false)
		};
		rules[4] = Rule(STMT_, lhs5, rhs5, 1);

		/* RULE 6 */

		Symbol lhs6 = Symbol(5, "", "Stmt_", false);
		Symbol rhs6[1] = {
			Symbol(KWDINPUT, "kwdinput","", true)
		};
		rules[5] = Rule(STMT_, lhs6, rhs6, 1);


		/* RULE 7 */

		Symbol lhs7 = Symbol(6, "", "S_Out", false);
		Symbol rhs7[4] = {
			Symbol(KWDPRINT, "kwdprint", "", true),
			Symbol(PAREN1, "(", "", true),
			Symbol(ELIST, "", "Elist", false),
			Symbol(PAREN2, ")", "", true),
		};
		rules[6] = Rule(STMT_, lhs7, rhs7, 4);

		/* RULE 8 */

		Symbol lhs8 = Symbol(7, "", "Elist", false);
		Symbol rhs8[2] = {
			Symbol(E, "", "E", false),
			Symbol(ELIST2, "", "Elist2", false)
		};
		rules[7] = Rule(ELIST, lhs8, rhs8, 2);

		/* RULE 9 */

		Symbol lhs9 = Symbol(8, "", "Elist2", false);
		Symbol rhs9[2] = {
			Symbol(COMMA, ",", "", true),
			Symbol(ELIST, "", "Elist", false)
		};
		rules[8] = Rule(ELIST2, lhs9, rhs9, 2);

		/* RULE 10 */

		Symbol lhs10 = Symbol(9, "", "E", false);
		Symbol rhs10[2] = {
			Symbol(T, "", "T", false),
			Symbol(E_, "", "E_", false)
		};
		rules[9] = Rule(E, lhs10, rhs10, 2);

		/* RULE 11 */

		Symbol lhs11 = Symbol(10, "", "E_", false);
		Symbol rhs11[3] = {
			Symbol(OPADD, "", "Opadd", false),
			Symbol(T, "", "T", false),
			Symbol(E_, "", "E_", false),
		};
		rules[10] = Rule(E_, lhs11, rhs11, 3);

		/* RULE 12 */

		Symbol lhs12 = Symbol(11, "", "T", false);
		Symbol rhs12[2] = {
			Symbol(F, "", "F", false),
			Symbol(T_, "", "T_", false)
		};
		rules[11] = Rule(T, lhs12, rhs12, 2);

		/* RULE 13 */

		Symbol lhs13 = Symbol(12, "", "T_", false);
		Symbol rhs13[3] = {
			Symbol(OPMUL, "", "Opmul", false),
			Symbol(F, "", "F", false),
			Symbol(T_, "", "T_", false),
		};
		rules[12] = Rule(T_, lhs13, rhs13, 3);

		/* RULE 14 */

		Symbol lhs14 = Symbol(13, "", "F", false);
		Symbol rhs14[1] = {
			Symbol(FATOM, "","Fatom", false)
		};
		rules[13] = Rule(STMT_, lhs14, rhs14, 1);


		/* RULE 15 */

		Symbol lhs15 = Symbol(14, "", "F", false);
		Symbol rhs15[3] = {
			Symbol(PAREN1, "(", "", true),
			Symbol(E, "", "E", false),
			Symbol(PAREN2, ")", "", true)
		};
		rules[14] = Rule(T_, lhs15, rhs15, 3);

		/* RULE 16 */

		Symbol lhs16 = Symbol(15, "", "Fatom", false);
		Symbol rhs16[1] = {
			Symbol(ID, "id","", true)
		};
		rules[15] = Rule(FATOM, lhs16, rhs16, 1);

		/* RULE 17 */

		Symbol lhs17 = Symbol(16, "", "Fatom", false);
		Symbol rhs17[1] = {
			Symbol(INT, "int","", true)
		};
		rules[16] = Rule(FATOM, lhs17, rhs17, 1);

		/* RULE 18 */

		Symbol lhs18 = Symbol(17, "", "Fatom", false);
		Symbol rhs18[1] = {
			Symbol(FLOAT, "float","", true)
		};
		rules[17] = Rule(FATOM, lhs18, rhs18, 1);

		/* RULE 19 */

		Symbol lhs19 = Symbol(18, "", "Fatom", false);
		Symbol rhs19[1] = {
			Symbol(STRING, "string","", true)
		};
		rules[18] = Rule(FATOM, lhs19, rhs19, 1);

		/* RULE 20 */

		Symbol lhs20 = Symbol(29, "", "Opadd", false);
		Symbol rhs20[1] = {
			Symbol(PLUS, "plus","", true)
		};
		rules[19] = Rule(OPADD, lhs20, rhs20, 1);

		/* RULE 21 */

		Symbol lhs21 = Symbol(20, "", "Opadd", false);
		Symbol rhs21[1] = {
			Symbol(MINUS, "minus","", true)
		};
		rules[20] = Rule(OPADD, lhs21, rhs21, 1);

		/* RULE 22 */

		Symbol lhs22 = Symbol(21, "", "Opmul", false);
		Symbol rhs22[1] = {
			Symbol(ASTER, "aster","", true)
		};
		rules[21] = Rule(OPMUL, lhs22, rhs22, 1);

		/* RULE 23 */

		Symbol lhs23 = Symbol(22, "", "Opmul", false);
		Symbol rhs23[1] = {
			Symbol(SLASH, "slash","", true)
		};
		rules[22] = Rule(OPMUL, lhs23, rhs23, 1);

		/* RULE 24 */

		Symbol lhs24 = Symbol(23, "", "Opmul", false);
		Symbol rhs24[1] = {
			Symbol(CARET, "caret","", true)
		};
		rules[23] = Rule(OPMUL, lhs24, rhs24, 1);

	}

	void printStack(const vector<Symbol>& stack) {
		vector<Symbol>::iterator it;
		for (auto& i : stack) {
			if (i.terminal) cout << i.lexeme << " ";
			else cout << i.identifier;
		}
		cout << endl;
	}
	void printHelper(vector<Symbol> input, vector<Symbol> stack) {
		printStack(input);
		printStack(stack);
	}
	void pushRevRHS(Symbol s[], int size) {
		for (int i = size - 1; i >= 0; i--) {
			stack.push_back(s[i]);
		}
	}
	void start() {
		buildRules();
		Symbol eof_ = Symbol(eof, "$", "", true);
		//push eof
		stack.push_back(eof_);
		stack.push_back(rules[0].lhs); // push "Pgm"
		printHelper(input, stack);

		while (stack.back().code != eof) {
			if (mtx[stack.back().code][input.back().mtxCode] == EPS) {
				stack.pop_back();
			}

			else if (!stack.back().terminal) {
				Symbol tmp = stack.back();
				stack.pop_back();
				pushRevRHS(rules[mtx[tmp.code][input.back().mtxCode]].rhs, rules[mtx[tmp.code][input.back().mtxCode]].size);
			}
			else if (stack.back().code == input.back().code) {
				stack.pop_back();
				input.pop_back();
			}
			else if (mtx[stack.back().code][input.back().mtxCode] == 0) {
				if (input.back().terminal) cout << "Syntax error at: " << input.back().identifier << " Error code: " << input.back().code << endl;
				else cout << "Syntax error at: " << input.back().lexeme << " Error code: " << input.back().code << endl;

				break;
			}
			printHelper(input, stack);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////

int main()
{
	lexer();
	//parser();
	ifstream in;
	in.open("test.txt");

	if (!in) 
	{ 
		cerr << "Error opening file."; 
	}
	do {
		in.get(nextChar);
		if (ispunct(nextChar) && nextChar != '"') {
			stateOnePuncDFA(nextChar);
		}
		else if (isalpha(nextChar)) {
			stateOneKeyWordDFA(nextChar);

		}
		else {
			stateOneRegex(nextChar);
		}

	} while (!eof);

	//cout << "(Tok: id= " << eof << " line = " << lineCount << " str= \"\")" << endl;
	input.push_back(Symbol(eof, "$", "", true));

	cout << "Starting parse machine...\n";

	vector<Symbol> reversedInput = reverseInput(input);

	Parser * parser = new Parser(reversedInput);

	parser->start();


	return getchar();
}


void lexer() {

	fstream inFile;
	inFile.open("test.txt");

	if (!inFile)
		cerr << "File did not open or does not exist.";

	cout << "Tokens\t";
	cout << "\tLexeme\n";
	cout << "----------------------\n";

	//intifinding keywords, operator, separators.
	string keyword;
	string operators;
	string seperators;
	string fsm_string;

	//Using count to double check correct amount of separators, operator, keywords, integers, floats, Indentifiers
	int count = 0;

	//Using c to get character by character from file
	char character;
	bool yesDec = false;

	while (inFile.get(character))
	{

		//converting to string for regex_match
		string temp_OneChar_String(1, character);

		if (regex_match(temp_OneChar_String, r_letter)) {
			//appending chars to string to make word
			keyword.append(temp_OneChar_String);
		}
		else if (regex_match(temp_OneChar_String, r_letter) == false) {
			//if it isn't a letter then end of word
			//checking to see if keyword is a keyword
			if (regex_match(keyword, r_key)) {
				count++;
				cout << "Keyword\t\t" << keyword << '\n';
				fsm_string.clear();
			}
			keyword.clear();

		}

		//================================================================
		//Fsm for Identifier
		if (regex_match(temp_OneChar_String, r_letter) == true && fsm_string.empty() == true)
		{
			fsm_string.append(temp_OneChar_String);
			continue;
		}
		else if ((regex_match(temp_OneChar_String, r_letter) || temp_OneChar_String == "#") && fsm_string.empty() == false)
		{
			fsm_string.append(temp_OneChar_String);
			continue;
		}
		else if (regex_match(temp_OneChar_String, r_letter) == false && temp_OneChar_String != "#" && regex_match(fsm_string, r_digit) == false && regex_match(temp_OneChar_String, r_digit) == false && yesDec == false)
		{

			if (regex_match(fsm_string, r_key) == false)
			{
				if (recognize(fsm_string) == true)			/*This inserts the string into the fsm to determine if its valid*/
				{
					count++;
					cout << "Identifier\t" << fsm_string << endl;
					fsm_string.clear();
				}
			}
		}

		//================================================================
		//Fsm for integers and reals
		if (regex_match(temp_OneChar_String, r_digit) == true && fsm_string.empty() == true)
		{
			fsm_string.clear();
			fsm_string.append(temp_OneChar_String);
		}
		else if (regex_match(temp_OneChar_String, r_digit) == true && fsm_string.empty() == false)
		{
			fsm_string.append(temp_OneChar_String);
			continue;

		}
		else if (temp_OneChar_String == ".")
		{
			yesDec = true;
			fsm_string.append(temp_OneChar_String);
			continue;
		}
		else if (regex_match(temp_OneChar_String, r_digit) == false && fsm_string.empty() == false)
		{
			bool decNum = false;
			for (std::string::iterator it = fsm_string.begin(); it != fsm_string.end(); ++it)
			{
				if (*it == '.')
				{
					decNum = true;
				}
			}

			if (decNum == true)
			{
				if (recognize(fsm_string) == true)
				{
					count++;
					cout << "Float\t\t" << fsm_string << endl;
					yesDec = false;
					fsm_string.clear();
				}
			}
			else if (decNum == false)
			{
				if (recognize(fsm_string) == true)
				{
					count++;

					cout << "Integer\t\t" << fsm_string << endl;
					fsm_string.clear();
				}
			}
		}

		//=======================================================
		//check colon to see if it's apart of separator or operators
		if (character == ':'  && seperators.empty() == true) {
			seperators.append(temp_OneChar_String);
			continue;

		}
		else if (character == '=' && seperators.empty() == false) {
			seperators.append(temp_OneChar_String);
			operators = seperators;
			count++;
			cout << "operators\t" << operators << '\n';
			operators.clear();
			seperators.clear();
			continue;
		}
		else if (seperators.length() == 1 && (character != '=' && character != '%') && regex_match(seperators, r_op) == false)
		{	//outputting single colon
			count++;
			cout << "Separator\t" << seperators << endl;
			seperators.clear();
			operators.clear();
			continue;
		}
		//======================================================
		//prints double char operatorss
		if ((character == '<' || character == '>' || character == '/') && operators.empty() == true)
		{
			operators.append(temp_OneChar_String);
			continue;
		}
		else if (character == '=' && operators.empty() == false)
		{
			count++;
			operators.append(temp_OneChar_String);
			cout << "operators\t" << operators << endl;
			operators.clear();
			continue;
		}
		//=======================================================
		//print out single operatorss
		if ((character != ':') && (regex_match(operators, r_op)) && (operators.length() == 1))
		{
			count++;
			cout << "operators\t" << operators << endl;
			operators.clear();
			continue;
		}
		else if (regex_match(temp_OneChar_String, r_op))
		{
			count++;
			cout << "operators\t" << temp_OneChar_String << " " << operators << endl;
			operators.clear();
			continue;
		}

		//=======================================================
		//prints out all other separators except :
		if (character != ':' && regex_match(temp_OneChar_String, r_sep))
		{
			count++;
			cout << "Separator\t" << temp_OneChar_String << endl;

		}

		//=========================================================
		//prints double %%
		if (character == '%' && seperators.empty() == true)
		{
			seperators.append(temp_OneChar_String);
			continue;
		}
		else if (character == '%' && seperators.empty() == false)
		{
			count++;
			seperators.append(temp_OneChar_String);
			cout << "Separator\t" << seperators << endl;
			seperators.clear();
			continue;
		}
	}

	cout << "\nToken Count: " << count << endl;

	inFile.close();
};

//================================================================
bool is_final_state(fsm_state state)
{
	if (state == 2 || state == 3 || state == 4 || state == 5 || state == 7 || state == 8)
	{
		return true;
	}
	else {
		return false;
	}
};


//================================================================
fsm_state get_start_state(void)
{
	return 1;
};

//================================================================

fsm_state move(fsm_state state, fsm_input input)
{
	string temp_string;
	temp_string = input;

	// our alphabet includes only 'a' and 'b'
	if (temp_string != "#" && temp_string != "." && regex_match(temp_string, r_letter) == true && regex_match(temp_string, r_digit) == true)
	{
		assert(1);
	}

	switch (state)
	{
	case 1:
		if (regex_match(temp_string, r_letter))
		{
			return 2;
		}
		else if (regex_match(temp_string, r_digit))
		{
			return 3;
		}
		break;
	case 2:
		if (regex_match(temp_string, r_letter))
		{
			return 4;
		}
		else if (temp_string == "#")
		{
			return 5;
		}
		break;
	case 3:
		if (regex_match(temp_string, r_digit))
		{
			return 3;
		}
		else if (temp_string == ".")
		{
			return 6;
		}
		break;
	case 4:
		if (regex_match(temp_string, r_letter))
		{
			return 4;
		}
		else if (temp_string == "#")
		{
			return 5;
		}
		break;
	case 5:
		if (regex_match(temp_string, r_letter))
		{
			return 7;
		}
		break;
	case 6:
		if (regex_match(temp_string, r_digit))
		{
			return 8;
		}
		break;
	case 7:
		if (regex_match(temp_string, r_letter))
		{
			return 4;
		}
		else if (temp_string == "#")
		{
			return 5;
		}
		break;
	case 8:
		if (regex_match(temp_string, r_digit))
		{
			return 8;
		}
		break;
	default:
		assert(1);
	}
};



bool recognize(string str)
{
	if (str == "")
	{
		return false;
	}
	fsm_state state = get_start_state();
	string::const_iterator i = str.begin();
	fsm_input input = *i;
	while (i != str.end())
	{
		state = move(state, *i);
		i++;
	}

	if (is_final_state(state))
	{
		return true;
	}
	else
	{
		return false;
	}
};

void stateOnePuncDFA(char c) {
	//printf("entering state 1 PUNC with: %c \n", c);
	if (c == '!' || c == '<' || c == '>' || c == '=') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == '=' || nextChar == '<' || nextChar == '>') {
			stateTwoPuncDFA(nextChar);
		}
		else {
			terminateState();
			stateOnePuncDFA(nextChar);
		}
	}
	//Non-problematic characters
	if (c == ',' || c == ';' || c == '*' || c == '^' || c == ':' || c == '.' || c == '{' || c == '}' || c == '[' || c == ']' || c == '(' || c == ')') {
		addChar(c);
		terminateState();
	}
	//Regex-problematic characters
	else if (c == '/') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == '/') {
			stateTwoRegex(nextChar);
		}
	}
	else if (c == '+' || c == '-') {
		addChar(c);
		nextChar = getc(file);
		if (isdigit(nextChar)) {
			stateTwoRegex(nextChar);
		}
		else {
			terminateState();
		}
	}
	else if (c == '\n') {
		lineCount++;
	}
}
void stateTwoPuncDFA(char c) {
	//printf("entering state 2 PUNC with: %c \n", c);
	if (c == '=' || c == '<' || c == '>') {
		addChar(c);
		terminateState();
	}
}

/****************************************/
/******** KEYWORD DFA FUNCTIONS *********/
/****************************************/
void stateOneKeyWordDFA(char c) {
	//printf("entering state 1 KEYWD with: %c \n", c);

	//Non-Problematic characters
	if (c == 'c' || c == 'e' || c == 'm' || c == 'n' || c == 'p' || c == 'r' || c == 's' || c == 'w') {
		addChar(c);
		stateTwoKeyWordDFA(nextChar = getc(file));
	}
	//Problem characters
	else if (c == 'f' || c == 'i') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == 'a' || nextChar == 'c' || nextChar == 'e' || nextChar == 'f' || nextChar == 'l' || nextChar == 'n') {
			stateTwoKeyWordDFA(nextChar);
		}
	}
	//If not a keyword, save as identifier
	else if (c == '_' || isalpha(nextChar)) {
		addChar(c);
		nextChar = getc(file);
		while ((nextChar == '_') || isalpha(nextChar)) {
			addChar((nextChar));
			nextChar = getc(file);
		}
		stateTwoRegex(nextChar);
	}
	else if (c == '\n') {
		lineCount++;
	}
}
void stateTwoKeyWordDFA(char c) {
	//printf("entering state 2 KEYWD with: %c \n", c);

	//Non-Problematic characters
	if (c == 'a' || c == 'c' || c == 'h' || c == 'r' || c == 't') {
		addChar(c);
		stateThreeKeyWordDFA(nextChar = getc(file));
	}
	//Problematic characters
	else if (c == 'e' || c == 'l' || c == 'n') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == 'a' || nextChar == 'n' || nextChar == 'o' || nextChar == 'p' || nextChar == 's' || nextChar == 't' || nextChar == 'w') {
			stateThreeKeyWordDFA(nextChar);
		}
		else {
			lexemeLength = 0;
		}
	}
	//Terminating character
	else if (c == 'f') {
		addChar(c);
		terminateState();
	}
	//If not a keyword, save as identifier
	else if (c == '_' || isalpha(nextChar)) {
		addChar(c);
		nextChar = getc(file);
		while ((nextChar == '_') || isalpha(nextChar)) {
			addChar((nextChar));
			nextChar = getc(file);
		}
		stateTwoRegex(nextChar);
	}

}
void stateThreeKeyWordDFA(char c) {
	//printf("entering state 3 KEYWD with: %c \n", c);

	//Special case for 'input' and 'int'
	if (c == 't') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == 'u') {
			stateFourKeyWordDFA(nextChar);
		}
		else {
			terminateState();
			stateOneKeyWordDFA(nextChar);
		}
	}
	//Non-Problematic characters
	else if (c == 'a' || c == 'o' || c == 'p' || c == 'r' || c == 's' || c == 'u') {
		addChar(c);
		stateFourKeyWordDFA(nextChar = getc(file));
	}
	//Problematic character
	else if (c == 'i') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == 'e' || nextChar == 'l' || nextChar == 'n' || nextChar == 'u') {
			stateFourKeyWordDFA(nextChar);
		}
		else {
			lexemeLength = 0;
		}
	}
	//Terminating characters
	else if (c == 'n' || c == 'w') {
		addChar(c);
		terminateState();
	}
}
void stateFourKeyWordDFA(char c) {
	//printf("entering state 4 KEYWD with: %c \n", c);

	//Special case for 'else'/'elseif' & 'main'/'print'
	if (c == 'e' || c == 'n') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == 'i' || nextChar == 't') {
			stateFiveKeyWordDFA(nextChar);
		}
		else {
			terminateState();
			stateOneKeyWordDFA(nextChar);
		}
	}
	//Non-problematic characters
	else if (c == 'a' || c == 'i' || c == 'l' || c == 's') {
		addChar(c);
		stateFiveKeyWordDFA(nextChar = getc(file));
	}
	//Problematic character
	else if (c == 'u') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == 'r' || nextChar == 't') {
			stateFiveKeyWordDFA(nextChar);
		}
	}
	//Terminating characters
	else if (c == 'g' || c == 'r') {
		addChar(c);
		terminateState();
	}
}
void stateFiveKeyWordDFA(char c) {
	//printf("entering state 5 KEYWD with: %c \n", c);
	//Non-problematic characters
	if (c == 'i' || c == 'n' || c == 'r') {
		addChar(c);
		stateSixKeyWordDFA(nextChar = getc(file));

	}
	//Terminating characters
	else if (c == 'e' || c == 's' || c == 't') {
		addChar(c);
		terminateState();
	}
}
void stateSixKeyWordDFA(char c) {
	//printf("entering state 6 KEYWD with: %c \n", c);
	//Terminating characters
	if (c == 'f' || c == 'g' || c == 'n') {
		addChar(c);
		terminateState();
	}
}

/****************************************/
/****** REGEX    DFA    FUNCTIONS *******/
/****************************************/

void stateOneRegex(char c) {
	//printf("entering state 1 RegEx with: %c \n", c);
	if (c == '/') {
		addChar(c);
		nextChar = getc(file);
		if (nextChar == '/') {
			stateTwoRegex(nextChar);
		}
	}
	else if (c == '"') {
		addChar(c);
		nextChar = getc(file);
		while (nextChar != '"' || nextChar == '\xff') {
			addChar(nextChar);
			nextChar = getc(file);
		}
		addChar(nextChar);
		stateTwoRegex(nextChar);
	}
	else if (isdigit(c) || c == '+' || c == '-') {
		addChar(c);
		nextChar = getc(file);
		if (isdigit(nextChar) || nextChar == '.') {
			stateTwoRegex(nextChar);
		}
		else {
			terminateStateRegex();
		}
	}
	else if (c == '_' || isalpha(nextChar)) {
		addChar(c);
		nextChar = getc(file);
		while ((nextChar == '_') || isalpha(nextChar)) {
			addChar((nextChar));
			nextChar = getc(file);
		}
		stateTwoRegex(nextChar);
	}
	else if (c == '\n') {
		lineCount++;
	}
}
void stateTwoRegex(char c) {
	//printf("entering state 2 RegEx with: %c \n", c);
	if (c == '/') {
		addChar(c);
		nextChar = getc(file);
		while (nextChar != '\n' || nextChar == '\xff') {
			addChar(nextChar);
			nextChar = getc(file);
		}
		stateThreeRegex(nextChar);
	}
	else if (isdigit(c) || c == '.') {
		addChar(c);
		nextChar = getc(file);
		while (isdigit(nextChar)) {
			addChar(nextChar);
			nextChar = getc(file);
		}
		terminateStateRegex();
	}
	else if (c == '\n' || c == '"' || ' ') {
		terminateStateRegex();
	}
	stateOnePuncDFA(nextChar);
}
void stateThreeRegex(char c) {
	//printf("entering state 3 RegEx with: %c \n", c);
	if (c == '\n') {
		terminateStateRegex();
	}
	else if (c == '.') {
		addChar(c);
		nextChar = getc(file);
		while (isdigit(nextChar)) {
			addChar(nextChar);
			nextChar = getc(file);
		}
		terminateStateRegex();
	}
	else if (!isdigit(c)) {
		terminateStateRegex();
	}
}
void terminateStateRegex() {
	string lex = lexeme;

	lexCode = getRegexCode(lex);
	if (lexCode == INT) {
		//cout << "(Tok: id= " << lexCode << " line = " << lineCount << "" << " str= \"" << lex << "\" int= "<< lex << ")" << endl;
		input.push_back(Symbol(lexCode, "int", "", true));
	}
	else if (lexCode == FLOAT) {
		//cout << "(Tok: id= " << lexCode << " line = " << lineCount << " str= \"" << lex << "\" float= "<< lex << ")" << endl;
		input.push_back(Symbol(lexCode, "float", "", true));
	}
	else if (lexCode == STRING) {
		//cout << "(Tok: id= " << lexCode << " line = " << lineCount << " str= " << lex << ")" << endl;
		input.push_back(Symbol(lexCode, "string", "", true));
	}
	else if (lexCode != COMMENT) {
		//cout << "(Tok: id= " << lexCode << " line = " << lineCount << " str= \"" << lex << "\")" << endl;
		input.push_back(Symbol(lexCode, "id", "", true));
	}

	memset(&lexeme, 0, 100);
	lexemeLength = 0;
}
int getRegexCode(string lex) {
	regex id("['_'*|[a-zA-Z0-9]*");
	regex integer("(-|\\+)?[0-9]+");
	regex flt("((-|\\+)?[0-9]+\\.)?[0-9]+");
	regex string("\".*\"");

	if (regex_match(lex, integer)) {
		return INT;
	}
	else if (regex_match(lex, flt)) {
		return FLOAT;

	}
	else if (regex_match(lex, id)) {
		return ID;
	}
	else if (regex_match(lex, string)) {
		return STRING;
	}
	else {
		return COMMENT;
	}
}

/****************************************/
/********** GENERAL FUNCTIONS ***********/
/****************************************/

void addChar(char c) {
	lexeme[lexemeLength++] = c;
}
void terminateState() {
	string lex = lexeme;

	lexCode = getCode(lex);
	if (lexCode != ERROR) {
		//cout << "(Tok: id= " << lexCode << " line = " << lineCount << " str= \"" << lex << "\")" << endl;
		input.push_back(Symbol(lexCode, lex, "", true));
	}
	memset(&lexeme, 0, 100);
	lexemeLength = 0;
}
int getCode(string lex) {

	if (lex == "prog") {
		return KWDPROG;
	}
	else if (lex == "main") {
		return KWDMAIN;
	}
	else if (lex == "fcn") {
		return KWDFCN;
	}
	else if (lex == "class") {
		return KWDCLASS;
	}
	else if (lex == "float") {
		return KWDFLOAT;
	}
	else if (lex == "int") {
		return KWDINT;
	}
	else if (lex == "string") {
		return KWDSTRING;
	}
	else if (lex == "if") {
		return KWDIF;
	}
	else if (lex == "elseif") {
		return KWDELSEIF;
	}
	else if (lex == "else") {
		return KWDELSE;
	}
	else if (lex == "while") {
		return KWDWHILE;
	}
	else if (lex == "input") {
		return KWDINPUT;
	}
	else if (lex == "print") {
		return KWDPRINT;
	}
	else if (lex == "new") {
		return KWDNEW;
	}
	else if (lex == "return") {
		return KWDRETURN;
	}
	else if (lex == ",") {
		return COMMA;
	}
	else if (lex == ";") {
		return SEMI;
	}
	else if (lex == "<") {
		return ANGLE1;
	}
	else if (lex == ">") {
		return ANGLE2;
	}
	else if (lex == "{") {
		return BRACE1;
	}
	else if (lex == "}") {
		return BRACE2;
	}
	else if (lex == "[") {
		return BRACKET1;
	}
	else if (lex == "]") {
		return BRACKET2;
	}
	else if (lex == "(") {
		return PAREN1;
	}
	else if (lex == ")") {
		return PAREN2;
	}
	else if (lex == "*") {
		return ASTER;
	}
	else if (lex == "^") {
		return CARET;
	}
	else if (lex == ":") {
		return COLON;
	}
	else if (lex == ".") {
		return DOT;
	}
	else if (lex == "=") {
		return EQUAL;
	}
	else if (lex == "-") {
		return MINUS;
	}
	else if (lex == "+") {
		return PLUS;
	}
	else if (lex == "/") {
		return SLASH;
	}
	else if (lex == "->") {
		return OPARROW;
	}
	else if (lex == "==") {
		return OPEQ;
	}
	else if (lex == "!=") {
		return OPNE;
	}
	else if (lex == "<=") {
		return OPLE;
	}
	else if (lex == ">=") {
		return OPGE;
	}
	else if (lex == "<<") {
		return OPSHL;
	}
	else if (lex == ">>") {
		return OPSHR;
	}
	else {
		return ERROR;
	}
}
