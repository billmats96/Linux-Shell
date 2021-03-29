# Operating-Systems

This repository contains a **Custom Linux Shell** implementation using C. 

The project involves individual work as part of the Operating Systems course of the Electrical and Computer Engineering Department, Aristotle University of Thessaloniki, Greece.

Academic Year: 2017-2018

## A Brief Description 
The shell supports **two modes** interactive and batchfile commands which are automatically recognized by the shape of user input.

**Interactive:** A dedicated buffer records user input, which is then parsed and tokenized to extract the commands to be executed by the system.
The execution is done by forking a child process and returns success/failure status. The shell supports consecutive input of commands separated
by delimiters in which case the algorithm decides how to execute them depending on the delimeter.

**Batch:** The user gives as input a .txt batchfile which is read and translated to system commands. Then the procedure is the same as interactive mode.



