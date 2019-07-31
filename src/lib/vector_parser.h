/*
 * vector_parser.h
 *
 *  Created on: Jun 24, 2019
 *      Author: marian
 */

#ifndef VECTOR_PARSER_H_
#define VECTOR_PARSER_H_

#include <vector>
#include "parser_traits.h"


template <typename T>
struct get_vector_element_type
{
	typedef T type;
};
template <typename T>
struct get_vector_element_type<arithmetic_expression<T>>
//we are interested in result of arithmetic operation to store into vector element
{
	typedef T type;
};

template <typename Elem,
		typename Action=std::function<void(std::vector<typename get_vector_element_type<Elem>::type>&&)> ,
		typename VariablesContainer=std::unordered_map<std::string,Elem>
		//,typename ParserTraitT=ParserTrait<Elem>
>
//This is a generic vector parser of elements of any type
// Elem ( Elem can be int or another vector<int>)
// parser_traits.h must have an entry for mapping from Elem to actual parser choice
//to get parser for matrix you use this : get{std::vector<Elem>{}}

struct VectorParser
{

	VectorParser(){};

	VectorParser(Elem){}; //only to pass type for deduction guides

	VectorParser(const VectorParser& other):
		action(other.action),
		vars(other.vars),
		data(other.data),
		left_bracket(other.left_bracket),
		right_bracket(other.right_bracket){
	};

	static std::reference_wrapper<const VariablesContainer> get_null_var_container()
	{
		static auto null_cont=VariablesContainer{};
		return std::reference_wrapper<const VariablesContainer>{null_cont};
	}

	VectorParser(Elem ,Action&& action,
			std::reference_wrapper<const VariablesContainer> vars=get_null_var_container(),
			char left_bracket='[',char right_bracket=']'):
		action(std::move(action)),
		vars(vars),
		left_bracket(left_bracket),
		right_bracket(right_bracket)
		{
		}

	VectorParser(Elem ,const Action& action,
			std::reference_wrapper<const VariablesContainer> vars=get_null_var_container(),
			char left_bracket='[',char right_bracket=']'):
		action(action),
		vars(vars),
		left_bracket(left_bracket),
		right_bracket(right_bracket)
	{
	}

	VectorParser& operator = (const VectorParser& other)
	{
		action=other.action;
		vars=other.vars;
		left_bracket=other.left_bracket;
		right_bracket=other.right_bracket;
		return *this;
	}

	void set_action(Action&& action1) {
		action=std::move(action1);
	}

	void set_action(const Action& action1) {
		action=action1;
	}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{
		data.clear();
		if (!parser.parse(it,ite))
			return false;
		action(std::move(data));
		return true;
	}

	private:
	Action action;
	std::reference_wrapper<const VariablesContainer> vars=get_null_var_container();

	typedef typename get_vector_element_type<Elem>::type Elem_;
	std::vector<Elem_> data;

	decltype (ParserTrait<Elem>::get_parser(
			std::function<void(Elem_&&)>{},vars)) elem_parser=
			ParserTrait<Elem>::get_parser(
					std::function<void(Elem_&&)>{[this](Elem_&& el)
				{
						data.push_back(std::move(el));
				}},vars);
	//careful about consistency. sub object integer_parser of this class
	// points to this class (capture this) and another subobject (data) of this class.
	//when you copy object to a new place you need to relink this relation for a new this
	// in copy constructor or rely on members initialization in declaration as a fallback to
	// member initialization list

	char left_bracket='[';
	char right_bracket=']';
	decltype(get_bracketed_list_parser(elem_parser)) parser=
			 get_bracketed_list_parser(elem_parser,',',left_bracket,right_bracket);


};




#endif /* VECTOR_PARSER_H_ */
