# Remote Search Engine (RSE)

A terminal-based Client-Server application written in C++17 that allows users to:

- Search files using absolute paths
- Search files by filename recursively
- Search for a string inside files
- Display file contents
- Communicate using TCP sockets

## Prerequisites

Linux environment with:

```bash
g++
make
```

Check compiler:

```bash
g++ --version
```

---

## Build the Project

### Clean Previous Build

```bash
make clean
```

### Compile Server and Client

```bash
make
```

This generates:

```text
server
client
```

## Running the Application

Open **two terminals**.

### Terminal 1 – Start Server

```bash
./server
```

Expected output:

```text
[INFO] Server listening on port 56789
```


### Terminal 2 – Start Client

```bash
./client
```

Example menu:

```text
=============================
 Remote Search Engine (RSE)
=============================
1. Search File
2. Search File By Name
3. Search String
4. Display File Contents
5. Exit
=============================
```

## Features

### 1. Search File

Search using an absolute path.

Example:

```text
Choice: 1

Enter file path:
/etc/passwd
```

Output:

```text
File found: /etc/passwd
```

### 2. Search File By Name

Search recursively for a filename.

Example:

```text
Choice: 2

Enter filename:
notes.txt
```

Output:

```text
/home/user/docs/notes.txt
```

### 3. Search String

Search recursively for a word or pattern within files.

Example:

```text
Choice: 3

Enter search query:
localhost
```

Output:

```text
/etc/hosts:1
```


### 4. Display File Contents

Display the contents of a file.

Example:

```text
Choice: 4

Enter file path:
/etc/hosts
```

Output:

```text
127.0.0.1 localhost
127.0.1.1 trainux01
```

---

### 5. Exit

Disconnect the client.

```text
Choice: 5
```

The client terminates.

The server continues waiting for new client connections.

---

## Stopping the Server

Use:

```bash
Ctrl + C
```

from the server terminal.

---

## Logging

Example:

```text
[12:10:05][INFO ] Server listening on port 56789
[12:10:20][INFO ] Client connected
[12:10:22][DEBUG] Dispatching SEARCH_FILE
```

---

## Error Handling

Handled cases:

- File not found
- Invalid file path
- Permission denied
- Invalid client menu choice
- Empty search results
- File open failures

Example:

```text
Error: Cannot open file
```

---

## Build Commands Summary

```bash
make clean
make
./server
./client
```

---

## Technologies Used

- C++17
- POSIX Sockets
- Linux System Calls
- Filesystem APIs
- GNU Make
