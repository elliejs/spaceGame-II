CFILES = $(wildcard */*/*.c */*.c)
OFILES = $(CFILES:.c=.o)
OBJDIR = build
OFILESFLAT = $(addprefix $(OBJDIR)/,$(notdir $(OFILES)))
LFLAGS = -lssh -lm
CFLAGS =

spacegame : $(OFILES)
	$(CC) $(OFILESFLAT) -o spaceGame $(LFLAGS)

OPTIMIZED :
	$(eval CFLAGS := $(CFLAGS) -O3 -ffast-math)

optimized : OPTIMIZED spacegame

DEBUG :
	$(eval CFLAGS := $(CFLAGS) -D SG_DEBUG)

debug : DEBUG spacegame

$(OFILES) : %.o: %.c
			$(CC) -c $< -o $(OBJDIR)/$(notdir $@) $(CFLAGS)

all : clean spacegame

clean :
	rm $(OFILESFLAT)

help :
	@echo	"make :: [optimized, debug, all, clean, shader, help(this message)]"
