# Welcome to SP2020!
## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

```C
#include <unistd.h>
int main() {
	write(1, "Hi! My name is Nidhi.", 21);
	return 0;
}
```

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
```C
void write_triangle(int n){
	int i;
	for(i = 1; i <= n; i++){
		int j;
		for(j = 0; j < i; j++){
			write(STDERR_FILENO, "*", 1);
		  }
		write(STDERR_FILENO, "\n", 1);
	}
}
```
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
```C
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int filedes = open("hello_world.txt", O_CREAT|O_TRUNC|O_RDWR, mode);
	write(filedes, "Hi! My name is Nidhi.", 21);
	close(filedes);
	return 0;
}
```
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
```C
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	close(1);
	int filedes = open("hello_world.txt", O_CREAT|O_TRUNC|O_RDWR, mode);
	printf("Hi! My name is Nidhi.\n");
	close(filedes);
	return 0;
}
```
5. What are some differences between `write()` and `printf()`?
```c
-Write() is a system call, printf() is a standard library function
-Write() only prints a sequence of bits, printf() supports multiple data types (such as int)
-Printf() uses a buffer, write() does not
-Printf() transforms arguments into a sequence of characters and eventually calls write()
```
## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
At least 1
2. How many bytes are there in a `char`?
1
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`
```c
int: 4
double: 8
float: 4
long: 4
long long: 8
```
### Follow the int pointer
4. If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?
Data + 2 : 0x7fbd9d50

5. What is `data[3]` equivalent to in C?
Data + 3

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
The string is in read-only memory, so it can not be written to.
7. What does `sizeof("Hello\0World")` return?
12
8. What does `strlen("Hello\0World")` return?
5
9. Give an example of X such that `sizeof(X)` is 3.
X = "ab"
10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.
Y = 5
## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?
```c
-Check argc (first argument in main), it is equal to # of arguments + 1
-Loop through argv, count as you go until you reach the end of the array
```
2. What does `argv[0]` represent?
execution name of program
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?
Somewhere else (not stack nor heap)
### String searching (strings are just char arrays)
4. What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?
```c
sizeof(ptr) is ptr is a pointer
sizeof(array) is 6, the amount of memory required to hold the entire array on the stack
```
### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables? stack

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
heap via malloc()
2. What are the differences between heap and stack memory?
```c
- Data in stack automatically managed, memory is deallocated for a variable when it goes out of scope
- Data in heap is manually requested,  memory must be explicity deallocated by program
```
3. Are there other kinds of memory in a process?
 Yes, constants, environment vars, kernel are stored in other memory
4. "In a good C program, for every malloc, there is a FREE()".
### Heap allocation gotchas
5. What is one reason `malloc` can fail?
  Not enough memory available for the amount of bytes requested
6. What are some differences between `time()` and `ctime()`?
```c
- ctime() formats a given time nicely (human readable) and returns time as char*,
- time() which gives us the current time as an int
- Ctime() returns time as static, so it will be overwritten if called again
```
7. What is wrong with this code snippet?
  Double free
8. What is wrong with this code snippet?
  Attempts to use memory that does not belong to program
9. How can one avoid the previous two mistakes? 
  Set a pointer to null after it has been freed
### `struct`, `typedef`s, and a linked list
10-13:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct person{
	char* name;
	int age;
	struct person** friends;
};
typedef struct person person_t;

person_t* person_create(char* name, int age){
	person_t* result = (person_t*)malloc(sizeof(person_t));
	result-> age = age;
	result->friends = (person_t**)malloc(sizeof(person_t*) * 10);
	result->name = strdup(name);
	return result;
}

void person_destroy(person_t* p){
	free(p->name);
	free(p->friends);
	memset(p, 0, sizeof(person_t));
	free(p);
}

int main() {
	person_t* asmith = person_create("Agent Smith", 128);
	person_t* smoore = person_create("Sonny Moore", 256);
	*(asmith->friends) = smoore;
	*(smoore->friends) = asmith;
	person_destroy(asmith);
	person_destroy(smoore);
	return 0;
}

```

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?
```c
- Getting from stdin: getchar(), gets()
- Writing to stdout: puts(), 
```
2. Name one issue with `gets()`.
  Gets() requires pre-allocating a buffer to a size. However, it still accepts input larger than the buffer, which results in overflow. Overflow can cause memory corruption and result in undefined behavior.
### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".
```c
char* data = "Hello 5 World";
char buff1[10];
char buff2[10];
int num = -5;
sscanf(data, "%s %d %s", buff1, &num, buff2);
```
### `getline` is useful
4. What does one need to define before including `getline()`?
  Define buffer(char*)  to null and capacity(size_t) to 0.
5. Write a C program to print out the content of a file line-by-line using `getline()`.
```c
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[]) {
	if(argc != 2){
		return 1;
	}
	FILE* stream = fopen(argv[1], "r");
	if(stream == NULL){
		return 1;
	}
	char* buffer = NULL;
	size_t capacity = 0;
	int result = getline(&buffer, &capacity, stream);
	while(result != -1){
		printf("%s", buffer);
		result = getline(&buffer, &capacity, stream);
	}
	return 0;
}
```
## C Development
1.	-g
2.	A change in the makefile means that dependencies may be affected. Make will attempt to reuse preexisting object files, which may not be the desired result. Just using make will not necessarily reflect the change in the Makefile.
3.	Tabs
4.	Git commit records changes in your (local) repository. A sha (in git) is a hash of the binary representation of a git object. It is used as an identifier for a commit.
5.	Git log shows a summary of commit history (name, desc, time/date) for your branch.
6.	Git status shows a list of all files, and their status relative to the last commit (unchanged, changed, new, etc.). If a file is listed in .gitignore, it will not show up in that list of files.
7.	 A commit only stores the changes locally. Git push sends your local commit history to a server. This means that the commit is now viewable (and testable) on the web, and that it can now be accessed from any machine (with credentials).
8.	Non-fast forward git push error means that your current repo is not up to date with the history git has. This typically occurs when someone else pushes to the same branch as you, and the changes have not been pulled/merged. This issue can be solved by pulling the latest version of your repo and merging changes.
