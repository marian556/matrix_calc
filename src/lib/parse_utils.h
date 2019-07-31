/*
 * parse_utils.h
 *
 *  Created on: Jun 24, 2019
 *      Author: marian
 */

#ifndef PARSE_UTILS_H_
#define PARSE_UTILS_H_
#include <utility>
#include "parse_integer.h"

//Definition: Parser is any class A having either static or non-static
// function with a signature
// template <typename Iterator>
// bool A::parse(Iterator& it, Iterator ite)
// parse function exits with Iterator it moved one element after the consumed input
// regardless weather or not the parsing was successful
// and returns true if parse was successful
// there are no other criteria for class being a parser
// often the Parser will hold a functor to call after successful parsing to deliver the result of parsing
// , but this is not required. The checking parsers will not need to have a functor , the only interesting
// information they deliver via return of parse function
// The main benefit of this design is composability as you can see below

struct WhiteSpace
{

	template <typename Iterator>
	static bool parse(Iterator& it,Iterator ite)
	//all parsers should fail on (it==ite)
	{
		while ((it!=ite)&&(//std::isspace // tab,space , \n \r we need \n\r not to jump, to denote end of statement
							std::isblank // tab and space
				(static_cast<unsigned char>(*it))))
						 ++it;
		return true;
	}
};

struct Char
//we don't need to store reading. because if this succeed we know what it is
//stateless parser
{
	Char(char ch):ch(ch){}
	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite) const
	{
		if (it==ite) {
			ERR(&*it,"End of stream.");
			return false;
		}
		if (*it!=ch) {
			ERR(&*it,std::string("Expected character is '")+ch+"'");
			return false;
		}
		++it;
		return true;
	}
	char ch;
};

struct StatementEnd
//we don't need to store reading. because if this succeed we know what it is
//stateless parser
{
	StatementEnd(){}
	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{
		if (it==ite) {
			return true;//end of stream is end of statement, good
		}
		if ((*it!=';')&&(*it!='\r')&&(*it!='\n')) {
			ERR(&*it,std::string("Expected statement separator."));
			return false;
		}
		while ((it!=ite)&&((*it==';')||(*it=='\r')||(*it=='\n')||(std::isspace(static_cast<unsigned char>(*it)) )))
			++it;
		return true;
	}
};



template <typename Int = int64_t ,typename Action=std::function<void(Int&&) > >
struct IntegerWithAction
// This is example design for all other parsers.
// action/callback handler/result_suplier  is triggered
// only if the parsing of the integer was successful
// action can be supplied at construction time or later via set_action

//note: lambda with the same call signature [](int&& val) {} but used/called
//in different contexts/functions are different types
// (with enclosing context/function being part of the lambda type)
// that could lead to great number of instances of this class if used unwisely
// with lambdas in many places like this
// auto parser=IntegerWithAction{long long int(0),
//					[&result] (long long int&& val){result=std::move(val);}};
// if you want to reuse-parser in different context you need to wrap lambda in std::function
// at construction time
// auto parser=IntegerWithAction{long long int(0),std::function([&result] (long long int&& val){result=std::move(val);})};
// or
// auto parser=IntegerWithAction{long long int(0)}
// parser.set_action( [&result] (long long int&& val){result=std::move(val);})
// std::function is one level of indirection with some run-time overhead
{
	IntegerWithAction(Int ,Action&& action)://first parameter Int is only to trigger c++17 deduction guides
		action(std::move(action)){}

	IntegerWithAction(Int) {  } // deduction guides, you need to set action later

	IntegerWithAction(Int ,const Action& action):action(action){}

	IntegerWithAction( )= default;//	you get default template type int64_t
							// IntegerWithAction<int,std::function<void(int)> >  int_parser;

	void set_action(Action&& action1) {
		action=std::move(action1);
	}

	void set_action(const Action& action1) {
		action=action1;
	}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{
		int val;
		auto ret=parse_integer(it,ite,val,default_method{});//default_parse_integer_policy{});
		if (ret)
			action(std::move(val));
		return ret;
	}

	static auto& get_singleton() {
		static auto parser=IntegerWithAction{Int(0)};
		return parser;
	}

	template <typename SingletonAction>
	static void set_singleton_action(SingletonAction&& sa)
	{
		get_singleton().set_action(std::forward<SingletonAction>(sa));
	}

	Action action;
};

