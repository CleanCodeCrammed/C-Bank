Mock C++ Terminal Banking System (for educational/demonstrational puposes only)

A secure, terminal-based banking application built entirely in standard C++. This project demonstrates fundamental software architecture, memory management, algorithmic security, and the practical application of multiple data structures.

Features

* Secure Authentication: Implements a two-state system (Logged Out / Logged In). Passwords are never stored in plain text; they are secured using unique cryptographic salts and the DJB2 hashing algorithm.
* Account Management: Robust account creation with strict validation (case-insensitive, alphanumeric enforcement, and character limits) ensuring O(1) unique username verification.
* Core Banking Operations: Users can check balances, deposit funds, withdraw money, and transfer funds to other users safely. 
* Transaction History: Automatically maintains a rolling ledger of the user's 5 most recent transactions using a First-In-First-Out (FIFO) methodology.
* Precision Formatting: Financial values are strictly formatted to standard two-decimal precision, preventing scientific notation overflow errors.

Data Structures Used

This application avoids traditional databases to showcase raw C++ data structure manipulation:

1. Linked List (Custom): Acts as the primary dynamic database, expanding memory only as new accounts are created.
2. Hash Tables (std::unordered_map): Maps usernames to account numbers, and account numbers to specific memory pointers, allowing for lightning-fast O(1) lookups during logins and transfers without needing to traverse the linked list.
3. Arrays: Utilized within the Account struct to maintain a highly efficient, fixed-size transaction history ledger.

How to Compile and Run

Prerequisites
* A C++ compiler (GCC, Clang, or MSVC)
* Visual Studio 2022 (Recommended for Windows users)

Running Locally (Terminal)
If you are compiling via a standard terminal, navigate to the directory containing Main.cpp and run:

g++ Main.cpp -o BankSystem
./BankSystem

Running Locally (Visual Studio 2022)
1. Open Visual Studio 2022 and select Create a new project.
2. Choose Console App (C++).
3. Replace the contents of the generated .cpp file with the code from Main.cpp.
4. Press F5 or click Local Windows Debugger to build and run the application.

Security Disclaimer
This project is built for educational purposes to demonstrate the logic and flow of salting and hashing. It utilizes the lightweight djb2 hash function and standard pseudo-random number generators. For real-world production environments, always rely on heavily audited, computationally expensive cryptography libraries like bcrypt or Argon2.
