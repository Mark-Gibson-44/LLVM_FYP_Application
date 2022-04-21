#include "opt.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/IR/Constants.h"


#include <iostream>
#include <string>


using namespace llvm;


void Opt::IdentifyOptSpace(){
	auto funcIter = M->begin();
	std::vector<Instruction*> opt_start;
	std::vector<Instruction*> opt_end;
	

	//Iterate over Instructions and record BinOps for manipulation
	while(funcIter != M->end()){
		for(auto& BB: *funcIter){
			for(auto& stmt : BB){
				
				if(stmt.isBitwiseLogicOp()){
					if(insert_point){
						//std::cout << "Can be Optimised" << std::endl;
						opt_start.push_back(&stmt);					
						insert_point = !insert_point;
					}else{
						//std::cout << "End of optimisable section" << std::endl;
						insert_point = !insert_point;
						opt_end.push_back(&stmt);
					}
					
				}
			}
		}
		funcIter++;
	}
	
	//For all insert points optimise corresponding to a certain version
	for(int i = 0; i < opt_start.size(); i++){
		dump_Instructions(opt_start[i], opt_end[i]);
	}
	//Code for printing modifier IR to stdout for redirection into a file
	raw_ostream &output2 = outs();
	M->print(output2, nullptr);
}

/*
	Function that manipulates IR instructions from Start to end instruction

	Start refers to first instruction within region suitable for optimisation
	End refers to end point where instruction iteration must finish.
*/

void Opt::dump_Instructions(Instruction* start, Instruction* end){
	int original = M->getInstructionCount();
	

	
	Value* resVal;
	
	raw_ostream &output = outs();	
	
	auto iterator = start;
	Value* prevLoad;
	while(iterator != end){
		//Initialise IR builder to location where current instruction exists
		IRBuilder<> builder(iterator);


		if(StoreInst *s = dyn_cast<StoreInst>(&*iterator)){
			//s->print(output);
			auto pTy = s->getPointerOperandType();
			//pTy->print(output);
			if(PointerType *p2 = dyn_cast<PointerType>(pTy)){
				auto nTy = p2->getElementType();
				if(nTy->isDoubleTy()){
					
					builder.CreateStore(resVal, s->getPointerOperand(), "StoreAddFin");
				}
			}
			
		} else if (LoadInst *l = dyn_cast<LoadInst>(&*iterator)){
			//Save data for previous load when trying to create a vector equivalent
			prevLoad = l->getPointerOperand();
			
		}else{
		


		auto opcode__ = iterator->getOpcode();
		//Switch manipulation performed base on the instruction type
		switch(opcode__){
			case Instruction::Load :{
					//Obtain vector type for building
					Type* vecTy = VectorType::get(Type::getDoubleTy(M->getContext()), 8);
					Value* help = UndefValue::get(vecTy);
					
					Constant* index = Constant::getIntegerValue(Type::getDoubleTy(M->getContext()), APInt(32, 0));

					auto allocV = builder.CreateAlloca(vecTy, 0, "L");
					Value *CurVar = builder.CreateLoad(allocV);
					Instruction* fullVector;
					
					fullVector = InsertElementInst::Create(help, CurVar, index);

					auto insertion = builder.Insert(fullVector);

					
					builder.CreateLoad(help, fullVector, "LOAD");
			}
			break;

			case Instruction::FMul: {
				for(int i = 0; i < 8; i++){
				
					Type* vecTy = VectorType::get(Type::getDoubleTy(M->getContext()), 8);
					Value* help = UndefValue::get(vecTy);
					Type* vecPtrTy = PointerType::get(vecTy, 0);
					Constant* index = Constant::getIntegerValue(Type::getDoubleTy(M->getContext()), APInt(32, 0));
				
					Instruction* fullVector = InsertElementInst::Create(help, iterator->getOperand(0), index);
					auto insertion = builder.Insert(fullVector);
					auto a = builder.CreateFMul(insertion, insertion, "inserter");

					Type* dblPtrTy = PointerType::get(Type::getDoubleTy(M->getContext()), 0);
					Value* v1 = builder.CreateExtractElement(insertion, builder.getInt32(0));
				
					auto cast1 = builder.CreateBitCast(prevLoad, vecPtrTy, "BC1" );
					auto realVec = builder.CreateLoad(cast1, vecTy, "BC2");
					auto realVecInstr = builder.CreateFMul(realVec, insertion, "real_inserter");
					auto realStore = builder.CreateStore(realVecInstr, cast1, "Store");
					auto cast3 = builder.CreateBitCast(cast1, dblPtrTy, "BC3");
				
				}
			}
			break;
			case Instruction::FAdd: {
				for(int i = 0; i < 8; i++){
				
					Type* vecTy = VectorType::get(Type::getDoubleTy(M->getContext()), 8);
					Value* help = UndefValue::get(vecTy);
					Type* vecPtrTy = PointerType::get(vecTy, 0);
					Constant* index = Constant::getIntegerValue(Type::getDoubleTy(M->getContext()), APInt(32, 0));
				
					Instruction* fullVector = InsertElementInst::Create(help, iterator->getOperand(0), index);
					auto insertion = builder.Insert(fullVector);
					auto a = builder.CreateFAdd(insertion, insertion, "inserterAdd");
				
					Type* dblPtrTy = PointerType::get(Type::getDoubleTy(M->getContext()), 0);
					Value* v1 = builder.CreateExtractElement(insertion, builder.getInt32(0));
				
					auto cast1 = builder.CreateBitCast(prevLoad, vecPtrTy, "BC1Add");
					auto realVec = builder.CreateLoad(cast1, vecTy, "BC2Add");
					auto realVecInstr = builder.CreateFMul(realVec, insertion, "real_inserterAdd");
					auto realStore = builder.CreateStore(realVecInstr, cast1, "StoreAdd");
				
					auto cast3 = builder.CreateBitCast(cast1, dblPtrTy, "BC3Add");
					resVal = cast3;
			
				}
			}

		}
		
		
		}
		//Move to next instruction
		iterator = iterator->getNextNonDebugInstruction();
	}

	
}
