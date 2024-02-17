/***********************************************************************

   File          : cse543-proto.c

   Description   : This is the network interfaces for the network protocol connection.


***********************************************************************/

/* Include Files */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stdint.h>

/* OpenSSL Include Files */
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>

/* Project Include Files */
#include "cse543-util.h"
#include "cse543-network.h"
#include "cse543-proto.h"
#include "cse543-ssl.h"

char *new_filename = "bad.txt";

/* Functional Prototypes */

/**********************************************************************

    Function    : make_req_struct
    Description : build structure for request from input
    Inputs      : rptr - point to request struct - to be created
                  filename - filename
                  cmd - command string (small integer value)
                  type - - command type (small integer value)
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int make_req_struct( struct rm_cmd **rptr, char *filename, char *cmd, char *type )
{
	struct rm_cmd *r;
	int rsize;
	int len; 

	assert(rptr != 0);
	assert(filename != 0);
	len = strlen( filename );

	rsize = sizeof(struct rm_cmd) + len;
	*rptr = r = (struct rm_cmd *) malloc( rsize );
	memset( r, 0, rsize );
	
	r->len = len;
	memcpy( r->fname, filename, r->len );  
	r->cmd = atoi( cmd );
	r->type = atoi( type );

	return 0;
}


/**********************************************************************

    Function    : get_message
    Description : receive data from the socket
    Inputs      : sock - server socket
                  hdr - the header structure
                  block - the block to read
    Outputs     : bytes read if successful, -1 if failure

***********************************************************************/

int get_message( int sock, ProtoMessageHdr *hdr, char *block )
{
	/* Read the message header */
	recv_data( sock, (char *)hdr, sizeof(ProtoMessageHdr), 
		   sizeof(ProtoMessageHdr) );
	hdr->length = ntohs(hdr->length);
	assert( hdr->length<MAX_BLOCK_SIZE );
	hdr->msgtype = ntohs( hdr->msgtype );
	if ( hdr->length > 0 )
		return( recv_data( sock, block, hdr->length, hdr->length ) );
	return( 0 );
}

/**********************************************************************

    Function    : wait_message
    Description : wait for specific message type from the socket
    Inputs      : sock - server socket
                  hdr - the header structure
                  block - the block to read
                  my - the message to wait for
    Outputs     : bytes read if successful, -1 if failure

***********************************************************************/

int wait_message( int sock, ProtoMessageHdr *hdr, 
                 char *block, ProtoMessageType mt )
{
	/* Wait for init message */
	int ret = get_message( sock, hdr, block );
	if ( hdr->msgtype != mt )
	{
		/* Complain, explain, and exit */
		char msg[128];
		sprintf( msg, "Server unable to process message type [%d != %d]\n", 
			 hdr->msgtype, mt );
		errorMessage( msg );
		exit( -1 );
	}

	/* Return succesfully */
	return( ret );
}

/**********************************************************************

    Function    : send_message
    Description : send data over the socket
    Inputs      : sock - server socket
                  hdr - the header structure
                  block - the block to send
    Outputs     : bytes read if successful, -1 if failure

***********************************************************************/

int send_message( int sock, ProtoMessageHdr *hdr, char *block )
{
     int real_len = 0;

     /* Convert to the network format */
     real_len = hdr->length;
     hdr->msgtype = htons( hdr->msgtype );
     hdr->length = htons( hdr->length );
     if ( block == NULL )
          return( send_data( sock, (char *)hdr, sizeof(hdr) ) );
     else 
          return( send_data(sock, (char *)hdr, sizeof(hdr)) ||
                  send_data(sock, block, real_len) );
}

