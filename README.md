# iot_server

## Implements
- Manage incoming connections
- Multithread those connections
- Craft HTTP Packets
- Serve Web Pages
- Graceful Error Handling
    - Exception Safe
    - Pass Valgrind
    - Resilient to bad connections and DOS
- Create a compilation script
- Compile to a docker container

clang -g -fsanitize=address -o ./executables/main ./web_api/http/http.c ./web_api/socket/socket.c ./web_api/thread_pool/thread_pool.c ./main.c

-Wall
-Werror
-Wextra
clang --analyze
valgrind


