COMPILATION:-
-)To compile the code, run the following command:-
gcc client.c -o client -lssl -lcrypto


EXECUTION:-
-)To run the client, enter the commant
./client <url>

Note:- 
1)Enclose url in quotes to make sure that the terminal does not treat characters like '&' in URL as special characters 
2)For SSL connection, snippets of code have been taken from http://fm4dd.com/openssl/sslconnect.shtm 
