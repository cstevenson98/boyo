#pragma once

#include <memory>
#include <string>
#include <vector>

#include "statement/statement.hpp"

namespace boyo {

using StatementList = std::vector<std::unique_ptr<Statement>>;

class Parser {
 public:
  /**
   * Parse the given lines and return a vector of Statement objects, one per
   * line
   * @param lines The lines to parse
   * @return A vector of Statement objects
   * @throws std::runtime_error if the line is invalid
   */
  StatementList Parse(const std::vector<std::string>& lines) const;
};

}  // namespace boyo
