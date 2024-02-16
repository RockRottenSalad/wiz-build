# WIZ BUILD

I'm currently working on a different library that I'm calling "[wizardry](https://github.com/RockRottenSalad/wizardry)",
which is supposed to be a general purpose library that implements data structures and functions I often need in C.
This project is somewhat related in that it's supposed to be a part of it. But I decided to make it its own thing.
<br><br>
This project is a very simple build system for C.(Not super functional as of now)
I was inspired by a different project called "[nobuild](https://github.com/tsoding/nobuild)".
Imagine a Makefile if it were to be written in C(and if it were 10 times uglier). That's pretty much all you need to know.
<br><br>
## Progress(Somewhat functional features)
- Macro for running shell commands
- Macro for looping through a directory 
- Basic logging
- Build file will recompile it self upon execution if source file has been edited 
- Memory managed via one fat malloc() which functions like a scratch allocator
- Rule to compare binary version versus source file version for files besides the build tool itself

## To-do list
- Test what I currently have made by using it as a build tool for a small project
- Maybe less hideous string macros(less hideous macros in general to be honest)
- Documentation
- Figure out what else to put here
