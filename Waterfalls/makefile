
CXX=g++
INCLUDES=

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
FLAGS=-D__LINUX_ALSASEQ__ -D__UNIX_JACK__ -c -g
#FLAGS=-D__LINUX_JACK__ -D__UNIX_JACK__ -c
LIBS=-lasound -lpthread -ljack -lstdc++ -lm
endif
ifeq ($(UNAME), Darwin)
FLAGS=-D__MACOSX_CORE__ -c
LIBS=-framework CoreAudio -framework CoreMIDI -framework CoreFoundation \
	-framework IOKit -framework Carbon  -framework OpenGL \
	-framework GLUT -framework Foundation -framework AppKit \
	-lstdc++ -lm
endif


FFT_OBJS=   RtAudio.o fft.o chuck_fft.o Thread.o Stk.o
OBJS=   RtAudio.o Waterfalls.o chuck_fft.o Thread.o Stk.o WvIn.o Waterfall.o RgbImage.o

Waterfalls: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)
	
Waterfalls.o: Waterfalls.cpp RtAudio.h chuck_fft.h Thread.h Stk.h Waterfall.h WvIn.h RgbImage.h
	$(CXX) $(FLAGS) Waterfalls.cpp

fft: $(FFT_OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)
	
fft.o: fft.cpp RtAudio.h chuck_fft.h Thread.h Stk.h
	$(CXX) $(FLAGS) fft.cpp

Thread.o: Thread.cpp Stk.h
	$(CXX) $(FLAGS) Thread.cpp

Stk.o: Stk.cpp
	$(CXX) $(FLAGS) Stk.cpp

RtAudio.o: RtAudio.h RtAudio.cpp RtError.h
	$(CXX) $(FLAGS) RtAudio.cpp

Waterfall.o: Waterfall.cpp Waterfall.h
	$(CXX) $(FLAGS) Waterfall.cpp

WvIn.o: WvIn.cpp WvIn.h Stk.h
	$(CXX) $(FLAGS) WvIn.cpp

RgbImage.o: RgbImage.cpp RgbImage.h
	$(CXX) $(FLAGS) RgbImage.cpp

clean:
	rm -f *~ *# *.o Waterfalls