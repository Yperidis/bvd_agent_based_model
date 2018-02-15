=Contents=
Introduction
Installation
Use
License

=Introduction=

This code has been developed in the course of two master thesises at the Institute of Theoretical Physics at
TU Berlin. It simulates a system of farms, which breed cows for the purpose of meat and milk production. Changes
in code are documented in CHANGELOG.txt. 

=Installation=

To use that programm you need to apply some configuration based on your operating system. See the files
installLinuxManual.txt or installMacManual for further information.



=Use=

After compiling the programm a vast amount of parameters can be configured usign an ini file. Some examples
are given in the inifiles directory. To start of with the example you can run the programm like

./bvd_agent_based_model --ini iniFiles/example.ini  

The bvd_agent_based_model executable should be located in the project's /bin directory.
There is also an executable called bvd_agent_simulation from older builds of the project. 
That version is not guaranteed to work, and if it does it will have a lot of output.

Some things can not (yet) be changed using the ini file. Those constant can be found in the file
Model_Constants.h and may be set there. If you do so, you need to recompile by using the

make programm

directive in order for those changes to apply.

There is a couple of python scripts to actually visualize the data created by running this simulation. They
are located at the pythonScripts subdirectory. They need the actual path to the outputfile as first argument. The main work when reading the files is done in those files starting with „retrieve“. The python script „plotSystemBehaviour.py“ is a good example on how to use those files.

=License=
