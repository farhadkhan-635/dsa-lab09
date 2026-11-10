# DSA Open Ended Lab: Student Help Desk

Data Structures and Algorithms, Department of Computing

| | |
|---|---|
| Name | Farhad Khan |
| CMS ID | 549034 |
| Class | BSCS-15E |
| Instructor | Sara Tariq Sheikh |
| Lab Engineer | Hassan Mohtashim |
| Lab Date | 10 November 2026 |

## About

A small C++ help desk for students. It stores student records (unique roll number and name) in a binary search tree, and uses a queue of roll numbers to handle help requests in first come, first served order. Both data structures are written by hand with no STL containers, and all memory is released before the program exits.

## Files

| Name | What it is |
|---|---|
| 549034_OpenEndedLab.cpp | The whole program: design explanation, complexity comments, code, and a test plan at the bottom |
| .gitignore | Keeps compiled files and editor folders out of the repository |

## Menu

| Option | What it does |
|---|---|
| 1 | Add a student. Duplicate roll numbers are rejected. |
| 2 | Search by roll number. Shows the roll numbers checked on the way, and a clear message if the student is missing. |
| 3 | Display all students sorted by roll number, with the total and the tree height. |
| 4 | Help requests: add a request, serve the next request, show the waiting line. |
| 5 | Load seven sample students for a quick demo. |
| 0 | Exit and release all memory. |

## How to run

**Using an IDE (easiest)**

1. Open `549034_OpenEndedLab.cpp` in your IDE.
2. Press the compile and run button.
   - Dev-C++: F11
   - Code::Blocks: F9
   - Visual Studio: Ctrl + F5
   - VS Code with a C++ extension: the play button at the top right
3. A console window opens with the menu.

**Using the terminal (optional)**

```
g++ -std=c++11 549034_OpenEndedLab.cpp -o helpdesk
./helpdesk
```

On Windows use `.\helpdesk` for the second line.

## Sample output

Menu 5 then menu 3:

```
  No.      Roll No   Name
  -----------------------------------------
    1       546775   Zainab Fatima
    2       547890   Sara Noor
    3       548120   Ayesha Malik
    4       549034   Farhad Khan
    5       551236   Hamza Ali
    6       552301   Usman Tariq
    7       553410   Bilal Ahmed
  -----------------------------------------
  Total students: 7   Tree height: 4
```

Searching for 552301:

```
  Roll numbers checked: 549034 551236 552301
  FOUND: 552301 - Usman Tariq
  Nodes checked: 3
```

Duplicate and unregistered requests:

```
  Rejected: roll number 551236 is already registered.
  Rejected: roll number 123456 is not registered.
```

Help requests for 551236, 546775, 551236 are served in the same order:

```
  Now serving: 551236 - Hamza Ali
  Now serving: 546775 - Zainab Fatima
  Now serving: 551236 - Hamza Ali
  No requests are waiting.
```

## Design choices

- **BST for students:** searches skip half of the remaining tree at every step, there is no fixed size, and printing in inorder gives sorted roll numbers for free.
- **Linked queue for requests:** a help desk is first in, first out. Adding at the rear and serving from the front are both O(1) and nothing is shifted.
- **Queue stores only roll numbers:** the name is looked up in the BST, so each student is saved in one place only.
- **Alternative:** an AVL tree would keep search and insertion at O(log n) even for sorted input, and a hash table would search in O(1) on average but cannot print in sorted order.

## Time complexity (worst case)

| Operation | Time |
|---|---|
| Add a student | O(h), O(n) for a chain-shaped tree |
| Search a student | O(h), O(n) for a chain-shaped tree |
| Display all | O(n) |
| Add a help request | O(1) for the queue, plus an O(h) check that the student exists |
| Serve a request | O(1) for the queue, plus an O(h) lookup for the name |

Here h is the height of the tree. It is about log2(n) when roll numbers arrive in mixed order and n when they arrive sorted.

## Testing

A step by step test plan with expected results is at the bottom of the source file. It covers empty structures, a duplicate roll number, a missing roll number, repeated requests, and an unregistered student asking for help.

## Memory safety

- Every node is created with `new (nothrow)` and checked, so running out of memory shows a message instead of crashing.
- A student node is created only after the roll number is confirmed new, so a rejected duplicate never leaks.
- `freeTree()` releases the tree children first and the node last, and `freeQueue()` releases any requests still waiting.
- The program prints how many records it released when it exits.
- It compiles with no warnings under `-Wall -Wextra -pedantic` and ran with AddressSanitizer and UndefinedBehaviorSanitizer with no errors or leaks.

## Reference

Goodrich, M. T., Tamassia, R., & Mount, D. M. (2011). *Data Structures and Algorithms in C++* (2nd ed.). Wiley.
