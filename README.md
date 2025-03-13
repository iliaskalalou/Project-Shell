This GitHub project is a custom shell developed entirely in C, designed to comply with the POSIX standard.
The goal is to recreate essential Unix shell functionalities including command parsing, process management, I/O redirection, pipelines, signal handling, and environment variable expansion.

To Start the Project : 

autoreconf --install 

./configure

./src/42sh -c "Your Shell Command"

example : ./src/42sh -c "echo toto"
