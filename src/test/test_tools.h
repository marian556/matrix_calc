/*
 * test_tools.h
 *
 *  Created on: Jun 18, 2019
 *      Author: marian
 */

#ifndef TEST_TOOLS_H_
#define TEST_TOOLS_H_
#include <iostream>

#define TT(a) std::cerr <<  __FILE__ << ":" << __LINE__ << "|" << ((bool)(a) ? "  OK    " : " Failed ") << "|" << #a << "|"   << std::endl;

template <typename T>
std::ostream& operator << (std::ostream& os,const std::vector<T>& v)
{
	std::cout << "[" ;
	for (auto& val:v)
		std::cout << val << ",";
	std::cout << "]" << std::flush;
	return os;
}

template <typename T>
void display(const std::vector<T>& v)
{
	std::cout << "[" ;
	for (auto& val:v)
		std::cout << val << ",";
	std::cout << "]" << std::flush;
}


#endif /* TEST_TOOLS_H_ */
