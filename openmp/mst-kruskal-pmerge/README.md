# OpenMP Implementation (Kruskal with PMerge)
This is the OpenMP implementation (Kruskal with PMerge) for finding the set of nearest balanced states of a signed graph

## Building Our Code
First, we need to make sure that the CMake module is loaded and that the GNU compiler is selected.
```
student@cori04:~/gradproj> module load cmake
student@cori04:~/gradproj> module swap PrgEnv-intel PrgEnv-gnu
```
You should put these commands in your ~/.bash_profile file to avoid typing them every time you log in.

Next, let's build the code. CMake prefers out of tree builds, so we start by creating a build directory.

```
student@cori04:~/gradproj> mkdir build
student@cori04:~/gradproj> cd build
student@cori04:~/gradproj/build>
```

Next, we have to configure our build. We can either build our code in Debug mode or Release mode. In debug mode, optimizations are disabled and debug symbols are embedded in the binary for easier debugging with GDB. In release mode, optimizations are enabled, and debug symbols are omitted. For example:
```
student@cori04:~/xxx/build> cmake -DCMAKE_BUILD_TYPE=Release ..
-- The C compiler identification is GNU 8.3.0
...
-- Configuring done
-- Generating done
-- Build files have been written to: /global/homes/s/students/gradproj/build
```

Once our build is configured, we may actually execute the build:
```
student@cori04:~/gradproj/build> make
```

Then, we could run the code via:
```
student@cori04:~/gradproj/build> sbatch job-openmp
```

## Running Our Code
We recommend running our code using the "[job-openmp](https://github.com/tezktenr/CS267-Spring2022-GradProject-Group46/blob/main/openmp/mst-kruskal-pmerge/job-openmp)" script provided. As an example, modify the parameters inside the "job-openmp" script as needed and follow the steps below to run the code on the [CORI server](https://docs.nersc.gov/systems/cori/):
```
student@cori04:~/gradproj/build> sbatch job-openmp
```

## Profiling
You should uncomment the the following macro in "main.cpp" and **recompile** to perform profiling:
```C++
#define PROFILING
```

## Checking Correctness
You **must uncomment** the following marcro in "main.cpp" and **recompile** to perform correctness check:
```C++
#define TEST_CORRECTNESS
```
Generating the output files is similar to the serial implementation. To run with 1000 iterations and seed 1, simply add the following line to "job-openmp":
```
srun -n 1 -c 68 --cpu_bind=cores ./openmp -f ../../../sample-graphs/graph2.csv -n 1000 -s 1 -o openmp.out
```
Then, run with
```
student@cori04:~/gradproj/build> sbatch job-openmp
```

