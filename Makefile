TARGET = winmm.dll

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -O3 #-g
CXXFLAGS += -shared -static #-D_DEBUG_CONSOLE -D_TDC
CXXFLAGS += -Iimgui
CXXFLAGS += -Wno-missing-field-initializers
CXXFLAGS += -Wno-invalid-offsetof -Wno-unused-parameter -Wno-cast-function-type
CXXFLAGS += `pkg-config --cflags --static glfw3 lua5.1`

LIBS = `pkg-config --libs --static glfw3 lua5.1`
LIBS += -lMinHook -ldwmapi -lcrypt32 -lws2_32 -lz
LIBS += -ld3d12 -lD3DCompiler -ldxgi -ldxguid

SOURCES = dllmain.cpp src/extended_api.cpp src/lua_utils.cpp src/memory.cpp src/networking.cpp src/recorder.cpp
SOURCES += imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
SOURCES += imgui/backends/imgui_impl_dx12.cpp imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_win32.cpp

OBJDIR = obj
OBJS = $(SOURCES:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(addprefix $(OBJDIR)/, $(notdir $(OBJS)))

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Nice try ;)"
	CXX = echo
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
endif

ifeq ($(UNAME_S), Darwin)
	ECHO_MESSAGE = "Really?"
	CXX = echo
endif

.PHONY: all clean rebuild

all: $(TARGET)
	@echo Build complete for: $(ECHO_MESSAGE)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/%.cpp src/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: imgui/backends/%.cpp imgui/backends/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rebuild: clean all

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o
