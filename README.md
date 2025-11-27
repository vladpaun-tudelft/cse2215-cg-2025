# cse2215-cg-2025
TU Delft CSE2215 Computer Graphics — lab assignments 

> This Course uses a custom framework for creating the graphics. Each practical needs to have this framework in their directories. In the interest of not bloating up this repo, a single copy of the frameowrk has been made at ```./framework.example/```.

> In order to make the practicals work, before running cmake, please run ```cp -r ./framework.example ./practical<num>/framework/``` from the repo root.

To build and run any of the code in the practicals from the repo root:
```bash
cmake -S ./practical<num>/ -B ./practical<num>/build/
cmake --build ./practical<num>/build
./practical<num>/build/<executable>
# Replace <num> with the no. of the practical and <executable> with the name of the executable file
```