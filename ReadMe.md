# Nopsh  
A minimal POSIX‑style shell written in pure C.

I built nopsh as a way to learn how shells actually work under the hood. It's small and intentionally easy to read. If you're curious about fork/exec, I/O redirection, or just want to poke around a simple REPL implementation, this could be a useful reference.

---

## Features

- Built‑in commands: `cd`, `pwd`, `echo`, `clear`, `help`, `exit`, `getrun`
- External command execution via `fork()` + `execvp()`
- Input/output redirection (`<` and `>`)
- An optional non‑interactive mode for remote execution

---

## Non‑Interactive Mode

You can point nopsh at a remote script and it'll download it, make it executable, run it, then delete it:
```
./nopsh --getrun <url>
```

---

## Building

```
make
```

---

## Usage

Start the interactive shell:

```
./nopsh
```

Example:

```
[nopsh] ➤ pwd
/home/user
[nopsh] ➤ echo hello world
hello world
```

---

## Architecture

- **input/** - reads a line from stdin  
- **parser/** - tokenizes input and picks out any `<`/`>`redirection  
- **builtins/** - handles the built‑in commands  
- **executor/** - forks, execs, and wires up redirection
- **net/** - a tiny HTTP client used just for `getrun`

---

## Notes

nopsh doesn't do pipes, job control, backgrounding, or anything fancy that a standard shell would do. That's intentional. The goal was a codebase you can read in an hour, not a full blown bash shell.


