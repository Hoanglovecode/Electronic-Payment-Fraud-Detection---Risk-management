CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

SRCS_CORE = src/common/Types.cpp \
            src/models/Location.cpp \
            src/models/Device.cpp \
            src/models/PaymentMethod.cpp \
            src/models/Merchant.cpp \
            src/models/Account.cpp \
            src/models/Customer.cpp \
            src/models/FraudAlert.cpp \
            src/models/RiskAssessment.cpp \
            src/models/Dispute.cpp \
            src/models/Transaction.cpp

SRCS_APP = src/main.cpp
SRCS_TEST = tests/main_test.cpp tests/test_architecture.cpp tests/test_domain_models.cpp

BIN_DIR = bin

TARGET_APP = $(BIN_DIR)/epfd_app.exe
TARGET_TEST = $(BIN_DIR)/epfd_tests.exe

.PHONY: all clean build test run

all: build test

build: $(TARGET_APP) $(TARGET_TEST)

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

$(TARGET_APP): $(SRCS_APP) $(SRCS_CORE) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS_APP) $(SRCS_CORE)

$(TARGET_TEST): $(SRCS_TEST) $(SRCS_CORE) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -Itests -o $@ $(SRCS_TEST) $(SRCS_CORE)

test: $(TARGET_TEST)
	@echo.
	@echo Running Tests...
	@$(TARGET_TEST)

run: $(TARGET_APP)
	@echo.
	@echo Running Application...
	@$(TARGET_APP)

clean:
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"
