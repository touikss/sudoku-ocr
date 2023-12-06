# SudokuSolver
Semestre 3 project.

## Compilation:
Run "make" to compile the whole project.
It will create a "main" executable from which you can run all that you want.

## Usage:
Usage: ./main command options

### Commands :
    canny: run canny filter
    solver: run sudoku solver
    xor: run xor neural network
    hough: run hough line detection
    grid: run grid detection detection (canny then hough)
    rotate: rotate an image
### Options:
    -d: display image information
    -z: resize image (keep proportions), to fit in the screen (CANNY, HOUGH, GRID)
    -t: display processing time
    -o <filename>: store output in a file
    -i <filename>: choose input from a file
    -s: run step by step (CANNY, HOUGH, GRID)
    -h <value>: set high threshold to value (float) (CANNY, HOUGH, GRID)
    -l <value>: set low threshold to value (float) (CANNY, HOUGH, GRID)
    -b <value>: set blur straight to value (int) (CANNY, HOUGH, GRID)
    -x <value>: set nb iteration of xor to value * 10000 (int) (XOR)
    -p <value>: set displaying step of xor to value * 1000 (int) (XOR)
    -r <value>: set hough threshold to value (int) (HOUGH, GRID)
    -a <value>: set rotation angle to value (float) (ROTATE)

## Remove:
"make clean" will remove executable and .o files.

