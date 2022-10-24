#include "src/cc_ast_tool_lib.h"
#include "src/cast_visitor.h"
#include "src/goto_visitor.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using ::testing::UnorderedElementsAre;

TEST(VisitASTOnCodeTest, CastNoop) {
  auto status_or_visitor = VisitASTOnCode<CastVisitor>(" ");
  EXPECT_TRUE(status_or_visitor.ok());
  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 0);
}

TEST(VisitASTOnCodeTest, CastComment) {
  auto status_or_visitor = VisitASTOnCode<CastVisitor>("// A comment in c++");
  EXPECT_TRUE(status_or_visitor.ok());
  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 0);
}

TEST(VisitASTOnCodeTest, CastEmptyFunction) {
  auto status_or_visitor = VisitASTOnCode<CastVisitor>("void f() {}");
  EXPECT_TRUE(status_or_visitor.ok());
  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 0);
}

// TODO: fix runToolOnCode's arguments and this test!
// TEST(RsFromCcTest, ErrorOnInvalidInput) {
//   ASSERT_THAT(RsFromCc("int foo(); But this is not C++"),
//               StatusIs(absl::StatusCode::kInvalidArgument));
// }

TEST(VisitASTOnCodeTest, CastFunctionValues) {
  auto cc_file_content = "#include <stdio.h>\n"
    "\n"
    "int GLOB = 0.3 + 3.4;\n"
    "\n"
    "namespace hello_world {\n"
    "  int bar();\n"
    "\n"
    "  void foo(int aaa, int bbb) {\n"
    "    int i = 0;\n"
    "    int j = 4;\n"
    "    int k = i + j;\n"
    "    int p = bar();\n" // 1
    "    double x = 3.3;\n"
    "    float y = 4.4f;\n"
    "    int z = x + y;\n" // 2
    "    double w = y + z;\n" // 2
    "    printf(\"\%d\", x + y);\n" // 1
    "  }\n"
    "\n"
    "  int bar() {\n"
    "    return 0.0;\n" // 1
    "  }\n"
    "\n"  
    "  namespace mynamespace {\n"
    "    int foo(int aaa, int bbb) {\n"
    "      return 0;\n" // 0
    "    }\n"
    "  }\n"
    "}\n"
    "\n"
    "int main() {\n"
    "  int i = 0;\n"
    "  int j = 4;\n"
    "  int k = i + j;\n"
    "  double x = 3.3;\n"
    "  float y = 4.4f;\n"
    "  int z = x + y;\n" // 2
    "  double w = y + z;\n" // 2
    "  hello_world::foo(10, 11);\n" // 1
    "  printf(\"\%f\", x + y);\n" // 1
    "  return 0;\n"
    "}\n";

  auto status_or_visitor = VisitASTOnCode<CastVisitor>(cc_file_content);
  EXPECT_TRUE(status_or_visitor.ok());

  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 3);

  auto function_name = "input.cc#31:1#main";
  EXPECT_EQ(visitor.GetNumCasts(function_name), 6);
  EXPECT_EQ(visitor.GetNumVars(function_name), 7);
  ASSERT_THAT(visitor.GetCastKinds(function_name), UnorderedElementsAre(
    clang::CastKind::CK_FunctionToPointerDecay,
    clang::CastKind::CK_IntegralToFloating,
    clang::CastKind::CK_FloatingCast,
    clang::CastKind::CK_FloatingToIntegral
  ));

  function_name = "input.cc#20:3#hello_world::bar";
  EXPECT_EQ(visitor.GetNumCasts(function_name), 1);
  EXPECT_EQ(visitor.GetNumVars(function_name), 0);
  ASSERT_THAT(visitor.GetCastKinds(function_name), UnorderedElementsAre(
    clang::CastKind::CK_FloatingToIntegral
  ));

  function_name = "input.cc#8:3#hello_world::foo";
  EXPECT_EQ(visitor.GetNumCasts(function_name), 6);
  EXPECT_EQ(visitor.GetNumVars(function_name), 8);
  ASSERT_THAT(visitor.GetCastKinds(function_name), UnorderedElementsAre(
    clang::CastKind::CK_FunctionToPointerDecay,
    clang::CastKind::CK_IntegralToFloating,
    clang::CastKind::CK_FloatingCast,
    clang::CastKind::CK_FloatingToIntegral
  ));
}

TEST(VisitASTOnCodeTest, GotoNoop) {
  auto status_or_visitor = VisitASTOnCode<GotoVisitor>(" ");
  EXPECT_TRUE(status_or_visitor.ok());
  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 0);
}

