/*
 * statement.h
 *
 *  Created on: Jun 27, 2019
 *      Author: marian
 */

#ifndef STATEMENT_PROCESSOR_H_
#define STATEMENT_PROCESSOR_H_

#include <functional>
#include "expression_parser_evaluator.h"

template<typename Elem>
struct NoFunctionsHandler {
	NoFunctionsHandler() {
	}
	;

	NoFunctionsHandler (Elem) { };

	typedef std::function<Elem(std::string&&, std::vector<Elem>&&)> type;

	Elem operator()(std::string&&, std::vector<Elem>&&) {
		return Elem { };
	}
};

///This class either process assignment or prints expression to output whichever occurs on input.
/**
 * This is a generic class written in c++17 which is
 * working with arbitrary arithmetic number type Elem supporting elementary
 * arithmetic operation (built in types, or custom types, such as Matrices)
 * In detail elements of types Elem  (Elem nl,nr) support
	at least the following arithmetic operators are defined: in place addition,subtraction,multiplication and prefix -:
	 @code nl+=nr; nl-=nr;  nl*=nr; -nl ;@endcode
	 Alternatively to the prefix minus operator the
	 following function needs to be defined  @code void flip_sign_in_place(Elem& val) @endcode
	 to support efficient operations.
	 Note division is not required.
	 You can use built-in types such as int ,int64_t or custom types,
	 such as Matrix<int64_t> for the basic type.
	 The type needs to be copy constructible , assignable and optionally
	 move constructible and move assignable.

 * This class parses input and either detects assignment of the form
 * @code
 * varname = <arithmetic expression>
 * or
 * <arithmetic expression>
 * @endcode
 * In case of assignment the class updates the variable table, in case of expression, it prints
 * expression to output.
 * Statement is delimited either by ';' or by end of input whichever comes first.
 * Arithmetic expression is any valid expression consisting of
 * the number, operations '+','-','*' , any number of nested round braces (potentially nested), variable names
 *  and function calls of any number of arguments each of it can be the expression itself .
 *  @code
 *  - 2 * ( 3-2*(2+7*k3)-2)* (4-3*2)+sum_square(2 +4*2,3,4) + (2-3*2)*2
 *  @endcode
 * blank space (' ') and tabs anywhere in between are ignored.
 * This class uses recursive descent parser to calculate expression.
 * you use this class with the function repeat_parse_r:
 * @code
 * StatementProcessor parser{..args..};
 * string_view sv="5*5*3+3";
 * bool success=repeat_parse_r(input,parser);
 * @endcode
 */
template<	typename Elem
			,typename VariablesContainer = std::unordered_map<std::string, Elem>,
			typename FunctionsHandler = std::function<Elem(std::string&&, std::vector<Elem>&&)>
		>
struct StatementProcessor {
	StatementProcessor(
			Elem el, ///< to pass the type only via passing default constructible object and depending on c++17 templates deduction
			std::reference_wrapper<VariablesContainer>
			vars = VariablesContainer { }
				///< [in] optional variables container that is compatible
				///< with std::unordered_map, supports at least look up: auto it=vars.find(std::string),
				///< vars.end() and *it resolves into reference to a value
			,FunctionsHandler functions_handler =FunctionsHandler { }
				///< [in] optional Callback/handler triggered with a signature Elem(std::string&&,std::vector<Elem>&&)
				///<to process a function call
			, std::ostream& ostr = std::cout
				///< [out] optional output stream, standard output is by default, you can redirect it to
				///< any other stream, such as stringstream

			) :
			vars(vars), functions_handler(
					functions_handler), ostr(ostr) {

	}

	/// A main parse function.
	/// This project definition of a parser is any class  having a member
	/// function with the signature of this function.
	/// parse function exits with Iterator it moved one element after the consumed input
	/// regardless weather or not the parsing was successful
	/// and returns true if parse was successful
	/// there are no other criteria for class being a parser
	/// often the Parser will hold a functor to call after successful
	/// parsing to deliver the result of parsing, but this is not required.
	/// The checking parsers will not need to have a functor , the only interesting
	/// information they deliver via return of parse function
	/// The main benefit of this design is to arbitrarily compose existing parsers into
	/// blocks or aggregates.
	template<typename Iterator>
	bool parse(Iterator& it, Iterator ite) {
		return parser.parse(it, ite);
		//no action for top level
	}

private:
	std::reference_wrapper<VariablesContainer> vars;
	FunctionsHandler functions_handler;
	std::ostream& ostr;

	std::string lhs_var_name;
	std::function<bool(std::string&&)> lhs_variable_handler =
			[this](std::string&& varname)
			{
				lhs_var_name=std::move(varname);
				return true;
			};

	std::function<void(Elem&&)> rhs_expr_handler = [this](Elem&& val)
	{
		vars.get()[lhs_var_name]=val;
	};

	std::function<void(Elem&&)> expr_handler = [this](Elem&& val)
	{
		ostr << val << std::endl;
	};

//shame we cannot use auto here
	decltype(Sequence {WhiteSpace {},VariableName {lhs_variable_handler},WhiteSpace {},
				Char('='), //WhiteSpace{}, expression parser skips at beginning
				ExpressionParserAndEvaluator {Elem {},rhs_expr_handler,std::cref(vars),functions_handler},WhiteSpace {},
				StatementEnd {}})

	assignment_parser

	= Sequence { WhiteSpace { }, VariableName { lhs_variable_handler },
			WhiteSpace { }, Char('='), //WhiteSpace{}, expression parser skips at beginning
			ExpressionParserAndEvaluator { Elem { }, rhs_expr_handler,
					std::cref(vars), functions_handler }, WhiteSpace { },
			StatementEnd { } };

	decltype(Alternative {
				assignment_parser,
				Sequence {ExpressionParserAndEvaluator {Elem {},expr_handler,
						std::cref(vars),functions_handler},WhiteSpace {},
					StatementEnd {}}})

	parser

	= Alternative { assignment_parser,
			Sequence { ExpressionParserAndEvaluator { Elem { }, expr_handler,
					std::cref(vars), functions_handler }, WhiteSpace { },
					StatementEnd { } } };

};

#endif /* STATEMENT_PROCESSOR_H_ */
