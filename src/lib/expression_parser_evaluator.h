/*
 * expression_parser_evaluator.h
 *
 *  Created on: Jun 24, 2019
 *      Author: marian
 */

#ifndef EXPRESSION_PARSER_EVALUATOR_H_
#define EXPRESSION_PARSER_EVALUATOR_H_
#include <functional>

#include "parse_utils.h"
#include "matrix.h"
#include "parse_integer.h"
#include "parser_traits.h"
#include "vector_parser.h"

#define DECL_REF_FUNCTION_CALL_ONLY
#include "ref_function_call_parser.h"

#define DECL_REF_EXPRESSION_PARSER_ONLY
#include "ref_expression_parser_evaluator.h"

///This class parses and calculates a term or multiplication 5453 * 232 * 322*../
/**
 * This class parses and calculates a term which is a sequence of
 * factors separated by multiplication sign '*' 5453 * 232 * 332 with optional spaces in between
 * Each factor can be a value , an variable , a function with multiple arguments
 * as expressions or another expression in round brackets
 * Term like this is possible: 34*43*f(5+5*par,634)*a*(5+4*4343)*6
 * Parsing stops after the last factor occuring sign any other than '*'
 */
template<typename Elem,
		 typename Action=std::function<void(Elem&&)>,
		 typename VariablesContainer = std::unordered_map<std::string, Elem>,
		 typename FunctionsHandler=std::function<Elem(std::string&&,std::vector<Elem>&&)>,
		 typename ParserTraitT = ParserTrait<Elem> >
struct TermParserAndEvaluator {
	TermParserAndEvaluator(
			Elem el,///< This is only to pass the element type and relying on c++17 templates deduction, value is not used
			Action&& action,///< Action that gets triggered after a successful parse of the term with signature void(Elem&&), used to deliver result
			std::reference_wrapper<const VariablesContainer> vars =VariablesContainer { },
					///< [in] optional variables container that is compatible
					///< with std::unordered_map, supports at least look up: auto it=vars.find(std::string),
					///< vars.end() and *it resolves into reference to a value
			FunctionsHandler  functions_handler=FunctionsHandler{},
					///< [in] optional Callback/handler triggered with a
					///< signature Elem(std::string&&,std::vector<Elem>&&)
			        ///< to process a function call
			ParserTraitT pt = ParserTrait<Elem> { }
					///< optional ParserTrait to control what Parser to trigger for Elem, see parser_traits.h
					///< You should not need to change this. Default should work for you.
			) :
			action(std::move(action)), vars(vars) ,functions_handler(functions_handler)
	{
	}

	TermParserAndEvaluator(
			Elem el,//to pass the type only via passing default constructible object
			//and relying  on c++17 templates deduction
			const Action& action,
			std::reference_wrapper<const VariablesContainer> vars =VariablesContainer { },
			FunctionsHandler  functions_handler=FunctionsHandler{},
			ParserTraitT pt = ParserTrait<Elem> { }) :
			action(action), vars(vars),functions_handler(functions_handler)
	{
	}

	TermParserAndEvaluator(const TermParserAndEvaluator& other) :
			action(other.action), vars(other.vars),
			term_accumulator(other.term_accumulator),
			functions_handler(other.functions_handler)
	{
		//we copy only external data, internal should be re-created
	}

	template<typename Iterator>
	bool parse(Iterator& it, Iterator ite) {
		term_accumulator.reset();
		if (!term_parser.parse(it, ite))
			return false;
		action(std::move(term_accumulator.result));
		return true;
	}


private:
	template <typename T>
	struct TermAccumulator
	{

		template <typename U>
		//U is usually T, but we want to trigger forwarding(universal) references here
		// so template is mandatory , specifying T directly would disable mechanism
		// also implicit conversion from U to T is usefull
		void operator()(U&& val) {
			if (cnt==0)
			{
			 result=std::forward<U>(val);
			 cnt++;
			}
			else
			 result*=std::forward<U>(val);
		}
		void reset() { cnt=0; }
		int cnt=0;
		T result{};
	};



	template <typename T,typename AccumT>
	struct RefTermAccumulator
	{
		RefTermAccumulator():pdata(0){}

		RefTermAccumulator(const RefTermAccumulator<T,AccumT>& other):
			pdata(other.pdata){}