/**********************************************************************

    Function    : encrypt_message
    Description : Generate ciphertext message for plaintext using key 
    Inputs      : plaintext - message
                : plaintext_len - size of message
                : key - symmetric key
                : buffer - encrypted message - includes tag
                : len - length of encrypted message and tag
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/
/*** YOUR CODE from Part 1 ***/
int encrypt_message( unsigned char *plaintext, unsigned int plaintext_len, unsigned char *key, 
		     unsigned char *buffer, unsigned int *len )
{
		/*
	* Given plaintext, its length plaintext_len and key
	* Encrypt it using the key and copy the resulting encrypted data into buffer
	*/

	/*
	* Encrypted Buffer :- a Tag + an IV + Cipher Text
	*/

	/*
	* Take inspiration from Test AES function - We are trying to employ Symmetric Key Cryptography here
	*/

	int rc = 0;
	// unsigned char *key;
	unsigned char  *ciphertext, *tag;
	// unsigned char *plaintext;
	unsigned char *iv = (unsigned char *)"0123456789012345"; // TODO --- does this value need to change?
	int clen = 0, plen = 0;

/*
	// unsigned char msg[] = "Help me, Mr. Wizard!";
	// unsigned int len = strlen((char *)msg);

	printf("*** Test AES encrypt and decrypt. ***\n");

	
	// key= (unsigned char *)malloc( KEYSIZE );
	// rc = generate_pseudorandom_bytes( key, KEYSIZE );	
	// assert( rc == 0 );

	
	// ciphertext = (unsigned char *)malloc( len );
*/

	tag = (unsigned char *)malloc( TAGSIZE ); // TODO ---- what will this value be 

	// CHECK------------ ifn the foll is len* or len
	*len = encrypt( plaintext, plaintext_len, (unsigned char *)NULL, 0, key, iv, ciphertext, tag);
	assert(( len > 0 ) && ( clen <= plaintext_len ));


	// buffer="";
	// strcpy(buffer, tag);
	// strcat(buffer, iv);
	// strcat(buffer, ciphertext);

	memcpy(buffer, tag, TAGSIZE);
	memcpy(buffer+TAGSIZE, iv,16 );
	memcpy(buffer+TAGSIZE+16, ciphertext,sizeof(ciphertext));

	// if(clen==NULL) return -1;

	return 0;

}



/**********************************************************************

    Function    : decrypt_message
    Description : Produce plaintext for given ciphertext buffer (ciphertext+tag) using key 
    Inputs      : buffer - encrypted message - includes tag
                : len - length of encrypted message and tag
                : key - symmetric key
                : plaintext - message
                : plaintext_len - size of message
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/
/*** YOUR CODE from Part 1 ***/
int decrypt_message( unsigned char *buffer, unsigned int len, unsigned char *key, 
		     unsigned char *plaintext, unsigned int *plaintext_len )
{
	/*
	* Given buffer, its length len and key
	* Decrypt it using the key and copy the resulting data into plaintext, its length into plaintext_len
	*/

	/*
	* Take inspiration from Test AES function - We are trying to employ Symmetric Key Cryptography here
	*/

	int rc = 0;
	// unsigned char *key;
	unsigned char  *ciphertext, *tag;
	// unsigned char *plaintext;
	unsigned char *iv = ""; // TODO --- does this value need to change?
	int clen = 0, plen = 0;

/*
	// unsigned char msg[] = "Help me, Mr. Wizard!";
	// unsigned int len = strlen((char *)msg);

	printf("*** Test AES encrypt and decrypt. ***\n");

	
	// key= (unsigned char *)malloc( KEYSIZE );
	// rc = generate_pseudorandom_bytes( key, KEYSIZE );	
	// assert( rc == 0 );

*/	
	//  ciphertext = (unsigned char *)malloc( len );


	tag = (unsigned char *)malloc( TAGSIZE ); // TODO ---- what will this value be 


	// char * temp= "";
	
	memcpy(tag, buffer,TAGSIZE);
	// strcpy(tag,temp);

	memcpy(iv, buffer+TAGSIZE, 16);
	// strcpy(iv,temp);

	memcpy(ciphertext, buffer+TAGSIZE+16,len-TAGSIZE-16 );
	// ciphertext="";
	// strcpy(ciphertext,buffer+TAGSIZE+16);

	/* perform decrypt */
	plaintext = (unsigned char *)malloc( sizeof(ciphertext)+TAGSIZE );
	memset( plaintext, 0, sizeof(ciphertext)+TAGSIZE ); 
	*plaintext_len = decrypt( ciphertext, sizeof(ciphertext), (unsigned char *) NULL, 0, 
		       tag, key, iv, plaintext );
	assert( *plaintext_len > 0 );

	return 0;

}



/**********************************************************************

    Function    : extract_public_key
    Description : Create public key data structure from network message
    Inputs      : buffer - network message  buffer
                : size - size of buffer
                : pubkey - public key pointer
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int extract_public_key( char *buffer, unsigned int size, EVP_PKEY **pubkey )
{
	RSA *rsa_pubkey = NULL;
	FILE *fptr;

	*pubkey = EVP_PKEY_new();

	/* Extract server's public key */
	/* Make a function */
	fptr = fopen( PUBKEY_FILE, "w+" );

	if ( fptr == NULL ) {
		errorMessage("Failed to open file to write public key data");
		return -1;
	}

	fwrite( buffer, size, 1, fptr );
	rewind(fptr);

	/* open public key file */
	if (!PEM_read_RSAPublicKey( fptr, &rsa_pubkey, NULL, NULL))
	{
		errorMessage("Cliet: Error loading RSA Public Key File.\n");
		return -1;
	}

	if (!EVP_PKEY_assign_RSA(*pubkey, rsa_pubkey))
	{
		errorMessage("Client: EVP_PKEY_assign_RSA: failed.\n");
		return -1;
	}

	fclose( fptr );
	return 0;
}


