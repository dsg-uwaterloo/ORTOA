FROM ubuntu:latest
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
thrift-compiler \
librocksdb-dev \
libsodium-dev \
libboost-all-dev \
automake \
libthrift-dev \
g++ \
git \ 
libevent-dev \
openjdk-11-jdk \
vim \
make

# RUN sudo apt-get install ant
# RUN wget http://www.apache.org/dyn/closer.cgi?path=/thrift/0.15.0/thrift-0.15.0.tar.gz
# RUN tar -xvf thrift-0.15.0.tar.gz
# RUN cd thrift-0.15.0
# RUN ./bootstrap.sh \
# ./configure \
# sudo make \
# sudo make install


RUN ln -s /usr/bin/clang-9 /usr/bin/clang
RUN ln -s /usr/bin/clang++-9 /usr/bin/clang++
ENV JAVA_HOME /usr/lib/jvm/java-11-openjdk-arm64
RUN git clone https://github.com/ySteinhart1/OpScure.git
WORKDIR "/OpScure"
RUN mkdir db
# RUN make
