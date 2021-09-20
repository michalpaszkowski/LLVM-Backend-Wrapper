#include "backend-wrapper.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace llvm;

extern "C" IBackend_Result *generateTargetCode(const char *Buf,
                                                size_t BufSize) {
  // Result components.
  SmallString<4096> ResultObject;
  raw_svector_ostream ResultStream(ResultObject);

  std::string Errors;
  raw_string_ostream ErrorStream(Errors);

  // Load the module to be compiled.
  std::string Bitcode(Buf, BufSize);
  std::unique_ptr<MemoryBuffer> Buffer =
      MemoryBuffer::getMemBuffer(StringRef(Bitcode));
  LLVMContext Context;
  SMDiagnostic ModuleError;
  std::unique_ptr<Module> Module =
      parseIR(Buffer->getMemBufferRef(), ModuleError, Context);

  if (Module == nullptr) {
    ErrorStream << ModuleError.getMessage();
    ErrorStream.flush();
    return new IBackend_Result("", Errors);
  }

  // Initialize LLVM.
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  std::string TargetError;
  auto Target =
      TargetRegistry::lookupTarget(Module->getTargetTriple(), TargetError);

  if (!Target) {
    ErrorStream << TargetError;
    ErrorStream.flush();
    return new IBackend_Result("", Errors);
  }

  TargetOptions Options;
  auto RM = Optional<Reloc::Model>();
  auto TheTargetMachine = Target->createTargetMachine(
      Module->getTargetTriple(), "generic", "", Options, RM);

  Module->setDataLayout(TheTargetMachine->createDataLayout());

  legacy::PassManager PM;
  auto ResultType = CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(PM, ResultStream, nullptr,
                                            ResultType)) {
    ErrorStream << "TheTargetMachine can't emit a file of this type";
    ErrorStream.flush();
    return new IBackend_Result("", Errors);
  }

  // Run the passes on the module.
  PM.run(*Module);

  ErrorStream.flush();
  if (!Errors.empty())
    return new IBackend_Result("", Errors);

  // Deep copy the SmallString -> std::string before passing.
  std::string ResultObjectDeepCopy(ResultObject.c_str(), ResultObject.size());
  return new IBackend_Result(ResultObjectDeepCopy, Errors);
}