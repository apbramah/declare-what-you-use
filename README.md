# declare-what-you-use

"Declare what you use" (DWYU) means this: for every declaration that you make across your project, there should be at least one reference.

## Introduction

### Why DWYU?

The compiler/linker will always ensure that you declare everything that you use - if you don't, the compiler or linker will throw an error.
However, if you don't use what you declare, you will not be informed by the build process. "Declare what you use" ensures that you are
informed about every unused declaration within your project. So, by using DWYU, you can ensure:

* Applications and libraries do not contain any unused source code
* That every API of a library has been somehow tested
* That every field in a structure has a purpose

### Inspiration

DWYU is inspired in great part by the fantastic [include-what-you-use](https://include-what-you-use.org/). You will notice some similarities
in style and documentation.

### Caveat

DWYU was written to support a C project. It has therefore not been extensively tested against C++ projects (but it ought to work!)

## Dependencies

DWYU has a number of dependencies. These dependencies are described in [Dockerfile](Dockerfile). Building a Docker image
using this Dockerfile is the easiest way to experiment with DWYU.

## Installing DWYU

The following steps assume that you have installed the necessary dependencies.

```
$ git clone https://github.com/apbramah/declare-what-you-use.git
$ cd declare-what-you-use
declare-what-you-use$ mkdir build; cd build
declare-what-you-use/build$ cmake ..
declare-what-you-use/build$ make install
```

The last command is likely to require super-user privileges.

## Components of DWYU

The installation procedure above installs two components:

* `dwyu` is a binary executable which is at the heart of DWYU
* `dwyu.sh` is a bash wrapper script which makes `dwyu` more useful in typical development scenarios

Advanced users may chose to supply their own wrapper scripts particular to their development flows.

## DWYU tutorial

This tutorial shows the use of the `dwyu.sh` wrapper script. It assumes that DWYU has been installed and makes use of the example within `example/example1`.

First, build the example and create the compile commands file:

```
declare-what-you-use/example/example1$ mkdir build; cd build
declare-what-you-use/example/example1/build$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
declare-what-you-use/example/example1/build$ make
```

As well as building the example, this generates a file called `compile_commands.json`. We provide this file as input to `dwyu.sh`, and supply a
`.dwyuignore` file:

```
declare-what-you-use/example/example1/build$ dwyu.sh -i ../.dwyuignore compile_commands.json
```

The following output is produced (along with a non-zero exit status):

```
Remove these from ../.dwyuignore:
Remove these unreferenced declarations from your code:
Function: bar   /home/andy/dev/declare-what-you-use/examples/example1/foo.c:7:5
```

This tells us that the function `bar` is declared, but not referenced anywhere in the project. There are three possible remedial actions:

* Use `bar`
* Remove `bar`
* Add `bar` to the `.dwyuignore` file

Once the fault is remedied, subsequent use of the `dwyu.sh` command as above will produce no output and a zero exit status.

## FAQs

### The example just shows an unused function. Doesn't the linker remove unused functions?

Probably, yes. So your binary won't be cluttered with the unused function. But your source code will be, and therefore unused functions
should be deleted.

### Does this only work with functions?

No. It works with any declaration. So (for example) if you declare a struct but never reference it, or a field within a struct and never
reference that, or indeed any unused declaration at all, then DWYU will raise an error. Try running it on one of your own projects and
see how much source code wasn't actually needed...

### The example `.dwyuignore` file lists the `main` function. Why?

The `dwyu` executable at the heart of DWYU is quite simplistic - it lists **every** declaration in your source code which is not
referenced elsewhere. The function `main` isn't referenced, so it must be ignored by `.dwyuignore`.

### Am I safe to delete every unused declaration identified by DWYU?

Usually, yes. However, take care if your project is built in multiple configurations. DWYU only 'knows' about the configurations it has
been provided with. Concatenation of `compile_commands.json` files can allow DWYU to only show unused declarations across the union
of the configurations.

### How do I prevent DWYU raising errors on 3rd-party libraries?

Use the `-k` option of `dwyu.sh` to provide a regex expression to say which file paths are relevant to your own source files.

### How do I apply DWYU to an existing project with lots of DWYU errors?

We recommend starting by providing a complete `.dwyuignore` file which means that DWYU will report a zero error status. With DWYU now
enabled and working, the `.dwyuignore` file can be slimmed down bit by bit.

Any easy way of providing a complete `.dwyuignore` file (or just squashing the errors in an experimental branch) is to copy one of the files
produced by DWYU over `.dwyuignore`. In the tutorial above:

```
declare-what-you-use/example/example1/build$ cp actual_unreffed_decls.txt ../.dwyuignore
```

...will silence any DWYU errors.
