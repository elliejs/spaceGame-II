CFILES = $(wildcard */*/*.c */*.c)
OFILES = $(CFILES:.c=.o)
OBJDIR = build
OFILESFLAT = $(addprefix $(OBJDIR)/,$(notdir $(OFILES)))
LFLAGS = -lssh -lm -lpthread
CFLAGS = -Wunused -msse4.2 -Wno-unused-command-line-argument

spacegame : $(OFILES)
	$(CC) $(OFILESFLAT) -o spaceGame $(LFLAGS)

OPTIMIZED :
	$(eval CFLAGS := $(CFLAGS) -O3 -ffast-math)

optimized : OPTIMIZED spacegame

DEBUG :
	$(eval CFLAGS := $(CFLAGS) -D SG_DEBUG -g3)

debug : DEBUG spacegame

$(OFILES) : %.o: %.c
			$(CC) -c $< -o $(OBJDIR)/$(notdir $@) $(CFLAGS)

all : clean spacegame

clean :
	rm $(OFILESFLAT)

help :
	@echo	"make :: [optimized, debug, all, clean, help(this message)]"
