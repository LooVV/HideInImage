CC	=	g++
CFLAGS	=	-g -Wall
INCLUDES=	-I/usr/local/include/libpng16
LFLAGS	=	-L/usr/local/lib
LIBS	=	-lpng16 -lz
SRCDIR  =	src
OBJDIR	=	build
BINDIR 	=   	bin

SRCS	:= 	$(wildcard $(SRCDIR)/*.cpp)
OBJS 	:= 	$(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET 	 = 	$(BINDIR)/hide_in_image

all: CHDR $(TARGET)


.PHONY: CHDR


CHDR:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)


$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)


$(OBJS) : ./$(OBJDIR)/%.o : ./$(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@




.PHONY: clean

clean:
	rm -r $(OBJDIR) && \
	rm -r $(BINDIR)
