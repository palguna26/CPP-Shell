#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <fcntl.h>

class MiniShell {
private:
    std::vector<std::string> history;
    std::ofstream logFile;

public:
    MiniShell() {
        logFile.open("shell_log.txt", std::ios::app);  // append mode
    }

    ~MiniShell() {
        logFile.close();
    }

    void run() {
        std::string input;

        while (true) {
            std::cout << "mysh> ";
            std::getline(std::cin, input);
            if (input.empty()) continue;

            history.push_back(input);
            logFile << input << std::endl;

            if (input.find('|') != std::string::npos) {
                handlePipes(input);
                continue;
            }

            std::vector<std::string> args = split(input);

            if (handleCustomCommands(args)) continue;
            if (handleBuiltInCommands(args)) continue;

            runExternalCommand(args);
        }
    }

private:
    std::vector<std::string> split(const std::string& input) {
        std::stringstream ss(input);
        std::string token;
        std::vector<std::string> tokens;
        while (ss >> token) tokens.push_back(token);
        return tokens;
    }

    bool handleBuiltInCommands(const std::vector<std::string>& args) {
        if (args.empty()) return true;
        if (args[0] == "exit") {
            std::cout << "👋 Goodbye!\n";
            exit(0);
        }
        if (args[0] == "cd") {
            const char* path = args.size() > 1 ? args[1].c_str() : getenv("HOME");
            if (chdir(path) != 0) perror("cd failed");
            return true;
        }
        return false;
    }

    bool handleCustomCommands(const std::vector<std::string>& args) {
        if (args.empty()) return false;

        if (args[0] == "greet") {
            std::cout << "👋 Hello! Welcome to your custom OOP-based shell!\n";
            return true;
        }

        if (args[0] == "help") {
            std::cout << "Available commands:\n"
                      << "  greet                - Greeting message\n"
                      << "  calc a b             - Add two numbers\n"
                      << "  repeat <msg>         - Repeat your message\n"
                      << "  history              - Show command history\n"
                      << "  runcpp <file.cpp>    - Compile and run C++ code\n"
                      << "  cd <path>            - Change directory\n"
                      << "  Pipes                - e.g., ls | grep txt\n"
                      << "  Redirection          - < input.txt, > output.txt\n"
                      << "  Background           - Run with & (e.g., sleep 5 &)\n"
                      << "  exit                 - Exit the shell\n";
            return true;
        }

        if (args[0] == "calc") {
            if (args.size() != 3) {
                std::cout << "Usage: calc <num1> <num2>\n";
            } else {
                try {
                    int a = std::stoi(args[1]), b = std::stoi(args[2]);
                    std::cout << "Result: " << (a + b) << "\n";
                } catch (...) {
                    std::cout << "Invalid numbers\n";
                }
            }
            return true;
        }

        if (args[0] == "repeat") {
            for (size_t i = 1; i < args.size(); ++i) std::cout << args[i] << " ";
            std::cout << "\n";
            return true;
        }

        if (args[0] == "history") {
            for (size_t i = 0; i < history.size(); ++i)
                std::cout << i + 1 << ": " << history[i] << "\n";
            return true;
        }

        if (args[0] == "runcpp") {
            if (args.size() != 2) {
                std::cout << "Usage: runcpp <file.cpp>\n";
                return true;
            }

            std::string filename = args[1];
            std::string outputBinary = filename.substr(0, filename.find_last_of(".")) + ".out";
            std::string compileCmd = "g++ " + filename + " -o " + outputBinary + " 2> compile_errors.txt";

            int status = system(compileCmd.c_str());
            if (status != 0) {
                std::cout << "❌ Compilation failed:\n";
                std::ifstream err("compile_errors.txt");
                std::string line;
                while (getline(err, line)) std::cout << line << "\n";
                err.close();
            } else {
                std::cout << "✅ Running " << outputBinary << "...\n";
                system(("./" + outputBinary).c_str());
            }
            return true;
        }

        return false;
    }

    void runExternalCommand(std::vector<std::string> args) {
        bool background = false;
        std::string inputFile, outputFile;

        if (!args.empty() && args.back() == "&") {
            background = true;
            args.pop_back();
        }

        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "<" && i + 1 < args.size()) {
                inputFile = args[i + 1];
                args.erase(args.begin() + i, args.begin() + i + 2);
                i--;
            } else if (args[i] == ">" && i + 1 < args.size()) {
                outputFile = args[i + 1];
                args.erase(args.begin() + i, args.begin() + i + 2);
                i--;
            }
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (!inputFile.empty()) freopen(inputFile.c_str(), "r", stdin);
            if (!outputFile.empty()) freopen(outputFile.c_str(), "w", stdout);

            std::vector<char*> c_args;
            for (auto& arg : args) c_args.push_back(const_cast<char*>(arg.c_str()));
            c_args.push_back(nullptr);

            execvp(c_args[0], c_args.data());
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {
            if (!background) waitpid(pid, nullptr, 0);
            else std::cout << "Started in background. PID: " << pid << "\n";
        } else {
            perror("fork failed");
        }
    }

    void handlePipes(const std::string& input) {
        size_t pipePos = input.find('|');
        std::string leftCmd = input.substr(0, pipePos);
        std::string rightCmd = input.substr(pipePos + 1);

        std::vector<std::string> leftArgs = split(leftCmd);
        std::vector<std::string> rightArgs = split(rightCmd);

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            return;
        }

        pid_t pid1 = fork();
        if (pid1 == 0) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);

            std::vector<char*> c_args;
            for (auto& arg : leftArgs) c_args.push_back(const_cast<char*>(arg.c_str()));
            c_args.push_back(nullptr);
            execvp(c_args[0], c_args.data());
            perror("pipe left exec failed");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[1]);
            close(pipefd[0]);

            std::vector<char*> c_args;
            for (auto& arg : rightArgs) c_args.push_back(const_cast<char*>(arg.c_str()));
            c_args.push_back(nullptr);
            execvp(c_args[0], c_args.data());
            perror("pipe right exec failed");
            exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);
        wait(nullptr);
        wait(nullptr);
    }
};

int main() {
    MiniShell shell;
    shell.run();
    return 0;
}
