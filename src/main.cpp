#include <filesystem>
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Tooling/AllTUsExecution.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Signals.h"

#include <iostream>
#include <map>
#include <memory>
#include <mutex>

using namespace clang;
using namespace clang::ast_matchers;

std::mutex Mutex;

std::set<std::string>                         Refs;
std::set<std::string>                         Decls;
std::set<std::pair<std::string, std::string>> RefPairs;
std::set<std::pair<std::string, std::string>> DeclPairs;

class FunctionDeclMatchHandler : public MatchFinder::MatchCallback {
 public:
  void run(const MatchFinder::MatchResult& Result) override {
    std::unique_lock<std::mutex> LockGuard(Mutex);

    if (const auto* F = Result.Nodes.getNodeAs<NamedDecl>("namedDecl")) {
      auto Begin = F->getSourceRange().getBegin();
      if (Result.SourceManager->isInSystemHeader(Begin))
        return;

      if (F->getLocation().isInvalid())
        return;

      std::string id       = F->getDeclKindName() + std::string(": ") + F->getNameAsString();
      std::string location = F->getLocation().printToString(*Result.SourceManager);

      if (F->isThisDeclarationReferenced()) {
        if (Refs.insert(id).second == true)
          RefPairs.insert(std::make_pair(id, location));
      } else {
        if (Decls.insert(id).second == true)
          DeclPairs.insert(std::make_pair(id, location));
      }
    }
  }
};

class DWYUASTConsumer : public ASTConsumer {
 public:
  DWYUASTConsumer() { Matcher.addMatcher(namedDecl().bind("namedDecl"), &Handler); }

  void HandleTranslationUnit(ASTContext& Context) override { Matcher.matchAST(Context); }

 private:
  FunctionDeclMatchHandler Handler;
  MatchFinder              Matcher;
};

// For each source file provided to the tool, a new FrontendAction is created.
class DWYUFrontendAction : public ASTFrontendAction {
 public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& /*CI*/, StringRef /*File*/) override {
    return std::make_unique<DWYUASTConsumer>();
  }
};

class DWYUFrontendActionFactory : public tooling::FrontendActionFactory {
 public:
  std::unique_ptr<FrontendAction> create() override { return std::make_unique<DWYUFrontendAction>(); }
};

bool compareFirst(const std::pair<std::string, std::string>& p1, const std::pair<std::string, std::string>& p2) {
  return (p1.first < p2.first);
}

int main(int argc, const char** argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);

  const char* Overview = R"(
  dwyu is tool to find unused declarations across a whole C/C++ project.
  )";

  tooling::ExecutorName.setInitialValue("all-TUs");

  auto Executor = clang::tooling::createExecutorFromCommandLineArgs(argc, argv, llvm::cl::GeneralCategory, Overview);

  if (!Executor) {
    llvm::errs() << llvm::toString(Executor.takeError()) << "\n";
    return 1;
  }
  auto Err = Executor->get()->execute(std::make_unique<DWYUFrontendActionFactory>());

  if (Err) {
    llvm::errs() << llvm::toString(std::move(Err)) << "\n";
  }

  std::vector<std::pair<std::string, std::string>> declaredButNotDefined;

  std::set_difference(DeclPairs.begin(), DeclPairs.end(), RefPairs.begin(), RefPairs.end(),
                      std::back_inserter(declaredButNotDefined), compareFirst);

  for (auto F : declaredButNotDefined) {
    std::cout << F.first << "\t" << F.second << "\n";
  }
}
