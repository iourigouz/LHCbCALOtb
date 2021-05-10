CXX      = g++ -std=c++11
CC       = gcc
CPPFLAGS = -I$(ROOTSYS)/include 
CAENFLAGS = -DLINUX -DUNIX
#CXXFLAGS = -O -Wall -Wno-deprecated -fPIC -g -gdwarf-2 
#CXXFLAGS = -O0 -Wall -fPIC -g -gdwarf-2 
CXXFLAGS = -O0 -w -fPIC -g -gdwarf-2 
CFLAGS += -O0 -fPIC -g -gdwarf-2 
LIBRARIES = -L$(ROOTSYS)/lib -lCore -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lGui -lRIO -lASImage -lASImageGui
LIBRARIES += -ldim
LIBRARIES += -lCAENVME -lCAENDigitizer -lCAENComm -lcaenhvwrapper
LIBRARIES += -pthread -lm -ldl -rdynamic -lstdc++ 

LDFLAGS  = $(LIBRARIES)
SOFLAGS  = -shared -Wl,-soname,

NAME     = tb

SOURCE := $(NAME).cpp ntp.cpp runparam.cpp
SOURCE += vme_operations.cpp
SOURCE += digitizer_operations.cpp wavedump_functions.cpp 
SOURCE += SY5527_operations.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCE))

all: $(NAME)

$(NAME):  $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) \
		-o $(NAME)

vme_operations.o: vme_operations.cpp
	@echo Makefile compiling $<
	$(CXX) $(CXXFLAGS) $(CAENFLAGS) -c $< -o $@

SY5527_operations.o: SY5527_operations.cpp
	@echo Makefile compiling $<
	$(CXX) $(CXXFLAGS) $(CAENFLAGS) -c $< -o $@

%.o: %.cpp
	@echo Makefile compiling $<
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY : clean

clean:
	rm -f $(OBJECTS) core *~ $(NAME)

test:
	echo $(SOURCE)
	echo 
	echo $(OBJECTS)
