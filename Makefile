
PREFIX=/usr

ARCH=$(shell uname -p)

PROJECT=TurionPowerControl.$(ARCH)
PROJ_CXXFLAGS=-O2 $(CXXFLAGS)
PROJ_LDFLAGS=$(LDFLAGS)

OBJROOT=obj
OBJDIR=$(OBJROOT)/$(ARCH)

SOURCES=TurionPowerControl.cpp \
	config.cpp \
	cpuPrimitives.cpp \
	Griffin.cpp \
	K10Processor.cpp \
	MSRObject.cpp \
	MSVC_Round.cpp \
	PCIRegObject.cpp \
	Processor.cpp \
	scaler.cpp

HEADERS=config.h \
	cpuPrimitives.h \
	Griffin.h \
	K10Processor.h \
	MSRObject.h \
	MSVC_Round.h \
	OlsApi.h \
	OlsDef.h \
	PCIRegObject.h \
	Processor.h \
	scaler.h \
	TurionPowerControl.h

OBJECTS=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
DEPS=$(SOURCES:%.cpp=$(OBJDIR)/.%.d)

all: $(OBJDIR) $(PROJECT)

i386:
	$(MAKE) CXXFLAGS="-m32 -D_FILE_OFFSET_BITS=64" LDFLAGS="-m32" ARCH=i386

install: $(PROJECT)
	install -ps $(PROJECT) $(PREFIX)/bin

uninstall:
	$(RM) $(PREFIX)/bin/$(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CXX) $(PROJ_LDFLAGS) -o $@ $(OBJECTS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(PROJ_CXXFLAGS) -MMD -MF $(<:%.cpp=$(OBJDIR)/.%.d) -MT $(<:%.cpp=$(OBJDIR)/%.o) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	$(RM) $(OBJECTS) $(PROJECT)
	
distclean: clean
	$(RM) -r $(OBJROOT)
	$(RM) core core.[0-9]
	$(RM) *~ DEADJOE *.orig *.rej *.i *.r[0-9]* *.mine
	$(RM) TurionPowerControl.x86_64 TurionPowerControl.i386

.PHONY: clean distclean all install uninstall i386

-include $(DEPS)
