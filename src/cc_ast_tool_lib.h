#ifndef CC_AST_TOOL_LIB_H_
#define CC_AST_TOOL_LIB_H_

#include "src/frontend_action.h"

#include <string>

#include "absl/strings/string_view.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "clang/Tooling/Tooling.h"

absl::StatusOr<std::string> GetFileContents(absl::string_view path);

template <typename T>
absl::StatusOr<typename T::VisitorInfo> VisitASTOnCode(
    const absl::string_view cc_file_content, const std::string& cc_in) {
  typename T::VisitorInfo visitor_info;
  if (clang::tooling::runToolOnCode(
          std::make_unique<FrontendAction<T>>(visitor_info), 
          cc_file_content, cc_in)) {
    return visitor_info;
  }
  return absl::Status(absl::StatusCode::kInvalidArgument, 
                      "Could not compile source file contents");
}

#endif // CC_AST_TOOL_LIB_H_