#pragma once

#include <liberror/Result.hpp>

#include <utility>
#include <string>

namespace libexec {

liberror::Result<std::pair<std::string, std::string>> execute(std::string command, std::vector<std::string> arguments);
liberror::Result<std::pair<std::string, std::string>> execute(std::string command);

}
