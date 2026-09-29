# GSH

GSH is a WIP, small, hobbyist unix-like shell written in C. I built it to learn more about processes,
systems programming, and how a shell works internally.

The project follows the progression of
[CodeCrafters' Build Your Own Shell course](https://app.codecrafters.io/courses/shell/overview),
while aiming to explore more edge cases and provide tests that can run locally.

## Implemented so far

- Interactive prompt with `exit`
- Command execution through `PATH`
- Built-ins: `echo`, `cd`, `pwd`, and `type`
- Single and double quotes, plus backslash escaping
- Relative, absolute, home-directory (`~`), `.` and `..` path handling
- Standard output redirection with `>` and `>>`
- Standard error redirection with `2>` and `2>>`
- Basic unit tests for path handling, home-directory expansion, and executable lookup

## Build and run

Requirements: a C compiler and `make`.

```sh
make
make run
```

Run the local tests with:

```sh
make test
```

## TODO

- [ ] Add pipes (`|`) and pipelines
- [ ] Support combined and numbered redirections, including `&>` and `2>&1`
- [ ] Improve parsing (unterminated quotes, escaped characters, and empty arguments)
- [ ] Add environment-variable expansion
- [ ] Add wildcard/glob expansion
- [ ] Add command history and history expansion
- [ ] Add tab completion
- [ ] Handle signals and foreground/background processes
- [ ] Expand integration coverage for built-ins, external commands, redirections, quoting, and parser errors
- [ ] Handle all errors returned by syscalls used in the project
- [ ] More tests
