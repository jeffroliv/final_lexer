
#ifndef Symbol_h
#define Symbol_h
#include <iostream>
using namespace std;

#define eof -1            // End-of-Input
#define ID 2            // "id"
#define INT 3         // "int"
#define FLOAT 4         // "float"
#define STRING 5        // "string"
#define COMMA 6         // ","
#define SEMI 7          // ";"
#define KWDPROG 10      // "prog"
#define KWDINPUT 22     // "input"
#define KWDPRINT 23     // "print"
#define BRACE1 33      // '{'
#define BRACE2 34     // '}'
#define PAREN1 37      // '('
#define PAREN2 38      // ')'
#define ASTER 41        // '*'
#define CARET 42        // '^'
#define EQUAL 45        // '='
#define MINUS 46        // '-'
#define PLUS 47        // '+'
#define SLASH 48        // '/'
#define ERROR  99       // Unknown token
#define EPS 1000

class Symbol {
public:
	int code = 0;
	int mtxCode;
	string lexeme = "";
	string identifier = "";
	bool terminal;

	Symbol() {

	}
	Symbol(int code_, string lexeme_, string id_, bool terminal_) {
		code = code_;
		lexeme = lexeme_;
		identifier = id_;
		terminal = terminal_;

		switch (code) {

		case KWDPROG: mtxCode = 0;
			break;
		case BRACE1: mtxCode = 1;
			break;
		case BRACE2: mtxCode = 2;
			break;
		case SEMI: mtxCode = 3;
			break;
		case ID: mtxCode = 4;
			break;
		case EQUAL: mtxCode = 5;
			break;
		case KWDINPUT: mtxCode = 6;
			break;
		case KWDPRINT: mtxCode = 7;
			break;
		case COMMA: mtxCode = 8;
			break;
		case PAREN1: mtxCode = 9;
			break;
		case PAREN2: mtxCode = 10;
			break;
		case INT: mtxCode = 11;
			break;
		case FLOAT: mtxCode = 12;
			break;
		case STRING: mtxCode = 13;
			break;
		case PLUS: mtxCode = 14;
			break;
		case MINUS: mtxCode = 15;
			break;
		case ASTER: mtxCode = 16;
			break;
		case SLASH: mtxCode = 17;
			break;
		case CARET: mtxCode = 18;
			break;
		case EOF: mtxCode = 19;

		}
	}
	~Symbol() {

	}

	void operator=(const Symbol& s) {
		code = s.code;
		lexeme = s.lexeme;
		identifier = s.identifier;
		terminal = s.terminal;
	}
	/*Symbol& operator=(const Symbol &s){

	Symbol tmp(s);

	swap1(code, tmp.code);
	swap2(lexeme, tmp.lexeme);
	swap2(identifier, tmp.identifier);
	swap3(terminal, tmp.terminal);

	return *this;
	}

	void swap1( int & one, int& two){
	int tmp(one);
	one = two;
	two = tmp;
	}
	void swap2( string & one, string& two){
	string tmp(one);
	one = two;
	two = tmp;
	}
	void swap3( bool & one, bool & two){
	bool tmp(one);
	one = two;
	two = tmp;
	}
	*/
	friend ostream& operator<<(ostream& os, const Symbol& s) {

		/* Detailed output

		if(!s.terminal) os << "Code: " << s.code << " ID: " << s.identifier << "\n";
		else os << "Code: " << s.code << " Lexeme: " << s.lexeme << "\n";
		*/
		/* Easy to read rule output*/

		if (!s.terminal) os << s.identifier << " ";
		else os << s.lexeme << " ";

		return os;

	}
};

#endif /* Symbol_h */