template<typename Action>
struct PlusOrMinus
{
	PlusOrMinus(Action&& action):action(std::move(action)){}

	PlusOrMinus(const Action& action):action(action){}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite) const
	{
		if (it==ite)
			return false;
		if ((*it!='+')&&(*it!='-'))
			return false;
		char ch=*it;
		++it;
		action(std::move(ch));
		return true;
	}
	Action action;
};

template <typename Action>
struct AlphaNumStringWithAction
{
	AlphaNumStringWithAction(Action&& action):
		action(std::move(action))
	{
	}

	AlphaNumStringWithAction(const Action& action):
			action(action)
	{
	}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite) const
	{
		Iterator itb=it;
		std::string str;
		while ((it!=ite)&&std::isalnum(static_cast<unsigned char>(*it)))
		{
		  str+=*it;
		  ++it;
		}
		if (it!=itb)
			action(std::move(str));

		return (it!=itb);
	}
	Action action;
};

template <typename Action>
struct VariableName
{
	VariableName(Action&& action):
		action(std::move(action))
	{
	}

	VariableName(const Action& action):
			action(action)
	{
	}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite) const
	{
		if (it==ite)
			return false;
		if (!std::isalpha(static_cast<unsigned char>(*it)))
			return false;
		std::string str;
		str+=*it;
		++it;
		while ((it!=ite)&&(std::isalnum(static_cast<unsigned char>(*it))||(*it=='_')))
		{
		  str+=*it;
		  ++it;
		}
		Iterator itsave=it;
		WhiteSpace::parse(it,ite);
		if (it!=ite)//look ahead
		{
			if (*it=='(')
			{
				it=itsave;
				return false;
			}
		}
		it=itsave;
		return action(std::move(str));//action can decide if parsing was successful
									//do we recognize the name?
	}
	Action action;
};

template <typename Action>
struct FunctionName
//like variable name, but we check '(' at the end and not consume it, only then is parsing success
{
	FunctionName(Action&& action):
		action(std::move(action))
	{
	}

	FunctionName(const Action& action):
			action(action)
	{
	}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite) const
	{
		if (it==ite)
			return false;
		if (!std::isalpha(static_cast<unsigned char>(*it)))
			return false;

		std::string str;
		str+=*it;
		++it;
		while ((it!=ite)&&(std::isalnum(static_cast<unsigned char>(*it))||(*it=='_')))
		{
		  str+=*it;
		  ++it;
		};
		Iterator itsave=it;
		WhiteSpace::parse(it,ite);
		if ((it==ite)||(*it!='('))//look ahead
		{
			it=itsave;
			return false;
		}
		it=itsave;
		return action(std::move(str));//action can decide if parsing was successful
									//do we recognize the name?
	}

	Action action;
};


template <typename Parser>
struct Optional
{
	Optional(Parser&& p):p(std::move(p)){}
	Optional(const Parser& p):p(p){}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite) const
	{
		Iterator itb=it;
		if (!p.parse(it,ite))
			it=itb;
		return true;
	}
	Parser p;
};

template <typename BackendParser>
struct Repeat
{
	Repeat(BackendParser&& p):p(std::move(p)){}
	Repeat(const BackendParser& p):p(p){}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{
		while ((it!=ite)&&p.parse(it,ite));
		return true;//(it==ite); we don't need to come to end to be successful, we parse prefix
	}

	BackendParser p;
};


template<typename ... Parsers>
struct Sequence  // c++ 17 trickery parameter packs and folds
{
	Sequence(Parsers&& ... parsers):
		parsers{std::move(parsers) ...}{}
	Sequence(const Parsers& ... parsers):
				parsers{parsers ...}{}

	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{// c++17 fold expressions work on variadic parameters,
		return std::apply([&it,&ite](auto&... parsers){
			return ( ... && parsers.parse(it,ite)  ) ;//unary left fold https://en.cppreference.com/w/cpp/language/fold
			//can we rely on c++ compilers to evaluate from left to right?
			//https://en.cppreference.com/w/cpp/language/operator_precedence
		}, parsers);
	}

	std::tuple<Parsers...> parsers;
};

