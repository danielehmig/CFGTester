#include "grammar.h"
using namespace std;

namespace cs451_project
{

	// IMPLEMENTATION OF GRAMMAR CLASS

	/*
	==========================================================
	grammar::grammar

	Initializes grammar from given string with minimal parsing
	==========================================================
	*/
	grammar::grammar(string s)
	{
		// minimize parsing and expensive computation in the constructor
		rules.reserve(10);
		init_alphabet(s);
		size_t length = s.length();
		size_t index = 0, sub_start = 0;
		for(index = 0; index < length; ++index)
		{
			if(s[index] == '\r')
			{
				s.replace(index, 1, "");
				--length;
			}
			if(s[index] == '\n')
			{
				s.replace(index, 1, "");
				--length;
			}
		}
		for(index = 0; index < length; ++index)
		{
			if(s[index] == ';')
			{
				rules.push_back(rule(s.substr(sub_start, index-sub_start)));
				sub_start = index + 1;
			}
		}
		if(rules.size() <= 0)
			throw logic_error("Could not create any rules.");
		init_vars();
		create_table(0);
		helper = "";
		// sub_start is increased if ANY rules are added
		if(sub_start > 0)
			start = (rules[0]).get_var();

	}

	/*
	=========================================
	grammar::grammar

	Default constructor for the grammar class
	=========================================
	*/
	grammar::grammar()
	{
		variables.insert('A');
		variables.insert('B');
		rules.push_back(rule("A->B"));
		create_table(0);
		helper = "";
		start = 'A';
	}

	/*
	=================================================================
	grammar::init_alphabet

	Scans every rule and adds every lower case letter to the alphabet
	=================================================================
	*/
	void grammar::init_alphabet(string s)
	{
		size_t length = s.length();
		size_t index = 0;
		size_t inner = 0;
		for(index = 0; index < length; ++index)
		{
			for(inner = 0; inner < ALPHA; ++inner)
			{
				if(s[index] == ALPHABET[inner])
				{
					// std::set ignores duplicate insertions
					alphabet.insert(s[index]);
				}
			}
		}
	}

	/*
	=========================================================================
	grammar::init_vars

	Scans every rule and adds every upper case letter to the set of variables
	=========================================================================
	*/
	void grammar::init_vars()
	{
		size_t length = rules.size();
		size_t index = 0;
		char variable = '\0';
		for(vector<rule>::iterator it = rules.begin(); 
			it != rules.end(); ++it)
		{
			variable = (*it).get_var();
			for(index = 0; index < ALPHA; ++index)
			{
				if(variable == toupper(ALPHABET[index]))
				{
					variables.insert(variable);
				}
			}
		}
	}

	/*
	==========================================================
	grammar::cnf_convert

	A public "wrapper" function that converts a grammar to CNF
	==========================================================
	*/
	void grammar::cnf_convert()
	{
		if(!add_start())
		{
			throw logic_error("Failed to add new start variable.");
		}

		// *** GOT TO HERE *** ; SOME EPSILON RULES NOT REMOVING CORRECTLY
		if(!rem_epsilons())
		{
			throw logic_error("Error while removing epsilon rules.");
		}

		if(!remove_units())
		{
			throw logic_error("Failed to remove unit rules.");
		}

		if(!compress_rules())
		{
			throw logic_error("Failed to compress rules of length > 2.");
		}

		if(!verify_cnf())
		{
			throw logic_error("Failed to convert to Chomsky Normal Form.");
		}
	}

	/*
	========================================================
	grammar::add_start

	First step of CNF conversion - adding new start variable
	========================================================
	*/
	bool grammar::add_start()
	{
		size_t index = 0;
		for(index = 0; index < ALPHA; ++index)
		{
			if(variables.find(toupper(ALPHABET[index])) == variables.end())
				break;
		}
		if(index < ALPHA)
		{
			rules.push_back(rule(construct_rule(toupper(ALPHABET[index]), helper += start)));
			helper = "";
			start = toupper(ALPHABET[index]);
			variables.insert(start);
			return true;
		}

		// There were no more variables left to use
		return false;
	}

