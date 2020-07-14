# bash shell

### How to run:
``` ./shell ```

#### Optional arguments:

```-h``` : name of history file, loads previous history from file. If none exists, the program will create one. This will also save the history of the current session upon exiting.

```-f``` : name of filename, contains list of commands (separated by newlines). Will run commands one after another until EOF is reached.

### Supported shell comamands:

To exit: ```exit``` or EOF (Ctrl-D) at beginning of line. Kills any background processes.

```cd {$PATH}``` changes directory to that of path

```ls``` lists contents of directory

```!history``` prints history of commands, including that of history file (if specified)

```#{i}``` re-executes the ith command in history if i is valid

```!{$PREFIX}``` re-executes the lastest command in history that has the matching prefix

#### External commands:
commands that are not recognized as built-in will be ran by creating a child process.

##### Logical operators: 
```&&, ||, ; ``` have similar usage and behavior as their counterparts in a typical bash shell. This shell does NOT support the chaining of logical operators.

Background process: Any command followed by a space and ampersand, like so: ``` &```.

```ps``` prints out information about all processes being executing by the shell.

#### Redirection operators: 
``` >>, >, < ``` have similar usage and behavior as their counterparts in a typical bash shell.

```>``` places output of command into specified file.

```>>``` appends output of command into file.

```<``` places file contents into stdin of command.

#### Signals:
```kill {$PID}``` kills process with specified PID by sending SIGTERM

```stop {$PID}``` stops (pauses) process with specified PID by sending SIGSTP

```cont {$PID}``` resumes process with specified PID by sending SIGCONT