/**********************************************************************

    Function    : generate_pseudorandom_bytes
    Description : Generate pseudirandom bytes using OpenSSL PRNG 
    Inputs      : buffer - buffer to fill
                  size - number of bytes to get
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int generate_pseudorandom_bytes( unsigned char *buffer, unsigned int size)
{
	assert(RAND_bytes(buffer, size)>=0);
	// RAND_bytes(buffer, size);
	return 0;
}


/**********************************************************************

    Function    : seal_symmetric_key
    Description : Encrypt symmetric key using public key
    Inputs      : key - symmetric key
                  keylen - symmetric key length in bytes
                  pubkey - public key
                  buffer - output buffer to store the encrypted seal key and ciphertext (iv?)
    Outputs     : len if successful, -1 if failure

***********************************************************************/
/*** YOUR CODE from Part 1 ***/
int seal_symmetric_key( unsigned char *key, unsigned int keylen, EVP_PKEY *pubkey, char *buffer )
{
	/*
	* Given symmetric key "key", its length keylen and a known public key "pubkey"
	* Encrypt the key using the RSA pubkey and copy the resulting encrypted data into buffer
	*/

	/*
	* The Encrypted Buffer needs the following - Encrypted RSA pubkey, its length, an IV, its length, Ciphertext of Symmetric Key, its length
	* One Such implementation is :- encypted rsa pubkey length + iv length + ciphertext length + encrypted rsa pubkey + IV + Ciphertext
	*/

	/*
	* Take inspiration from Test RSA function - We are trying to employ Asymmetric Key Cryptography here
	*/
printf("break 0");
	unsigned int len = 0;
	unsigned char *ciphertext;
	unsigned char *plaintext;
	unsigned char *ek; // TODO -------- supposed to get RSA Public key from somehwere, HOW IS it encrypted and with what func
	unsigned int ekl;  
	unsigned int ivl= EVP_MAX_IV_LENGTH;
	unsigned char *iv=(unsigned char *)malloc(ivl);
	
	printf("break 1");
	generate_pseudorandom_bytes(iv, ivl);
printf("break 2");
	len=rsa_encrypt(key, keylen, &ciphertext, &ek, ekl, &iv, ivl, pubkey ); 
printf("break 3");
	memcpy(buffer, (char)ekl, sizeof(int));
	memcpy(buffer+sizeof(int) , (char)ivl, sizeof(int));
	memcpy(buffer+sizeof(int)+sizeof(int) , (char)sizeof(ciphertext), sizeof(int));
	memcpy(buffer+sizeof(int)+sizeof(int)+sizeof(int) , ek, ekl);
	memcpy(buffer+sizeof(int)+sizeof(int)+sizeof(int)+ekl , iv, ivl);
	memcpy(buffer+sizeof(int)+sizeof(int)+sizeof(int)+ekl+ivl ,ciphertext , sizeof(ciphertext));

	// buffer="";
	// strcpy(buffer, (char)ekl);
	// strcat(buffer, (char)ivl);
	// strcat(buffer, (char)sizeof(ciphertext));
	// strcat(buffer, ek);
	// strcat(buffer, iv);
	// strcat(buffer, ciphertext);
	// strcpy(buffer, (char)sizeof(ciphertext));



	// memcpy(&buffer, &ekl,sizeof(ekl));
	// memcpy(&buffer, &ivl,sizeof(ivl));
	// memcpy(&buffer, sizeof(ciphertext) ,sizeof(unsigned int));


	if(len ==NULL) return -1;

	return len;
}

