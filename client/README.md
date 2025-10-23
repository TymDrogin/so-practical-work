# Client Module

The Client program handles user interactions for the system.
It reads information from clients.txt and identifies registered clients.

---

## Build Instructions

1. Navigate to the client directory:
   cd client

2. Compile the project:
   make

This will create the executable inside the build/ directory.

---

## Run Instructions

You can run the client in one of two ways:

Option 1 — From the project root:
   ./build/client <ClientName>

Option 2 — From inside the build directory:
   cd build# so-practical-work
Practical assignement for the SO subject
   ./client <ClientName>

Alternatively, you can use the provided Makefile target:
   make run ARGS="<ClientName>"

Example:
   make run ARGS="Joao"

---

## Notes

- The program supports being run either from the project root or from within build/.
- The clients.txt file must remain in the client/ directory to preserve persistent data.
- When running the program, ensure clients.txt is accessible according to your current working directory.

---