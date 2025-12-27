#include "utils/code_printer.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>

namespace boyo {

// ANSI color codes
namespace Color {
const char *Reset = "\033[0m";
const char *Blue = "\033[34m";
const char *Green = "\033[32m";
const char *Yellow = "\033[33m";
const char *Cyan = "\033[36m";
} // namespace Color

// Helper function to escape shell strings
static std::string ShellEscape(const std::string &str) {
  std::string escaped;
  escaped.reserve(str.size() + 2);
  escaped += "'";

  for (char c : str) {
    if (c == '\'') {
      escaped += "'\\''";
    } else {
      escaped += c;
    }
  }

  escaped += "'";
  return escaped;
}

bool CodePrinter::Print(const std::string &code, bool use_color) const {
  std::string formatted_code = Format(code);
  bool used_clang_format = !formatted_code.empty();

  if (use_color) {
    std::cout << Color::Cyan << "=== Generated C++ Code ===" << Color::Reset
              << "\n\n";
  } else {
    std::cout << "=== Generated C++ Code ===\n\n";
  }

  if (used_clang_format) {
    std::cout << formatted_code;
    if (!formatted_code.empty() && formatted_code.back() != '\n') {
      std::cout << "\n";
    }
    if (use_color) {
      std::cout << Color::Green << "(formatted with clang-format)"
                << Color::Reset << "\n";
    } else {
      std::cout << "(formatted with clang-format)\n";
    }
  } else {
    PrintBasic(code, use_color);
    if (use_color) {
      std::cout << Color::Yellow << "(basic formatting)" << Color::Reset
                << "\n";
    } else {
      std::cout << "(basic formatting)\n";
    }
  }

  return used_clang_format;
}

std::string CodePrinter::Format(const std::string &code) const {
  std::string formatted;
  if (ExecuteCommand("clang-format --style=Google", code, formatted)) {
    return formatted;
  }

  return code;
}

bool CodePrinter::IsClangFormatAvailable() {
  // Try to run clang-format --version to check if it's available
  std::string output;
  return ExecuteCommand("clang-format --version 2>&1", "", output);
}

void CodePrinter::PrintBasic(const std::string &code, bool use_color) const {
  std::istringstream stream(code);
  std::string line;
  int line_number = 1;

  while (std::getline(stream, line)) {
    if (use_color) {
      // Format line number with padding
      std::string num_str = std::to_string(line_number);
      std::string padding(4 - num_str.length(), ' ');
      std::cout << Color::Blue << padding << num_str << " | " << Color::Reset;
    } else {
      std::cout << line_number << " | ";
    }
    std::cout << line << "\n";
    line_number++;
  }
}

bool CodePrinter::ExecuteCommand(const std::string &command,
                                 const std::string &input,
                                 std::string &output) {
  std::string full_command = command;

  // If we have input, use echo -n to pipe it (preserves formatting)
  if (!input.empty()) {
    full_command = "echo -n " + ShellEscape(input) + " | " + command;
  }

  std::unique_ptr<FILE, decltype(&pclose)> pipe(
      popen(full_command.c_str(), "r"), pclose);
  if (!pipe) {
    return false;
  }

  std::array<char, 128> buffer;
  output.clear();
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    output += buffer.data();
  }

  int exit_code = pclose(pipe.release());
  return exit_code == 0;
}

} // namespace boyo
