/*
=====================================================================
 Department of Computing
 CS: Data Structures and Algorithms
 Open Ended Lab: Applying Data Structures
 Project: Student Help Desk
=====================================================================
 Name         : Farhad Khan
 CMS ID       : 549034
 Section      : BSCS-15E
 Instructor   : Sara Tariq Sheikh
 Lab Engineer : Hassan Mohtashim
 Lab Date     : 10 November 2026
=====================================================================

 WHAT THIS PROGRAM DOES
 ----------------------
 This is a small help desk for students. The desk can:
   1. Add a student (roll number and name)
   2. Search for a student by roll number
   3. Display all students
   4. Manage help requests (my extra feature, a queue)
   5. Load sample students, so a demo is quick
   0. Exit

 TASK 1: MY DESIGN (WHY I PICKED THESE TWO DATA STRUCTURES)
 ----------------------------------------------------------
 Data structure 1: Binary Search Tree (BST) for the students.
   - Every student goes in a tree node, ordered by roll number.
     Smaller roll numbers go left, bigger ones go right.
   - Searching is fast. At every node I can throw away half of
     the remaining tree, so I do not check every student.
   - An array has a fixed size and can become full. It also needs
     shifting to keep the numbers sorted.
   - A linked list has no size limit, but searching it always
     means walking from the first node, one by one.
   - A BST has no size limit, searches fast, and when I print it
     in inorder the students come out sorted by roll number
     without any extra sorting work.
   - The BST also stops duplicates naturally, because during
     insertion I walk down the tree and will meet the same roll
     number if it already exists.

 Data structure 2: Queue (made with linked nodes) for help requests.
   - A help desk works first come, first served. The student who
     asked first must be helped first. That is exactly what a
     queue does (First In, First Out).
   - New requests are added at the rear. The next student to help
     is taken from the front.
   - With linked nodes the queue can grow as needed, so it never
     gets full, and nothing needs to be shifted when we remove
     someone. Both operations are quick.
   - The queue stores only the roll number. The name is found by
     searching the BST when it is needed. This way a student's
     data is saved in one place only.
   - The same student may ask for help many times, so repeated
     requests are allowed.
   - A request is accepted only if the roll number is registered
     in the BST. This is where the two structures work together.

 NOTE ABOUT FULL ARRAYS
   I did not use fixed size arrays for the records, so there is no
   "array is full" case. The only way to run out of space is when
   the computer has no memory left, and I check for that too. Every
   "new" in this program is checked, and the user gets a message if
   it fails.

 TIME COMPLEXITY (WORST CASE)
 ----------------------------
 Let n be the number of students and h be the height of the tree.
   - Add a student    : O(h). In the worst case this is O(n).
                        The worst case happens when roll numbers
                        are added in sorted order (for example
                        1, 2, 3, 4, 5). Every new student goes to
                        the right and the tree becomes a straight
                        chain with h = n. If the roll numbers come
                        in a mixed order the tree stays fairly
                        balanced and h is about log2(n).
   - Search a student : O(h). The worst case is also O(n), for the
                        same reason as above.
   - Display all      : O(n), every student is printed once.
   - Add help request : O(1) for the queue itself. The check that
                        the student is registered is a search, so
                        the full action costs O(h).
   - Serve a request  : O(1) for the queue. Finding the name
                        to show costs O(h).

 ONE ALTERNATIVE DATA STRUCTURE
 ------------------------------
 An AVL tree (a self balancing BST) would keep h at about log2(n)
 all the time, so add and search would always be O(log n), even
 if roll numbers come in sorted order. A hash table would give
 O(1) search on average, but it cannot print students in sorted
 order without sorting them first.

 ACKNOWLEDGEMENT
 ---------------
 The BST and queue ideas come from my class notes and my earlier
 labs (Lab 7 queues and Lab 8 BST). I got help from Claude (an AI
 assistant) in writing and commenting of  this program. I have read all of
 it and I can run it and explain it.

 HOW TO COMPILE AND RUN
 ----------------------
   g++ -std=c++11 549034_OpenEndedLab.cpp -o helpdesk
   ./helpdesk          (on Windows use .\helpdesk)
 A test plan with expected results is at the bottom of this file.
=====================================================================
*/