		RefTermAccumulator<T,AccumT>& operator =(const RefTermAccumulator<T,AccumT>& other)
		{
			pdata=other.pdata;
			return *this;
		}
		void operator()(T&& val) {
			if (pdata)
			  (*pdata)(std::move(val));
		}
		RefTermAccumulator(T,AccumT& data):pdata(std::addressof(data)){}
		AccumT* pdata;
	};


	Action action;
	std::reference_wrapper<const VariablesContainer> vars;

	TermAccumulator<Elem> term_accumulator;


	std::function<bool(std::string&&)>

	variable_handler =

	[this](std::string&& varname)
	{
		//auto it=vars.get().find(varname);
		auto it=vars.get().find(varname);
		if (it==vars.get().end())
		{
		   throw parse_exception(std::string("variable '")+varname+"' not defined yet.");
		   //return false;//parsing of variable failed
		}
		term_accumulator(it->second);
		return true;
	};

	std::function<Elem(std::string&&,std::vector<Elem>&&)>
	  functions_handler=
	[](std::string&&,std::vector<Elem>&&)
	{
		return Elem{};
	};

	std::function<bool(std::string&&,std::vector<Elem>&&)> internal_function_handler
			=
	[this](std::string&& fname,std::vector<Elem>&&args) {
		term_accumulator(functions_handler(std::move(fname),std::move(args)));
		return true;
	};

	RefTermAccumulator<Elem, TermAccumulator<Elem> > copyable_term_accumulator =
			RefTermAccumulator { Elem { }, term_accumulator };

	inline static auto  get_factor_parser(
			RefTermAccumulator<Elem, TermAccumulator<Elem> >& copyable_term_accumulator,
			std::function<bool(std::string&&)>& variable_handler,
			std::function<bool(std::string&&,std::vector<Elem>&&)>& internal_function_handler,
			std::function<Elem(std::string&&,std::vector<Elem>&&)>& functions_handler,
			const std::function<bool(std::string&&)>& function_name_checker,
			std::reference_wrapper<const VariablesContainer>& vars)
	{
			return Alternative {
						ParserTraitT::get_parser( copyable_term_accumulator,vars),
						RefFunctionCall{ Elem{},internal_function_handler, function_name_checker,vars},
						//we want to check first function call then variable, function call checks if
						//potential name (variable of function) is followed by '('
						VariableName { variable_handler },
						Sequence{ Char('('),
							RefExpressionParserAndEvaluator{
								Elem { },copyable_term_accumulator,vars,functions_handler},
							WhiteSpace { }, Char(')')
						}
					};

			//the 2 lines above (Sequence4) makes both TermsParserAndEvaluator and Expression parser and evaluator
			//a recursive descent parser

			//Careful: We cannot use simpler in-place lambdas (instead of old fashioned copyable_term_accumulator)
			// in constructors of both classes the ExpressionParserAndEvaluator and TermParserAndEvaluator
			// If you do you you get a recursive template instantiation exceeding the compiler limit for depth
			// Reason: each lambda (in both classes) is a type with a signature that includes
			// templates types of enclosing class , it is a compile time recursive process that follows the structure
			// of recursive nature of this recursive descent parser

	}

	//unfortunately c++17 does not allow use of auto here.
	decltype(get_factor_parser(copyable_term_accumulator,variable_handler,
			internal_function_handler,functions_handler,
			std::function<bool(std::string&&)>{},vars))

	factor_parser =

			get_factor_parser(copyable_term_accumulator,
					variable_handler,internal_function_handler,functions_handler,
					std::function<bool(std::string&&)>{}, vars);

	decltype(get_list_parser(factor_parser,'*'))

	term_parser =

			get_list_parser(factor_parser, '*');

};






template <typename T>
void flip_sign_in_place(T& val)
//specialize for custom types/Matrix via ADL argument dependent lookup
 //for efficient operation
{
	val=-val;
}

