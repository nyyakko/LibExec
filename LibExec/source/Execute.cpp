#include "Execute.hpp"

#include <liberror/Try.hpp>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>

#include <algorithm>
#include <unistd.h>

static liberror::Result<void> executed_command(std::array<int, 2> const& stdoutPipe, std::array<int, 2> const& stderrPipe, std::string command, std::vector<std::string> arguments)
{
    dup2(stdoutPipe.at(1), STDOUT_FILENO);
    std::ranges::for_each(stdoutPipe, close);

    dup2(stderrPipe.at(1), STDERR_FILENO);
    std::ranges::for_each(stderrPipe, close);

    auto devNull = open("/dev/null", O_RDONLY);
    assert(devNull >= 0);
    dup2(devNull, STDIN_FILENO);
    close(devNull);

    std::vector<char*> argumentsData { command.data() };
    // cppcheck-suppress constParameterReference
    std::transform(arguments.begin(), arguments.end(), std::back_inserter(argumentsData), [] (std::string& str) {
        return str.data();
    });
    argumentsData.push_back(nullptr);

    execvp(command.data(), argumentsData.data());

    return liberror::make_error(strerror(errno));
}

liberror::Result<std::pair<std::string, std::string>> libexec::execute(std::string const& command, std::vector<std::string> const& arguments, Mode mode)
{
    std::array<int, 2> stdoutPipe {};
    pipe(stdoutPipe.data());

    std::array<int, 2> stderrPipe {};
    pipe(stderrPipe.data());

    auto forkId = fork();
    if (forkId == 0)
    {
        if (mode == Mode::DETACHED)
        {
            setsid();
            auto innerForkId = fork();
            if (innerForkId == 0)
            {
                TRY(executed_command(stdoutPipe, stderrPipe, command, arguments));
            }

            std::exit(EXIT_SUCCESS);
        }
        else
        {
            TRY(executed_command(stdoutPipe, stderrPipe, command, arguments));
        }
    }

    close(stdoutPipe.at(1));
    close(stderrPipe.at(1));

    std::string out, err;

    if (mode == Mode::ATTACHED)
    {
        std::array pfds {
            pollfd { stdoutPipe.at(0), POLLIN, 0 },
            pollfd { stderrPipe.at(0), POLLIN, 0 }
        };

        for (auto reading = true; reading;)
        {
            auto result = poll(pfds.data(), pfds.size(), -1);
            if (result <= 0) break;

            for (auto& pfd : pfds)
            {
                if (pfd.revents & POLLIN)
                {
                    std::array<char, 1024> buffer {};
                    auto count = read(pfd.fd, buffer.data(), buffer.size());

                    if (pfd.fd == stdoutPipe[0])
                        out.append(buffer.data());
                    else
                        err.append(buffer.data());

                    reading = count > 0;
                }
                else if (pfd.revents & (POLLHUP | POLLERR))
                {
                    reading = false;
                }
            }
        }

        if (out.back() == '\n') out.pop_back();
        if (err.back() == '\n') err.pop_back();
    }

    close(stdoutPipe.at(0));
    close(stderrPipe.at(0));

    waitpid(forkId, nullptr, 0);

    return std::pair { out, err };
}

liberror::Result<std::pair<std::string, std::string>> libexec::execute(std::string const& command, Mode mode)
{
    return execute(command, std::vector<std::string>{}, mode);
}

