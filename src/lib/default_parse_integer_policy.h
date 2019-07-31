/*
 * default_parse_integer_policy.h
 *
 *  Created on: Jun 18, 2019
 *      Author: marian
 */

#ifndef DEFAULT_PARSE_INTEGER_POLICY_H_
#define DEFAULT_PARSE_INTEGER_POLICY_H_

template<typename method_tag>
struct method {};

struct generic {};
struct fromchars {};

#if __cplusplus >= 201703L
typedef method<fromchars> default_method;
#else
typedef method<generic> default_method;
#endif


#endif /* DEFAULT_PARSE_INTEGER_POLICY_H_ */
