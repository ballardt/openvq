![OpenVQ](//bitbucket.org/mpg_code/openvq/raw/master/extra/logo/openvq.png)

### The project
OpenVQ is a video quality assessment toolkit. The goal of this project is to provide anyone interested in video quality assessment with a toolkit that *a*) provides ready to use video quality metric implementations; and *b*) makes it easy to implement other video quality metrics.

Version 1 of OpenVQ contains the following metrics:

 * OPVQ - The Open Perceptual Video Quality metric
 * PSNR - Peak signal-to-noise-ratio (full reference)
 * SSIM - Structural similarity index (full reference)

### Setup

#### 1. Dependencies
We depend on the following programs and libraries:

 * [CMake](http://www.cmake.org) to generate a project and/or build.
 * [Libav](https://libav.org) (v.9)
 * [OpenCV](http://opencv.org) (v.2)
 * [Boost](http://www.boost.org) Program Options (v.1.54 or newer)

On Ubuntu, install the dependencies by running the following command: 

    sudo apt-get install cmake libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libopencv-dev libboost-program-options-dev
    
#### 2. Build and install
OpenVQ is distributed as a CMake project. For UNIX systems, CMake can be invoked from the command line. If no other project type is specified, CMake will generate a Makefile that can be invoked with Make. If no errors are reported, OpenVQ was built successfully and is ready to be installed into the system path. The install command may have to be invokes with `sudo`, depending on the user's privileges and the permissions of the install prefix.

    mkdir openvq-build && cd openvq-build
    cmake <path to openvq source dir>
    make
    make install

The default install prefix is `/usr/local`. A different prefix can be changed when CMake is invoked to generate the build files.

    cmake <path to openvq source dir> -DCMAKE_INSTALL_PREFIX=<prefix>

### Running the program
OpenVQ is run from the command line

    openvq [global options] command [command specific options]

Run `openvq` with `--help` or without any arguments to display info about the available options

### License and copyright
Carsten Griwodz (<griff@simula.no>) is the maintainer and contact person for the OpenVQ project. Version 1 was authored by Henrik Bjørlo and Kristian Skarseth.

OpenVQ is Free Software; you can redistribute it and/or modify it under the terms of the GNU Affero General Public License version 3 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.