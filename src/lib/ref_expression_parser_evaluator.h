/*
 * ref_expression_parser_evaluator.h
 *
 *  Created on: Jun 28, 2019
 *      Author: marian
 */

#ifndef REF_EXPRESSION_PARSER_EVALUATOR_H_
#define REF_EXPRESSION_PARSER_EVALUATOR_H_

#include <memory>

template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler >
struct ExpressionParserAndEvaluator;

template<typename Elem,
typename Action=std::function<void(Elem&&)>,
typename VariablesContainer=std::unordered_map<std::string,Elem> ,
typename FunctionsHandler=std::function<Elem(std::string&&,std::vector<Elem>&&)>
>

struct RefExpressionParserAndEvaluator
//to deal with:
//error: invalid use of incomplete type 'struct ExpressionParserAndEvaluator<int, RefWrapper<int, TermAccumulator<int> >, std::unordered_map<std::__cxx11::basic_string<char>, int> >'};
{

	RefExpressionParserAndEvaluator(const RefExpressionParserAndEvaluator&);
	RefExpressionParserAndEvaluator(RefExpressionParserAndEvaluator&&);

	RefExpressionParserAndEvaluator(const ExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>&);

	RefExpressionParserAndEvaluator(
			Elem el,//to pass the type only via passing default constructible object and depending on c++17 templates deduction
			const Action& action,
			std::reference_wrapper<const VariablesContainer> vars=VariablesContainer{},
			FunctionsHandler  functions_handler=FunctionsHandler{}
			//,ParserTraitT pt=ParserTrait<Elem>{}
	    );


	template<typename Iterator>
	bool parse(Iterator& it, Iterator ite);

	std::shared_ptr<ExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>> parser;//pimpl
	Action action;
	std::reference_wrapper<const VariablesContainer> vars;
	FunctionsHandler functions_handler;
};



#endif /* REF_EXPRESSION_PARSER_EVALUATOR_H_ */

#ifndef DECL_REF_EXPRESSION_PARSER_ONLY

template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler>
RefExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>
::RefExpressionParserAndEvaluator(const ExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>& other)
 : action(other.action),vars(other.vars),functions_handler{other.functions_handler}
 {

 };



template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler>
RefExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>
::RefExpressionParserAndEvaluator(RefExpressionParserAndEvaluator&& other)
 :action(std::move(other.action)),vars(std::move(other.vars)),
  functions_handler{std::move(other.functions_handler)}
 {
 }

template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler>
RefExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>
::RefExpressionParserAndEvaluator(const RefExpressionParserAndEvaluator& other)
 :action(other.action),
  vars( other.vars ),
  functions_handler(other.functions_handler)
 {

 };

template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler>
RefExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>
::RefExpressionParserAndEvaluator(
			Elem el,//to pass the type only via passing default constructible object and depending on c++17 templates deduction
			const Action& action,
			std::reference_wrapper<const VariablesContainer> vars,
			FunctionsHandler functions_handler
	    )
		:action(action),vars(vars),functions_handler(functions_handler)
{
};

template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler>
template<typename Iterator>
bool RefExpressionParserAndEvaluator<Elem,Action,VariablesContainer,FunctionsHandler>
	::parse(Iterator& it, Iterator ite)
{

	//auto stack_parser=ExpressionParserAndEvaluator{Elem{},action,vars};
	//return stack_parser.parse(it,ite);
	//stack parser has to recreate parser every time, heap parser does it only once first time it enters
	// bracket depth level.  Each bracket depth level has it's own parser , but expressions on the same level share
	// the same parser. It is a reasonable trade-off between cpu overhead and memory overhead

	if (!parser)
	  parser=std::make_shared<ExpressionParserAndEvaluator<Elem,Action,VariablesContainer> >
				(Elem{},action,vars);
	return parser->parse(it,ite);
}

#endif
