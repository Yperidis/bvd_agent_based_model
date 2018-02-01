OS := $(shell uname -s)
DISTRO := $(shell lsb_release -si)
CFLAGS = -Wall -g -pedantic -fopenmp #-D_DEBUG_ # -D _OUTPUT_DEBUG_ #-D_FARM_MANAGER_DEBUG_ -D_INITIALIZER_DEBUG_ -D_HERD_DEBUG_   -D_MARKET_DEBUG_
CCFLAGS = -std=c++11 $(CFLAGS)
LFLAGS = -lgsl -lgslcblas -lm -lsqlite3
LD_INCLUDE=  -L $(CUSTOMLIBSPATH)

ifeq ($(OS),Darwin)#MacOS
CC = clang-omp++
CCC = clang-omp
LFLAGS := $(LFLAGS) -lhdf5 -lhdf5_hl
else
CC = g++
CCC = g++
ifeq ($(DISTRO),Debian)#Debian detection for serial hdf5
LDINCLUDE := $(LDINCLUDE) -L/usr/lib/x86_64-linux-gnu/serial -L/usr/lib -L/usr/lib/x86_64-linux-gnu/
LFLAGS := $(LFLAGS) -lhdf5_serial -lhdf5_serial_hl # for some reason or Linux systems required these library names in order to work as expected
else
LFLAGS := $(LFLAGS) -lhdf5 -lhdf5_hl
endif
endif
SRCDIR := src
BUILDDIR := build
TARGET := build/bvd_agent_simulation
CUSTOMLIBSPATH := lib
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
INC = -I/usr/include -I/usr/include/hdf5/serial -I/usr/local/include -I. -I./projectImports/inih/ -I include


INIOBJECTS := $(CUSTOMLIBSPATH)/ini.o $(CUSTOMLIBSPATH)/INIReader.o

CUSTOMLIBS := $(INIOBJECTS)

$(TARGET): $(OBJECTS) $(CUSTOMLIBS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LFLAGS) $(LD_INCLUDE)"; $(CC) $^ -o $(TARGET) $(LFLAGS) $(LD_INCLUDE)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(CUSTOMLIBSPATH)
	@echo " $(CC) $(CCFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CCFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET) $(CUSTOMLIBSPATH)

.PHONY: clean

$(CUSTOMLIBSPATH)/INIReader.o:
	$(CC) $(CCFLAGS) -c projectImports/inih/cpp/INIReader.cpp -o $(CUSTOMLIBSPATH)/INIReader.o
$(CUSTOMLIBSPATH)/ini.o:
	$(CCC) $(CFLAGS) -c projectImports/inih/ini.c -o $(CUSTOMLIBSPATH)/ini.o
