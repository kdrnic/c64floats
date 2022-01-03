OBJDIR=obj
BINNAME=game
CFLAGS2=$(CFLAGS)

#Change obj and exe name for animation test thing
ifeq ($(filter animtest,$(MAKECMDGOALS)),animtest)
	#Add flag that changes program behaviour
	CFLAGS2+=-g -DANIMTEST
	OBJDIR:=$(OBJDIR)anim
	BINNAME:=anim
endif

#Change obj and exe name for DOS builds
ifneq (,$(findstring djgpp,$(CC)))
	OBJDIR:=$(OBJDIR)dos
	BINNAME:=$(BINNAME)dos
endif

#Change obj and exe name for debug builds
ifeq ($(filter debug,$(MAKECMDGOALS)),debug)
	OBJDIR:=$(OBJDIR)dbg
	CFLAGS2+=-g -O0 -DDEBUG
	BINNAME:=$(BINNAME)d
endif

#Profiling build options
ifeq ($(filter profile,$(MAKECMDGOALS)),profile)
	CFLAGS2+=-gdwarf-2 -fno-omit-frame-pointer -O2
endif

#Optimise more for release build and disable asserts
ifeq ($(filter regular,$(MAKECMDGOALS)),regular)
	CFLAGS2+=-O3
endif

#Useful flags
CFLAGS2+=-Wall -Wuninitialized -Werror=implicit-function-declaration -Wno-unused -fplan9-extensions -Wstrict-prototypes
CPPFLAGS=$(filter-out -fplan9-extensions -Wstrict-prototypes,$(CFLAGS2))

#Set CC and CXX on Windows for Windows build
ifeq ($(origin CC),default)
	ifeq ($(OS),Windows_NT)
		CC = gcc
		CXX = g++
	endif
endif

C_FILES=$(wildcard src/*.c)
CPP_FILES=$(wildcard src/*.cpp)
MAIN_HEADERS=
HEADER_FILES=$(filter-out $(MAIN_HEADERS),$(wildcard src/*.h))

C_OBJECTS=$(patsubst src/%,$(OBJDIR)/%,$(patsubst %.c,%.o,$(C_FILES)))
CPP_OBJECTS=$(patsubst src/%,$(OBJDIR)/%,$(patsubst %.cpp,%.o,$(CPP_FILES)))
OBJECTS=$(C_OBJECTS) $(CPP_OBJECTS)

HAVE_LIBS=
INCLUDE_PATHS=
LINK_PATHS=

#Add Allegro to the libs
ifeq ($(OS),Windows_NT)
	INCLUDE_PATHS+=-I$(ALLEGRO_PATH)include
	LINK_PATHS+=-L$(ALLEGRO_PATH)lib
endif


ifeq ($(OS),Windows_NT)
	ifneq ($(NO_PTHREADS),1)
		INCLUDE_PATHS+=-I$(PTHW32_PATH)include
		LINK_PATHS+=-L$(PTHW32_PATH)lib
		HAVE_LIBS+=-lpthreadGC2
	else
		CFLAGS2+=-DNO_PTHREADS
	endif
endif

LINK_FLAGS:=$(LINK_FLAGS)

#If 'small' build, attempt to optimize size
ifeq ($(filter regular,$(MAKECMDGOALS)),regular)
	LINK_FLAGS+=-Wl,--gc-sections
endif

$(OBJDIR)/%.o: src/%.c $(HEADER_FILES)
	$(CC) $(INCLUDE_PATHS) $(CFLAGS2) $< -c -o $@

$(OBJDIR)/main.o: src/main.cpp $(HEADER_FILES) $(MAIN_HEADERS)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) $< -c -o $@
	
$(OBJDIR)/%.o: src/%.cpp $(HEADER_FILES)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) $< -c -o $@

regular: $(BINNAME).exe

clean:
	rm -f $(OBJDIR)/*.o

$(OBJDIR):
	mkdir $(OBJDIR)

$(BINNAME).exe: $(OBJDIR) $(OBJECTS)
	$(CXX) $(LINK_PATHS) $(LINK_FLAGS) $(CFLAGS2) $(OBJECTS) -o $(BINNAME).exe $(HAVE_LIBS) -lalleg -lm

debug: $(BINNAME).exe
