#pragma once

#include <string>

namespace boyo {

/**
 * Utility class for printing C++ code to the terminal with formatting.
 * Attempts to use clang-format for proper formatting, falls back to basic
 * printing if clang-format is not available.
 */
class CodePrinter {
public:
  CodePrinter() = default;

  /**
   * Print C++ code to stdout with formatting.
   * @param code The C++ code to print
   * @param use_color Whether to use ANSI color codes (default: true)
   * @return true if formatted with clang-format, false if using fallback
   */
  bool Print(const std::string &code, bool use_color = true) const;

  /**
   * Format C++ code using clang-format.
   * @param code The C++ code to format
   * @return The formatted code, or the original if clang-format fails
   */
  std::string Format(const std::string &code) const;

  /**
   * Check if clang-format is available on the system.
   * @return true if clang-format is found
   */
  static bool IsClangFormatAvailable();

private:
  /**
   * Print code with basic indentation (fallback mode).
   * @param code The code to print
   * @param use_color Whether to use ANSI color codes
   */
  void PrintBasic(const std::string &code, bool use_color) const;

  /**
   * Execute a command and capture its output.
   * @param command The command to execute
   * @param input The input to pipe to the command
   * @param output Where to store the output
   * @return true if the command succeeded
   */
  static bool ExecuteCommand(const std::string &command,
                             const std::string &input, std::string &output);
};

} // namespace boyo
