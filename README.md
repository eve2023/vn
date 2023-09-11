# VN String Library

This repository contains a C string library that is an alternative to C-style strings, which are null-terminated. Instead, the library uses a 2-byte length field followed by the string contents.

## Table of Contents
1. [Features](#features)
2. [Installation](#installation)
3. [Usage](#usage)
4. [Contributing](#contributing)
5. [License](#license)

## Features
- Initialize a string with the length field: `vnstr_init`
- Get the length of a string: `vnstr_len`
- Split a string into an array of substrings based on a separator: `vnstra_init`

## Installation
1. Clone the repository to your local machine:
```sh
git clone https://github.com/eve2023/vnstring.git
```
2. Navigate to the project directory:
```sh
cd vnstring
```
3. Compile the library using your preferred C compiler (e.g., GCC):
```sh
gcc -c vn.c
```
4. Create a shared library from the compiled object file:
```sh
gcc -shared -o libvnstr.so vn.o
```
5. Add the shared library to your system's library path or link it with your project.

## Usage
Include the `vn.h` header in your C files and link against the `libvnstr.so` shared library. Here are some examples of how to use the functions:

```c
#include "vn.h"

int main() {
    char *s = strdup("string");
    vnstr_init(s);
    assert(vnstr_len(s) == 6);
    free(s);
}
```

## Contributing
This project welcomes contributions and suggestions. Please feel free to submit pull requests or open issues on the repository's GitHub page.

## License
[GNU General Public License version 3 (GPLv3)](https://www.gnu.org/licenses/gpl-3.0)

