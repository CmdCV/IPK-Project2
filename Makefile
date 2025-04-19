# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -DDEBUG_PRINT
LDFLAGS =

# Faculty XLOGIN
XLOGIN = xurbana00

# Target name
TARGET = ipk25chat-client

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

run: clean all
	@clear
	@chmod +x $(TARGET)

run-tcp: run
	@./$(TARGET) -t tcp -s vitapavlik.cz
run-udp: run
	@./$(TARGET) -t udp -s localhost

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
	@# Remove @enduml for editing
	@sed -i '' -E 's/@enduml//g' uml.puml
	@echo 'MessageFactory *-- Message' >> uml.puml
	@echo 'TCPClient *-- MessageFactory' >> uml.puml
	@echo 'UDPClient *-- MessageFactory' >> uml.puml
	@echo 'ArgHandler *-- ParsedArgs' >> uml.puml
	@echo 'class Main {' >> uml.puml
	@echo '    +main(int argc, char* argv) : int' >> uml.puml
	@echo '}' >> uml.puml
	@echo 'Main ..> InputHandler' >> uml.puml
	@echo 'Main ..> ArgHandler' >> uml.puml
	@# Insert @enduml after editing
	@echo '@enduml' >> uml.puml
	plantuml -svg uml.puml -o $(DOC_DIR)/images/
	@rm -f uml.puml

# Zip rule for submission
zip: uml
	zip -r $(XLOGIN).zip $(DOC_DIR)/ $(SRC_DIR)/ CHANGELOG.md README.md LICENSE Makefile -x "*.DS_Store"



# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(XLOGIN).zip


# Phony targets
.PHONY: all uml zip clean
