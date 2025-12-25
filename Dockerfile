FROM gcc:latest
RUN apt-get update && apt-get install -y libsqlite3-dev
WORKDIR /app
COPY . .
RUN make clean && make
EXPOSE 3490
CMD ["./server"]