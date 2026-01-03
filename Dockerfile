FROM gcc:12
RUN apt-get update && apt-get install -y \
     libsqlite3-dev \
     cmake \
     gdb \
     rsync
WORKDIR /app
COPY . .
RUN make clean && make
EXPOSE 3490
CMD ["./bin/server"]