
# PROGRAM   := a.out    # the executable name
PROGRAM   := apcheck

# SRCDIRS   := .        # current directory
SRCDIRS   := .

# The source file types (headers excluded).
SRCEXTS   := .c

# CPPFLAGS  := -Wall -Werror # show all warnings and take them as errors
CPPFLAGS  :=

CFLAGS    := -Wall -Os
CFLAGS    += $(TARGET_CFLAGS)

CXXFLAGS  := -Wall -Os
CXXFLAGS  +=

# The library and the link options ( C and C++ common).
# LDFLAGS   := -Wall -Os
LDFLAGS   += $(TARGET_LDFLAGS)

## Stable Section: usually no need to be changed. But you can add more.
##=============================================================================
SHELL   = /bin/sh
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))

#LDFLAGS  += -luci -lucapi -lae_event -ltlv -lnla -pthread
#LDFLAGS  += -lucapi -lae_event -ltlv -lnla -pthread
LDFLAGS  += -lae_event -ltlv -lnla -pthread

OBJS 	= main.o 
#OBJS    = $(foreach x,$(SRCEXTS), \
#     		$(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES))))

DEPS    = $(patsubst %.o,%.d,$(OBJS))

.PHONY : ftp nfs all objs clean cleanall rebuild strip

compile : $(PROGRAM)

# Rules for producing the objects.
#---------------------------------------------------
objs : $(OBJS)

%.o : %.c
	$(CC)  -c $(CPPFLAGS) $(CFLAGS) $<

%.o : %.C
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o : %.cc
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o : %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o : %.CPP
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o : %.c++
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o : %.cp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o : %.cxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

# Rules for producing the executable.
#----------------------------------------------
$(PROGRAM) : $(OBJS)
ifeq ($(strip $(SRCEXTS)), .c)  # C file
	$(CC)  $(LDFLAGS) -o $(PROGRAM) $(OBJS) 
else                            # C++ file
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJS) 
endif
	
-include $(DEPS)

rebuild: clean compile

strip: all
	$(CROSS)strip $(PROGRAM)

clean :
	$(RM) *.o *.d
	$(RM) $(PROGRAM)

distclean: clean
	$(RM) $(PROGRAM)

### End of the Makefile ##  Suggestions are welcome  ## All rights reserved ###
###############################################################################
