/*
 * parser_traits.h
 *
 *  Created on: Jun 25, 2019
 *      Author: marian
 */

#ifndef PARSER_TRAITS_H_
#define PARSER_TRAITS_H_
#include <utility>
#include <functional>

template <typename Elem,
		typename Action ,
		typename VariablesContainer
>
struct VectorParser;

template <typename Elem>
struct arithmetic_expression
//This new type is necessary if we want to distinguish between parsing an Elem directly
//and parsing an arithmetic_expression with the result of type Elem.

{
	arithmetic_expression()
		:result{}{};

	arithmetic_expression(Elem&& el)
		:result(std::move(el)){}

	arithmetic_expression(const Elem& el):
		result(el){}

	inline operator Elem() const {
		return result;
	}

	Elem result;
};

template <typename Integer>
struct ParserTrait
{
	template <typename Action>
	static auto get_parser(Action&& action) {
		return IntegerWithAction{Integer{},std::forward<Action>(action)};
	}

	template <typename Action,typename VariablesContainer>
	static auto get_parser(Action&& action,std::reference_wrapper<VariablesContainer>) {
		return IntegerWithAction{Integer{},std::forward<Action>(action)};
	}

	static auto get_parser() {
		return IntegerWithAction{Integer{}};
	}
};

template <typename Elem>
struct ParserTrait<std::vector<Elem> >
{
	template <typename Action>
	static auto get_parser(Action&& action) {
		return VectorParser{Elem{},std::forward<Action>(action)};
	}

	template <typename Action,typename VariablesContainer>
	static auto get_parser(Action&& action,std::reference_wrapper<VariablesContainer> vars) {
		return VectorParser{Elem{},std::forward<Action>(action),vars};
	}

	static auto get_parser() {
		return VectorParser{Elem{}};
	}
};


template <typename Elem>
struct ParserTrait<Matrix<Elem> >
{
	template <typename Action>
	static auto get_parser(Action&& action) {
		return VectorParser{std::vector<Elem>{},std::forward<Action>(action)};
	}
	template <typename Action,typename VariablesContainer>
	static auto get_parser(Action&& action,std::reference_wrapper<VariablesContainer> vars) {
		return VectorParser{std::vector<Elem>{},std::forward<Action>(action),vars};
	}
};

template<typename Elem,typename Action,typename VariablesContainer,typename FunctionsHandler >
struct ExpressionParserAndEvaluator;

template <typename Elem>
struct ParserTrait<arithmetic_expression<Elem> >
{
	template <typename Action>
	static auto get_parser(Action&& action) {
		return ExpressionParserAndEvaluator{Elem{},std::forward<Action>(action)};
	}
	template <typename Action,typename VariablesContainer>
	static auto get_parser(Action&& action,std::reference_wrapper<VariablesContainer> vars) {
		return ExpressionParserAndEvaluator{Elem{},std::forward<Action>(action),vars};
	}
};


#endif /* PARSER_TRAITS_H_ */