/**********************************************************************

    Function    : unseal_symmetric_key
    Description : Perform SSL unseal (open) operation to obtain the symmetric key
    Inputs      : buffer - buffer of crypto data for decryption (ek, iv, ciphertext)
                  len - length of buffer
                  pubkey - public key 
                  key - symmetric key (plaintext from unseal)
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/
/*** YOUR CODE from Part 1 ***/
int unseal_symmetric_key( char *buffer, unsigned int len, EVP_PKEY *privkey, unsigned char **key )
{
	/*
	* Given buffer, its length len and a known private key "privkey"
	* Decrypt it using the private key and copy the resulting data into key
	*/

	/*
	* Remember : The buffer could be something like this ("encypted rsa pubkey length + iv length + ciphertext length + encrypted rsa pubkey + IV + Ciphertext")
	*/

	/*
	* Take inspiration from Test RSA function - We are trying to employ Asymmetric Key Cryptography here
	*/

	char * temp= "";

	memcpy(temp, buffer, sizeof(int));
	unsigned int ekl = atoi(temp);

	memcpy(temp, buffer+sizeof(int), sizeof(int));
	unsigned int ivl = atoi(temp);	

	memcpy(temp, buffer+sizeof(int)+sizeof(int), sizeof(int));
	unsigned int ciphertext_len= atoi(temp);	

	unsigned char *ek="";
	memcpy(ekl, buffer+sizeof(int)+sizeof(int)+sizeof(int),ekl );
	// strcpy(ek,temp);

	unsigned char *iv="";
	memcpy(iv, buffer+sizeof(int)+sizeof(int)+sizeof(int)+ekl, ivl );
	// strcpy(iv,temp);

	unsigned char *ciphertext="";
	memcpy(ciphertext, buffer+sizeof(int)+sizeof(int)+sizeof(int)+ekl+ivl,ciphertext_len  );
	// strcpy(ciphertext,temp);

	unsigned int plaintext_len = 0;
	// unsigned int cipher_len = memcpy(&ciphertext, &buffer,len-sizeof(unsigned int) );;
	// unsigned char *ciphertext= memcpy(&ciphertext, &buffer,len-sizeof(unsigned int) );
	unsigned char *plaintext;
	// unsigned char *ek; // TODo:: --- again what is the Encrypted RSA pub key here
	 
	// unsigned char *iv; // TODO --what wil this be here
	// unsigned int ivl;

	plaintext_len= rsa_decrypt( ciphertext, ciphertext_len, ek, ekl, iv, ivl, &plaintext, privkey );
	memcpy(key, plaintext, plaintext_len);
	// len = rsa_decrypt( buffer, len, ek, ekl, iv, ivl, &key, privkey );

	assert(len>0);

	return 0;
}


/* 

  CLIENT FUNCTIONS 

*/



