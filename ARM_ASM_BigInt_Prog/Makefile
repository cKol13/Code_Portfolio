

# "make" and "make bigfact" are the same thing: build the factorial program
# "make timing" bulids bigfact (if necessary) and reports time for 10 runs
# "make testbigint" builds the unit test program
# "make clean" cleans up
# "make realclean" cleans even better
# "make depend" Creates the dependencies to help make know what needs to be
#               rebuilt. You only need to run this once after unpacking.


# Change the following two lines to your first and last name!

FIRST_NAME=chris
LAST_NAME=kolegraff

# You can add more assembly files as you go.  Just edit the C code and
# put #ifdef around the function you want to change to assembly (look
# at bigint_adc() for an example), then create the new assembly (.S)
# file and add it to the ASMOBJS list.  Use .o instead of .S in the
# ASMOBJS list, and make will find the .S file. Do "make clean" to
# force a complete rebuild.

ASMOBJS = bigint_adc.o bigint_mul_uint.o bigint_shift_left_chunk.o
ASMOBJS += bigint_from_int.o bigint_alloc.o bigint_smallmod.o bigint_mul.o
ASMOBJS += qdiv64by10.o accumulate_noTrim.o bigint_free.o divide_Functions.o
DEFINES = -DUSE_ASM

CC = gcc
COBJS = bigint.o 
DEPENDFLAGS = -M 
INCLUDES = -I.
LIBDIRS = 
LIBS =

# Use the following flags for profiling
# CFLAGS = -pg -Wall -marm -O2
# LFLAGS = -pg -marm -O2

# Use the following flags for debugging
# CFLAGS = -g -Wall -marm 
# LFLAGS = -g -marm 

# Use the following flags for SPEED TESTS!
 CFLAGS = -Wall -marm -O2
 LFLAGS = -marm -O2

# Or use the following flags for SPEED TESTS!
# CFLAGS = -finline-functions -flto -Wall -marm -O2
# LFLAGS = -finline-functions -flto -marm -O2

#CFLAGS = -Wall -marm
#LFLAGS = -marm


bigfact: $(COBJS) $(ASMOBJS) bigfact.o
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $(COBJS) $(ASMOBJS) bigfact.o $(LIBDIRS) $(LIBS) -lm


testbigint: $(COBJS) $(ASMOBJS) testbigint.o
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $(COBJS) $(ASMOBJS) testbigint.o $(LIBDIRS) $(LIBS) -lm

.c.o:
	$(CC) -c $(DEFINES) $(CFLAGS) $(INCLUDES) $<

.s.o:
	$(CC) -c $(DEFINES) $(CFLAGS) $(INCLUDES) $<

.S.o:
	$(CC) -c $(DEFINES) $(CFLAGS) $(INCLUDES) $<

tarfile: realclean
	rm -f $(LAST_NAME)_$(FIRST_NAME).tgz
	cp -rp . /tmp/$(LAST_NAME)_$(FIRST_NAME)
	cd /tmp; tar cfz $(LAST_NAME)_$(FIRST_NAME).tgz $(LAST_NAME)_$(FIRST_NAME)
	mv /tmp/$(LAST_NAME)_$(FIRST_NAME).tgz .
	rm -rf /tmp/$(LAST_NAME)_$(FIRST_NAME)
	make depend


timing:	bigfact
	./report_time.sh

clean:
	rm -f *.o testbigint bigfact core.* gmon.out

realclean: clean
	rm -f *~ .depend

# make depend will create a file ".depend" with all the C dependencies
depend:
	rm -f .depend
	$(CC) $(DEFINES) $(INCLUDES) $(DEPENDFLAGS) $(COBJS:.o=.c) > .depend

# if we have a .depend file, include it
ifeq (.depend,$(wildcard .depend))
include .depend
endif


