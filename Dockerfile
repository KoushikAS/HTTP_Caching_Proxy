FROM gcc:4.9
COPY . /server/
WORKDIR /server/
RUN make all
CMD ["./cache-server", ">&1"]
EXPOSE  12345