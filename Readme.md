Requirements

    C++ compiler with C++17 support or higher.
    Linux-based operating system with POSIX support.

Installation and Usage

    Compile the project with the following command:

    mkdir build
    cd build
    cmake ..
    make

Run the program with the input and output file paths as arguments:

    ./word_frequency_analyzer <input_file_path> <output_file_path>

Example

    ./word_frequency_analyzer input.txt output.txt

This will read input.txt, count the frequency of each word, and write the results to output.txt, sorted by frequency in descending order.