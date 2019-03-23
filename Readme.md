# Preamble
This program, written in C++ (C++11 standard), simulates the spread of bovine viral diarrhea disease (BVD) in a trade network of farms. It has been developed at the Institute of Theoretical Physics (ITP), [from the team of professor Hövel](http://www.itp.tu-berlin.de/ag_empirische_netzwerke_und_neurodynamik/hoevel/members/parameter/en/) at the TU Berlin, and is a collaboration between that team and the epidemiological department of the [Friedrich Loeffler Institute](https://www.fli.de/en/institutes/institute-of-epidemiology-ife/) (FLI).

# Use of the code
## Compilation
The code is presently built via [Cmake](https://cmake.org/). It has been tested on a Linux (Debian) system and on Windows 7 Professional via [Cygwin](https://cygwin.com/). The CMakeLists file has been set up for 
a Mac enviornment as well, but it has not been tested yet. An ongoing effort is being made to build a solution of the project for [Visual Studio 2017](https://www.visualstudio.com/). 

Finally, it is possible to compile the project through the [automake](https://www.gnu.org/software/automake/) suite of tools, but the procedure is more involved (has been tested and works though in Linux-Debian and through Cygwin in Windows 7 Professional). The instructions below should facilitate any of the two selected builds.

### Step by Step Guide
First you need to install all of the packages listed here. Unfortunately, it is still necessary to have all libraries such as HDF5 and SQLite included, even if you're just using CSV export. In the future this might change.

In all the UNIX and UNIX-like cases of operating systems below, a previously installed gcc compiler suite is assumed.

#### Packages
Depending on your platform different steps have to be taken.

##### macOS
Use macports or home-brew to install
* git
* hdf5 
* sqlite
* gsl
* clang-omp
* cmake
* automake
* autoconf
* libtool

##### Linux
Use the package manager of your choice to install
* git
* gsl
* sqlite
* automake
* autoconf
* hdf5

##### Windows
At the current state of the project, the easiest build would be through [Cygwin](https://cygwin.com/) to emulate a UNIX-like environment. The following instructions have been tested on Windows 7, 64-bit professional. According to Cygwin's [FAQ](https://cygwin.com/faq/faq.html) there may be a number of non-transparent causes and failures from other software interferences from the same PC. A list of software known to cause problems is given on the aforementioned website. Try deactivating them if you face problems with the building procedure. Otherwise open an issue about it.

* Make sure the following packages are installed via Cygwin (the debug versions are not necessary).
    * g++ 
    * hdf5
    * sqlite
    * gsl
    * cmake

      Additionally for the automake build case:
      
    * make
    * automake
    * autoconf
    * libtool

* In the cpp file "src/Utilities.cpp" comment include "execinfo.h" (line 5), because it is Linux-only header. Then comment the body of the last function "Utilities::printStackTrace" (lines 70-87).


#### Cloning and Building

##### Cmake Case

* Clone this repository via `git clone https://github.com/Yperidis/bvd_agent_based_model.git`, unpack the project in a directory called `bvd_agent_based_model` and make it your working directory.
* Run `cmake ./` from the `bvd_agent_based_model` directory. If you are running this on Windows do this from the Cygwin command-line console.
* If you are on Windows, modify the generated Makefile by adding on line 5 the library calls "-lhdf5 -lhdf5_hl".
* Run `Make` (similarly from the Cygwin terminal if you are on Windows).

##### Automake Tools Case
* Clone this repository via `git clone https://github.com/Yperidis/bvd_agent_based_model.git`, unpack the project in a directory called `bvd_agent_based_model` and make it your working directory.
* Copy or transfer all the contents of the `/Automake_build_files` folder to the working directory.
* Run the `./configure` script in the `bvd_agent_based_model` directory. If you are running this on Windows do this from the Cygwin command-line console.
* If you are on Windows, modify the generated Makefile by adding in line 5 the library calls "-lhdf5 -lhdf5_hl".
* Run `Make` (similarly from the Cygwin terminal if you are on Windows).

If you experience problems in the build process, try running `autoreconf -i`. If this does not work, open an issue [(please follow the guidelines)](https://guides.github.com/features/issues/) on the matter.


## Running the code
After the successful compilation and linking an executable called 'bvd_agent_based_model' (through the automake tools build this will be called 'bvd_agent_simulation' due to legacy reasons) can be found in the `build`-directory, which should now appear in the working directory `bvd_agent_based_model`. So far it only has two options which can be added on the command line, namely `--help` and `--ini`. The latter has to be invoked in order to provide an ini file which then specifies various parameters of the simulated system. Some sample ini files can be found in the folder `/iniFiles/` so that the program can be run by executing `./build/bvd_agent_based_model --ini iniFiles/Test.ini` (respectively `./build/bvd_agent_simulation --ini iniFiles/Test.ini` for the automake build case), if your current working directory is the root directory of the the repository. A detailed description of the set parameters is provided in the Test.ini file in the iniFiles folder.

Note that a csv file with two columns separated by an upper colon `;` is needed as a farm list input to the simulation. There should optionally be a header indicating in a human readable way the attributes of each column. 
Both columns should consist of non-negative integers. The first column should be in an ascending order and represents the animal count. The second column should depend on the fields of the first and represents the corresponding 
number of farms.


# Development
A programme following the needs of the FLI and the TU Berlin has been proposed for the continuation and extension of the project.

## Programme

Several tasks have already been defined for the development of the code.

* Simulation Capability
   * At the moment (March 2018) the behavior of the market is determined by simple rules of supply and demand, which are dictated by a condition to preserve the trading farm's animal population. This leads to some kind of worst case scenario. In order to come up with more realistic results, a market model based on loyalties between premises could be considered.
   * The largest amount of time is used by the market in the process of choosing cattle to put them into the offer and demand queues by the market, and doing the matchmaking. Making this part of the simulation thread safe and doing it in N threads should increase the performance of the code significantly.

* Output
    * Fix the SQLite output 
    * Include support for CSV file output
    * Support turning on and off different output data (to minimize data size). This has been partly implemented in the debug code.
    * Consider including in the source code a compression of the output data upon runtime.

* Tests
	* Several tests should be written utilizing the [framework Catch](https://github.com/philsquared/Catch). A unit test scheme would be recommended (see the effort started in the folder `/tests`).
	
* Usability 
    * Make it possible to generate makefiles which exclude certain files if for example HDF5 support is not needed. The cmake implementation can largely account for that and is apready implemented. Likewise for suppressing selected parts of output data. An implementation is already working for UNIX-like systems (see the CMakeLists.txt at the parent directory of the project).

* Miscellaneous
	* Make project follow conventions 
		* Include system libraries in header files and project files in cpp files
		* Move all options to BVDSettings
		* Clean up system
		
* Default Behavior
    * The project should be set up in such a way so as to run for default values even without any input or an ini file.
  

## Contribute
You can easily contribute to this project by forking it and making pull requests. If you are interested in the corresponding research, feel free to reach out to one of the main developers of this package. Note that we're trying to stick to the infamous [git flow](https://danielkummer.github.io/git-flow-cheatsheet/). 

## History
This project has been developed during a collaboration between Philipp Hövel and his working group at the TU Berlin, and the FLI, which belongs to the German federal ministry of agriculture. The idea has been to implement an event-driven, agent based simulation for the spread of BVD on the German cattle trade network with the cattle serving as agents, so as to emulate the BVD and trading dynamics, and develop and test mitigation strategies for the containment of BVD in Germany in conjunction with a cost-benefit analysis.

The project started in November 2015 by Thomas Isele, Ph.D. and then was continued by Inia Steinbach, M.Sc. and Pascal Blunk, M.Sc.. The data for the simulation (csv farm lists) as well as all information on the disease as well as German legislation was provided by Jörn Gethmann, Ph.D. (FLI) and Hartmut Lentz, Ph.D. (FLI).


## Includes from other projects
The directory `projectImports` includes libraries provided by other sources. So far it contains a project called `inih` [https://github.com/benhoyt/inih](https://github.com/benhoyt/inih) (BSD License) for setting the code's parameters from a single .ini file, and for testing purposes [Catch2](https://github.com/catchorg/Catch2) (Boost License) following the [TDD](https://en.wikipedia.org/wiki/Test-driven_development) paradigm, as well as [Fake It](https://github.com/eranpeer/FakeIt) (MIT License) for [mocking](https://en.wikipedia.org/wiki/Mock_object) objects. **Note that you will have to clone the contents of each import in its respective directory separately from this project.**


# Research

## Academic theses
[Pascal Blunk's](https://github.com/Gerungofulus/) master thesis was the first milestone of this project and can be found at his [git repository](https://github.com/Gerungofulus/Masterarbeit/settings). Inia Steinbach also used a premature form of this code for some results found in her master thesis. The project was brought to a conclusion of its inception with [Jason Bassett's](https://github.com/Yperidis/) [doctoral dissertation](http://depositonce.tu-berlin.de/handle/11303/8985) (end of 2018) and an exhaustive outline of the model to which the code corresponds, which can be found on [arXiv](https://arxiv.org/abs/1812.06964).



# Acknowledgements
Special thanks to Bryan Iotti (university of Turin in early 2018) and Denis Nikhitin (ITMO university of Saint Petersburg in early 2018) for their invaluable technical advise and efforts in testing and compiling this project.