	/*
	============================================================
	grammar::rem_epsilons

	Second step of CNF conversion - remove illegal epsilon rules
	============================================================
	*/
	bool grammar::rem_epsilons()
	{
		queue<char> unremoved;
		for(set<char>::reverse_iterator it = variables.rbegin();
			it != variables.rend(); ++it)
		{
			if(*it != start)
				unremoved.push(*it);
		}

		set<char> alrem;
		set<string> to_add;
		while(!unremoved.empty())
		{
			char variable = unremoved.front();
			for(vector<rule>::iterator it = rules.begin();
				it != rules.end(); ++it)
			{
				if((*it).get_var() == variable && (*it).is_eps())
				{
					for(vector<rule>::iterator it2 = rules.begin(); 
						it2 != rules.end(); ++it2)
					{
						if(it != it2)
						{
							if((*it2).contains(variable))
							{
								string rhs = (*it2).get_rhs();
								string to_modify = rhs;
								size_t r_len = rhs.length();
								if(r_len > 1)
								{
									set<string> new_rhs = gen_rhs_set(rhs, variable);
									for(set<string>::iterator rhs_it = new_rhs.begin();
										rhs_it != new_rhs.end(); ++rhs_it)
									{
										if((*rhs_it).length() == 0)
										{
											if(alrem.find((*it2).get_var()) == alrem.end() &&
												(*it2).get_var() != (*it).get_var())
											{
												to_add.insert(construct_rule((*it2).get_var(), "$"));
											}
										}
										else
										{
											to_add.insert(construct_rule((*it2).get_var(), *rhs_it));
										}
									}

								}
								else
								{
									// We know the rule is a unit because the contains() returned true
									if(alrem.find((*it2).get_var()) == alrem.end() && 
										(*it2).get_var() != variable)
									{
										to_add.insert(construct_rule((*it2).get_var(), "$"));
									}
									else
									{
										(*it2).to_erase(true);
									}
								}
							}
						}
					}
					(*it).to_erase(true);
				}

			}
			alrem.insert(variable);
			unremoved.pop();
			for(set<string>::iterator it = to_add.begin(); it != to_add.end(); ++it)
			{
				rules.push_back(rule(*it));
			}
			to_add.clear();
		}

		mErase();

		// We have to check if we no longer need some of the variables
		for(set<char>::iterator it = alrem.begin(); it != alrem.end(); ++it)
		{
			vector<rule>::iterator it2;
			for(it2 = rules.begin(); it2 != rules.end(); ++it2)
			{
				if((*it) == (*it2).get_var())
					break;
			}
			if(it2 == rules.end())
			{
				// We don't need the variable anymore, so free it up for later use
				variables.erase(*it);
			}
		}
		return true; 
	}

	/*
	================================================================
	grammar::remove_units

	Third step of CNF conversion - remove all rules of the form A->B
	================================================================
	*/
	bool grammar::remove_units()
	{
		set<string> to_add;
		set<string> alrem;
		size_t length = rules.size();
		rules.reserve(length * 2);
		// Fuck it, I'm going to Nantucket
		for(size_t i = 0; i < length; ++i)
		{
			string rhs = (rules[i]).get_rhs();
			size_t r_len = rhs.length();
			if(r_len == 1)
			{
				// Ensure that the right hand side is not just a terminal
				if(variables.find(rhs[0]) != variables.end())
				{
					if(rhs[0] != (rules[i]).get_var())
					{
						// Retrieve all rules with rhs[0] as its LEFT HAND SIDE
						for(size_t j = 0; j < length; ++j)
						{
							if((rules[j]).get_var() == rhs[0])
							{
								// Ensure that this rule has not already been removed
								string rhs2 = (rules[j]).get_rhs();
								string already = construct_rule((rules[i]).get_var(), rhs2);
								if(alrem.find(already) == alrem.end())
								{
									if(rhs.length() == 1)
									{
										rules.push_back(rule(already));
										++length;
									}
									else
									{
										to_add.insert(already);
									}
								}
							}
						}
					}
					(rules[i]).to_erase(true);
					alrem.insert((rules[i]).to_string());
				}
			}
		}
		mErase();
		for(set<string>::iterator it = to_add.begin(); it != to_add.end(); ++it)
		{
			rules.push_back(rule(*it));
		}

		set<char> to_remove;
		// Now check to see if we can free up any variables
		for(set<char>::iterator it = variables.begin(); 
			it != variables.end(); ++it)
		{
			vector<rule>::iterator it2;
			for(it2 = rules.begin(); it2 != rules.end(); ++it2)
			{
				if((*it) == (*it2).get_var())
					break;
			}
			if(it2 == rules.end())
			{
				to_remove.insert(*it);
			}
		}

		for(set<char>::iterator itty = to_remove.begin(); 
			itty != to_remove.end(); ++itty)
		{
			variables.erase(*itty);
		}
		return true; 
	}

