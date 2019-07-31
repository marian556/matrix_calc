/*
 * parse_integer.h
 *
 *  Created on: Jun 15, 2019
 *      Author: marian
 */

#ifndef PARSE_INTEGER_H_
#define PARSE_INTEGER_H_

#include <cstddef>
#include <iterator>
#include <type_traits>

#if __cplusplus >= 201703L
#include <charconv>
#endif

//#include "utils.h"
#include "parse_integer_generic.h"
#include "parse_error.h"
#include "ranges.h"
#include "default_parse_integer_policy.h"


//const method
// StringView/Range of characters can be either std::string_view ( with: g++ -std=c++17  ...)
//or always this_project_utils::string_view or any other range of characters

template<typename Iterator, typename IntegerType>
inline parse_error parse_integer(Iterator& it, Iterator ite, IntegerType& val,
		method<generic>) {
	return parse_integer_generic(it, ite, val);
}

template<typename Iterator, typename IntegerType>
inline parse_error parse_integer(const Iterator& it, Iterator ite,
		IntegerType& val, method<generic>) {
	Iterator tmp = it;
	return parse_integer_generic(tmp, ite, val);
}

template<typename Range, typename IntegerType,typename Method>
inline parse_error parse_integer(const Range& r, IntegerType& val, Method m) {
	//using namespace this_project_utils::ranges;
	using namespace std;
	return parse_integer(begin(r),end(r), val,m);
}

template<typename Range, typename IntegerType,typename Method>
inline parse_error parse_integer(Range& r, IntegerType& val, Method m) {
	using namespace this_project_utils::ranges;
	using namespace std;
	auto it = begin(r);
 	parse_error ret = parse_integer(it,end(r), val,m);

	//using this_project_utils::ranges::move_begin_range;
	//this allows for Koenig ADL (argument dependent lookup) when
	//optimized version version of the function is specified in the same naspace as the
	//container/Range type definition
	// also it falls back to non-optimized generic variant, when the optimized function is absent
 	using namespace this_project_utils::ranges;
	move_begin_range(r, it);

	return ret;
}

#if __cplusplus >= 201703L
//c++17
//std::string_view requires c++17, we have native this_project_utils::string_view
//std::from_chars and std::stoll requires c++17


parse_error errc2pe(std::errc ec)
{
	if (ec == std::errc::invalid_argument)
		return parse_error_code::invalid_argument;

	if (ec == std::errc::result_out_of_range)
		return parse_error_code::result_out_of_range;

	return (ec == std::errc(0)) ?
			parse_error_code::success : parse_error_code::unknown_error;//no error
}

template<typename Iterator, typename IntegerType>
inline parse_error parse_integer(const Iterator& it, Iterator ite,
		IntegerType& val, method<fromchars>) {
	return errc2pe(std::from_chars(&*it, &*ite, val).ec);
}

template<typename Iterator, typename IntegerType>
inline parse_error parse_integer(Iterator& it, Iterator ite, IntegerType& val,
		method<fromchars>) {
	Iterator itb = it;
	std::from_chars_result res=std::from_chars(&*it, &*ite, val);
	std::advance(it, res.ptr - &*itb);
	return errc2pe(res.ec);
}

#endif
//end of c++17
/*
template<typename Iterator, typename IntegerType>
inline parse_error parse_integer(const Iterator& it, Iterator ite,
		IntegerType& val) {
	return parse_integer(it, ite, val, default_method { });
}

template<typename Iterator, typename IntegerType>
inline parse_error parse_integer(Iterator& it, Iterator ite, IntegerType& val) {
	return parse_integer(it, ite, val, default_method { });
}*/

template<typename Range, typename IntegerType>
inline parse_error parse_integer(const Range& r, IntegerType& val) {
	return parse_integer(r, val, default_method { });
}

template<typename Range, typename IntegerType>
inline parse_error parse_integer(Range& r, IntegerType& val) {
	return parse_integer(r, val, default_method { });
}
/*
template<typename Iterator, typename IntegerType,typename Policy=default_parse_integer_policy>
inline ENABLE_IF_INTEGRAL(parse_error) parse( Iterator& it, Iterator ite, IntegerType& val,Policy p=default_parse_integer_policy{}) {
	return parse_integer(it, ite, val, typename Policy::parse_integer_method{});
}

template<typename Range, typename IntegerType,typename Policy=default_parse_integer_policy>
inline ENABLE_IF_INTEGRAL(parse_error) parse(const Range& r, IntegerType& val,Policy p=default_parse_integer_policy{}) {
	return parse_integer(r, val, typename Policy::parse_integer_method{});
}*/

#endif /* PARSE_INTEGER_H_ */

