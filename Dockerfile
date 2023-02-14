FROM gcc:4.9
COPY . /server/
WORKDIR /server/
RUN make all
CMD ["./cache-server"]