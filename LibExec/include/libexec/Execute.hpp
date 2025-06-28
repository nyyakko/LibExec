#pragma once

#include <liberror/Result.hpp>

#include <utility>
#include <string>

namespace libexec {

enum class Mode {
    ATTACHED,
    DETACHED
};

liberror::Result<std::pair<std::string, std::string>> execute(std::string const& command, Mode mode = Mode::ATTACHED);
liberror::Result<std::pair<std::string, std::string>> execute(std::string const& command, std::vector<std::string> const& arguments, Mode mode = Mode::ATTACHED);

}