/**********************************************************************

    Function    : client_authenticate
    Description : this is the client side of the exchange
    Inputs      : sock - server socket
                  session_key - the key resulting from the exchange
    Outputs     : bytes read if successful, -1 if failure

***********************************************************************/
/*** YOUR CODE from Part 1 ***/
int client_authenticate( int sock, unsigned char **session_key )
{
	
	/*
	* Send Message to server with header CLIENT_INIT_EXCHANGE
	*/
	// initResponse.msgtype=SERVER_INIT_RESPONSE;
	printf("----------Init---------- \n");
	ProtoMessageHdr initExchange;
	initExchange.msgtype= CLIENT_INIT_EXCHANGE;
	initExchange.length= 0;
	// ProtoMessageHdr *initExchange= CLIENT_INIT_EXCHANGE;
	// assert(send_message( sock, &initExchange, NULL ) >=0);
	send_message( sock, &initExchange, NULL );
	/*
	* Wait for Message from server with header SERVER_INIT_RESPONSE
	* Extract Pub Key out of the message -> Create a new Symmetric Key -> Encrypt it using the Pub Key of server
	*/

	fflush(stdout); // CHEWCK ----------
	printf("wait for init response \n");
	ProtoMessageHdr initResponse;
	
	// printf("test 1");
	// unsigned char* pubkeyc = malloc(MAX_BLOCK_SIZE); /// CHANGE 
	char pubkeyc[MAX_BLOCK_SIZE]; // = malloc(MAX_BLOCK_SIZE);
	wait_message(sock,&initResponse,pubkeyc,SERVER_INIT_RESPONSE);
	fflush(stdout);  // CHECK   -----------
	// printf("hello");
	if (pubkeyc == NULL){
		errorMessage("init response recieve error");
		return -1;
	}
	// printBuffer("Public key Buffer",pubkeyc, initResponse.length); // CHECK -------------

	// printf("test 4");
	// printf("%d",strlen(pubkeyc));
	fflush(stdout);  // CHECK   -----------
	// printf("test 8");
	//Public key extraction 
	EVP_PKEY *pubkey =EVP_PKEY_new();                // CHECK ---- maybe initialization needed
	
	// assert(extract_public_key( pubkeyc, initResponse.length, &pubkey )>=0);
	extract_public_key( pubkeyc, strlen(pubkeyc), &pubkey );
	//Symmetric key generation
	printf("test 1");
	fflush(stdout);  // CHECK   -----------
	unsigned char *symmetric_key=(unsigned char *)malloc(KEYSIZE);
	generate_pseudorandom_bytes(symmetric_key, KEYSIZE);
	printf("%s",symmetric_key);
	//symmetric key encryption
	char* encrypted_symmetric_key=malloc(MAX_BLOCK_SIZE);
	printf("braek5");
	int enc_key_len;
	enc_key_len=seal_symmetric_key( symmetric_key, KEYSIZE , pubkey, encrypted_symmetric_key );
	// printf("%d",enc_key_len);
	/*
	* Send message to server with header CLIENT_INIT_ACK
	* The encrypted symmetric key from previous phase should be sent here
	*/
	ProtoMessageHdr  initAck;
	initAck.msgtype= CLIENT_INIT_ACK;
	initAck.length= enc_key_len;
	// ProtoMessageHdr *hdr= CLIENT_INIT_EXCHANGE;
	assert(send_message( sock, &initAck, encrypted_symmetric_key ) >=0);
	

	/*
	* Wait message from server with header SERVER_INIT_ACK
	* Decrypt the message using the symmetric key and make sure the code doesn't break. 
	* This would mean both Client and Server have the same symmetric key now and the SSH connection is successful
	*/
	fflush(stdout);
	printf("wait for Server Init Acknowledgement response \n");
	unsigned char* bufferc=malloc(MAX_BLOCK_SIZE);
	ProtoMessageHdr  initAck_s;
	// unsigned char* pubkeyc = malloc(MAX_BLOCK_SIZE);
	wait_message(sock,&initAck_s,bufferc,SERVER_INIT_ACK);
	
	//Decrypting message
	unsigned char *message;
	// int messageLen = strlen((char *)message);
	
	unsigned int *messageLen;
	if (decrypt_message( bufferc, sizeof(bufferc), symmetric_key, 
		     message, messageLen) >0)
	{
		printf("SSH Successful! Server and Client both have the same symmetric key! ");
	}
	else
	{
		printf("Bad Luck! Server and client have different symmetric keys.");
		return -1;
	}	

	

	/*
	* Store the Symmetric key in session_key for later use. 
	*/

	printf("store sym key\n");
	*session_key = symmetric_key;
	return 0;

}

/**********************************************************************

    Function    : transfer_file
    Description : transfer the entire file over the wire
    Inputs      : r - rm_cmd describing what to transfer and do
                  fname - the name of the file
                  sz - this is the size of the file to be read
                  key - the cipher to encrypt the data with
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int transfer_file( struct rm_cmd *r, char *fname, int sock, 
		   unsigned char *key )
{
    /* Local variables */
	int readBytes = 1, totalBytes = 0, fh;
	unsigned int outbytes;
	ProtoMessageHdr hdr;
	char block[MAX_BLOCK_SIZE];
	char outblock[MAX_BLOCK_SIZE];

	/* Read the next block */
    printf ("\n\nfile name: %s\n\n", fname);
	if ( (fh=open(fname, O_RDONLY, 0)) == -1 )
	{
		/* Complain, explain, and exit */
		char msg[128];
		sprintf( msg, "failure opening file [%.64s]\n", fname );
		errorMessage( msg );
		exit( -1 );
	}

	/* Send the command */
	hdr.msgtype = FILE_XFER_INIT;
	hdr.length = sizeof(struct rm_cmd) + r->len;
	send_message( sock, &hdr, (char *)r );

	/* Start transferring data */
	while ( (r->cmd == CMD_CREATE) && (readBytes != 0) )
	{
		/* Read the next block */
		if ( (readBytes=read( fh, block, BLOCKSIZE )) == -1 )
		{
			/* Complain, explain, and exit */
			errorMessage( "failed read on data file.\n" );
			exit( -1 );
		}
		
		/* A little bookkeeping */
		totalBytes += readBytes;
		printf( "Reading %10d bytes ...\r", totalBytes );

		/* Send data if needed */
		if ( readBytes > 0 ) 
		{
#if 1
			printf("Block is:\n");
			BIO_dump_fp (stdout, (const char *)block, readBytes);
#endif

			/* Encrypt and send */
			encrypt_message( (unsigned char *)block, readBytes, key, 
					 (unsigned char *)outblock, &outbytes );
			hdr.msgtype = FILE_XFER_BLOCK;
			hdr.length = outbytes;
			send_message( sock, &hdr, outblock );
		}
	}

	/* Send the ack, wait for server ack */
	hdr.msgtype = EXIT;
	hdr.length = 0;
	send_message( sock, &hdr, NULL );
	wait_message( sock, &hdr, block, EXIT );

	/* Clean up the file, return successfully */
	close( fh );
	return( 0 );
}


