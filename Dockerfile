FROM ubuntu
RUN apt-get update && apt-get install -y cmake libboost-all-dev g++
COPY . /server/
WORKDIR /server/
RUN make all
CMD ["./cache-server", ">&1"]
EXPOSE  12345