#include <iostream>   // cout, cin, endl
#include <iomanip>    // setw, to line up the table columns
#include <cstring>    // strncpy, to copy the name text
#include <new>        // nothrow, so a failed "new" gives nullptr
using namespace std;

// ---------------------------------------------------------------
// CONSTANTS AND DATA TYPES
// ---------------------------------------------------------------

// Longest name we keep is 49 letters. The last place in the
// array is kept for the end of text marker ('\0').
const int NAME_SIZE = 50;

// One student record: a unique roll number and a name.
struct Student {
    int  rollNo;
    char name[NAME_SIZE];
};

// One node of the BST. It holds a student and two child pointers.
struct StudentNode {
    Student      data;
    StudentNode* left;    // roll numbers smaller than this one
    StudentNode* right;   // roll numbers bigger than this one
};

// One node of the help request queue. It holds only a roll number.
struct RequestNode {
    int          rollNo;
    RequestNode* next;    // the student waiting right behind this one
};

// The queue itself. "front" is the next student to be helped and
// "rear" is the last student who asked. "count" tells how many
// requests are waiting, so we never have to count the nodes.
struct HelpQueue {
    RequestNode* front;
    RequestNode* rear;
    int          count;
};

// The three things that can happen when we try to add a student.
enum InsertResult {
    INSERT_OK,          // student was added
    INSERT_DUPLICATE,   // that roll number is already in the tree
    INSERT_NO_MEMORY    // the computer could not give us space
};

// This turns true when the input ends (for example when a file is
// piped in). It lets all the menu loops stop instead of running
// forever.
bool inputClosed = false;

// ---------------------------------------------------------------
// INPUT HELPERS
// ---------------------------------------------------------------

// Reads one whole number. If the user types letters, we clear the
// error, throw away that line and ask again. Without this, cin
// would get stuck and the menu would loop forever.
int readInt(const char* prompt) {
    int value = 0;
    cout << prompt;
    while (!(cin >> value)) {
        if (cin.eof()) {          // no more input at all
            inputClosed = true;
            return 0;
        }
        cin.clear();                  // remove the error flag
        cin.ignore(10000, '\n');      // throw away the bad line
        cout << "That is not a number. Please try again: ";
    }
    cin.ignore(10000, '\n');      // remove the Enter key left behind
    return value;
}

// Reads a name (spaces allowed) into the given array.
void readName(char name[]) {
    cout << "Enter name: ";
    cin.getline(name, NAME_SIZE);
    if (cin.eof() && name[0] == '\0') {
        inputClosed = true;
        return;
    }
    if (cin.fail()) {             // the name was longer than we can store
        cin.clear();
        cin.ignore(10000, '\n');  // skip the extra letters
        cout << "  (Name was too long, so it was cut to "
             << (NAME_SIZE - 1) << " letters.)" << endl;
    }
}

// ---------------------------------------------------------------
// BST FUNCTIONS (the student list)
// ---------------------------------------------------------------

