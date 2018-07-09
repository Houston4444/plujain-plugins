
NAME = triswitch

# installation path
INSTALL_PATH = /usr/local/lib/lv2
COMPLETE_INSTALL_PATH = $(DESTDIR)$(INSTALL_PATH)/$(NAME).lv2

# compiler and linker
CXX ?= g++

# flags
CXXFLAGS += -I. -c -ffast-math -fPIC -DPIC -Wall
LDFLAGS += -shared -lm

# libs
LIBS =

# remove command
RM = rm -f

# sources and objects
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

# plugin name
PLUGIN = $(NAME).so

$(PLUGIN): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(PLUGIN) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	$(RM) src/*.o $(PLUGIN)

install:
	mkdir -p $(COMPLETE_INSTALL_PATH)
	cp $(PLUGIN) $(COMPLETE_INSTALL_PATH)
	cp ttl/*.ttl $(COMPLETE_INSTALL_PATH)
# 	mkdir -p $(COMPLETE_INSTALL_PATH)/modgui
# 	cp ttl/modgui/* $(COMPLETE_INSTALL_PATH)/modgui
