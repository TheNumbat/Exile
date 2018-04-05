
#pragma once

#include <iostream>

#define TSTRING2(...) #__VA_ARGS__
#define TSTRING(...) TSTRING2(__VA_ARGS__)

#define begin()   {std::cout << __FILE__ << " tests:" << std::endl << std::endl;}
#define end() 	  {std::cout << std::endl << "ALL TESTS PASSED" << std::endl; return 0;}

#define ntest(name, ...) if(__VA_ARGS__) {std::cout << "\tPASSED:\t" << name << std::endl;} else {std::cerr << "\tFAILED:\t" << name << std::endl; return 1;}
#define test(...) if(__VA_ARGS__) {std::cout << "\tPASSED:\t" << TSTRING(__VA_ARGS__) << std::endl;} else {std::cerr << "\tFAILED:\t" << TSTRING(__VA_ARGS__) << std::endl; return 1;}
#define testeq(l, r) if((l) == (r)) {std::cout << "\tPASSED:\t" << TSTRING(l) << " == " << TSTRING(r) << std::endl;} else {std::cerr << "\tFAILED:\t" << TSTRING(l) << " == " << TSTRING(r) << " (" << l << ")" << std::endl; return 1;}
#define testfeq(l, r) if(fudge((l),(r))) {std::cout << "\tPASSED:\t" << TSTRING(l) << " == " << TSTRING(r) << std::endl;} else {std::cerr << "\tFAILED:\t" << TSTRING(l) << " == " << TSTRING(r) << " (" << l << ")" << std::endl; return 1;}
#define testneq(l, r) if((l) != (r)) {std::cout << "\tPASSED:\t" << TSTRING(l) << " == " << TSTRING(r) << std::endl;} else {std::cerr << "\tFAILED:\t" << TSTRING(l) << " == " << TSTRING(r) << " (" << l << ")" << std::endl; return 1;}
