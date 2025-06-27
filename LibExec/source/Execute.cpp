#include "Execute.hpp"

#include <liberror/Try.hpp>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>

#include <algorithm>

liberror::Result<void> executed_command(std::array<int, 2> const& outPipe, std::array<int, 2> const& errPipe, std::string& command, std::vector<std::string>& arguments)
{
    dup2(outPipe.at(1), STDOUT_FILENO);
    std::ranges::for_each(outPipe, close);

    dup2(errPipe.at(1), STDERR_FILENO);
    std::ranges::for_each(errPipe, close);

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

liberror::Result<std::pair<std::string, std::string>> libexec::execute(std::string command, std::vector<std::string> arguments)
{
    std::array<int, 2> outPipe {};
    pipe(outPipe.data());

    std::array<int, 2> errPipe {};
    pipe(errPipe.data());

    auto forkID = fork();
    if (forkID == 0)
    {
        TRY(executed_command(outPipe, errPipe, command, arguments));
    }

    close(outPipe.at(1));
    close(errPipe.at(1));

    std::string out;
    std::string err;

    std::array pfds {
        pollfd { outPipe.at(0), POLLIN, 0 },
        pollfd { errPipe.at(0), POLLIN, 0 }
    };

    for (auto reading = true; reading;)
    {
        auto result = poll(pfds.data(), pfds.size(), -1);
        if (result <= 0) break;

        for (auto& pfd : pfds)
        {
            if (pfd.revents & POLLIN)
            {
                std::array<char, 256> buffer {};
                auto count = read(pfd.fd, buffer.data(), buffer.size());

                if (pfd.fd == outPipe[0])
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

    close(outPipe.at(0));
    close(errPipe.at(0));

    waitpid(forkID, nullptr, 0);

    return std::make_pair(out, err);
}

liberror::Result<std::pair<std::string, std::string>> libexec::execute(std::string command)
{
    return execute(command, {});
}