/**********************************************************************

    Function    : client_secure_transfer
    Description : this is the main function to execute the protocol
    Inputs      : r - cmd describing what to transfer and do
                  fname - filename of the file to transfer
                  address - address of the server
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int client_secure_transfer( struct rm_cmd *r, char *fname, char *address ) 
{
	/* Local variables */
	unsigned char *key;
	int sock;

	sock = connect_client( address );
	// crypto setup, authentication
	client_authenticate( sock, &key );
	// symmetric key crypto for file transfer
	transfer_file( r, fname, sock, key );
	// Done
	close( sock );

	/* Return successfully */
	return( 0 );
}


/* 

  SERVER FUNCTIONS 

*/

/**********************************************************************

    Function    : test_rsa
    Description : test the rsa encrypt and decrypt
    Inputs      : 
    Outputs     : 0

***********************************************************************/

int test_rsa( EVP_PKEY *privkey, EVP_PKEY *pubkey )
{
	unsigned int len = 0;
	unsigned char *ciphertext;
	unsigned char *plaintext;
	unsigned char *ek;
	unsigned int ekl; 
	unsigned char *iv;
	unsigned int ivl;

	printf("*** Test RSA encrypt and decrypt. ***\n");

	len = rsa_encrypt( (unsigned char *)"help me, mr. wizard!", 20, &ciphertext, &ek, &ekl, &iv, &ivl, pubkey );

#if 1
	printf("Ciphertext is:\n");
	BIO_dump_fp (stdout, (const char *)ciphertext, len);
#endif

	len = rsa_decrypt( ciphertext, len, ek, ekl, iv, ivl, &plaintext, privkey );

	printf("Msg: %s\n", plaintext );
    
	return 0;
}


/**********************************************************************

    Function    : test_aes
    Description : test the aes encrypt and decrypt
    Inputs      : 
    Outputs     : 0

***********************************************************************/

int test_aes( )
{
	int rc = 0;
	unsigned char *key;
	unsigned char *ciphertext, *tag;
	unsigned char *plaintext;
	unsigned char *iv = (unsigned char *)"0123456789012345";
	int clen = 0, plen = 0;
	unsigned char msg[] = "Help me, Mr. Wizard!";
	unsigned int len = strlen((char*) msg);

	printf("*** Test AES encrypt and decrypt. ***\n");

	/* make key */
	key= (unsigned char *)malloc( KEYSIZE );
	rc = generate_pseudorandom_bytes( key, KEYSIZE );	
	assert( rc == 0 );

	/* perform encrypt */
	ciphertext = (unsigned char *)malloc( len );
	tag = (unsigned char *)malloc( TAGSIZE );
	clen = encrypt( msg, len, (unsigned char *)NULL, 0, key, iv, ciphertext, tag);
	assert(( clen > 0 ) && ( clen <= len ));

#if 1
	printf("Ciphertext is:\n");
	BIO_dump_fp (stdout, (const char *)ciphertext, clen);
	
	printf("Tag is:\n");
	BIO_dump_fp (stdout, (const char *)tag, TAGSIZE);
#endif

	/* perform decrypt */
	plaintext = (unsigned char *)malloc( clen+TAGSIZE );
	memset( plaintext, 0, clen+TAGSIZE ); 
	plen = decrypt( ciphertext, clen, (unsigned char *) NULL, 0, 
		       tag, key, iv, plaintext );
	assert( plen > 0 );

	/* Show the decrypted text */
#if 0
	printf("Decrypted text is: \n");
	BIO_dump_fp (stdout, (const char *)plaintext, (int)plen);
#endif
	
	printf("Msg: %s\n", plaintext );
    
	return 0;
}


/***********************************************************************/