	/*
	==================================================
	grammar::compress_rules

	Fourth step of CNF conversion - enforce RHS length 
	==================================================
	*/
	bool grammar::compress_rules()
	{
		set<string> to_add;
		for(vector<rule>::iterator it = rules.begin(); it != rules.end(); ++it)
		{
			string rhs = (*it).get_rhs();
			size_t r_len = rhs.length();
			if(r_len > 2)
			{
				char curr_lhs = (*it).get_var();
				char last = rhs[r_len-1];
				for(size_t i = 0; i < r_len - 1; ++i)
				{
					char next_lhs;
					try
					{
						next_lhs = new_var();
					}
					catch(domain_error e)
					{
						// There were no more upper case letters to use
						return false;
					}
					if(i == (r_len - 2))
					{
						to_add.insert(construct_rule(curr_lhs, (helper += rhs[i]) += last));
					}
					else
					{
						to_add.insert(construct_rule(curr_lhs, (helper += rhs[i]) += next_lhs));
						curr_lhs = next_lhs;
					}
					helper = "";
				}
				(*it).to_erase(true);
			}

		}
		for(set<string>::iterator looper = to_add.begin(); 
			looper != to_add.end(); ++looper)
		{
			rules.push_back(rule(*looper));
		}
		mErase();
		to_add.clear();
		// Now, we need to take care of any anomalous length 2 rules
		for(vector<rule>::iterator it = rules.begin(); it != rules.end(); ++it)
		{
			string rhs = (*it).get_rhs();
			size_t len = rhs.length();
			if(len == 2)
			{
				// There are three cases to consider
				// CASE 1: first terminal, second variable
				if(alphabet.find(rhs[0]) != alphabet.end() &&
					variables.find(rhs[1]) != variables.end())
				{
					char next_lhs;
					try
					{
						next_lhs = new_var();
					}
					catch(domain_error e)
					{
						return false;
					}
					char terminal = rhs[0];
					string next_rhs = rhs.replace(0, 1, helper += next_lhs);
					helper = "";
					(*it).set_rhs(next_rhs);
					to_add.insert(construct_rule(next_lhs, helper += terminal));
					helper = "";
				}
				// CASE 2: first and second terminal
				else if(alphabet.find(rhs[0]) != alphabet.end() &&
					alphabet.find(rhs[1]) != alphabet.end())
				{
					char next_lhs1;
					char next_lhs2;
					try
					{
						next_lhs1 = new_var();
						next_lhs2 = new_var();
					}
					catch(domain_error e)
					{
						return false;
					}
					to_add.insert(construct_rule(next_lhs1, helper += rhs[0]));
					helper = "";
					to_add.insert(construct_rule(next_lhs2, helper += rhs[1]));
					helper = "";
					string add = rhs.replace(0, 1, helper += next_lhs1);
					helper = "";
					add = add.replace(1, 1, helper += next_lhs2);
					helper = "";
					(*it).set_rhs(add);
				}
				// CASE 3: first variable, second terminal (almost identical to first case)
				else if(variables.find(rhs[0]) != variables.end() &&
					alphabet.find(rhs[1]) != alphabet.end())
				{
					char next_lhs;
					try
					{
						next_lhs = new_var();
					}
					catch(domain_error e)
					{
						return false;
					}
					char terminal = rhs[1];
					string new_rhs = rhs.replace(1, 1, helper += next_lhs);
					helper = "";
					(*it).set_rhs(new_rhs);
					to_add.insert(construct_rule(next_lhs, helper += terminal));
					helper = "";
				}

			}
		}
		for(set<string>::iterator itty = to_add.begin(); 
			itty != to_add.end(); ++itty)
		{
			rules.push_back(rule(*itty));
		}
		return true;
	}

