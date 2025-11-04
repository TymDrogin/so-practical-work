# Client Module

The Client program handles user interactions for the system.
It reads information from clients.txt and identifies registered clients.

---

## Build Instructions

1. Navigate to the client directory if not inside of it already:

```bash
cd client
```

2. Compile the project:

```bash
make
```

This will create the executable inside the build/ directory.

---

## Run Instructions

To run the client flom the client dir use this:

```bash
   ./build/client **ClientName**
```

Alternatively, you can use the provided Makefile target:

```bash
   make run ARGS="**ClientName**"
```

---

