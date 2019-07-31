/*
 * ranges.h
 *
 *  Created on: Jun 16, 2019
 *      Author: marian
 */
//#include <experimental/ranges/range>
#ifndef RANGES_H_
#define RANGES_H_
#include <iterator>

namespace this_project_utils {
//Range is a concept defined in c++20.
//Object o is a range if and only if begin(o), end(o) form a valid range of iterators.
//we assume older c++ standard container types (string,string_view,vector,list,span,set,map) implement commonly
//member functions T::begin() and T::end and we wrap it to make it a range.
// I extend the concept of range to also support range_size,to_range and update_range.
// range_size is for optimization only. If objects store size internally,
// they can specialize/reimplement range_size and to_range

namespace ranges {

//#if __cplusplus < 201103L

#if __cplusplus >= 201103L

#if __cplusplus < 201703L
//bring c++17 functions to older compilers
template<typename RangeT>
inline  auto begin(const RangeT& r) -> decltype(std::begin(r))
{ return std::begin(r); }

template<typename RangeT>
inline  auto begin(RangeT& r) -> decltype(std::begin(r))
{ return std::begin(r); }

template<typename RangeT>
inline  auto end(const RangeT& r) -> decltype(std::end(r))
{ return std::end(r); }

template<typename T, size_t N>
inline  T* begin(T (&arr)[N]) { return arr; }

template<typename T, size_t N>
inline  T* end(T (&arr)[N]) { return arr+N; }

#endif

#else

template<typename RangeT>
typename RangeT::iterator begin(const RangeT& r) {
		return r.begin();
}

template<typename RangeT>
typename RangeT::iterator end(const RangeT& r) {
		return r.end();
}

#endif


//slow fallback function, it is preferred type-specific overload optimized
//functions are called via ADL(argument dependent Koenig lookup)
template<typename RangeT>
typename std::iterator_traits<typename RangeT::iterator>::difference_type range_size(
		const RangeT& r) {
	using namespace ranges;
	using namespace std;
	return std::distance(begin(r), end(r));
}

//slow fallback function, it is preferred type-specific overload optimized
//functions are called via ADL(argument dependent Koenig lookup)
template<typename RangeT>
void remove_prefix_range(RangeT& t, size_t n) {
	using namespace std;
	t = RangeT(begin(t) + n, range_size(t) - n);
}

//slow fallback function, it is preferred type-specific overload optimized
//functions are called via ADL(argument dependent Koenig lookup)
template<typename RangeT,typename Iterator>
void move_begin_range(RangeT& t, Iterator it) {
	using namespace std;
	t = RangeT(it, range_size(t)-std::distance(begin(t),it));
}

}

}

#endif /* RANGES_H_ */
