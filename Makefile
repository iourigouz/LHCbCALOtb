CXX      = g++
CC       = gcc
CPPFLAGS = -I$(ROOTSYS)/include 
CAENFLAGS = -DLINUX
#CXXFLAGS = -O -Wall -Wno-deprecated -fPIC -g -gdwarf-2 
#CXXFLAGS = -O0 -Wall -fPIC -g -gdwarf-2 
CXXFLAGS = -O0 -w -fPIC -g -gdwarf-2 
CFLAGS += -O0 -fPIC -g -gdwarf-2 
LIBRARIES = -L$(ROOTSYS)/lib -lCore -lCint -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lGui 
LIBRARIES += -lspecs -lSpecsUser -ldim
LIBRARIES += -lCAENVME -lCAENDigitizer -lCAENComm
LIBRARIES += -pthread -lm -ldl -rdynamic -lstdc++ 

LDFLAGS  = $(LIBRARIES)
SOFLAGS  = -shared -Wl,-soname,

NAME     = tb

SOURCE := tb.cpp ntp.cpp runparam.cpp
SOURCE += specs_operations.cpp vme_operations.cpp
SOURCE += digitizer_operations.cpp wavedump_functions.cpp 
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCE))

all: $(NAME)

$(NAME):  $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) \
		-o $(NAME)

vme_operations.o: vme_operations.cpp
	@echo Makefile compiling $<
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(CAENFLAGS) -c $< -o $@

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