// Makes one new tree node. Both child pointers start as nullptr.
// Returns nullptr if the computer is out of memory.
StudentNode* createStudentNode(int rollNo, const char name[]) {
    StudentNode* node = new (nothrow) StudentNode;
    if (node == nullptr) {
        return nullptr;
    }
    node->data.rollNo = rollNo;
    strncpy(node->data.name, name, NAME_SIZE - 1);
    node->data.name[NAME_SIZE - 1] = '\0';   // make sure the text ends
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

// Adds a student to the BST without using recursion.
// "root" is passed by reference because the very first student
// becomes the new root and we must change the caller's pointer.
//
// Time: O(h), worst case O(n) when the tree is a straight chain.
InsertResult insertStudent(StudentNode*& root, int rollNo, const char name[]) {
    StudentNode* parent = nullptr;     // the node just above "current"
    StudentNode* current = root;

    // Walk down the tree to find the empty place for this number.
    while (current != nullptr) {
        parent = current;
        if (rollNo == current->data.rollNo) {
            return INSERT_DUPLICATE;   // already registered, stop here
        } else if (rollNo < current->data.rollNo) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    // We only create the node now, after we know the roll number is
    // new. This way a rejected duplicate never wastes memory.
    StudentNode* newNode = createStudentNode(rollNo, name);
    if (newNode == nullptr) {
        return INSERT_NO_MEMORY;
    }

    // Hang the new node under its parent.
    if (parent == nullptr) {
        root = newNode;                       // the tree was empty
    } else if (rollNo < parent->data.rollNo) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }
    return INSERT_OK;
}

// Searches the BST for a roll number without using recursion.
// Returns the node if found, or nullptr if it is missing.
// "nodesChecked" is filled with how many nodes we looked at.
// If "showPath" is true, it prints each roll number we compared.
//
// Time: O(h), worst case O(n) when the tree is a straight chain.
StudentNode* searchStudent(StudentNode* root, int rollNo,
                           int& nodesChecked, bool showPath) {
    nodesChecked = 0;
    StudentNode* current = root;

    if (showPath) {
        cout << "  Roll numbers checked: ";
    }
    while (current != nullptr) {
        nodesChecked++;
        if (showPath) {
            cout << current->data.rollNo << " ";
        }
        if (rollNo == current->data.rollNo) {
            if (showPath) cout << endl;
            return current;                   // found it
        } else if (rollNo < current->data.rollNo) {
            current = current->left;          // smaller, go left
        } else {
            current = current->right;         // bigger, go right
        }
    }
    if (showPath) cout << endl;
    return nullptr;                           // fell off the tree, not found
}

// Prints all students in order of roll number (smallest first).
// Inorder means: left side, then this node, then right side.
// "rowNumber" counts the rows, so the table gets 1, 2, 3 ...
//
// Time: O(n), every node is visited exactly once.
void printInorder(StudentNode* node, int& rowNumber) {
    if (node == nullptr) {         // base case: nothing here
        return;
    }
    printInorder(node->left, rowNumber);
    rowNumber++;
    cout << "  " << setw(3) << rowNumber << "   "
         << setw(10) << node->data.rollNo << "   "
         << node->data.name << endl;
    printInorder(node->right, rowNumber);
}

// Height of the tree. An empty tree has height 0 and a tree with
// only a root has height 1. It tells us the worst case number of
// nodes a search would have to check.
int treeHeight(StudentNode* node) {
    if (node == nullptr) {
        return 0;
    }
    int leftHeight = treeHeight(node->left);
    int rightHeight = treeHeight(node->right);
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// Deletes the whole tree. Postorder means both children are deleted
// first and the node itself last. If we deleted the node first, we
// would lose the pointers to its children. Returns how many nodes
// were deleted.
int freeTree(StudentNode* node) {
    if (node == nullptr) {
        return 0;
    }
    int freed = 0;
    freed += freeTree(node->left);
    freed += freeTree(node->right);
    delete node;
    return freed + 1;
}

// ---------------------------------------------------------------
// QUEUE FUNCTIONS (the help requests)
// ---------------------------------------------------------------

// Sets up an empty queue.
void initQueue(HelpQueue& q) {
    q.front = nullptr;
    q.rear = nullptr;
    q.count = 0;
}

bool isQueueEmpty(const HelpQueue& q) {
    return q.front == nullptr;
}

// Adds a request at the rear (the back of the line).
// Returns false only if the computer is out of memory.
//
// Time: O(1). We go straight to the rear, no walking.
bool enqueue(HelpQueue& q, int rollNo) {
    RequestNode* node = new (nothrow) RequestNode;
    if (node == nullptr) {
        return false;
    }
    node->rollNo = rollNo;
    node->next = nullptr;

    if (q.rear == nullptr) {       // queue was empty
        q.front = node;            // so the new node is also the front
    } else {
        q.rear->next = node;       // join it behind the old last node
    }
    q.rear = node;
    q.count++;
    return true;
}

// Removes the request at the front (the earliest one still waiting).
// The roll number goes into "rollNo". Returns false if the queue
// was empty, so the caller can show a message.
//
// Time: O(1). We only touch the front node.
bool dequeue(HelpQueue& q, int& rollNo) {
    if (q.front == nullptr) {
        return false;
    }
    RequestNode* oldFront = q.front;
    rollNo = oldFront->rollNo;
    q.front = oldFront->next;
    if (q.front == nullptr) {      // the line is now empty
        q.rear = nullptr;          // do not leave rear pointing at freed memory
    }
    delete oldFront;
    q.count--;
    return true;
}

// Deletes every request that is still waiting. Returns how many.
int freeQueue(HelpQueue& q) {
    int freed = 0;
    int ignoredRoll;
    while (dequeue(q, ignoredRoll)) {
        freed++;
    }
    return freed;
}

// ---------------------------------------------------------------
// MENU ACTIONS
// ---------------------------------------------------------------

void addStudentAction(StudentNode*& root) {
    cout << endl << "--- Add a student ---" << endl;
    int rollNo = readInt("Enter roll number: ");
    if (inputClosed) return;
    if (rollNo <= 0) {
        cout << "  Rejected: roll number must be a positive number." << endl;
        return;
    }

    char name[NAME_SIZE];
    readName(name);
    if (inputClosed) return;
    if (name[0] == '\0') {
        cout << "  Rejected: the name cannot be empty." << endl;
        return;
    }

    InsertResult result = insertStudent(root, rollNo, name);
    if (result == INSERT_OK) {
        cout << "  Added: " << rollNo << " - " << name << endl;
    } else if (result == INSERT_DUPLICATE) {
        cout << "  Rejected: roll number " << rollNo
             << " is already registered." << endl;
    } else {
        cout << "  Error: the computer is out of memory." << endl;
    }
}

void searchAction(StudentNode* root) {
    cout << endl << "--- Search a student ---" << endl;
    if (root == nullptr) {
        cout << "  There are no students yet. Add some first." << endl;
        return;
    }
    int rollNo = readInt("Enter roll number to search: ");
    if (inputClosed) return;

    int nodesChecked = 0;
    StudentNode* found = searchStudent(root, rollNo, nodesChecked, true);
    if (found != nullptr) {
        cout << "  FOUND: " << found->data.rollNo << " - "
             << found->data.name << endl;
    } else {
        cout << "  NOT FOUND: no student has roll number "
             << rollNo << "." << endl;
    }
    cout << "  Nodes checked: " << nodesChecked << endl;
}

void displayAction(StudentNode* root) {
    cout << endl << "--- All students (sorted by roll number) ---" << endl;
    if (root == nullptr) {
        cout << "  There are no students yet. Add some first." << endl;
        return;
    }
    cout << "  " << setw(3) << "No." << "   " << setw(10) << "Roll No"
         << "   Name" << endl;
    cout << "  -----------------------------------------" << endl;
    int rowNumber = 0;
    printInorder(root, rowNumber);
    cout << "  -----------------------------------------" << endl;
    cout << "  Total students: " << rowNumber
         << "   Tree height: " << treeHeight(root) << endl;
}

// Adds seven sample students so the program can be demonstrated
// quickly. If some of them are already there, the duplicate check
// reports it, so this is safe to run more than once.
void loadSampleAction(StudentNode*& root) {
    const int SAMPLE_COUNT = 7;
    int rolls[SAMPLE_COUNT] = {549034, 548120, 551236, 547890,
                               552301, 546775, 553410};
    const char* names[SAMPLE_COUNT] = {"Farhad Khan", "Ayesha Malik",
                                       "Hamza Ali", "Sara Noor",
                                       "Usman Tariq", "Zainab Fatima",
                                       "Bilal Ahmed"};
    cout << endl << "--- Load sample students ---" << endl;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        InsertResult result = insertStudent(root, rolls[i], names[i]);
        if (result == INSERT_OK) {
            cout << "  Added: " << rolls[i] << " - " << names[i] << endl;
        } else if (result == INSERT_DUPLICATE) {
            cout << "  Skipped: " << rolls[i] << " is already registered."
                 << endl;
        } else {
            cout << "  Error: the computer is out of memory." << endl;
            return;
        }
    }
}

// ---------------------------------------------------------------
// HELP REQUEST FEATURE (the queue)
// ---------------------------------------------------------------

// Adds a help request, but only for a registered student.
void addRequestAction(StudentNode* root, HelpQueue& q) {
    cout << endl << "--- Add a help request ---" << endl;
    int rollNo = readInt("Enter roll number asking for help: ");
    if (inputClosed) return;

    // Look the student up first. showPath is false here because we
    // only want the answer, not a printout.
    int nodesChecked = 0;
    StudentNode* student = searchStudent(root, rollNo, nodesChecked, false);
    if (student == nullptr) {
        cout << "  Rejected: roll number " << rollNo
             << " is not registered." << endl;
        return;
    }

    if (enqueue(q, rollNo)) {
        cout << "  Request added for " << student->data.name
             << ". Place in line: " << q.count << endl;
    } else {
        cout << "  Error: the computer is out of memory." << endl;
    }
}

// Serves the student who has been waiting the longest.
void serveRequestAction(StudentNode* root, HelpQueue& q) {
    cout << endl << "--- Serve next request ---" << endl;
    int rollNo = 0;
    if (!dequeue(q, rollNo)) {
        cout << "  No requests are waiting." << endl;
        return;
    }
    int nodesChecked = 0;
    StudentNode* student = searchStudent(root, rollNo, nodesChecked, false);
    cout << "  Now serving: " << rollNo;
    if (student != nullptr) {
        cout << " - " << student->data.name;
    }
    cout << endl << "  Requests still waiting: " << q.count << endl;
}

// Shows the waiting line from the front to the rear without
// removing anything. We use a temporary pointer, so the queue
// itself is not changed.
void showRequestsAction(StudentNode* root, const HelpQueue& q) {
    cout << endl << "--- Waiting requests (front of line first) ---" << endl;
    if (isQueueEmpty(q)) {
        cout << "  No requests are waiting." << endl;
        return;
    }
    int place = 1;
    for (RequestNode* current = q.front; current != nullptr;
         current = current->next) {
        int nodesChecked = 0;
        StudentNode* student = searchStudent(root, current->rollNo,
                                             nodesChecked, false);
        cout << "  " << setw(3) << place << "   " << setw(10)
             << current->rollNo << "   "
             << (student != nullptr ? student->data.name : "(unknown)");
        if (place == 1) {
            cout << "   <- served next";
        }
        cout << endl;
        place++;
    }
    cout << "  Total waiting: " << q.count << endl;
}

void helpDeskMenu(StudentNode* root, HelpQueue& q) {
    int choice = -1;
    while (choice != 0 && !inputClosed) {
        cout << endl << "========== HELP REQUESTS ==========" << endl;
        cout << "  Requests waiting: " << q.count << endl;
        cout << "  1. Add a help request" << endl;
        cout << "  2. Serve the next request" << endl;
        cout << "  3. Show waiting requests" << endl;
        cout << "  0. Back to main menu" << endl;
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addRequestAction(root, q);   break;
            case 2: serveRequestAction(root, q); break;
            case 3: showRequestsAction(root, q); break;
            case 0:                              break;
            default: cout << "  Please choose 0, 1, 2 or 3." << endl;
        }
    }
}

