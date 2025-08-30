#include "Execute.hpp"

#include <liberror/Try.hpp>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <span>

static liberror::Result<void> executed_command(std::span<int> stdoutPipe, std::span<int> stderrPipe, std::string command, std::vector<std::string> arguments)
{
    dup2(stdoutPipe[1], STDOUT_FILENO);
    std::ranges::for_each(stdoutPipe, close);

    dup2(stderrPipe[1], STDERR_FILENO);
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

    return liberror::make_error("{}", strerror(errno));
}

liberror::Result<std::pair<std::string, std::string>> libexec::execute(std::string const& command, std::vector<std::string> const& arguments, Mode mode)
{
    std::array<int, 2> stdoutPipe {};

    if (pipe(stdoutPipe.data()) == -1)
    {
        return liberror::make_error("{}", strerror(errno));
    }

    std::array<int, 2> stderrPipe {};

    if (pipe(stderrPipe.data()) == -1)
    {
        return liberror::make_error("{}", strerror(errno));
    }

    auto pid1 = fork();
    if (pid1 == -1)
    {
        return liberror::make_error("{}", strerror(errno));
    }

    if (pid1 == 0)
    {
        if (mode == Mode::DETACHED)
        {
            setsid();

            auto pid2 = fork();
            if (pid2 == -1)
            {
                return liberror::make_error("{}", strerror(errno));
            }

            if (pid2 == 0)
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

    waitpid(pid1, nullptr, 0);

    return std::pair { out, err };
}

liberror::Result<std::pair<std::string, std::string>> libexec::execute(std::string const& command, Mode mode)
{
    return execute(command, std::vector<std::string>{}, mode);
}

