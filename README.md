# c_encrypted_chat

## Encrypted Chat System in C

Chat System where the messages are encrypted locally from one Client, sent through an intermidiary server node, and decrypted at arival by the other Client, thus securing the connection from potential attacks of the server, or package sniffing. It is a symmetric key encryption since both Clients are using the same key.

## Usage:

Before running, make sure you have the required packages installed: `gcc` and `OpenSSL`.

To compile the files run the Makefile:

	$ make

Now open 3 terminal windows in this directory. On one of them, run the server:

	$ ./chat_server

On the other two, run the Client to setup a connection with the server:

	$ ./chat_client

At the prompt enter your username and the key you will be using for the session. Make sure both clients have the exact same key in order to be able to communicate effectively. Even though OpenSSL can derives a SHA256 hash key from your password, for optimal security, pick a stronger key.

## Encryption:

By default, the encryption used is `AES256-CBC`, if you wish to use other types of encryption, modify the `chat_client.c` file's global string literal `ENCRYPTION_MODE`

## Real World Application:

If you would like to use this Chat System, you must set up the server on static machine and run it on a given port (you can change ports in the Makefile), and port forward the connection through your router.
On the clients side, you will need to modify the `chat_client.c` files with the correct IP.

## Future Expansion:

The buffer currently is very small and can only send messages of less than 24 characters length or less, if you try to send more you will get a `bad magic number` openssl error because encryption code was not transfered. 