	/*
	====================================================================
	grammar::check_format

	Ensure that the CFG is the proper format and ready to convert to CNF
	====================================================================
	*/
	void grammar::check_format()
	{
		set<string> to_add;
		bool erase = false;
		bool had_or = false;
		// FIRST: We need to check that each variable in the rhs of
		// a rule is actually in the lhs of any other rule
		for(vector<rule>::iterator it = rules.begin(); 
			it != rules.end(); ++it)
		{
			string rhs = (*it).get_rhs();
			size_t r_len = rhs.length();
			size_t sub_start = 0;
			for(size_t i = 0; i < r_len; ++i)
			{
				if(rhs[i] > 64 && rhs[i] < 91)
				{
					if(variables.find(rhs[i]) == variables.end())
					{
						throw domain_error("The rule with rhs: " + rhs + 
							" cannot contain the variable " + rhs[i]);
					}

				}
				// Parse each rule separated by the OR operator
				if(rhs[i] == '|')
				{
					had_or = true;
					to_add.insert(construct_rule((*it).get_var(), rhs.substr(sub_start, i - sub_start)));
					sub_start = i + 1;
					erase = true;
				}
			}
			if(erase)
			{
				(*it).to_erase(true);
				erase = false;
			}
			// Make sure to catch the last rule in a list of RHS's separated by '|'
			if(had_or)
			{
				had_or = false;
				to_add.insert(construct_rule((*it).get_var(), rhs.substr(sub_start, r_len - sub_start)));
			}
		}

		mErase();
		for(set<string>::iterator itty = to_add.begin();
			itty != to_add.end(); ++itty)
		{
			// You fucking idiot, Daniel
			rules.push_back(rule(*itty));
		}

		// Check that epsilon rules contain ONLY '$' in their RHS
		// Also take care of erroneous characters
		for(vector<rule>::iterator looper = rules.begin(); 
			looper != rules.end(); ++looper)
		{
			string rhs = (*looper).get_rhs();
			size_t r_len = rhs.length();
			for(size_t i = 0; i < r_len; ++i)
			{
				if(rhs[i] == '$' && r_len > 1)
				{
					throw logic_error("Error: Epsilon rules may only be singletons.");
				}
				if(rhs[i] < 65 || rhs[i] > 122 || (rhs[i] > 90 && rhs[i] < 97))
				{
					if(rhs[i] != '$')
						throw logic_error("Error: Invalid character " + rhs[i]);
				}
			}
		}
	}

	/*
	======================================================
	grammar::construct_rule

	Helper function for constructing a rule in string form
	======================================================
	*/
	string grammar::construct_rule(char v, string rhs)
	{
		char rule[] = 
		{v, '-', '>', '\0'};
		string result(rule);
		result.append(rhs);
		return result;
	}

	/*
	========================================================
	grammar::create_table

	Initialize the table for the string generation algorithm
	========================================================
	*/
	void grammar::create_table(size_t n)
	{
		size_t i = 0;
		rule_table.resize(n);
		for(i = 0; i < n; ++i)
		{
			(rule_table[i]).resize(n);
		}
	}

	/*
	===========================================
	grammar::output_table

	Print the current dynamic programming table
	===========================================
	*/

	void grammar::output_table() const
	{
		size_t row_len = rule_table.size();
		ofstream file;
		file.open("table.txt");
		for(size_t i = 0; i < row_len; ++i)
		{
			for(size_t j = 0; j < row_len; ++j)
			{
				size_t num_vars = (rule_table[i][j]).size();
				for(size_t k = 0; k < num_vars; ++k)
				{
					file << rule_table[i][j][k] << ", ";
				}
				file << "\t";
			}
			file << "\n";
		}
		file.close();
	}

	/*
	======================================================================
	grammar::table_insert

	Inserts a new entry into the table for the string generation algorithm
	======================================================================
	*/
	void grammar::table_insert(size_t i, size_t j, char r)
	{
		(rule_table[i][j]).push_back(r);
	}