TEST(VisitASTOnCodeTest, GotoComment) {
  auto status_or_visitor = VisitASTOnCode<GotoVisitor>("// A comment in c++");
  EXPECT_TRUE(status_or_visitor.ok());
  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 0);
}

TEST(VisitASTOnCodeTest, GotoEmptyFunction) {
  auto status_or_visitor = VisitASTOnCode<GotoVisitor>("void f() {}");
  EXPECT_TRUE(status_or_visitor.ok());
  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 0);
}

// TODO: fix runToolOnCode's arguments and this test!
// TEST(RsFromCcTest, ErrorOnInvalidInput) {
//   ASSERT_THAT(RsFromCc("int foo(); But this is not C++"),
//               StatusIs(absl::StatusCode::kInvalidArgument));
// }

TEST(VisitASTOnCodeTest, GotoFunctionValues) {
  auto cc_file_content = "#include <string>\n"
    "#include <iostream>\n"
    "int foo(int z, std::string s) {\n"
    "   return s.size() + z;\n"
    "}\n"
    "\n"
    "void checkEvenOrNot(int num)\n"
    "{\n"
    "   if (num % 2 == 0)\n"
    "       // jump to even\n"
    "       goto even;\n"
    "   else\n"
    "       // jump to odd\n"
    "       goto odd;\n"
    "\n"
    "even:\n"
    "   printf(\"\%d is even\", num);\n"
    "   // return if even\n"
    "   return;\n"
    "odd:\n"
    "   printf(\"\%d is odd\", num);\n"
    "}\n"
    "\n"
    "void switch_breaks(int c) {\n"
    "   switch (c) {\n"
    "     int z;\n"
    "     case 0:\n"
    "       c++;\n"
    "       break;\n"
    "     case 1:\n"
    "       c++;\n"
    "       break;\n"
    "     case 2:\n"
    "       c++;\n"
    "       c++;\n"
    "       break;\n"
    "     case 3:\n"
    "       break;\n"
    "     case 4:\n"
    "       break;\n"
    "       c++;\n"
    "       break;\n"
    "     case 5:\n"
    "       c++;\n"
    "       break;\n"
    "       break;\n"
    "     case 6:\n"
    "       while (true) {\n"
    "         ;\n"
    "       }\n"
    "       break;\n"
    "     case 7:\n"
    "       c++;\n"
    "       c++;\n"
    "       c++;\n"
    "       break;\n"
    "   }\n"
    "}\n"
    "\n"
    "void switch_nobreaks(char c) {\n"
    "   switch (c) {\n"
    "     case 'a':\n"
    "       c++;\n"
    "     case 'e':\n"
    "     case 'i':\n"
    "       c++;\n"
    "     case 'o':\n"
    "     case 'u':\n"
    "       break;\n"
    "   }\n"
    "}\n"
    "\n"
    "int main() {\n"
    "  int x = 3;\n"
    "  int y = foo(5, \"hello_world\");\n"
    "  checkEvenOrNot(y);\n"
    "  int z = y + 5;\n"
    "  return 0;\n"
    "}";

  auto status_or_visitor = VisitASTOnCode<GotoVisitor>(cc_file_content);
  EXPECT_TRUE(status_or_visitor.ok());

  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 1);

  auto function_name = "input.cc#7:1#checkEvenOrNot";
  EXPECT_TRUE(visitor.ContainsFunction(function_name));
}

TEST(VisitASTOnCodeTest, GotoFunctions) {
  auto status_or_visitor = VisitASTOnCode<GotoVisitor>(
    "namespace foo {\n"
    "int f(int x) {\n"
    "   if (x % 2 == 0)\n"
    "     goto f_goto;\n"
    "   return x + 1;\n"
    "f_goto:\n"
    "   return x - 1;\n"
    "}\n"
    "}\n"
    "int g(int x) {\n"
    "   if (x % 2 == 0)\n"
    "     goto g_goto;\n"
    "   return x - 1;\n"
    "g_goto:\n"
    "   return x + 1;\n"
    "}\n"
  );
  EXPECT_TRUE(status_or_visitor.ok());

  auto visitor = std::move(*status_or_visitor);
  EXPECT_EQ(visitor.GetNumFunctions(), 2);

  auto function_name = "input.cc#2:1#foo::f";
  EXPECT_TRUE(visitor.ContainsFunction(function_name));

  function_name = "input.cc#10:1#g";
  EXPECT_TRUE(visitor.ContainsFunction(function_name));
}

}
