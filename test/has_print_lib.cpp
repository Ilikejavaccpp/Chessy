#include <iostream>

// 1. Core feature test for the <print> header
#if __has_include(<print>)
#include <print>
#define HAS_PRINT 1
#else
#define HAS_PRINT 0
#endif

// 2. Core feature test for std::print implementation
#ifdef __cpp_lib_print
#define HAS_PRINT_FUNC 1
#else
#define HAS_PRINT_FUNC 0
#endif

void CheckPrintSupport() {
  if constexpr (HAS_PRINT && HAS_PRINT_FUNC) {
    std::println(
        "Success! C++23 <print> and std::println are fully operational.");
  } else {
    std::cout << "Falling back: <print> is missing or incomplete in your "
                 "current libstdc++ version.\n";
  }
}

int main() {
  CheckPrintSupport();
  return 0;
}
