# CSC209 Software Tools and Systems Programming

This repository contains my coursework labs and assignments for CSC209: Software Tools and Systems Programming course at the University of Toronto.

The course involves programming in C and using scripting languages in a Unix-style environment. The basic topics include: using and creating software tools, pipes and filters, file processing, shell programming, processes, system calls, signals, and basic network programming.

## Getting Started

### Prerequisites
* **GCC** (C Compiler or Clang Compiler)
* **Unix Shell**

### Running Files

All Labs and Assignments contain a Makefile to compile all code so can execute 'make' in the Shell to compile all required files and create necessary executables.
```
$ make
```

For individual file compiling and running executables (Mac):
```
$ gcc -Wall -std=gnu99 -g -o <filename> <filename.c>
$ ./filename
```
## Labs and Assignment Summaries

### Assignments

* Assignment 1: **Exploring Benford's Law and a 2D Game of Life Intro C**
  * This involves exploring Benford's Law with a program to find the distribution of digits in a set of data, as well as a variant program of Conway's Game of Life. The assignment involved C basics, including command-line arguments, arrays, and processing data from standard input.
  
* Assignment 2: **Structs and Dynamic Memory**
  * This involves traversing over a file system to display all files, directories, and symbolic links, using system calls to the OS file system. The goal of the assignment was to practice linked lists, strings, pointers, and dynamic memory allocation in C.
  
* Assignment 3: **Processes and Pipes**
  * This involves using Parallel Computing through processes, pipes, and forking to find the smallest distance in sets of points using a multi-process implementation through a Divide-and-Conquer Algorithm. The goal of this assignment was to practice creating and using multiple processes in C.
  
* Assignment 4: **Twitter Server**
   * This involves creating a simple Twitter server run through the Shell where users can connect and interact with each other, by following and unfollowing other users, sending messages and showing all messages. The interactions would be done through the Shell Terminal using sockets to send information to each other.


### Labs

**Lab 1:** Compiling and running C scripts and basic shell utilities (input/output redirection and pipes)

**Lab 2:** Writing C programs with pointers and using scanf() to read values from standard input

**Lab 3:** Using pointers and dynamic memory allocation in the heap with malloc()

**Lab 4:** Comparing, truncating, and stripping strings

**Lab 5:** Using structs and File I/O with reading binary data from files (images) and storing them in a two-dimensional array in the heap

**Lab 6:** Using the C gdb debugger with breakpoints to examine overflow and Segmentation Faults

**Lab 7:** Forking to create multiple processes with fork(), wait(), and reading pid

**Lab 8:** Using system calls (fork, exec, pipe, dup2, write, wait) to read user input and run a separate password validation program in a child process

**Lab 9:** Practice using signal handlers and alarms to time reading and writing from a binary file to find the number of read operations performed in a given time

**Lab 10:** Using sockets to create a server-client connection to read incomplete lines from a buffering server and output a complete line

**Lab 11:** Creating a chat server with multiple clients using select() to read from multiple connections

## Acknowledgements

The labs and assignments were created by instructors at the University of Toronto for CSC209. They included starter code and a list of objectives to complete the assignment. The labs and assignments were completed myself
