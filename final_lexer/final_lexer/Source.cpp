#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <assert.h>


using namespace std;


regex r_letter("[a-zA-Z]+");
regex r_digit("[0-9]+");

regex r_blank("\s");

regex r_op("(@|=|/=|>|:=|<|>=|<=|\\+|\\*|/|-)"); // operators
regex r_sep("(\\(|\\)|,|;|:|%%|\\{|\\}|\\[|\\])"); // separator
regex r_key("(boolean|else|false|fi|floating|if|integer|read|return|true|while|write)"); // keyword


typedef int fsm_state;
typedef char fsm_input;

bool recognize(string str);
bool is_final_state(fsm_state state);
fsm_state get_start_state(void);
fsm_state move(fsm_state state, fsm_input input);
void lexer();

int main()
{
	lexer();

	return getchar();
}


void lexer() {

	fstream inFile;
	inFile.open("test.txt");

	if(!inFile)
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
		//=======================================================


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
				else if (recognize(fsm_string) == false)
				{
					//fsm_string.clear();

				}
			}
			//fsm_string.clear();

		}

		//=======================================================


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

	cout << "\nCount of Tokens: " << count << endl;

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
		++i;
	}

	if (is_final_state(state))
	{
		return true;
	}
	else { return false; }

};
