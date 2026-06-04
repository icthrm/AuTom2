# Guide on CTFMeasure


## 1. Installation

Decompress first, and then enter the folder.

```
tar -zxvf CTFMeasure_v1.4.0.tar.gz
cd CTFMeasure_v1.4.0
```

1. Install dependencies

Install FFTW3 and NLOPT to the "3rdlib" directory.

```
cd external
./build-fftw.sh
./build-nlopt.sh
cd ..
```

2. Compilation

Compile, and install the binaries to the "bin" directory.

```
make clean
make
```

If compilation fails, you can consider deleting the "3rdlib" directory, and repeat the above steps.


## 2. Input micrograph list

Three columns, with the first column "frame number@micrograph name", the second colum "tilt angle", and the third column "tilt axis angle".

e.g.

```
# Column 1: Frame number@Micrograph name
# Column 2: Tilt angle (in degree)
# Column 3: Tilt axis angle (in degree)

0@tomo_001.ali -60.000000 175.000000
1@tomo_001.ali -57.000000 175.000000
...
40@tomo_001.ali 60.000000 175.000000
```

To convert a tilt-series of micrographs and their corresponding tilt angles and tilt axis angles to an input micrograph list, run
```
{BINARY_PATH}/stack_to_list <input tilt-series stack> <input tilt angle file> <tilt axis angle> <output micrograph list file name>
```
e.g.
```
{BINARY_PATH}/stack_to_list tomo1.st tomo1.rawtlt 86.6 tomo1.list
```


## 3. Coordinate system

Let the specimen lie in a cartesian coordinate.
The electron beam incident from the positive half-axis of the z-axis to the negative half-axis of the z-axis.
The tilt angle regards the y-axis as the tilt axis. Positive value means rotating **clockwise**. It can be obtained from the rawtlt/tlt file from SerialEM or IMOD.
The tilt axis angle is the angle between the negative half-axis of the y-axis and the tilt axis. Positive value means rotating the specimen **clockwise** is needed to align the tilt-axis to y-axis. It can be obtained from the mdoc file from SerialEM. 


## 4. Run the program

Please run the program with input micrograph list, pixel size, spherical aberration, acceleration voltage and amplitude contrast. If additional parameters are needed, please clarify the location of the parameter file. (If you use relative path, it should be relative to the "bin" directory.)

```
{BINARY_PATH}/CTFMeasure --input_micrograph_list <input micrograph list> --pixel_size <pixel size, in Angstrom> --cs <spherical aberration, in mm> --voltage <acceleration volatge, in kV> --w <amplitude contrast> [--params <parameter file>]
```

For estimating CTF parameters of single-particle micrographs, you can specify a sign for single-particle, and run the program with input micrograph name, tilt angle, tilt axis angle, pixel size, spherical aberration, acceleration voltage and amplitude contrast. If tilt angle and tilt axis angle are not given, they will be regarded as 0 by default.

```
{BINARY_PATH}/CTFMeasure --single_particle --input_micrograph <input micrograph file> [--theta <tilt angle, in degree>] [--psi <tilt axis angle, in degree>] --pixel_size <pixel size, in Angstrom> --cs <spherical aberration, in mm> --voltage <acceleration volatge, in kV> --w <amplitude contrast> [--params <parameter file>]
```



## 5. Parameter file (optional)

### Basic parameters
| Parameter | Default value | Description |
| ---- | ---- | ---- |
| workpath | ./ | The folder for all input and output files |
| prfx | {file basename of "--input_micrograph_list"} | Prefix for output files |
| average_mrc | {prfx}_avg.mrc | Output, MRC file, the average power spectrum of the tilt-series, and evaluation of Thon ring fitting with the average defocus |
| output_mrc | {prfx}_diag.st | Output, MRC file, the power spectrum and evaluation of Thon ring fitting for each micrograph |
| output_file | {prfx}_ctf.txt | Output, a text file with the estimated CTF parameters and angle parameters |
| j | 10 | Number of threads for parallelization (With OpenMP parallelization) |

### Parameters for CTF estimation
| Parameter | Default value | Description |
| ---- | ---- | ---- |
| box | 512 | Box size for power spectrum estimation (In pixels) |
| N_zeros | 10 | Number of CTF zeros for background estimation and CTF estimation |
| defocus_min | 10000.0 | Minimum defocus for searching during initial estimation (In Angstrom) |
| defocus_max | 100000.0 | Maximum defocus for searching during initial estimation (In Angstrom) |
| defocus_step | 100.0 | Defocus step for searching during initial estimation (In Angstrom) |
| resolution_max | 4*{pixel size} | Maximum resolution for fitting (In Angstrom) |
| resolution_min | 30.0 | Minimum resolution for fitting (In Angstrom) |
| resolution_adaptive | 1 | Using adaptive resolution range for fitting (0-No; 1-Yes. If set to 1, the maximum and minimum resolution for fitting will be adjusted according to the given “N_zeros” and the estimated CTF parameters in the initial estimation step.) |
| adaptive_range | 1 | Apply adaptive fitting range adjustment (0-No; 1-Yes. If set to 1, the resolution range for fitting will be determined along the trajectory of the Thon rings according to the currently estimated CTF parameters. Useful for high-astigmatism datasets.) |
| it | 3 | Number of iterations in the iterative per-micrograph estimation step |

