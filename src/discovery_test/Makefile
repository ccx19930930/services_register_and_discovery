INC_DIR:= ./ ../zk_util/ /usr/local/include/zookeeper/ /usr/local/include/json/
SRCS:= $(wildcard ./*cpp ../zk_util/*cpp)
OBJS:= $(patsubst %.cpp, %.o, $(SRCS))
LIBS:= -lpthread -lzookeeper_mt -ljsoncpp

CXX:= g++

CXXFLAGS:= -w -g -std=c++11 $(addprefix -I, $(INC_DIR)) $(LIBS) -Wl,-rpath="/usr/local/lib" -D _DEBUG_

EXE:= ../../bin/discovery_test

$(EXE):$(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS)

clean:
	rm -rf $(EXE)
	rm -rf $(OBJS)
