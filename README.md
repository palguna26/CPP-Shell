# 🐚 MiniShell (C++ Project)

A university-level OOP-based mini shell written in C++ that supports custom commands, basic UNIX shell features, and can compile & execute C++ programs.

---

## 📦 Features

- [x] Custom Commands: `greet`, `calc`, `repeat`, `history`, `help`
- [x] Command Execution (built-in and external commands)
- [x] Pipes (`|`)
- [x] Input/Output Redirection (`<`, `>`)
- [x] Background Execution (`&`)
- [x] Logging: Saves all commands to `shell_log.txt`
- [x] `runcpp`: Compile and execute `.cpp` files from the shell

---

## 🧪 Example Usage

```bash
mysh> greet
Hello! Welcome to your custom OOP-based shell!

mysh> calc 5 10
Result: 15

mysh> repeat I love C++
I love C++

mysh> runcpp hello.cpp
✅ Compilation successful. Running hello.out...
Hello from compiled C++ file!

mysh> ls | grep cpp
mini_shell.cpp

mysh> cat < input.txt > output.txt

mysh> sleep 10 &
Started in background. PID: 13452
