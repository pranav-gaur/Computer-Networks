COMPILATION:-
-)To compile client, run:-
gcc client.c -o client -lcrypto

-)To compile server, run:-
gcc server.c -o server


EXECUTION:-
-)public_0.pem and private_0.pem is a pair of corresponding public-private keys. Similarly, public_1.pem and private_1.pem 
  is another pair of corresponding public-private keys.

-) To execute server, run:-
./server <portno.>

-) To execute client, run:-
./client <server_ip_address> <server_portno.> <private_key_of_client> <public_key_other_client> 


SAMPLE RUN:-
-)In server's directory, run:-
./server 9009

-)For first client, run:-
./client localhost 9009 private_0.pem public_1.pem 

-)For second client, run:-
./client localhost 9009 private_1.pem public_0.pem 
