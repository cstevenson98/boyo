#pragma once

#include <memory>
#include <string>
#include <vector>

#include "statement.hpp"

namespace boyo {

class Parser {
 public:
  /**
   * Parse the given lines and return a vector of Statement objects, one per line
   * @param lines The lines to parse
   * @return A vector of Statement objects
   * @throws std::runtime_error if the line is invalid
   */
  std::vector<std::unique_ptr<Statement>> Parse(
      const std::vector<std::string>& lines) const;
};

}  // namespace boyo