// ---------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------

void showMainMenu() {
    cout << endl;
    cout << "==========================================" << endl;
    cout << "            STUDENT HELP DESK" << endl;
    cout << "==========================================" << endl;
    cout << "  1. Add a student" << endl;
    cout << "  2. Search a student by roll number" << endl;
    cout << "  3. Display all students" << endl;
    cout << "  4. Help requests (queue)" << endl;
    cout << "  5. Load sample students (for demo)" << endl;
    cout << "  0. Exit" << endl;
}

int main() {
    StudentNode* root = nullptr;    // the BST starts empty
    HelpQueue requests;
    initQueue(requests);            // the queue starts empty

    int choice = -1;
    while (choice != 0 && !inputClosed) {
        showMainMenu();
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addStudentAction(root);               break;
            case 2: searchAction(root);                   break;
            case 3: displayAction(root);                  break;
            case 4: helpDeskMenu(root, requests);         break;
            case 5: loadSampleAction(root);               break;
            case 0:                                       break;
            default: cout << "  Please choose a number from 0 to 5." << endl;
        }
    }

    // Give all memory back before the program ends.
    int freedRequests = freeQueue(requests);
    int freedStudents = freeTree(root);
    root = nullptr;
    cout << endl << "Memory released: " << freedStudents
         << " student record(s) and " << freedRequests
         << " waiting request(s)." << endl;
    cout << "Goodbye!" << endl;
    return 0;
}