	/*
	==================================
	grammar::contains

	Check if r is in table(i,j)
	==================================
	*/
	bool grammar::contains(size_t i, size_t j, char r)
	{
		size_t length = (rule_table[i][j]).size();
		for(size_t k = 0; k < length; ++k)
		{
			if((rule_table[i][j][k]) == r)
				return true;
		}
		return false;
	}

	/*
	============================================================
	grammar::clear_table

	Clear the entries in the table to prep for a new test string
	============================================================
	*/
	void grammar::clear_table()
	{
		size_t rows = rule_table.size();
		for(size_t i = 0; i < rows; ++i)
		{
			for(size_t j = 0; j < rows; ++j)
			{
				(rule_table[i][j]).clear();
			}
			(rule_table[i]).clear();
		}
		rule_table.clear();
	}

	/*
	=====================================================================
	grammar::generate_string

	Primary algorithm for attempting to generate a string for the grammar
	=====================================================================
	*/
	bool grammar::generate_string(string s)
	{
		size_t len = s.length();
		create_table(len+1);
		if(len == 1 && s[0] == '$')
		{
			for(vector<rule>::iterator it = rules.begin(); 
				it != rules.end(); ++it)
			{
				if((*it).get_var() == start && (*it).is_eps())
				{
					return true;
				}
			}
			// The input is the empty string, and the start rule does not have an epsilon rhs
			return false;
		}
		s.insert(0,"&");
		for(size_t i = 1; i <= len; ++i)
		{
			for(set<char>::iterator it = variables.begin(); it != variables.end(); ++it)
			{
				for(vector<rule>::iterator it2 = rules.begin();
					it2 != rules.end(); ++it2)
				{
					if((*it2).get_var() == (*it))
					{
						string rhs = (*it2).get_rhs();
						if(rhs.length() == 1 && rhs[0] == s[i])
							table_insert(i, i, *it);
					}
				}
			}
		}

		// ***************** HERE THERE BE DRAGONS ******************8
		for(int k = 2; k <= len; ++k)
		{
			for(int i = 1; i <= (len - k + 1); ++i)
			{
				for(int j = i; j <= (i+k-2); ++j)
				{
					for(vector<rule>::iterator it = rules.begin(); 
						it != rules.end(); ++it)
					{
						string rhs = (*it).get_rhs();
						if(rhs.length() == 2)
						{
							if(contains(i, j, rhs[0]) && contains(j+1, i+k-1, rhs[1]))
							{
								table_insert(i, i+k-1, (*it).get_var());
							}
						}
					}
				}
			}
		}

		return (contains(1, len, start));
	}

	/*
	=======================================================
	grammar::mErase

	Helper function to erase all rules marked as "to erase"
	=======================================================
	*/
	void grammar::mErase()
	{
		size_t length = rules.size();
		for(size_t i = 0; i < length; ++i)
		{
			if((rules[i]).erasing())
			{
				rules[i] = rules[length - 1];
				rules.pop_back();
				--length;
			}
		}
		length = rules.size();
		for(vector<rule>::iterator it = rules.begin(); 
			it != rules.end(); ++it)
		{
			if((*it).erasing())
			{
				rules.erase(it);
				return;
			}

		}
	}

	/*
	======================================================
	grammar::new_var

	Returns an unused upper case letter to use as variable
	======================================================
	*/
	char grammar::new_var()
	{
		for(size_t i = 0; i < ALPHA; ++i)
		{
			if(variables.find(toupper(ALPHABET[i])) == 
				variables.end())
			{
				variables.insert(toupper(ALPHABET[i]));
				return toupper(ALPHABET[i]);
			}
		}
		throw domain_error("Error: No more available variables.");
	}

	/*
	============================================================================
	grammar::verify_cnf 

	After (somewhere else) performing CNF conversion, verifies grammar is in CNF
	============================================================================
	*/
	bool grammar::verify_cnf()
	{
		for(vector<rule>::iterator it = rules.begin();
			it != rules.end(); ++it)
		{
			string rhs = (*it).get_rhs();
			size_t r_len = rhs.length();
			// Not the proper length
			if(r_len != 2 && r_len != 1)
			{
				return false;
			}
			if(alphabet.find(rhs[0]) != alphabet.end())
			{
				// A terminal can only appear as a singleton
				if(r_len > 1)
					return false;
			}
			else
			{
				// We can't have any unit rules
				if(r_len == 1)
				{
					if(rhs[0] == '$')
					{
						continue;
					}
					else
					{
						return false;
					}
				}
				else
				{
					// We can't have rules of the form A->Bc, only A->BC
					if(variables.find(rhs[1]) == variables.end())
					{
						return false;
					}
				}
			}
			// Only the start rule can have epsilon on its RHS
			if((*it).is_eps() && (*it).get_var() != start)
			{
				return false;
			}
		}
		// There were no red flags, so the grammar is in Chomsky Normal Form
		return true;
	}

