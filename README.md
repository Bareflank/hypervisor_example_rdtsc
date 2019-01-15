# Bareflank Hypervisor RDTSC Example

## Description

This example demonstrates how to extend the Bareflank hypervisor to
emulate the RDTSC instruction. For more information on how Bareflank
extensions work, please see the following:

[API Documentation](http://bareflank.github.io/hypervisor/html/)

## Compilation / Usage

To setup our extension, first setup your directory structure.
You should clone this repo into the directory containing the bareflank/hypervisor repo.
Place a config.cmake (see bareflank/hypervisor/scripts/cmake/config/example_config.cmake) in the same directory and turn on the ENABLE_HYPERVISOR_EXAMPLE_RDTSC flag.
In summary, to build:
```
git clone https://github.com/Bareflank/hypervisor.git
cp ./hypervisor/scripts/cmake/config/example_config.cmake config.cmake
vim config.cmake # set ENABLE_HYPVERVISOR_EXAMPLE_RDTSC to on
git clone https://github.com/Bareflank/hypervisor_example_rdtsc.git
mkdir build; cd build
cmake ../hypervisor
make -j<# cores + 1>
```

To build the VMM driver, load it, and start it use the following:

```
make driver_quick
make quick
```

to get status information, use the following:

```
make status
make dump
```

to reverse this:

```
make unload
make driver_unload
```
