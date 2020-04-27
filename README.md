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

