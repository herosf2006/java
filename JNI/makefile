TARGET     = Hello
LIBRARY    = libhello
CLASS_PATH = .
JAVA_PATH  = /usr/local/java/jdk1.8.0_144/include

#vpath %.class $(CLASS_PATH)

all : $(TARGET).so

$(TARGET).so : $(TARGET).o
	g++ -I $(JAVA_PATH) -shared -o $(LIBRARY).so -fPIC $(TARGET).cpp -lc

$(TARGET).o :  $(TARGET).h
	g++ -I $(JAVA_PATH) -c $(TARGET).cpp

$(TARGET).h :  $(TARGET).class
	javah -jni $(TARGET).class

$(TARGET).class :
	javac $(TARGET).java

run :
	java -Djava.library.path=$(CLASS_PATH) $(TARGET)

clean:
	rm *.h *.so *.o *.class


1. http://ethdocs.org/en/latest/introduction/index.html
2. https://solidity.readthedocs.io/en/develop/
3. https://www.edx.org/course/blockchain-business-introduction-linuxfoundationx-lfs171x
4. https://hyperledger-fabric.readthedocs.io/en/release/
