#pragma once

#include <liberror/Result.hpp>

#include <utility>
#include <filesystem>
#include <string>

namespace libexec {

enum class Mode : char
{
    ATTACHED,
    DETACHED
};

liberror::Result<std::pair<std::string, std::string>> execute(std::string const& command, Mode mode = Mode::ATTACHED);
liberror::Result<void> execute(std::string const& command, std::filesystem::path const& output, Mode mode = Mode::ATTACHED);
liberror::Result<std::pair<std::string, std::string>> execute(std::string const& command, std::vector<std::string> const& arguments, Mode mode = Mode::ATTACHED);
liberror::Result<void> execute(std::string const& command, std::vector<std::string> const& arguments, std::filesystem::path const& output, Mode mode = Mode::ATTACHED);

}