/*
=====================================================================
 TEST PLAN (Task 3)
=====================================================================
 Start the program and try these in order. The expected result is
 written next to each step.

 A. Empty structures (do this first, before adding anything)
    - Menu 2 (search)             : "There are no students yet"
    - Menu 3 (display)            : "There are no students yet"
    - Menu 4, then 2 (serve)      : "No requests are waiting"
    - Menu 4, then 3 (show)       : "No requests are waiting"

 B. Adding students (menu 5 loads these seven, or add your own)
      549034 Farhad Khan      548120 Ayesha Malik
      551236 Hamza Ali        547890 Sara Noor
      552301 Usman Tariq      546775 Zainab Fatima
      553410 Bilal Ahmed
    - Menu 3 : all seven are listed in sorted order by roll number,
               Total students: 7, Tree height: 4

 C. Duplicate roll number
    - Menu 1, enter 551236 : "Rejected: roll number 551236 is
      already registered." The list still has seven students.

 D. Search
    - Menu 2, enter 552301 : FOUND, Usman Tariq. Roll numbers
      checked: 549034 551236 552301 (3 nodes).
    - Menu 2, enter 999999 : NOT FOUND (a clear message).

 E. Help request queue (first in, first out)
    - Menu 4, then 1 with 551236, then 1 with 546775, then 1 with
      551236 again (repeated requests are allowed).
    - Menu 4, then 1 with 123456 : "Rejected ... not registered."
    - Show waiting requests: 1) Hamza Ali  2) Zainab Fatima
      3) Hamza Ali
    - Serve three times: Hamza Ali, then Zainab Fatima, then
      Hamza Ali again. A fourth serve says "No requests are
      waiting."

 F. Exit with 0. The program prints how many records it released.

 EXPLAINING ONE OPERATION WITH THE SAMPLE DATA
 Searching for 552301 in the sample tree:
   - Start at the root, 549034. 552301 is bigger, so go right.
   - Next node is 551236. 552301 is bigger, so go right again.
   - Next node is 552301. It matches, so the student is found
     after checking 3 nodes. We never looked at the left side of
     the root at all. That is why a BST search is faster than
     checking all 7 students one by one.
=====================================================================
*/
