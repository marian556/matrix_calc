/*
 * ref_function_call_parser.h
 *
 *  Created on: Jun 26, 2019
 *      Author: marian
 */

#ifndef REF_FUNCTION_CALL_PARSER_H_
#define REF_FUNCTION_CALL_PARSER_H_
#include <memory>

template<typename Elem,
typename Action,
typename FunctionNameChecker,
typename VariablesContainer,
typename ParserTraitT>
struct FunctionCall;


template<typename Elem,
typename Action=std::function<bool(std::string&&,std::vector<Elem>&&)>,
typename FunctionNameChecker=std::function<bool(std::string&&)>,
typename VariablesContainer=std::unordered_map<std::string,Elem>,
typename ParserTraitT =ParserTrait<Elem>
>
struct RefFunctionCall
{

	RefFunctionCall(
			Elem el,//to pass the type only via passing default constructible object
					//and relying on c++17 templates deduction
			const Action& action,
			FunctionNameChecker fname_check=[](std::string&&){return true;},
			std::reference_wrapper<const VariablesContainer> vars =VariablesContainer { },
			ParserTraitT pt =ParserTraitT{ });

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite);

	Action action;
	std::reference_wrapper<const VariablesContainer> vars;
	FunctionNameChecker fname_check;
	std::shared_ptr<FunctionCall<Elem,Action,FunctionNameChecker,VariablesContainer,ParserTraitT>> parser;
};
#endif /* REF_FUNCTION_CALL_PARSER_H_ */

#ifndef DECL_REF_FUNCTION_CALL_ONLY

#include "function_call_parser.h"

template<typename Elem,typename Action,typename FunctionNameChecker,typename VariablesContainer,typename ParserTraitT >
RefFunctionCall<Elem,Action,FunctionNameChecker,VariablesContainer,ParserTraitT>
	::RefFunctionCall(
		Elem ,//to pass the type only via passing default constructible object
				//and relying on c++17 templates deduction
		const Action& action,
		FunctionNameChecker fname_check,
		std::reference_wrapper<const VariablesContainer> vars,
		ParserTraitT pt) :
		  action(action), vars(vars),fname_check(fname_check) {};

template<typename Elem,typename Action,typename FunctionNameChecker,typename VariablesContainer,typename ParserTraitT >
template <typename Iterator>
bool RefFunctionCall<Elem,Action,FunctionNameChecker,VariablesContainer,ParserTraitT>
			::parse(Iterator& it,Iterator ite)
{
	if (!parser)
		parser=std::make_shared<FunctionCall<Elem,Action,FunctionNameChecker,VariablesContainer> >
					(Elem{},action,fname_check,vars);
	return parser->parse(it,ite);
}

#endif


