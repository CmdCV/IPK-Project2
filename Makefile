# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -DDEBUG_PRINT
LDFLAGS =

# Faculty XLOGIN
XLOGIN = xurbana00

# Target name
TARGET = ipk25-chat

# Directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
DOC_DIR = doc
SRC_DIR = src
INC_DIR = $(SRC_DIR)/inc
LIB_DIR = $(LIB_DIR)/inc
TEST_DIR = test

# Find all source files
SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Main rule
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compilation rule
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generate UML diagram
uml:
	hpp2plantuml -i "$(INC_DIR)/*.h" -o uml.puml
	plantuml -svg uml.puml -o $(DOC_DIR)/images/
	@rm -f uml.puml

# Zip rule for submission
zip: uml
	zip -r $(XLOGIN).zip $(DOC_DIR)/ $(SRC_DIR)/ CHANGELOG.md README.md LICENSE Makefile -x "*.DS_Store"

# Test rule
#test: $(TARGET)
#	@clear
#	@chmod +x $(TEST_DIR)/test_arg.sh
#	@$(TEST_DIR)/test_arg.sh


# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(XLOGIN).zip


# Phony targets
.PHONY: all uml zip clean
