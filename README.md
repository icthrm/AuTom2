# AuTom2

### An integrated toolkit for automated cryo-electron tomography processing

AuTom2 is an integrated toolkit for cryo-electron tomography (cryo-ET) data processing. It provides a graphical workflow for tilt-series alignment, optional CTF-related processing, tomographic reconstruction, result inspection, and batch processing.

The software supports both marker-free alignment and marker-based alignment, allowing users to process tilt series with or without fiducial markers within a unified interface.

## Features

* Single-dataset and batch cryo-ET processing workflows
* Marker-free tilt-series alignment
* Marker-based alignment with fiducial-track inspection
* Optional fiducial-marker erasing
* Optional CTF estimation and correction
* TiltRec-based tomographic reconstruction
* MRC visualization for projection stacks and reconstructed volumes
* Docker-based build and launch workflow

## Quick Start

Build and run AuTom2 from the project root:

```bash
./docker-build.sh
./docker-run.sh
```

The run script mounts the local `data/` directory to `/workspace/data` inside the container. Place input datasets in `data/`, or modify the mount path in `docker-run.sh` if needed.

If a prebuilt Docker image archive is provided, import it first:

```bash
docker load -i autom2-dist.tar
```

Then run AuTom2:

```bash
./docker-run.sh
```

## Local Build

Advanced users can build AuTom2 directly on Linux when all dependencies are available:

```bash
cmake -B build -S .
cmake --build build -j$(nproc)
cmake --install build
```

After installation, executables can be run directly if the installation directory is available in `PATH`.

## Main Modules

AuTom2 integrates several cryo-ET processing modules, including:

```text
Markerfree
Markerauto2
TiltRec-cuda
TiltRec-mpi
TiltRecZ-cuda
TiltRecZ-mpi
CTFMeasure
markererase
mrcstack
```

## Documentation

Detailed installation instructions, workflow descriptions, parameter explanations, screenshots, and troubleshooting information are available in the AuTom2 user manual:

```text
manual.pdf
```

## Repository Layout

```text
src/              Source code and processing modules
lib/              Third-party and shared libraries
docs/             Additional project documentation
docker-build.sh   Docker-based build script
docker-run.sh     Docker-based GUI launch script
BUILD.md          Build notes
manual.pdf        User manual
```

## Citation

If you use AuTom2 in academic work, please cite the corresponding AuTom2 publication or software release.

## License

See the LICENSE file for licensing information.
::: 
