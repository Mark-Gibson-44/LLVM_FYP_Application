#include <iostream>

#include "opt.h"
#include "parse.cpp"

int main(int argv, char** argc){
	//No File provided
	if(argv == 1){
		std::cout << "ERROR: Expected Bitcode Filename Argument\n";
		return 1;
	}

	LLVMContext theContext;
	SMDiagnostic err;
	auto parsed = ParseFile(argc[1], theContext, err);
	if(parsed == nullptr){
		std::cout << "ERROR: Invalid FileName\n";
		return 1;
	}

	//Construction optimisation object
	auto opt = Opt(std::move(parsed));
	//Optimise
	opt.IdentifyOptSpace();

	return 0;
}

