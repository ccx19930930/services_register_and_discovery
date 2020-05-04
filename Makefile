INC_DIR:=./src/ /usr/local/include/zookeeper/ /usr/local/include/json/
SRCS:=$(wildcard ./src/*cpp)
OBJS:= $(patsubst %.cpp, %.o, $(SRCS))
LIBS:= -lpthread -lzookeeper_mt -ljsoncpp

CXX:=g++

CXXFLAGS:= -w -g -std=c++11 $(addprefix -I, $(INC_DIR)) $(LIBS)

EXE:=./bin/zk_handle.exe

$(EXE):$(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS)

clean:
	rm -rf $(EXE)
	rm -rf $(OBJS)
