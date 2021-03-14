# Zudoku

The name remind you of Sudoku, right? Sure. Zudoku is a blazingly-fast interactive command-line application to solve Sudoku tables. It uses CSV files for input/output (thanks to [rapidcsv](https://github.com/d99kris/rapidcsv)), which makes it easy to use.

## Building and Running

First, as this project uses Git submodules for its external libraries, do:

```
git clone https://github.com/machitgarha/zudoku --recurse-modules
```

Now, everything is ready:

```
cmake -B build
make -C build

# Run it.
./build/Zudoku
```

### Available Examples

For inputs and outputs, there are predefined CSV file samples living in `examples/sudoku-table/` directory.

## Implementation

At code level, implementation is built upon two containers: Stacks and arrays. If you are curious about the details, comprehensive code comments, drastic identifier names and perfect underlying design should help you understanding what is done under the hood.

If you know Persian, you can also switch to `report-fa` and read the report available there (inside `docs/` directory), in PDF (or ODF) format(s).

## License

[GPLv3](./LICENSE.md). Did you expect something else? ;)

## Miscellaneous

I'm tired of writing this section. Go somewhere else.
