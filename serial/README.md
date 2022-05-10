# Serial Implementation
This is the serial implementation for finding the set of nearest balanced states of a signed graph

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
student@cori04:~/gradproj/build> ./serial -f ../../sample-graphs/sample_graph.csv -n 1000
```

## Command Line Args
```
Check the Usage:
  -h: see this help
  -f <filename>: refer to the graph file name
  -n <num>: number of spanning trees to generate
  -o <filename>: refer to the output file for correctness
  -s <seed_num>: the seed number
```

## Running Our Code

We recommend running our code using the "[job-serial](https://github.com/tezktenr/CS267-Spring2022-GradProject-Group46/blob/main/serial/job-serial)" script provided. As an example, modify the parameters inside the "job-serial" script as needed and follow the steps below to run the code on the [CORI server](https://docs.nersc.gov/systems/cori/):
```
student@cori04:~/gradproj/build> sbatch job-serial
```

You could also run the code without the batch script:
```
student@cori04:~/gradproj/build> ./serial -f ../../sample-graphs/sample_graph.csv -n 1000
```


## Checking Correctness
To check the correctness for the code on "[sample_graph.csv](https://github.com/tezktenr/CS267-Spring2022-GradProject-Group46/blob/main/sample-graphs/sample_graph.csv)" with seed 1:
```
student@cori04:~/gradproj/build> ./serial -f ../../sample-graphs/sample_graph.csv -n 1000 -s 1 -o serial.out
student@cori04:~/gradproj/build> diff serial.out ../../correctness/sample_graph_s1_n1000.correct.out
```


