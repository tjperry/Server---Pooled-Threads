This project involved creating a server that could respond to multiple clients by having a pool of threads.
These threads would process the client's requests when a thread was available. This required a system in which
wait() and signal() are implemented to handle server load and limitations without losing clients.
