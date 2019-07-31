/*
 * function_call_parser.h
 *
 *  Created on: Jun 26, 2019
 *      Author: marian
 */

#ifndef FUNCTION_CALL_PARSER_H_
#define FUNCTION_CALL_PARSER_H_
#include <string>
#include "vector_parser.h"

struct AllFunctionNames
{
	bool operator()  (std::string&&) const {return true;}
	bool operator()  (const std::string&) const {return true;}
};

template<typename Elem,
typename Action=std::function<bool(std::string&&,std::vector<Elem>&&)>,
typename FunctionNameChecker=std::function<bool(std::string&&)>,
typename VariablesContainer=std::unordered_map<std::string,Elem> ,
typename ParserTraitT =ParserTrait<Elem> >
struct FunctionCall
{

	FunctionCall(
		Elem el,//to pass the type only via passing default constructible object
				//and relying on c++17 templates deduction
		const Action& action,//):action(action){}

		FunctionNameChecker fname_check=std::function<bool(std::string&&)>{},
		std::reference_wrapper<const VariablesContainer> vars =VariablesContainer { },
		ParserTraitT pt =ParserTrait<Elem> { }) :
		  action(action), vars(vars),fname_check(fname_check) {}

	FunctionCall(const FunctionCall& other)
		:action(other.action),vars(other.vars),fname_check(other.fname_check),
		 function_name(other.function_name),args(other.args)
	{
		std::cout << "FunctionCall cc" << std::endl;
	}
	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{

		if (!parser.parse(it,ite))
			return false;
		return action(std::move(function_name),std::move(args));
	}

	Action action;
	static auto& get_empty_var_container() {
		static VariablesContainer vc=VariablesContainer{};
		return vc;
	}
	std::reference_wrapper<const VariablesContainer> vars=get_empty_var_container();
	FunctionNameChecker fname_check=FunctionNameChecker{};

	std::string function_name;
	typedef typename get_vector_element_type<Elem>::type Elem_;
	std::vector<Elem_> args;

	std::function<bool(std::string&&)> function_name_handler=[this](std::string&& name)
	{
			function_name=std::move(name);//we can destroy name here
			return true;
	};

	std::function<void(std::vector<Elem_>&&)> args_handler=[this](std::vector<Elem_>&& args1)
	{
		args=std::move(args1);
	};

	//shame we cannot use auto here
	decltype(VectorParser{arithmetic_expression<Elem>{},args_handler,vars,'(',')'}) expr_vector
   	        =VectorParser{arithmetic_expression<Elem>{},args_handler,vars,'(',')'};

	decltype(Sequence{FunctionName{function_name_handler},expr_vector}) parser
			=Sequence{FunctionName{function_name_handler},expr_vector};

};



#endif /* FUNCTION_CALL_PARSER_H_ */
