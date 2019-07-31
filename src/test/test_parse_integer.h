/*
 * test_parse_integer.h
 *
 *  Created on: Jun 16, 2019
 *      Author: marian
 */

#ifndef TEST_PARSE_INTEGER_H_
#define TEST_PARSE_INTEGER_H_

#include "parse_integer.h"
#include <string_view>
//#include "string_view.h"
#include "test_tools.h"
using std::string_view;

template<typename methodT >
void test_LL_parse_integer(void) {
	long long val = 0;
	parse_error pe;
	//about 18 dec digits: 2^63= (2^10)^6 = (1024) ^ 6:   3*6 digits
	TT(
				parse_integer("1234567890123456789a", val, methodT())
						&& (val == 1234567890123456789LL));

	TT(
			parse_integer(string_view("1234567890123456789a"), val, methodT())
					&& (val == 1234567890123456789LL));

	TT(
			parse_integer(
					string_view(
							"00000000000000000000000001234567890123456789a"),
					val, methodT()) && (val == 1234567890123456789LL));

	//	assert(parse_integer(string_view("+1234567890123456789a"),val,methodT())
	//							&&(val==1234567890123456789LL));

	//max
	TT(
			parse_integer(string_view("9223372036854775807"), val, methodT())
					&& (val == 9223372036854775807LL));

	//max+1, val is unchanged from previous step
	TT(
			(!(pe=parse_integer(string_view("9223372036854775808"), val, methodT())))
					&& (val == 9223372036854775807LL) && (pe==parse_error_code::result_out_of_range));

	TT(
				(!(pe=parse_integer(string_view("aa9223372036854775808"), val, methodT())))
						&& (val == 9223372036854775807LL) && (pe==parse_error_code::invalid_argument));

	//min
	TT(
			parse_integer(string_view("-9223372036854775808"), val, methodT())
					&& (val == -9223372036854775807LL - 1));
	// this above gives warning when you use directly the valid min value,
	// because the g++ compiler parses sign - and value and suffix LL as sepparate tokens
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52661

	TT(
			parse_integer(
					string_view(
							"-00000000000000000000000009223372036854775808"),
					val, methodT()) && (val == -9223372036854775807LL - 1));

	string_view sv("-9223372036854775808fsomething");
	TT(
			parse_integer(sv, val, methodT())
					&& (val == -9223372036854775807LL - 1)
					&& (std::string(sv.data()) == "fsomething"));

	sv = "-9223372036854775808044444444fsomething";
		TT(
				!(pe=parse_integer(sv, val, methodT()))
				&&(pe==parse_error_code::result_out_of_range)
						&& (val == -9223372036854775807LL - 1) //old value, val is not touched
						&& (std::string(sv.data()) == "fsomething"));//we test moving string in case conversion fails
}

template<typename methodT  >
void test_ULL_parse_integer(void) {
	unsigned long long val = 0;
	parse_error pe;

	//about 18 dec digits: 2^63= (2^10)^6 = (1024) ^ 6:   3*6 digits
	TT(
			parse_integer(string_view("12345678901234567890ab"), val, methodT())
					&& (val == 12345678901234567890ULL));

	TT(
			parse_integer(
					string_view(
							"000000000000000000000000012345678901234567890a"),
					val, methodT()) && (val == 12345678901234567890ULL));


	//max
	TT(
			parse_integer(string_view("18446744073709551615"), val, methodT())
					&& (val == 18446744073709551615ULL));

	//max+1, val is unchanged from previous step
	TT(
			!parse_integer(string_view("18446744073709551616adam"), val, methodT())
					&& (val == 18446744073709551615ULL));

	TT(
				!(pe=parse_integer(string_view("0000000000000000000000000000184467440737095516150adam"), val, methodT()))
				&& (pe==parse_error_code::result_out_of_range)
				&& (val == 18446744073709551615ULL));

	//reading negative number for unsigned type will fail with invalid_argument
	TT(
					(!(pe=parse_integer(string_view("-00000018adam"), val, methodT())))
							&& (pe==parse_error_code::invalid_argument)
							&&(val == 18446744073709551615ULL));

	string_view sv("000000000000000000000000000018446744073709551615fsomething");
	TT(
			parse_integer(sv, val, methodT())
					&& (val == 18446744073709551615ULL)
					&& (std::string(sv.data()) == "fsomething"));
}


template <int i>//to avoid multiple definition warnings
void test_parse_integer(void) {

	long long val = 123456789012345678LL;
	fast_multiply10(val);
	TT(val == 1234567890123456780LL);

	//my own implementation
	test_LL_parse_integer<method<generic> >();
	test_ULL_parse_integer<method<generic> >();


#if __cplusplus >= 201703L
	test_LL_parse_integer<method<fromchars> >();
	test_ULL_parse_integer<method<fromchars> >();
#endif

	int ival=0;
	//default version without method parameter
	TT(parse_integer(string_view("-00123adam"), ival)&&(ival==-123));

}


#endif /* TEST_PARSE_INTEGER_H_ */