	/*
	==================================================
	grammar::gen_rhs_set

	Prepares for the rhs expansion recursive procedure
	==================================================
	*/
	set<string> grammar::gen_rhs_set(string rhs, char offending)
	{
		set<string> new_rhs;
		queue<string> not_recursed;
		rhs_explosion(rhs, new_rhs, not_recursed, offending);
		return new_rhs;
	}

	/*
	==================================================================================
	grammar::rhs_explosion

	Recursively generates all combinations for an rhs in the epsilon-removal procedure
	==================================================================================
	*/
	void grammar::rhs_explosion(string rhs, set<string>& new_rhs,
		queue<string>& not_recursed, char offending)
	{
		string to_modify = rhs;
		bool found_pivot = false;

		// Finding all of the pivot points
		size_t len = rhs.length();
		for(size_t i = 0; i < len; ++i)
		{
			if(to_modify[i] == offending)
			{
				found_pivot = true;
				to_modify.replace(i, 1, "");
				new_rhs.insert(to_modify);
				not_recursed.push(to_modify);
				to_modify = rhs;
			}
		}

		if(!found_pivot)
		{
			new_rhs.insert(rhs);
			return;
		}

		// BEWARE OF INFINITE RECURSION
		while(!not_recursed.empty())
		{
			string next = not_recursed.front();
			not_recursed.pop();
			rhs_explosion(next, new_rhs, not_recursed, offending);
		}

	}

	// IMPLEMENTATION OF RULE CLASS

	/*
	===================================================================
	rule::rule

	Initializes a new rule with the given string; performs some parsing
	===================================================================
	*/
	rule::rule(string s)
	{
		size_t len = s.length();
		size_t index = 0;
		// FIRST: Remove all wHitespace
		for(index = 0; index < len; ++index)
		{
			if(isspace(s[index]))
			{
				s[index] = s[len - 1];
				s.pop_back();
				--len;
			}
		}
		index = 0;
		while(s[index] != '-' && index < len) ++index;
		if(index == len)
			throw logic_error("Error: Invalid rule form.");
		lhs = s.substr(0, index);
		lhs = lhs[0];
		// Increase the position to the character directly after the '->'
		index += 2;
		if(index >= len)
			throw logic_error("Error: Rule has no right-hand side.");

		rhs = s.substr(index, (len - index));
		erase = false;
		// MIGHT NOT BE DONE PARSING. CONSIDER DOING MORE.
	}

	/*
	=======================================================
	rule::contains

	Checks whether the given variable is in this rule's RHS
	=======================================================
	*/
	bool rule::contains(char variable) const 
	{
		size_t r_len = rhs.length();
		for(size_t i = 0; i < r_len; ++i)
		{
			if(rhs[i] == variable)
				return true;
		}
		return false;
	}

	/*
	========================================================
	rule::is_eps

	Just returns whether the calling rule is an epsilon rule
	========================================================
	*/
	bool rule::is_eps() const 
	{
		size_t r_len = rhs.length();
		if(r_len > 1)
		{
			return false;
		}
		else
		{
			// The |rhs| = 1 at this point
			if(rhs == "$")
				return true;
			else
				return false;
		}
	}

	/*
	===================================================
	rule::set_rhs

	Set the RHS of the calling rule to the given string
	===================================================
	*/
	void rule::set_rhs(string r)
	{
		if(r.length() == 0)
		{
			// Implied epsilon rule
			rhs = "$";
		}
		else
		{
			rhs = r;
		}
	}

	ostream& operator <<(ostream& os, rule& r)
	{
		os << r.lhs << "->" << r.rhs;
		return os;
	}
}