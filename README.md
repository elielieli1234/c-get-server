# Get Head

## About
A web server in C that serves GET and HEAD requests. Supports markup,
stylesheets, and binary data.

## Ramble
This is really just a hobby project in which I bolted on features I cared 
to implement. It obviously does not cover the full scope of HTTP 1.0, but 
I did implement chunked encoding. I also implemented some of HTTP's error
responses, with endpoints for each. And for whatever reason I enjoy playing
with mildly obscure Unix APIs, so I used kqueue (MACs analogue to epoll),
in order to push socket events into a thread pool. The threads do all the
fun parsing.

## Compile Script
clang -g -fsanitize=address -o ./executables/main ./web_api/http/http.c ./web_api/http/message.c ./web_api/http/verify.c ./web_api/http/get_file.c ./web_api/socket/socket.c ./web_api/thread_pool/thread_pool.c ./main.c

### Compile Flags
-Wall
-Werror
-Wextra
clang --analyze
valgrind

### Implementation Sludge
The main thread registers new listeners on the kernel queue. 
It also detaches a marshaller thread.

The marshaller thread pushes the socket events into the thread pool.
The threads read the messages off the file descriptor, then parse.

It only parses the first line. If the first line is semantically correct, we open the file, create the header, and write it back in however many chunks
are necessary. If something goes wrong, we create the filepath to the corresponding error endpoint and write that back instead.

That's really it.

#### Notes Quirks
If the error code uri somehow exceeds 32 bytes it is noted as an internal error

If we add more response codes, but not endpoints for them, they will logically become a 404.

The MTU is used pretty much ubiquitously for any stack array.




