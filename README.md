# 42sh - A Custom POSIX-Compliant Shell

This GitHub project is a custom shell developed entirely in C, designed to comply with the POSIX standard. The goal is to recreate essential Unix shell functionalities, including:

- Command parsing
- Process management
- I/O redirection
- Pipelines
- Signal handling
- Environment variable expansion

## Getting Started

Follow these steps to build and run the project:

1. **Install the necessary tools**  
   Ensure you have `autoreconf`, `configure`, and a C compiler installed.

2. **Build the Project**

   ```bash
   autoreconf --install
   ./configure
   make

3. **Run the Shell**

  Execute the shell with a specific command using:

  ```bash
  ./src/42sh -c "echo toto"
  toto
