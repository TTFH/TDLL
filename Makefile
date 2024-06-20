DLL = pros.sdk.x64.dll
ODIR = obj
IMGUI_DIR = imgui

SOURCES = dllmain.cpp lua_utils.cpp extended_api.cpp glad/glad.c
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_win32.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -Wall -Wextra -Werror -Wpedantic
CXXFLAGS += -s -shared -static -g -DDEBUGCONSOLE
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -Wno-cast-function-type -Wno-unused-parameter -Wno-invalid-offsetof
CXXFLAGS += -Wno-missing-field-initializers
CXXFLAGS += `pkg-config --cflags glfw3`
LIBS = `pkg-config --libs --static libcurl`
LIBS += -lMinHook -lz -ldwmapi -lgdi32 -limm32 -Llua5.1.4 -llua5.1 -lcrypt32 -lws2_32 -lglfw3 -lopengl32

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux)
	CXX = x86_64-w64-mingw32-g++
	ECHO_MESSAGE = "Linux"
endif

ifeq ($(OS), Windows_NT)
	CXX = g++
	ECHO_MESSAGE = "Windows"
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
.PHONY: all clean

$(ODIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: glad/%.c glad/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(IMGUI_DIR)/backends/%.cpp $(IMGUI_DIR)/backends/%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(DLL)
	@echo Build complete for $(ECHO_MESSAGE)

$(DLL): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(DLL) $(OBJS)
