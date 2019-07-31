/*
 * tests.h
 *
 *  Created on: Jun 24, 2019
 *      Author: marian
 */

#ifndef TESTS_H_
#define TESTS_H_

#include <unordered_map>
#include <string_view>
#include <functional>
#include <utility>
#include <sstream>
#include "test_tools.h"
#include "test_parse_integer.h"
#include "expression_parser_evaluator.h"
#include "function_call_parser.h"
#include "statement_processor.h"
#include "test_tools.h"

using std::string_view;

template<typename Key,typename Value>
struct my_map
{

	my_map(){};
	my_map(const my_map&) = delete;
	my_map(my_map&&) = delete;
	my_map& operator =(const my_map& ) = delete;
	my_map& operator =(my_map&& ) = delete;

	auto find( const Key& key )
	{
		return data.find(key);
	}
	auto find( const Key& key ) const
	{
		return data.find(key);
	}
	auto end() noexcept
	{
		return data.end();
	}
	auto end() const noexcept
	{
		return data.end();
	}

	Value& operator[]( const Key& key )
	{
		return data[key];
	}

	Value& operator[]( Key&& key )
	{
		return data[std::move(key)];
	}

	std::unordered_map<Key,Value> data;
};


void tests()
{

    bool on=true;//flip (on) to (!on) for the test you want to focus on and set this variable to false
	test_parse_integer<0>();



    if (on)
    {
    	Matrix<int> m;
    	TT (( m={{1,2},{3,4}},m.det()==-2 ));
    	TT (( m={{1,2,3},{4,5,6},{7,8,9}},m.det()==0));
    	TT (( m={{2,-1,5},{3,2,1},{4,2,1}},m.det()==-11));
    	TT (( m={{4,7,5,2,1},{3,2,1,4,3},{2,1,3,2,5},{1,2,3,4,5}
    				,{3,2,1,5,-2}},m.det()==832));
    }
    if (on)
	{


		int result1=0;

		IntegerWithAction  p;

		p.set_action([&result1](int&& val){result1=val;});
		TT(parse_r("12345678",p)&&(result1==12345678))

		int result2=0;

		p.set_action([&result2](int&& val){result2=val;});
		//we can re-use the same instance of parser with the changed callback function
		TT(parse_r("45678",p)&&(result2==45678))


	}

    if (on)
	{


		int result1=0;

//		IntegerWithAction  p;

		//auto result_getter=[&result1](int&& val){result1=val;};

		IntegerWithAction<>::set_singleton_action([&result1](int&& val){result1=val;});

//		p.set_action([&result1](int&& val){result1=val;});
		TT(parse_r("12345678",IntegerWithAction<>::get_singleton())&&(result1==12345678))

		int result2=0;

		IntegerWithAction<>::set_singleton_action([&result2](int&& val){result2=val;});

//		p.set_action([&result2](int&& val){result2=val;});
		//we can re-use the same instance of parser with the changed callback function
		TT(parse_r("45678",IntegerWithAction<>::get_singleton())&&(result2==45678))


	}

    if (on)
	{


		int result1=0;

		//auto result_getter=[&result1](int&& val){result1=val;};

		IntegerWithAction  p{int64_t(0),[&result1](int&& val){result1=val;}};

//		p.set_action([&result1](int&& val){result1=val;});
		TT(parse_r("12345678",p)&&(result1==12345678))


	}

	//return ;
	//test_parse_integer<0>();
    if (on)
	{
		string_view input;
		std::vector<int> result;
		VectorParser parser(0,
								[&result](std::vector<int>&& res){ result=res;}
						);
		TT(( input="  [ 3, -4 , 7 , 8 ] adam", parse_r(input,parser)
				&&(input==" adam")&&(result.size()==4)&&
				(result[0]==3)&&(result[1]==-4)&&
				(result[2]==7)&&(result[3]==8)) ) ;
	//	std::cout << "hi" << result;
	}

    if (on)
	{
    	//test for parsing vector of int inside round braces
		string_view input;
		std::vector<int> result;
		std::unordered_map<std::string, int> vars;
		VectorParser parser(0,
								[&result](std::vector<int>&& res){ result=res;}
								,cref(vars),'(',')'
						);
		TT(( input="  ( 3, -4 , 7 , 8 ) adam", parse_r(input,parser)
				&&(input==" adam")&&(result.size()==4)&&
				(result[0]==3)&&(result[1]==-4)&&
				(result[2]==7)&&(result[3]==8)) ) ;
		//std::cout << result;
	}

    if (on)
	{
    	//test for parsing vector of int inside round braces
		string_view input;
		std::vector<int> result;
		std::unordered_map<std::string, int> vars;
		VectorParser parser(arithmetic_expression<int>{},
								[&result](std::vector<int>&& res){ result=res;}
								,cref(vars),'(',')'
						);
		TT((vars["a"] = 3, input="  ( 2+1, -a-1 , 7 , 8 ) adam", parse_r(input,parser)
				&&(input==" adam")&&(result.size()==4)&&
				(result[0]==3)&&(result[1]==-4)&&
				(result[2]==7)&&(result[3]==8)) ) ;
		//std::cout << result;
	}


    if (on)
   	{
   		std::unordered_map<std::string, int> vars;

   		std::vector<int> args_result;
   		std::string fname_result;
   		//auto result_getter = [&result](int val) {result=val;};
   		auto function_call_handler = [&fname_result,&args_result](std::string&& fname,std::vector<int>&& args)
		{
   			fname_result=std::move(fname);
   			args_result=std::move(args);
   			return true;//accepted. Parser succeeded after consuming both fname and args
		};

   		string_view input;
   		//bool success=false;
   		RefFunctionCall eval_parser(int{},
   				function_call_handler,
				std::function<bool(std::string&&)>{AllFunctionNames{}},std::cref(vars));

   		TT(
   				(vars["a"] = 3, input = "pow ( 3+a , 2 ) adam",
   						parse_r(input,eval_parser) &&
						(fname_result == "pow") &&
						(args_result.size()==2)&&(args_result[0] == 6)&&(args_result[1] == 2)&&
						(input == " adam")));
//   		std::cout << "here" << std::endl;

   	}

    if (on)
	{
		std::unordered_map<std::string, int> vars;

		int result = 0;
		auto result_getter = [&result](int val) {result=val;};
		string_view input;
		std::function<int(std::string&& ,std::vector<int>&&)> functions_handler = [](std::string&& fname,std::vector<int>&& args)
		{
			if (fname=="sum_square")
			{
				int sum=0;
				for(int val:args)
					sum+=val*val;
				return sum;
			} else if (fname=="print")
			{
				for(int val:args)
					std::cout << val << "," ;
				std::cout << std::flush;
			}
			return 0;
		};
		TermParserAndEvaluator eval_parser( (int)0
				//arithmetic_expression<int>{}
		, result_getter, std::cref(vars),
		functions_handler
		);

		TT(
				(vars["a"] = 2, input = " 2 * sum_square( 1+a ,5,6) * 5 + adam", parse_r(input,
						eval_parser) && (result == 700) && (input == "+ adam")));


	}


	if (on) {
		std::unordered_map<std::string, int> vars;

		std::vector<int> args_result;
		std::string fname_result;
		//auto result_getter = [&result](int val) {result=val;};
		auto function_call_handler =
				[&fname_result,&args_result](std::string&& fname,std::vector<int>&& args)
				{
					fname_result=std::move(fname);
					args_result=std::move(args);
					return true; //accepted. Parser succeeded after consuming both fname and args
				};

		string_view input;
		//bool success=false;
		FunctionCall eval_parser(int(0), function_call_handler,
				AllFunctionNames { }, std::cref(vars));

		TT(
				(vars["a"] = 3, input = "pow ( 2+1 , 2 ) adam", parse_r(input,
						eval_parser) && (fname_result == "pow")
						&& (args_result.size() == 2) && (args_result[0] == 3)
						&& (args_result[1] == 2) && (input == " adam")));
//		std::cout << "here" << std::endl;

	}

    if (on)//this test the correct fail to detect function, because of missing '('
   	{
   		std::unordered_map<std::string, int> vars;

   		std::vector<int> args_result;
   		std::string fname_result;
   		//auto result_getter = [&result](int val) {result=val;};
   		auto function_call_handler = [&fname_result,&args_result](std::string&& fname,std::vector<int>&& args)
		{
   			fname_result=std::move(fname);
   			args_result=std::move(args);
   			return true;//accepted. Parser succeeded after consuming both fname and args
		};

   		string_view input;
   		//bool success=false;
   		FunctionCall eval_parser((int) 0, std::move(function_call_handler),
   				std::function<bool(std::string&&)>{AllFunctionNames{}},std::cref(vars));

   		TT(
   				(vars["a"] = 3, input = "pow = 3 adam",
   						(!parse_r(input,eval_parser)) &&
						(fname_result == "") &&
						(args_result.size()==0)&&
						(input == " = 3 adam")));

   	}


    if (on)
	{
    	//test for parsing vector of int inside round braces
		string_view input;
		std::vector<int> result;
		std::unordered_map<std::string, int> vars;
		VectorParser parser(0,
								[&result](std::vector<int>&& res){ result=res;}
								,cref(vars),'(',')'
						);
		TT(( input="  ( 3, -4 , 7 , 8 ) adam", parse_r(input,parser)
				&&(input==" adam")&&(result.size()==4)&&
				(result[0]==3)&&(result[1]==-4)&&
				(result[2]==7)&&(result[3]==8)) ) ;
		//std::cout << result;
	}


    if (on)
	{
		string_view input;
		std::vector<std::vector<int> > result;
		auto getter=[&result](std::vector<std::vector<int> > && res){ result=res;};
		VectorParser parser(std::vector<int>{},getter

						);
		TT(( input=" [ [ 3, -4] ,[ 7 , 8 ] ] adam",
				parse_r(input,parser) )&&(input==" adam")
				&&(result.size()==2)
				&&(result[0].size()==2)&&(result[1].size()==2)
				&&(result[0][0]==3)&&(result[0][1]==-4)
				&&(result[1][0]==7)&&(result[1][1]==8)) ;
	}



    if (on)
	{
		std::unordered_map<std::string, int> vars;

		int result = 0;
		auto result_getter = [&result](int val) {result=val;};
		string_view input;
		//bool success=false;
		TermParserAndEvaluator eval_parser((int) 0, result_getter, std::cref(vars));

		TT(
				(vars["a"] = 3, input = " 2 * a * 5 + adam", parse_r(input,
						eval_parser) && (result == 30) && (input == "+ adam")));

	}


    if(on)
	{
		std::unordered_map<std::string, int> vars;

		int result = 0;
		auto result_getter = [&result](int val) {result=val;};
		string_view input;
		TermParserAndEvaluator eval_parser((int) 0, result_getter, cref(vars));
//		bool exc = true;

		TT((input = " 2 * a * 5 + adam", ([&]() {bool exc=false;
			try {
				parse_r(input,eval_parser);
			}
			catch(parse_exception& pe)
			{

				exc=(std::string(pe.what())=="variable 'a' not defined yet.");
			};
			return exc;}())));

	}

    if (on)
	{
		std::unordered_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,std::function<void(int&&)>(result_getter),cref(vars));

		TT((input=" - 2 * 3* 5 + 7*3*2 adam", parse_r(input,eval_parser)&&(result==12)&&(input=="adam")));
	}

    if(on)
	{
		std::unordered_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

		TT((vars["a4"]=3,input=" - 2 * a4* 5 + 7*3*2 adam", parse_r(input,eval_parser)&&(result==12)&&(input=="adam")));
	}


    if(on)
	{

		//custom container
		my_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

		TT((vars["a4"]=3,input=" - 2 * a4* 5 + 7*3*2 adam", parse_r(input,eval_parser)&&(result==12)&&(input=="adam")));
	}

    if(on)
	{
		std::unordered_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

		TT((input=" - 2 * ( 53-5*10)* 5 + 7*3*2 adam", parse_r(input,eval_parser)&&(result==12)&&(input=="adam")));

	}

    if(on)
	{
		std::unordered_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

		TT((input=" - 2 * ( 3-2*7)* (4-3*2) + (2-3*2)*2 adam", parse_r(input,eval_parser)&&(result==-52)&&(input=="adam")));

	}

    if(on)
	{
		std::unordered_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

		TT((input=" - 2 * ( 3-2*(2+7*8)-2)* (4-3*2) + (2-3*2)*2 adam", parse_r(input,eval_parser)&&(result==-468)&&(input=="adam")));
	}

    if(on)
	{
		//custom container
		std::unordered_map<std::string,int> vars;

		int result=0;
		auto result_getter=[&result](int val){result=val;};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

		TT((vars["a4"]=10,input=" - 2 * ( 53-5*a4)* 5 + 7*3*2 adam", parse_r(input,eval_parser)&&(result==12)&&(input=="adam")));
	}

    if(on)
	{
			std::unordered_map<std::string,int> vars;

			int result=0;
			auto result_getter=[&result](int val){result=val;};
			string_view input;
			//bool success=false;
			//IntegerWithAction iwas{0,empty};
			ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars));

			TT((vars["k3"]=8,input=" - 2 * ( 3-2*(2+7*k3)-2)* (4-3*2) + (2-3*2)*2 adam", parse_r(input,eval_parser)&&(result==-468)&&(input=="adam")));
	}

    if(on)
	{//working
			std::unordered_map<std::string,int> vars;

			int result=0;
			auto result_getter=[&result](int val){result=val;};
			string_view input;
			//bool success=false;
			//IntegerWithAction iwas{0,empty};
			std::function<int(std::string&& ,std::vector<int>&&)> functions_handler
			  					= [](std::string&& fname,std::vector<int>&& args)
			{
				if (fname=="sum_square")
					{
						int sum=0;
						for(int val:args)
							sum+=val*val;
						return sum;
					} else if (fname=="print")
					{
						for(int val:args)
							std::cout << val << "," ;
						std::cout << std::flush;
					}
					return 0;
			};
			ExpressionParserAndEvaluator eval_parser((int)0,result_getter,cref(vars),functions_handler);
			TT((input="2*sum_square(1,2,3) adam",parse_r(input,eval_parser)&&(result==28)&&(input=="adam")));
	}
    if(on)
	{//working
			std::unordered_map<std::string,int> vars;


			std::function<int(std::string&& ,std::vector<int>&&)> functions_handler
			  					= [](std::string&& fname,std::vector<int>&& args)
			{
				if (fname=="sum_square")
					{
						int sum=0;
						for(int val:args)
							sum+=val*val;
						return sum;
					} else if (fname=="print")
					{
						for(int val:args)
							std::cout << val << "," ;
						std::cout << std::flush;
					}
					return 0;
			};

			string_view input;
			std::stringstream ss;
			StatementProcessor eval_parser((int)0,ref(vars),functions_handler,ss);
			TT((input="2*sum_square(1,2,3) ",parse_r(input,eval_parser)&&(ss.str()=="28\n")&&(input=="")));
	}
    if(on)
 	{
 			std::unordered_map<std::string,int> vars;

 			string_view input;
 			std::stringstream ss;
 			StatementProcessor eval_parser((int)0,ref(vars),NoFunctionsHandler{int(0)},ss);

 			TT((input="k3=2+3*2 ; adam", parse_r(input,eval_parser)
 					&&(vars.size()==1)&&(vars["k3"]==8)&&(input=="adam")));

 			TT((input=" - 2 * ( 3-2*(2+7*k3)-2)* (4-3*2) + (2-3*2)*2 ; adam",
 					parse_r(input,eval_parser)&&(ss.str()=="-468\n")&&(input=="adam")));
 	}

    if(on)
 	{
 			std::unordered_map<std::string,int> vars;

 			string_view input;
 			std::stringstream ss;
 			std::string output;
 			StatementProcessor eval_parser((int)0,ref(vars),NoFunctionsHandler{int(0)},ss);

 			TT((input="  k3 = 2 + 3 * 2  ;  ; - 2 * ( 3-2*(2+7*k3)-2)* (4-3*2) + (2-3*2)*2 ; ",
 					repeat_parse_r(input,eval_parser)&&(output=ss.str(),output=="-468\n")));
 			//std::cout << "here" << std::endl;
 	}
    if(on)
 	{
 			std::unordered_map<std::string,int> vars;
 			string_view input;
 			std::stringstream ss;
 			std::string output;
 			StatementProcessor eval_parser((int)0,ref(vars),NoFunctionsHandler{int(0)},ss);

 			TT((input=" k3 = 2 + 3 * 2  \n   -  2 * ( 3-2*(2+7*k3)-2)* (4-3*2) + (2-3*2)*2 ;; 5",
 					repeat_parse_r(input,eval_parser)&&(output=ss.str(),output=="-468\n5\n")));

 	}
    if(on)
 	{
 			std::unordered_map<std::string,int> vars;

 			string_view input;

 			std::stringstream ss;
 			std::string output;
 			StatementProcessor eval_parser((int)0,ref(vars),NoFunctionsHandler{int(0)},ss);

 			TT((input="  k3 = 2 + 3 * 2 ; - 2 * ( 3-2*(2+7*k3)-2)* (4-3*2) + (2-3*2)*2 ; ",
 					repeat_parse_r(input,eval_parser)&&(output=ss.str(),output=="-468\n")));

 	}

    if(on)
 	{
 			std::unordered_map<std::string,int> vars;

 			string_view input;
 			std::stringstream ss;
 			std::string output;

 			std::function<int(std::string&&,std::vector<int>&&)>
 			functions_handler
 					=[](std::string&& fname,std::vector<int>&& args)
			{
 				if (fname=="sum_square")
				{
					int sum=0;
					for(int val:args)
						sum+=val*val;
					return sum;
				} else if (fname=="print")
				{
					for(int val:args)
						std::cout << val << "," ;
					std::cout << std::flush;
				}
				return 0;
			};
 			StatementProcessor eval_parser((int)0,ref(vars),functions_handler,ss);

 			TT((input="  k3 = 2 + 3 * 2 ; - 2 * ( 3-2*(2+7*k3)-2)* (4-3*2)+sum_square(2,3,4) + (2-3*2)*2 ; ",
 					repeat_parse_r(input,eval_parser)&&(output=ss.str(),output=="-439\n")));
 	}

    if(on)
	{
		//custom container
		//my_map<std::string,int> vars;

		std::unordered_map<std::string,Matrix<int> > vars;

		Matrix<int> result;
		auto result_getter=[&result](Matrix<int>&& val){result=std::move(val);};
		string_view input;
		//bool success=false;
		//IntegerWithAction iwas{0,empty};
		ExpressionParserAndEvaluator eval_parser(Matrix<int>{},result_getter,cref(vars));

//		vars["M3"]={ { 1 }, {2} };
		TT((vars["M3"]={ { 1 }, {2} },input=" - [[1 , 2 ], [3,4 ] ] +  M3*[ [3 , 4] ]  adam",
				parse_r(input,eval_parser)&&(input=="adam")
				&& (result[0][0]==2) && (result[0][1]==2)
				&& (result[1][0]==3) && (result[1][1]==4)
				 ));
	}

    if(on)
  	{
  		//custom container
  		//my_map<std::string,int> vars;

  		std::unordered_map<std::string,Matrix<int> > vars;

  		Matrix<int> result;
  		auto result_getter=[&result](Matrix<int>&& val){result=std::move(val);};
  		string_view input;

  		std::function<Matrix<int>(std::string&& ,std::vector<Matrix<int>>&&)> functions_handler
  					= [](std::string&& fname,std::vector<Matrix<int>>&& args)
  		{
  			if (fname=="print")
  			{
  				for(auto& matrix:args)
  				{
  					matrix.print();
  					std::cout << "," << std::flush;
  				}
  			} else
  			if (fname=="det")
  			{
  				if (args.size()!=1)
  					std::cerr << "det function takes exactly one matrix.";
  				else
  				  if (!args[0].is_square())
  					std::cerr << "det function needs a square matrix.";
  				int val=args[0].det();
  				return Matrix<int>{{val}};// we return value in Matrix 1x1
  			}
  			return Matrix<int>{};
  		};

  		ExpressionParserAndEvaluator eval_parser(Matrix<int>{},result_getter,cref(vars),
  				functions_handler);

  //		vars["M3"]={ { 1 }, {2} };
  		TT((vars["M3"]={ { 1 }, {2} },input=" det(- [[1 , 2 ], [3,4 ] ] +  M3*[ [3 , 4] ] ) adam",
  				parse_r(input,eval_parser)&&(input=="adam")
				&& (result.height()==1)&&(result[0][0]==2)
  				 ));
  	}

    if(on)
  	{
  		//custom container
  		//my_map<std::string,int> vars;

  		std::unordered_map<std::string,Matrix<int> > vars;

  		Matrix<int> result;
  		auto result_getter=[&result](Matrix<int>&& val){result=std::move(val);};
  		string_view input;
  		//bool success=false;
  		//IntegerWithAction iwas{0,empty};
  		std::function<Matrix<int>(std::string&& ,std::vector<Matrix<int>>&&)> functions_handler
  					= [](std::string&& fname,std::vector<Matrix<int>>&& args)
  		{
  			if (fname=="print")
  			{
  				for(auto& matrix:args)
  				{
  					matrix.print();
  					std::cout << "," << std::flush;
  				}
  			} else
  			if (fname=="det")
  			{
  				if (args.size()!=1)
  					std::cerr << "det function takes exactly one matrix.";
  				else
  				  if (!args[0].is_square())
  					std::cerr << "det function needs a square matrix.";
  				int val=args[0].det();
  				return Matrix<int>{{val}};// we return value in Matrix 1x1
  			}
  			return Matrix<int>{};
  		};

  		ExpressionParserAndEvaluator eval_parser(Matrix<int>{},result_getter,cref(vars),
  				functions_handler);

  //		vars["M3"]={ { 1 }, {2} };
  		TT((vars["M3"]={{4,7,5,2,1},{3,2,1,4,3},{2,1,3,2,5},{1,2,3,4,5}
				,{3,2,1,5,-2}},input=" det(M3) adam",
  				parse_r(input,eval_parser)&&(input=="adam")
				&& (result.height()==1)&&(result[0][0]==832)
  				 ));
  	}

    if(on)
  	{
  		//custom container
  		//my_map<std::string,int> vars;

  		std::unordered_map<std::string,Matrix<int> > vars;

  		Matrix<int> result;
  		auto result_getter=[&result](Matrix<int>&& val){result=std::move(val);};
  		string_view input;
  		//bool success=false;
  		//IntegerWithAction iwas{0,empty};
  		std::function<Matrix<int>(std::string&& ,std::vector<Matrix<int>>&&)> functions_handler
  					= [](std::string&& fname,std::vector<Matrix<int>>&& args)
  		{
  			if (fname=="print")
  			{
  				for(auto& matrix:args)
  				{
  					matrix.print();
  					std::cout << "," << std::flush;
  				}
  			} else
  			if (fname=="det")
  			{
  				if (args.size()!=1)
  					std::cerr << "det function takes exactly one matrix.";
  				else
  				  if (!args[0].is_square())
  					std::cerr << "det function needs a square matrix.";
  				int val=args[0].det();
  				return Matrix<int>{{val}};// we return value in Matrix 1x1
  			}
  			return Matrix<int>{};
  		};

  		ExpressionParserAndEvaluator eval_parser(Matrix<int>{},result_getter,cref(vars),
  				functions_handler);

  //		vars["M3"]={ { 1 }, {2} };
  		TT((input=" det([[4,7,5,2,1],[3,2,1,4,3],[2,1,3,2,5],[1,2,3,4,5],[3,2,1,5,-2]]) adam",
  				parse_r(input,eval_parser)&&(input=="adam")
				&& (result.height()==1)&&(result[0][0]==832)
  				 ));
  	}
    if(on)
 	{
    		typedef int64_t Int;
    		typedef Matrix<Int>  Elem;
 			std::unordered_map<std::string,Elem> vars;



 			string_view input;
 			std::stringstream ss;
 			std::string output;

 			std::function<Elem(std::string&&,std::vector<Elem>&&)>
 			functions_handler
 					=[](std::string&& fname,std::vector<Elem>&& args)
			{
 				if (fname=="sum_square")
				{
					Elem sum{};
					for(auto& val:args)
					{
						Elem tmp{val};
						tmp*=val;
						sum+=tmp;
					}
					return sum;
				} else if (fname=="print")
				{
					for(auto& val:args)
						std::cout << val << "," ;
					std::cout << std::flush;
					return Elem{};
				} else if (fname=="det")
				{
					Int det0=det(args[0]);
					return Elem{{det0}};
				}
 				return Elem{};
			};
 			StatementProcessor eval_parser(Elem{},ref(vars),functions_handler,ss);

 			TT((input="  A=[[1,2],[4,5]]; B=[[2,3],[4,5]] ; C=A*B; det(C);D=[[4,7,5,2,1],[3,2,1,4,3],[2,1,3,2,5],[1,2,3,4,5],[3,2,1,5,-2]];det(D); ",
 					repeat_parse_r(input,eval_parser)&&(output=ss.str(),output=="[[6]]\n[[832]]\n")));
 	}

}


#endif /* TESTS_H_ */