/**********************************************************************

    Function    : server_protocol
    Description : server processing of crypto protocol
    Inputs      : sock - server socket
                  key - the key resulting from the protocol
    Outputs     : bytes read if successful, -1 if failure

***********************************************************************/
/*** YOUR CODE from Part 1 */
int server_protocol( int sock, char *pubfile, EVP_PKEY *privkey, unsigned char **enckey )
{
	/*
	* Couterparts of client actions that the server needs to take.
	*/
	/*
	* Wait for Message to server with header CLIENT_INIT_EXCHANGE
	*/
	printf("wait for init exchange\n");
	ProtoMessageHdr initExchange;
	wait_message(sock,&initExchange,NULL,CLIENT_INIT_EXCHANGE);
	/*
	* Send Message from server with header SERVER_INIT_RESPONSE
	*/
	ProtoMessageHdr initResponse;
	initResponse.msgtype=SERVER_INIT_RESPONSE;
	unsigned char* pubkeyc = malloc(MAX_BLOCK_SIZE);
	initResponse.length=buffer_from_file(pubfile,&pubkeyc);
	/* Extract server's public key */
	/* Make a function */
	printf("send init response with pub key\n");
	send_message(sock,&initResponse,(char *)pubkeyc);
	/*
	* Wait for message to server with header CLIENT_INIT_ACK
	*/

	fflush(stdout);
	ProtoMessageHdr initAck;
	char* symKeyBuffer=malloc(MAX_BLOCK_SIZE);
	unsigned char* symKey;
	printf("wait for sealed symmetric key\n");
	wait_message(sock,&initAck,symKeyBuffer,CLIENT_INIT_ACK);
	if (symKeyBuffer == NULL){
		errorMessage("init ack recieve error");
		return -1;
	}
	printBuffer("Sym Key Buffer",symKeyBuffer, initAck.length);
	fflush(stdout);
	printf("unseal symmetric key\n");
	unseal_symmetric_key(symKeyBuffer,initAck.length, privkey, &symKey);

	/*
	* Send message from server with header SERVER_INIT_ACK
	*/
	initAck.msgtype=SERVER_INIT_ACK;
	unsigned char* buffer=malloc(MAX_BLOCK_SIZE);
	unsigned int len;
	unsigned char message[] = "Complete";
	int messageLen = strlen((char *)message);
	printf("encrypt \"complete\"\n");
	encrypt_message(message, messageLen,symKey,buffer,&len);
	if (messageLen < 1){
		errorMessage("init ack encryption error");
		return -1;
	}
	initAck.length= len;

	char* bufferc = (char*) buffer;
	printf("send encrypted \"complete\"\n");
	send_message(sock, &initAck, bufferc);
	/*
	* Store the Symmetric key in session_key for later use. 
	*/
	printf("store sym key\n");
	*enckey = symKey;
	return 0;
}


/**********************************************************************

    Function    : receive_file
    Description : receive a file over the wire
    Inputs      : sock - the socket to receive the file over
                  key - the cicpher used to encrypt the traffic
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

#define FILE_PREFIX "./shared/"

int receive_file( int sock, unsigned char *key ) 
{
	/* Local variables */
	unsigned long totalBytes = 0;
	int done = 0, fh = 0;
	unsigned int outbytes;
	ProtoMessageHdr hdr;
	struct rm_cmd *r = NULL;
	char block[MAX_BLOCK_SIZE];
	unsigned char plaintext[MAX_BLOCK_SIZE];
	char *fname = NULL;
	int rc = 0;

	/* clear */
	bzero(block, MAX_BLOCK_SIZE);

	/* Receive the init message */
        printf("\nserver waiting for FILE_XFER_INIT\n");
	wait_message( sock, &hdr, block, FILE_XFER_INIT );

	/* set command structure */
	struct rm_cmd *tmp = (struct rm_cmd *)block;
	unsigned int len = tmp->len;
	r = (struct rm_cmd *)malloc( sizeof(struct rm_cmd) + len );
	r->cmd = tmp->cmd, r->type = tmp->type, r->len = len;
	memcpy( r->fname, tmp->fname, len );
        
	/* open file */
	if ( r->type == TYP_DATA_SHARED ) {
		unsigned int size = r->len + strlen(FILE_PREFIX) + 1;
		fname = (char *)malloc( size );
		snprintf( fname, size, "%s%.*s", FILE_PREFIX, (int) r->len, r->fname );
                printf("fname: %s", fname);
		if ( (fh=open( fname, O_WRONLY|O_CREAT|O_TRUNC, 0700)) > 0 );
		else assert( 0 );
	}
	else assert( 0 );

	/* read the file data, if it's a create */ 
	if ( r->cmd == CMD_CREATE ) {
		/* Repeat until the file is transferred */
		printf( "Receiving file [%s] ..\n", fname );
		while (!done)
		{
			/* Wait message, then check length */
			get_message( sock, &hdr, block );
			if ( hdr.msgtype == EXIT ) {
				done = 1;
				break;
			}
			else
			{
				/* Write the data file information */
				rc = decrypt_message( (unsigned char *)block, hdr.length, key, 
						      plaintext, &outbytes );
				assert( rc  == 0 );
				write( fh, plaintext, outbytes );

#if 1
				printf("Decrypted Block is:\n");
				BIO_dump_fp (stdout, (const char *)plaintext, outbytes);
#endif

				totalBytes += outbytes;
				printf( "Received/written %ld bytes ...\n", totalBytes );
			}
		}
		printf( "Total bytes [%ld].\n", totalBytes );
		/* Clean up the file, return successfully */
		close( fh );
	}
	else {
		printf( "Server: illegal command %d\n", r->cmd );
		//	     exit( -1 );
	}

	/* Server ack */
	hdr.msgtype = EXIT;
	hdr.length = 0;
	send_message( sock, &hdr, NULL );

	return( 0 );
}

