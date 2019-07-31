//============================================================================
// Name        : Matrix_calc3.cpp
// Author      : MK
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string_view>

#include "statement_processor.h"

#define MATRIX_TYPE 1

int main(int argc,char* argv[]) {

	typedef int64_t Elem;
#ifdef	MATRIX_TYPE
	typedef Matrix<Elem> value_type;
#else
	typedef Elem value_type;
#endif

	std::unordered_map<std::string,value_type > vars;

	std::ostream& myerr=std::cerr;
//	std::ostream& myerr=std::cout;

	std::function<value_type(std::string&& ,std::vector<value_type>&&)>
	functions_handler
				= [&myerr](std::string&& fname,std::vector<value_type>&& args)
	{
		if (fname=="print")
		{
			int cnt=0;
			for(auto& arg:args)
			{
				myerr << arg;
				if (cnt++!=(int)args.size()-1)
					myerr << ",";
				myerr << std::flush;
			}
			value_type{};
		}
		else
		if (fname=="sum_square")
		{
			std::remove_reference<decltype(args[0])>::type sum{};
			for(auto& val:args)
			{
				auto val2=val;
				val2*=val;
				sum+=val2;
			}
			return sum;
		}
#ifdef MATRIX_TYPE
		else
		if (fname=="det")
		{
			if (args.size()!=1)
			{
				myerr << "det function takes exactly one matrix.";
				return value_type{};
			}
			if (!is_square(args[0]))
			{
				myerr << "det function needs a square matrix.";
				return value_type{};
			}
			Elem val=det(args[0]);
			value_type out;
			out=val;
			return out;// we return value in Matrix 1x1

		} else if (fname=="transpose")
		{
			args[0].transpose();
			return std::move(args[0]);
		}
#endif
		return value_type{};
	};

	StatementProcessor eval_parser(value_type{},ref(vars),functions_handler);

	using std::string_view;
	string_view sv;
	for (std::string line; std::getline(std::cin, line); ) {
		try{
			sv=line;
			if (!repeat_parse_r(sv,eval_parser))
			{
				myerr << "Parsing failed. Unprocessed input:" << std::string(sv) << std::endl;
			}
		}
		catch(const parse_exception& exc)
		{
			myerr << "Parse exception:" << exc.what() << std::endl;
		}
		catch(const matrix_exception& exc)
		{
			myerr << "Matrix exception:" << exc.what() << std::endl;
		}
		catch(const std::exception& exc)
		{
			myerr << "Exception:" << exc.what() << std::endl;
		}
		catch(...)
		{
			myerr << "Uncaught exception."  << std::endl;
		}

	}


	return 0;
}
