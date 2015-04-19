
#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <exception>

#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_
namespace cs451_project
{
	/*
	===========
	CLASS: RULE
	===========
	*/
	class rule
	{
	public:
		// THROWS EXCEPTIONS
		rule(std::string s); // Beta

		// Modification member functions
		void to_erase(bool val) { erase = val; }
		void set_rhs(std::string r); // Beta

		// Constant member functions
		bool contains(char variable) const; // Beta
		std::string get_rhs() const { return rhs; }
		bool erasing() { return erase; }
		bool is_eps() const; // Beta
		

		char get_var() const { return lhs[0]; }
		bool operator ==(rule to_compare) 
		{
			return (rhs == to_compare.rhs && 
				lhs == to_compare.lhs);
		}
		friend std::ostream& operator <<(std::ostream& os, rule& r);
		std::string to_string(){
			return std::string(lhs + "->" + rhs);
		}
	private:

		std::string rhs;
		std::string lhs;
		bool erase;
	};

	/*
	==============
	CLASS: GRAMMAR
	==============
	*/
	class grammar
	{
	public:
		static const std::size_t ALPHA = 26;
		grammar();
		grammar(std::string s);	// Beta
		void cnf_convert(); // throws an exception; Beta
		void check_format(); // throws an exception; Beta
		void create_table(std::size_t n); // Beta
		void table_insert(std::size_t i, std::size_t j, char r); // BETA
		bool contains(std::size_t i, std::size_t j, char r); // Beta
		// To prep for a new string
		void clear_table(); // Beta
		bool generate_string(std::string s);
		std::vector<rule>& get_rules() { return rules; }
		void output_table() const;

	private:
		//CNF conversion functions
		bool add_start(); // Beta
		bool rem_epsilons(); // Beta
		bool remove_units(); // Beta
		bool compress_rules(); // Beta
		

		// CNF conversion helpers
		bool verify_cnf(); // Beta
		void mErase(); // Beta
		char new_var(); // Beta
		std::string construct_rule(char v, std::string rhs);
		std::set<std::string> gen_rhs_set(std::string rhs, char offending);
		void rhs_explosion(std::string rhs, std::set<std::string>& new_rhs, 
			std::queue<std::string>& not_recursed, char offending);

		// initialization helpers
		void init_alphabet(std::string s); // Beta
		void init_vars(); // Beta

		// member variables
		std::vector<rule> rules;
		std::set<char> alphabet;
		std::set<char> variables;
		char start;
		std::string helper;
		std::size_t num_vars;
		std::vector<std::vector<std::vector<char>>> rule_table;
		
	};
	static const char ALPHABET[] = "abcdefghijklmnopqrstuvwxyz";
}
#endif