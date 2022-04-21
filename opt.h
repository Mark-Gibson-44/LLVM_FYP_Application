#pragma once
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"




class Opt{
	std::unique_ptr<llvm::Module> M;//LLVM Module for a given file storing IR data
	bool insert_point; //Boolean to identify whether current instruction is within insert point
public:
	Opt(std::unique_ptr<llvm::Module> M) : M(std::move(M)), insert_point(true){}
	
	void IdentifyOptSpace();	
	void dump_Instructions(llvm::Instruction* start, llvm::Instruction* end);
};	