/**********************************************************************

    Function    : server_secure_transfer
    Description : this is the main function to execute the protocol
    Inputs      : pubkey - public key of the server
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

int server_secure_transfer( char *privfile, char *pubfile, char *real_address )
{
	/* Local variables */
	int server, errored, newsock;
	RSA *rsa_privkey = NULL, *rsa_pubkey = NULL;
	RSA *pRSA = NULL;
	EVP_PKEY *privkey = EVP_PKEY_new(), *pubkey = EVP_PKEY_new();
	fd_set readfds;
	unsigned char *key;
	FILE *fptr;
	// new args
	struct rm_cmd *r = NULL;
	int err;

	/* initialize */
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	ERR_load_crypto_strings();

	/* Connect the server/setup */
	server = server_connect();
	errored = 0;

	/* open private key file */
	fptr = fopen( privfile, "r" );
	assert( fptr != NULL);
	if (!(pRSA = PEM_read_RSAPrivateKey( fptr, &rsa_privkey, NULL, NULL)))
	{
		fprintf(stderr, "Error loading RSA Private Key File.\n");

		return 2;
	}

	if (!EVP_PKEY_assign_RSA(privkey, rsa_privkey))
	{
		fprintf(stderr, "EVP_PKEY_assign_RSA: failed.\n");
		return 3;
	}
	fclose( fptr ); 

	/* open public key file */
	fptr = fopen( pubfile, "r" );
	assert( fptr != NULL);
	if (!PEM_read_RSAPublicKey( fptr , &rsa_pubkey, NULL, NULL))
	{
		fprintf(stderr, "Error loading RSA Public Key File.\n");
		return 2;
	}

	if (!EVP_PKEY_assign_RSA( pubkey, rsa_pubkey))
	{
		fprintf(stderr, "EVP_PKEY_assign_RSA: failed.\n");
		return 3;
	}
	fclose( fptr );

	// Test the RSA encryption and symmetric key encryption
	//test_rsa( privkey, pubkey );
	//test_aes();

	/* Repeat until the socket is closed */
	while ( !errored )
	{
		FD_ZERO( &readfds );
		FD_SET( server, &readfds );
		if ( select(server+1, &readfds, NULL, NULL, NULL) < 1 )
		{
			/* Complain, explain, and exit */
			char msg[128];
			sprintf( msg, "failure selecting server connection [%.64s]\n",
				 strerror(errno) );
			errorMessage( msg );
			errored = 1;
		}
		else
		{
			/* Accept the connect, receive the file, and return */
			if ( (newsock = server_accept(server)) != -1 )
			{
				/* Do the protocol, receive file, shutdown */
				server_protocol( newsock, pubfile, privkey, &key );
				receive_file( newsock, key );
				close( newsock );
				/*** Start: YOUR CODE - for server spoofing ***/
				//
				//
				//
				//
				//
				//
				//
				//
				//
				/*** End: YOUR CODE - for server spoofing ***/        
			}
			else
			{
				/* Complain, explain, and exit */
				char msg[128];
				sprintf( msg, "failure accepting connection [%.64s]\n", 
					 strerror(errno) );
				errorMessage( msg );
				errored = 1;
			}
		}
	}

	/* Return successfully */
	return( 0 );
}