### Advanced parameters
| Parameter | Default value | Description |
| ---- | ---- | ---- |
| search_phase_shift | 0 | Search CTF phase shift for datasets collected with phase plates (0-No; 1-Yes) |
| box_conv | {box}/20 | Convolution box size for background estimation |
| N_avg | 0 | Number of adjacent micrographs averaged during initial estimation |
| N_ref | {Micrograph with minimum tilt angle} | The No. of the micrograph used for initial estimation (The number starts from 0.) |
| box_conv_first | 1 | Perform box convolution first or interpolation first in background estimation (0-Interpolation first; 1-Box convolution first) |
| skip_box_conv_coarse | 0 | Skip box convolution in background estimation (0-Not skip; 1-Skip) |
| hard_restriction | 0 | Apply tight restrictions on the deviation in per-micrograph estimation (0-Loose restriction; 1-Tight restrction) |
| skip_unified | 0 | Skip coarse overall estimation (0-Not skip; 1-Skip) |
| optimize_with_average | 0 | Fit with the average power spectrum during estimation (0-Use original power spectrum for fitting; 1-Use average power spectrum for fitting) |
| per_tilt_estimation | 0 | Estimate each micrograph independently (0-Joint estimation; 1-Independent estimation) |
| tight_blocks | 0 | Ignore the outermost blocks to neglect the boundary of the micrographs (0-Using all blocks; 1-Using tight block, ignoring the outermost blocks) |
| astigmatism_angstrom | 2000 | Expected astigmatsim (In Angstrom) |
| dose_weighting | 0 | Whether enabling dose weighting in estimation (0-No dose weighting; 1-Enable dose weighting) |
| dose_acc_file | &lt;accumulated dose file name&gt; | A text file with one column per line, standing for the accumulated dose in e-/A^2 for each micrograph in the input micrograph list (required if dose_weighting = 1) |
| convergence_defocus | 100.0 | Convergence criteria for defocus values (In Angstrom) |
| convergence_astigmatism | 1.0 | Convergence criteria for astigmatism angle (In Angstrom) |
| convergence_angle | 0.1 | Convergence criteria for angle estimation (In Angstrom) |
| output_raw | 0 | Whether write out the power spectrums before background subtraction (0-Write out the power spectrums after background subtraction; 1-Write out the power spectrums before background subtraction) |

### Parameters for angle estimation
| Parameter | Default value | Description |
| ---- | ---- | ---- |
| tlt_offset | 0.0 | Initial absolute tilt angle offset, the angle to make the specimen horizontal (In degree) |
| xtilt | 0.0 | Initial off-plane tilt angle of the tilt axis (Often known as x-tilt) (In degree) |
| pre_offset_estimation | 0 | Apply absolute tilt angle offset estimation during the initial estimation step (0-Not apply; 1-Apply) |
| dose_file | &lt;dose file name&gt; | A text file with one column per line, standing for the dose rate in e-/A^2 for each micrograph in the input micrograph list |
| skip_offset_estimation | 1 | Skip estimating the absolute tilt angle offset during the iterative per-micrograph estimation step (0-Not skip; 1-Skip) |
| skip_offset_refinement | 1 | Skip refining the absolute tilt angle offset during the iterative per-micrograph estimation step (0-Not skip; 1-Skip) |
| skip_offset_estimation_x | 1 | Skip estimating the off-plane tilt angle of the tilt axis (x-tilt) during the iterative per-micrograph estimation step (0-Not skip; 1-Skip) |
| skip_offset_refinement_x | 1 | Skip refining the off-plane tilt angle of the tilt axis (x-tilt) during the iterative per-micrograph estimation step (0-Not skip; 1-Skip) |
| skip_axis_refinement | 1 | Skip refining the stage tilt angle of the tilt axis (0-Not skip; 1-Skip) |
| skip_angle_refinement | 1 | Skip per-micrograph tilt angle refinement (0-Not skip; 1-Skip) |
| N_avg_block | 2 | Number of adjacent blocks used when computing local power spectrums during per-micrograph tit angle refinement |

Please refer to `conf/para_example.conf`.
To write out a parameter file with default values for reference, run
```
{BINARY_PATH}/CTFMeasure --params_example <example parameter file>
```

## Notice

- The input tilt-series can be either the original stack or the aligned stack for CTF estimation.
- An aligned stack is recommended.
- It is OK to use ".rawtlt" file as input tilt file.


## Alternative: Build with CMake

If you prefer to use CMake for building, follow these steps:

1. Install dependencies (same as above)

```
cd external
./build-fftw.sh
./build-nlopt.sh
cd ..
```

2. Build with CMake

```
mkdir build
cd build
cmake ..
make -j$(nproc)
```

The binaries will be generated in `build/bin/` directory.

3. Clean build

```
rm -rf build
```

Note: When using CMake build, replace `{BINARY_PATH}` in the examples above with `build/bin`.

