# LSM stores in Adversarial Environments

## Set up

You will need the following packages:
- leveldb (v1.23.2) 
- rocksdb (v9.10.0)
- openssl (v3.4.0)

If you are on a Mac, you can simply run 

```
make dependencies
```

You will also need [meson](http://mesonbuild.com/) on your system.

To build, simply run

```
make
```

## Experiments

To run **without** our pseudorandom permutation (PRP) mitigation, comment out the following line

```
#define PRP
```

in all `*.cpp` files.

Our performance degradation experiments used `experiment_1_*.cpp`. You can run them with 

```
make run-leveldb
make run-rocksdb
```

Our insertion performance was measured with `experiment_4_*.cpp`. You can run the experiments with

```
make run-inserts-leveldb
make run-inserts-rocksdb
```

Our adversarial attack feasibility was measured using `insertion_attack_leveldb.cpp`, you can run it with

```
make run-insertion-attack-leveldb
```

The `results` folder contains helpful plotting scripts in python.