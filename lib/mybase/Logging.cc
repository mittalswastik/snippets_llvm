#define DEBUG_TYPE "Logging"
#include "Logging.hh"
#include "Version.hh"

#include <string>

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"

#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Support/raw_ostream.h"

#define INST_KIND_DUMP(err, stop, Key)          \
  {                                             \
    if (Key) err << logging::InstTypeStr(#Key); \
    {                                           \
      if (Key && stop) {                        \
        err << "\n";                            \
        break;                                  \
      }                                         \
      if (Key && !stop) err << " <-- ";         \
    }                                           \
  }

namespace llvm {
namespace logging {

#define casePrint(Key) \
  case Key:            \
    errs() << (#Key);  \
    break;

void printValueInfo(Value const *v) {
  errs() << "[ValueID]";
  switch (v->getValueID()) {
    casePrint(Value::ArgumentVal);
    casePrint(Value::BasicBlockVal);
    /// Constants
    casePrint(Value::FunctionVal);
    casePrint(Value::GlobalAliasVal);
    casePrint(Value::GlobalVariableVal);
    casePrint(Value::UndefValueVal);
    casePrint(Value::BlockAddressVal);
    casePrint(Value::ConstantExprVal);
    casePrint(Value::ConstantAggregateZeroVal);
    casePrint(Value::ConstantDataArrayVal);
    casePrint(Value::ConstantDataVectorVal);
    casePrint(Value::ConstantIntVal);
    casePrint(Value::ConstantFPVal);
    casePrint(Value::ConstantArrayVal);
    casePrint(Value::ConstantStructVal);
    casePrint(Value::ConstantVectorVal);
    casePrint(Value::ConstantPointerNullVal);
/// Constants end
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#else
    casePrint(Value::MDNodeVal);
    casePrint(Value::MDStringVal);
#endif
    casePrint(Value::InlineAsmVal);
    /// casePrint(Value::PseudoSourceValueVal);
    /// casePrint(Value::FixedStackPseudoSourceValueVal);
    default: {
      // >=InstructionVal
      errs() << "Value::InstructionVal";
      break;
    }
  }
  errs() << "\n";
}

void printTypeInfo(Type const *type) {
  BEG_FUN_LOG();
  type->dump();
  switch (type->getTypeID()) {
    casePrint(Type::VoidTyID);
    casePrint(Type::FloatTyID);
    casePrint(Type::DoubleTyID);
    casePrint(Type::X86_FP80TyID);
    casePrint(Type::FP128TyID);
    casePrint(Type::PPC_FP128TyID);
    casePrint(Type::LabelTyID);
    casePrint(Type::MetadataTyID);
    casePrint(Type::X86_MMXTyID);
    casePrint(Type::IntegerTyID);
    casePrint(Type::FunctionTyID);
    casePrint(Type::StructTyID);
    casePrint(Type::ArrayTyID);
    casePrint(Type::PointerTyID);
    casePrint(Type::VectorTyID);
    casePrint(Type::HalfTyID);
    default:
      assert(0);
  }
  WITH_COLOR(raw_ostream::GREEN,
             errs() << " IntOrIntVector=" << type->isIntOrIntVectorTy());
  WITH_COLOR(raw_ostream::GREEN,
             errs() << " FPOrFPVector=" << type->isFPOrFPVectorTy());
  /// errs() << " Abstract=" << type->isAbstract();
  errs() << " Aggregate=" << type->isAggregateType();
  errs() << " FirstClass=" << type->isFirstClassType();
  errs() << " SingleValue=" << type->isSingleValueType();
  errs() << " Sized=" << type->isSized();
  errs() << "\n";
  END_FUN_LOG();
}

void printInstKind(Instruction const &I) {
  errs() << "\n" << I << "\n";
  while (true) {
    /// INST_TYPE_DUMP(errs(), true, isa<AtomicRMWInst>(I));
    /// INST_TYPE_DUMP(errs(), true, isa<AtomicCmpXchgInst>(I));
    INST_KIND_DUMP(errs(), true, isa<BinaryOperator>(I));
    INST_KIND_DUMP(errs(), false, isa<CallInst>(I));
    {
      if (CallInst const *CI = dyn_cast<CallInst>(&I)) {
        if (CI->isInlineAsm()) {
          errs() << "InlineAsm\n";
          break;
        }
        INST_KIND_DUMP(errs(), false, isa<IntrinsicInst>(I));
        {
          INST_KIND_DUMP(errs(), false, isa<DbgInfoIntrinsic>(I));
          {
            INST_KIND_DUMP(errs(), true, isa<DbgValueInst>(I));
            INST_KIND_DUMP(errs(), true, isa<DbgDeclareInst>(I));
          }
          INST_KIND_DUMP(errs(), false, isa<MemIntrinsic>(I));
          {
            INST_KIND_DUMP(errs(), false, isa<MemSetInst>(I));
            INST_KIND_DUMP(errs(), true, isa<MemTransferInst>(I));
            INST_KIND_DUMP(errs(), true, isa<MemCpyInst>(I));
          }
          /// INST_TYPE_DUMP(errs(),true, isa<VACopyInst>(I));
          /// INST_TYPE_DUMP(errs(),true, isa<VAEndInst>(I));
          /// INST_TYPE_DUMP(errs(),true, isa<VAStartInst>(I));
        }
        errs() << "[Simple CallInst]\n";
        break;
      }
    }
    INST_KIND_DUMP(errs(), false, isa<CmpInst>(I));
    {
      INST_KIND_DUMP(errs(), true, isa<ICmpInst>(I));
      INST_KIND_DUMP(errs(), true, isa<FCmpInst>(I));
    }
    INST_KIND_DUMP(errs(), true, isa<ExtractElementInst>(I));
    /// INST_TYPE_DUMP(errs(), true, isa<FenceInst>(I));
    INST_KIND_DUMP(errs(), true, isa<GetElementPtrInst>(I));
    INST_KIND_DUMP(errs(), true, isa<InsertValueInst>(I));
    /// INST_TYPE_DUMP(errs(), true, isa<LandingPadInst>(I));
    INST_KIND_DUMP(errs(), true, isa<PHINode>(I));
    INST_KIND_DUMP(errs(), true, isa<SelectInst>(I));
    INST_KIND_DUMP(errs(), true, isa<ShuffleVectorInst>(I));
    INST_KIND_DUMP(errs(), true, isa<StoreInst>(I));
    INST_KIND_DUMP(errs(), false, isa<TerminatorInst>(I));
    {
      INST_KIND_DUMP(errs(), true, isa<BranchInst>(I));
      INST_KIND_DUMP(errs(), true, isa<IndirectBrInst>(I));
      INST_KIND_DUMP(errs(), true, isa<InvokeInst>(I));
      /// INST_TYPE_DUMP(errs(), true, isa<ResumeInst>(I));
      INST_KIND_DUMP(errs(), true, isa<ReturnInst>(I));
      INST_KIND_DUMP(errs(), true, isa<SwitchInst>(I));
      INST_KIND_DUMP(errs(), true, isa<UnreachableInst>(I));
    }
    INST_KIND_DUMP(errs(), false, isa<UnaryInstruction>(I));
    {
      INST_KIND_DUMP(errs(), true, isa<AllocaInst const>(I));
      INST_KIND_DUMP(errs(), true, isa<CastInst>(I));
      {
        INST_KIND_DUMP(errs(), true, isa<BitCastInst>(I));
        INST_KIND_DUMP(errs(), true, isa<FPExtInst>(I));
        INST_KIND_DUMP(errs(), true, isa<FPToSIInst>(I));
        INST_KIND_DUMP(errs(), true, isa<FPToUIInst>(I));
        INST_KIND_DUMP(errs(), true, isa<FPTruncInst>(I));
        INST_KIND_DUMP(errs(), true, isa<IntToPtrInst>(I));
        INST_KIND_DUMP(errs(), true, isa<PtrToIntInst>(I));
        INST_KIND_DUMP(errs(), true, isa<SExtInst>(I));
        INST_KIND_DUMP(errs(), true, isa<SIToFPInst>(I));
        INST_KIND_DUMP(errs(), true, isa<TruncInst>(I));
        INST_KIND_DUMP(errs(), true, isa<UIToFPInst>(I));
        INST_KIND_DUMP(errs(), true, isa<ZExtInst>(I));
      }
      INST_KIND_DUMP(errs(), true, isa<ExtractValueInst>(I));
      INST_KIND_DUMP(errs(), true, isa<LoadInst>(I));
      INST_KIND_DUMP(errs(), true, isa<VAArgInst>(I));
    }
    std::string instStr;
    raw_string_ostream OS(instStr);
    OS << "unknown Instruction Type\n";
    logging::rbscope_diagnostics(RB_Diagnostics::FATAL, OS.str().c_str());
  }
}

std::string InstTypeStr(char const *instTypeChars) {
  std::string instTypeString(instTypeChars);
  unsigned typeBegIndex = instTypeString.find("isa<") + 4;
  unsigned typeEndIndex = instTypeString.find(">");
  return instTypeString.substr(typeBegIndex, typeEndIndex - typeBegIndex);
}

void prettyPrint(Value const *V, unsigned endLine, unsigned startLine) {
  if (V == nullptr) {
    errs() << "NULL value\n";
    return;
  }
  while (startLine--) errs() << "\n";
  if (isa<GlobalVariable>(V)) {
    endLine++;
    WITH_COLOR(raw_ostream::RED, errs() << "[GV]" << V->getName() << " ");
  } else if (isa<GlobalAlias>(V)) {
    WITH_COLOR(raw_ostream::MAGENTA, errs() << "[GA]" << V->getName() << " ");
  } else if (isa<Function>(V)) {
    WITH_COLOR(raw_ostream::GREEN, errs() << "[Fn]" << V->getName() << " ");
  } else if (isa<BasicBlock>(V)) {
    WITH_COLOR(raw_ostream::YELLOW, errs() << "[BB]" << V->getName() << " ");
  } else if (isa<Instruction>(V)) {
    WITH_COLOR(raw_ostream::RED, errs() << "[Inst]" << *V;);
    WITH_COLOR(
        raw_ostream::GREEN,
        errs() << " ("
               << cast<Instruction>(V)->getParent()->getParent()->getName()
               << ")\n");
  } else if (isa<Argument>(V)) {
    WITH_COLOR(raw_ostream::YELLOW, errs() << "[Arg] " << *V;);
    WITH_COLOR(raw_ostream::GREEN,
               errs() << " (" << cast<Argument>(V)->getParent()->getName()
                      << ")\n");

  } else if (V->hasName()) {
    WITH_COLOR(raw_ostream::YELLOW, errs() << V->getName() << " ");
  } else {
    WITH_COLOR(raw_ostream::RED, errs() << *V << "\n");
  }

  while (endLine--) errs() << "\n";
}

}  //  namespace llvm::logging

}  // namespace llvm