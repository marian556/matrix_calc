/*
 * parse_integer_generic.h
 *
 *  Created on: Jun 24, 2019
 *      Author: marian
 */


#ifndef PARSE_INTEGER_GENERIC_H_
#define PARSE_INTEGER_GENERIC_H_

#include <limits>
#include <iterator>
#include "parse_error.h"
//#include "is_char_type.h"

template<typename Char>
inline unsigned digit(Char ch) {
	return ch - '0';
}

template<typename IntegerType>
inline void fast_multiply10(IntegerType& val) {
	IntegerType tmp = val << 1;
	val <<= 3;
	val += tmp;
}

//T can be any integer type ,
// return false when accumulation is no longer possible due to the having reached the limits of integer type
template<typename T, typename Char>
bool accumulate_integer(T& val, Char ch) {
	static T const max = (std::numeric_limits<T>::max)();
	static T const max10 = max / 10;

	if (val > max10)
		return false;

	//val*=10;
	fast_multiply10(val);

	T dig = digit(ch);

	if (val > max - dig)
		return false;

	val += dig;

	return true;
}

template<typename T, typename Char>
inline bool accumulate_negative_integer(T& val, Char ch) {
	// Ensure n *= Radix will not underflow
	static T const min = (std::numeric_limits<T>::min)();
	static T const min10 = min / 10; //calculated only once at the first run
	if (val < min10)
		return false;

	//val *= 10;
	fast_multiply10(val);

	// Ensure n -= digit will not underflow
	T dig = digit(ch);
	if (val < min + dig)
		return false;

	val -= dig;

	return true;
}

//returns true if value is positive and false for negative
template<typename Iterator>
inline bool parse_sign(Iterator& it, Iterator ite) {
	bool neg = (*it == '-');
	if (neg || (*it == '+'))
		++it;
	return !neg;
}

template<typename T>
T mymin(T a, T b) {
	return a > b ? b : a;
}

template <typename Char>
inline bool is_dec_digit(Char ch)
{
	//return (ch >= '0' && ch <= '9');
	return std::isdigit(static_cast<unsigned char>(ch));
}


template <typename Iterator>
inline void skip_and_ignore_digits(Iterator& it, Iterator ite)
{
  while ((it != ite) && is_dec_digit(*it))
			++it;
}


// returns false when parsing/conversion is no longer possible due to the having reached the limits of IntegerType
template<typename Iterator, typename IntegerType>
parse_error parse_integer_generic(Iterator& it, Iterator ite, IntegerType& val) {
	if (it == ite)
		return parse_error_code::invalid_argument;

	if (!std::numeric_limits<IntegerType>::is_signed)
	{
		if (*it == '-')
			return parse_error_code::invalid_argument;
	}

	bool positive = parse_sign(it, ite);

	if (it == ite)
		return parse_error_code::invalid_argument;

	IntegerType tmp = 0;



	Iterator itb=it;
	while ((it != ite) && (*it == '0')) // I assume numbers with leading zeros are not that many
		++it;

	int n = mymin((int) std::distance(it, ite),
			std::numeric_limits<IntegerType>::digits10);
	//optimization common case: we don't need to check end of input after each character for the next n chars,
	//assuming std::distance constant complexity

	if (positive) {
		int cnt=0;
		for (; is_dec_digit(*it) // more likely to be false, so we place it first for optimization
		&& (cnt < n); ++it, ++cnt) {
			//tmp*=10;
			fast_multiply10(tmp);
			tmp += (IntegerType) digit(*it);
		}

		if ((it != ite) && is_dec_digit(*it)) {
			if (!accumulate_integer(tmp, *it))
			{
				skip_and_ignore_digits(it,ite);
				return parse_error_code::result_out_of_range;
			}
			++it;
			if ((it != ite) && is_dec_digit(*it)) { //any dec digit here cannot fit the type
				//to have consistent behaviour with std::from_chars we skip/ignore all digits now
				skip_and_ignore_digits(it,ite);
				return parse_error_code::result_out_of_range;;
			}
		}

	} else
	if (std::numeric_limits<IntegerType>::is_signed)
		// this compile time constant causes the else block to go away compile time for unsigned type IntegerType
	{

		for (int cnt = 0; is_dec_digit(*it) // more likely to be false, so we place it first
		&& (cnt < n); ++it, ++cnt) {
			//tmp*=10;
			fast_multiply10(tmp);
			tmp -= (IntegerType) digit(*it);
		}

		if ((it != ite) && is_dec_digit(*it)) {
			if (!accumulate_negative_integer(tmp, *it))
			{
				skip_and_ignore_digits(it,ite);
				return parse_error_code::result_out_of_range;
			}
			++it;
			if ((it != ite) && is_dec_digit(*it)) { //any dec digit here cannot fit the type
				skip_and_ignore_digits(it,ite);
				return parse_error_code::result_out_of_range;
			}
		}
	}
	if (it==itb)
		return parse_error_code::invalid_argument;
	val = tmp;
	return parse_error_code::success;
}

#endif /* PARSE_INTEGER_GENERIC_H_ */