///This class parses and calculates arithmetic expression of the type 5453 * 232 + 43* 36 + ...
/**
 * This is a generic class working for arbitrary arithmetic number type Elem
 * This class parses and calculates the result of an arithmetic expression which is a sequence of
 * terms separated by addition sign '+' or subtraction sign '-'  with optional spaces in between terms and signs:
 * term1 [(+/-) term2 (+/-) term3 (+/-) ...] !(+/-)
 * Parsing stops when after the last term (and after skipped
 * blank spaces ' ' and tab) there is a character other than '+' or '-' or end of stream.
 * At least one term is required for this parser to succeed.
 *
 * Note: This class is indirectly a self-referential to implement a recursive descent parser.
 * The class TermParserAndEvaluator is aggregated inside this one and TermParserAndEvaluator
 * also refers to or uses  another instance of this class via RefExpressionAndEvaluator.
 * Also TermParserAndEvaluator refers to or uses FunctionCall (via RefFunctionCall)
 * ,which in turn aggregates this parser for its arguments
 * Overall you can calculate arbitrary recursive expressions
 * such as 4*7*(2+3*(4+3*2)*a)*func(3+6*(3+4),7)+6*7 defined
 * with recursive nesting by round braces.
 */
template<typename Elem,	///< An arbitrary type representing a number for elements of which (Elem nl,nr)
						///< at least the following arithmetic operators are defined: in place addition
						///< nl+=nr, in place subtraction nl-=nr , in place multiplication  nl*=nr and
						///< either prefix unary operator minus -n
						///< or the following function is defined "void flip_sign_in_place(Elem& val) "
						///< You can use built-in types such as int ,int64_t or custom types,
						///< such as Matrix<int64_t> for the basic type.
						///< The type needs to be copy constructible , assignable and optionally
						///< move constructible and move assignable.
typename Action=std::function<void(Elem&&)>,
typename VariablesContainer=std::unordered_map<std::string,Elem> ,
typename FunctionsHandler=std::function<Elem(std::string&&,std::vector<Elem>&&)>
>
struct ExpressionParserAndEvaluator
{

	ExpressionParserAndEvaluator(
			const ExpressionParserAndEvaluator& other)
		:action(other.action),vars(other.vars),
		 functions_handler(other.functions_handler),
		elem(other.elem),
		cnt(other.cnt),sign(other.sign)
	{
	}

	ExpressionParserAndEvaluator(
			Elem el,///< This is only to pass the element type and relying on c++17 templates deduction, value is not used
			Action&& action,///< Action that gets triggered after a successful parse of the term with signature void(Elem&&), used to deliver result
			std::reference_wrapper<const VariablesContainer> vars =VariablesContainer { },
					///< [in] optional variables container that is compatible
					///< with std::unordered_map, supports at least look up: auto it=vars.find(std::string),
					///< vars.end() and *it resolves into reference to a value
			FunctionsHandler  functions_handler=FunctionsHandler{}
					///< [in] optional Callback/handler triggered with a signature Elem(std::string&&,std::vector<Elem>&&)
			        ///<to process a function call
	    ):action(action),vars(vars),functions_handler(functions_handler)
	{
	}



	ExpressionParserAndEvaluator(
				Elem el,//to pass the type only via passing default constructible object and depending on c++17 templates deduction
				const Action& action,
				std::reference_wrapper<const VariablesContainer> vars=VariablesContainer{},
				FunctionsHandler  functions_handler=FunctionsHandler{}
				//,ParserTraitT pt=ParserTrait<Elem>{}
	):action(action),vars(vars),functions_handler(functions_handler)
	{
	}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{
		cnt=0;
		sign='+';
		if (!parser.parse(it,ite))
			return false;
		action(std::move(elem));
		return true;
	}

 private:
	Action action;
	std::reference_wrapper<const VariablesContainer> vars;
	FunctionsHandler functions_handler;


	Elem elem;
	int cnt=0;
	char sign='+';
	std::function<void(Elem&&)> result_handler=[this](Elem&& el)
	{
		if (cnt==0) {
			if (sign=='-')
				flip_sign_in_place(el);
			elem=std::move(el);
			cnt++;
		}
		else {
			if (sign=='+')
				elem+=el;
			else
				elem-=el;
		}

	};

	std::function<void(char&&)>  sign_handler = [this](char&& sgn) { sign=sgn;};

	decltype(get_plus_minus_list_parser(sign_handler,
				TermParserAndEvaluator{Elem{},result_handler,vars,functions_handler}))
	parser =

	get_plus_minus_list_parser(sign_handler,
				TermParserAndEvaluator{Elem{},result_handler,vars,functions_handler});


};

#undef DECL_REF_EXPRESSION_PARSER_ONLY
#include "ref_expression_parser_evaluator.h"

#undef DECL_REF_FUNCTION_CALL_ONLY
#include "ref_function_call_parser.h"

#endif /* EXPRESSION_PARSER_EVALUATOR_H_ */
