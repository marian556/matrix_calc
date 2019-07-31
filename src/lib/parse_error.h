/*
 * parse_error.h
 *
 *  Created on: Jun 16, 2019
 *      Author: marian
 */

#ifndef PARSE_ERROR_H_
#define PARSE_ERROR_H_

#include <exception>
#include <string>
#include <list>

#if __cplusplus >= 201100L
enum struct parse_error_code {
	success = 0, no_error = 0, unknown_error=1,invalid_argument = 2, result_out_of_range = 3
};
#endif

template<int>
struct parse_error_ {

#if __cplusplus >= 201100L

	typedef parse_error_code scope_error_code;
	typedef parse_error_code error_code_type;
#else
	typedef parse_error_ scope_error_code;
	enum code_type {
		success = 0, no_error = 0, unknown_error=1,invalid_argument = 2, result_out_of_range = 3
	};

	typedef code_type error_code_type;
#endif


	parse_error_(const parse_error_& other) : code(other.code) {
	}

	parse_error_(bool b)
	  :code(b ? scope_error_code::success : scope_error_code::unknown_error){
	}

	parse_error_(error_code_type error_code=scope_error_code::no_error) :
			code(error_code) {
	}

	operator bool() const {
		return code == scope_error_code::success;
	}

	parse_error_& operator = (const parse_error_& other)
	{
		code=other.code;
		return *this;
	}
	bool operator ==(error_code_type error_code) const{
		return (code==error_code);
	}

	const char* to_cstring() const {
		static const char *msgtab[] = { "No Error", "Unknown Error", "Invalid argument",
				"Result out of range" };
		return msgtab[static_cast<int>(code)];
	}

	error_code_type code;
};

typedef parse_error_<0> parse_error;
typedef parse_error_<0>::scope_error_code parse_error_code;

struct parse_exception:std::exception
{
	parse_exception(const std::string& msg=""):msg(msg){}
	virtual const char* what() const throw()
	{
		return msg.c_str();
	}
	~parse_exception() throw()
	{}
	std::string msg;
};


// We cannot just halt program when parser fails.
// It can be an alternative, so next parser may succeed.
// 12345*m2 ,non-digit number is not a problem , we have a variables now
// when error propagates to the highest level, then we can display log.
template <int i=0>
struct Log
{
	std::list<std::pair<const char*,std::string>> messages;
	void add(const char* location,std::string&& msg)
	{
		messages.push_back(std::pair{location,std::move(msg)});
	}
	void add(const char* location,const std::string& msg)
	{
		messages.push_back(std::pair{location,msg});
	}
	bool is_enabled=true;
};

Log<>& log()
{
	static Log<> log=Log<>{};
	return log;
}

bool is_logging()
{
	return log().is_enabled;
}

void error(const char* location,std::string&& msg)
{
	log().add(location,msg);
}
#define ERR(LOC,MSG)   if (is_logging()) error(LOC,MSG)

#endif /* PARSE_ERROR_H_ */
