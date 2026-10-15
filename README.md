# Sudoku OCR

Take a photo of a sudoku, get it solved.

The program finds the grid in the picture, reads each digit with a small neural network,
solves the puzzle and shows the result in a window. It works on 9×9 grids and on 16×16
hexadecimal grids.

Everything is written in C from scratch, with no machine learning library: the image
processing, the neural network and the solvers.

- **Developed:** November-December 2023, as a team project at EPITA (semester 3).
- **Cleaned up and retrained:** October 2026.

## How it works

1. **Find the grid.** The image is turned to grayscale and contrast is boosted. Edges are
   extracted, and the program keeps the square shapes that form a 9×9 or 16×16 pattern.
2. **Cut out the cells.** Each cell is cropped, cleaned, and the digit is centred in a
   28×28 picture.
3. **Read the digits.** A small neural network (784 → 128 → 16) gives the digit or the
   hexadecimal letter. Empty cells are left blank.
4. **Solve.** A backtracking solver fills the grid.

The network is trained on digits drawn from about 200 fonts, on MNIST samples, and on real
cells taken from the test photos.

## Build and run

You need GTK 3, SDL2, SDL2_image and SDL2_ttf.

```sh
make
./main
```

Open an image, rotate it if needed, check the detected grid, fix any wrong digit, then solve.

## Results

On a grid labelled by hand (`images_test/image_04.jpeg`, 39 digits):

- **36 / 39** digits correct when no cell of this grid was used for training.
- **16 / 17** correct on cells never seen, when the network was trained on the other rows of
  the grid.

This is one grid and one typeface, so it only gives an idea.

On the 18 test images, the program finds a grid in 17 of them and correctly reports nothing
for the one picture without a grid (a pig). Reading is reliable on clean, upright, printed
grids; it is not on the cases below.

## Limits

- Strongly tilted grids: empty cells may be read as digits.
- Very low contrast photos: digits can be missed.
- Hexadecimal grids: letters A-F are less reliable.
- Coloured tiles and photos of objects are out of scope.

## Project layout

```
sources/          C code
headers/          headers
glade/            window layout
neural_network/   trained weights
test_neural/      network training code
images_test/      test photos
```

## License

MIT, see [LICENSE](LICENSE). MNIST, the fonts and the photos in `images_test/` keep their own
licences.
