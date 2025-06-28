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

liberror::Result<std::pair<std::string, std::string>> execute(std::string command, Mode mode = Mode::ATTACHED);
liberror::Result<std::pair<std::string, std::string>> execute(std::string command, std::vector<std::string> arguments, Mode mode = Mode::ATTACHED);

liberror::Result<void> execute(std::string command, std::filesystem::path output, Mode mode = Mode::ATTACHED);
liberror::Result<void> execute(std::string command, std::vector<std::string> arguments, std::filesystem::path output, Mode mode = Mode::ATTACHED);

}
