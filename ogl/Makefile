SYSCONF_LINK = g++
CPPFLAGS     = -I/opt/homebrew/include
LDFLAGS      = -L/opt/homebrew/lib
LIBS         = -lm -lGLEW -framework OpenGL -framework GLUT

DESTDIR = ./
TARGET  = main_gl

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
