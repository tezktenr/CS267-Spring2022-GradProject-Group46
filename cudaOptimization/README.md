## Build

```shell
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Dataset

Download datasets in the directory of `~/datasets/`

## Run

```shell
sbatch job-gpu NameOfinputCSV outPutCSV
```

For example
```shell
sbatch job-gpu amazonVideo_core5_edges2.csv output_amazonVideo_core5_edges2.csv
```