template<typename ... Parsers>
struct Alternative  // c++ 17 trickery parameter packs and folds
{//work in progress: not working yet, not used, it does compile
	Alternative(Parsers&& ... parsers):
		parsers{std::move(parsers) ...}{}

	Alternative(const Parsers& ... parsers):
				parsers{parsers ...}{}


	template <typename Iterator>
	bool parse(Iterator& it,Iterator ite)
	{// c++17 fold expressions work on variadic parameters,

		return std::apply([&it,&ite](auto&... parsers){
			Iterator itb=it;
			return ( ... || (it=itb,parsers.parse(it,ite))  ) ;//unary left fold https://en.cppreference.com/w/cpp/language/fold
			//we always restart parser on previous fail
			//can we rely on c++ compilers to evaluate from left to right?
			//https://en.cppreference.com/w/cpp/language/operator_precedence
		}, parsers);
	}

	std::tuple<Parsers...> parsers;
};



template <typename Parser>
inline auto get_list_parser(const Parser& p,Char sep)
{//we cannot use forwarding references (including option for move &&)
// here on parser p ,because we have more than one target below
 //we just need to copy parser/ pass by reference
	Sequence additional_entry{ WhiteSpace{ },Char{sep}, WhiteSpace { }, p};
	return Sequence{WhiteSpace { },p,Repeat{additional_entry}};
}

template <typename OperatorParser,typename Parser>
inline auto get_operator_list_parser(const OperatorParser& op,const Parser& p)
//optional first operator (suitable for +/- calculation to allow for -4 + 3 expressions
// for multiplication use directly above get_list_parser as  you cannot have  *6*7* expression
{
	Sequence additional_entry{ WhiteSpace{},op,
								WhiteSpace{},p};

	return Sequence{WhiteSpace{},Optional{op},
					WhiteSpace{},p,
					Repeat{additional_entry}};
}

template <typename SignAction,typename Parser>
inline auto get_plus_minus_list_parser(SignAction&& sign_handler,Parser&& parser)
{
	return get_operator_list_parser(PlusOrMinus{std::forward<SignAction>(sign_handler)},
									std::forward<Parser>(parser));
}

template <typename Parser>
inline auto get_bracketed_list_parser(Parser&& p,char separator=',',char left_bracket='[',char right_bracket=']')
{
	return Sequence{WhiteSpace{},Char{left_bracket},WhiteSpace{},
		//comma_separated_non_empty_list_parser(std::forward<Parser>(p))
		get_list_parser(std::forward<Parser>(p),separator)
		,WhiteSpace{},Char{right_bracket}};
}

template <typename Range,typename Parser >
inline bool parse_r(const Range& r,Parser& p)
{
	auto it=std::begin(r);
	return p.parse(it,std::end(r));
}

template <typename Range,typename Parser >
inline bool parse_r(Range& r,Parser& p)
{
	using namespace this_project_utils::ranges;
	auto it =std::begin(r);
	bool ret=p.parse(it,end(r));

	using this_project_utils::ranges::move_begin_range;
	move_begin_range(r, it);
	return ret;
}

template <typename Iterator,typename Parser >
inline bool repeat_parse_r(Iterator& it,Iterator ite,Parser& p)
{

	for (;(it!=ite);)
	{
		Iterator itb=it;
		if (!p.parse(it,ite))
			return false;
		if (it==itb)//no progress, something is wrong
			return false;
	}
	return (it==ite);
}

template <typename Range,typename Parser >
inline bool repeat_parse_r(const Range& r,Parser& p)
{
	auto it=std::begin(r);
	return repeat_parse_r(it,std::end(r),p);

}

///Range is a concept defined in c++20.
///Object o is a range if and only if begin(o), end(o) form a valid range of iterators.
/// we assume older c++ standard container types (string,string_view,vector,list,span,set,map)
/// You use this function on StatementProcessor parser
/// Range can be string_view or a const C-string too, so you can call directly
/// repeat_parse_r("5*6*3",parser);
template <typename Range,typename Parser >
inline bool repeat_parse_r(Range& r,Parser& p)
{
	using namespace this_project_utils::ranges;
	auto it =std::begin(r);
	bool ret=repeat_parse_r(it,end(r),p);

	using this_project_utils::ranges::move_begin_range;
	move_begin_range(r, it);
	return ret;
}

#endif /* PARSE_UTILS_H_ */
