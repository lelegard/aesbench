EXECS = aesbench
CPPFLAGS += -std=c++11
CXXFLAGS += -Werror -Wall -Wextra -Wno-unused-parameter
LDLIBS += -lcrypto -lm

default: $(EXECS)
	@true
clean:
	rm -rf $(EXECS) *.o *.tmp *.log __pycache__
