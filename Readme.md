# Preamble
This program is a simulation code written in C++ which is used to simulate the bovine viral diarrhea (BVD) and it's spread among farms. It has been developed at ITP (Institute of Theoretical Physics) at TU Berlin.

# Use of the code
## Compilation
Like most open source projects, this projects has not been build for users who do not already posses some knowledge in building C++ code on a UNIX platform. As you can see in the roadmap, making the code more accessible is a main aim. As for Windows, the project might be possible to build under [Cygwin](https://www.cygwin.com/), but this has not been tested yet. Furthermore, the cmake implementation should make it easier in principle to build a solution in [Visual Studio](https://www.visualstudio.com/), but this has also not been tested yet.

### Step by Step Guide
First you need to install the packages listed here. Unfortunately at this point in time it is still mandatory to have all libraries such as HDF5 and SQLite included, even if you're just using CSV export. In the future this might change. 

#### Packages
Depending on your platform different steps have to be taken.

##### macOS
* Use macports or home-brew to install
	* git
	* hdf5 
	* sqlite
	* gsl
	* clang-omp	
	* automake
	* autoconf
	* libtool

##### Linux
* Use your package manager of choice to install 
	* git
	* gsl
	* sqlite
	* automake
	* autoconf
	* hdf5
#### Cloning and Building
* Clone this repository via `git clone https://github.com/Yperidis/bvd_agent_based_model.git` and change into it using `cd bvd_agent_based_model`.
* Run the `./configure` script in the directory
* Run `Make`
* If you're experiencing problems in the build process, try running `autoreconf -i`. If this does not work, feel free to write me an e-mail.

## Running the code
After successful compilation an executable called 'bvd_agent_based_model' can be found in the `build`-directory. So far it only has two options which can be added on the command line `--help` and `--ini`. The latter has to be invoked in order to provide an ini file which then specifies the behavior of the simulated system. Some ini files can be found in `/iniFiles/` so that the program can be run by executing `./build/bvd_agent_based_model --ini iniFiles/Test.ini` (if your current working directory is indeed the main directory of the the repository).

# Development
A roadmap following the needs of FLI and TU Berlin has been proposed. 
## Roadmap
Several tasks have already been defined for the development of the code.

* Simulation Capability
	  * At the moment (February 2018) the behavior of the market is determined by simple rules of supply and demand, which are dictated by a condition to preserve the trading farm's animal population. This leads to some kind of worst case scenario. In order to come up with more realistic results, a market model based on loyalties between premises should be considered.
	  * The largest amount of time is used by the market in the process of choosing cattle to put them into the offer and demand queues by the market, and doing the matchmaking. Making this part of the simulation thread safe and doing it in N threads should increase the performance of the code significantly.
* Output
	  * Fixing of the SQLite output 
	  * Including support for CSV file output
	  * Supporting turning on and off different output data (to minimize data size). This has been partly implemented in the bebug code.
* Tests
	* Several tests should be written utilizing the [framework Catch](https://github.com/philsquared/Catch)
	  * Since the code's size has grown in the course of writing multiple theses, the code needs to be tested properly
* usability 
	  * Make it possible to generate makefiles which exclude certain files if for example HDF5 support is not needed. The cmake implementation can largely take care of that.

* misc
	* make project follow conventions 
		* include system libraries in header files and project files in cpp files
		* clean up system
		* move all options to BVDSettings
  

## Contribute
You can easily contribute to this project by forking it and starting pull requests. If you are interested in the corresponding research, feel free to reach out to one of the main developers of this package. Note that we're trying to stick to the infamous [git flow](https://danielkummer.github.io/git-flow-cheatsheet/). 

## History
This project has been developed during a collaboration of Philipp Hövel and his working group at TU Berlin and Friedrich Loeffler Institut (FLI), which belongs to the German federal ministry of agriculture. The idea was to implement an agent based simulation with the cattle being the agents to simulate BVD on the German cattle trade network, since BVD is a very complex disease, which can not easily be described analytically. 

The project started in November 2015 by Thomas Isele, PhD and then continued by Inia Steinbach, M.Sc. and Pascal Blunk, M.Sc.. The data for the simulation as well as all information on the disease as well as German legislation was provided by Jörn Gethmann, PhD (FLI) and Hartmut Lentz, PhD (FLI).


## Includes from other projects
The directory `projectImports` includes those libraries provided by other sources. By now it contains a project called `inih` [https://github.com/benhoyt/inih](https://github.com/benhoyt/inih) (BSD License) and a good [Catch2](https://github.com/catchorg/Catch2)(Boost License) for testings, as well as [Fake It](https://github.com/eranpeer/FakeIt) (MIT License) for mocking objects during tests.

# Research 
Pending

## Pascal Blunk's master thesis
Pascal's thesis is the first milestone of this project and can be found at [git repository](https://github.com/Gerungofulus/Masterarbeit/settings).
