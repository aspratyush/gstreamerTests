OBJS =		gstOggPlayer.o

TARGET =	gstOggPlayer
#*****************************************************************************
# Compiler and assembler
#*****************************************************************************
CROSSPREFIX = 
#PC tool chain
AR				?= ar
CC				?= gcc
CXX				?= g++
LD				?= gcc
NM				?= nm
RANLIB			?= ranlib
STRIP			?= strip
test			?= 1

#*****************************************************************************
# Compiler and Linker Flags
#*****************************************************************************
CXXFLAGS 		+=	-O2 -g -Wall -fmessage-length=0
CXXFLAGS		+=	$(INCLUDES) $(COMMONFLAGS) -std=c++11 -Os
CXXFLAGS		+=	`pkg-config --cflags gstreamer-1.0`
CFLAGS			+=	-g `pkg-config --cflags gstreamer-1.0`
LIBS 			+=	`pkg-config --libs gstreamer-1.0`

$(TARGET):	$(OBJS)
	$(CROSSPREFIX)$(CC) $(TARGET).c -o $(TARGET) $(LIBS) $(CFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
