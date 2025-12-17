#pragma once

#include <string>
#include <vector>

namespace boyo {

class Statement {
 public:
  virtual ~Statement() = default;

  // Generate the C++ code for this statement
  virtual std::string GenerateCode() const = 0;
};

class PrintStatement : public Statement {
 public:
  PrintStatement(const std::vector<std::uint8_t>& byte_string);
  std::string GenerateCode() const override;

 private:
  std::vector<std::uint8_t> byte_string_;
};

}  // namespace boyo
