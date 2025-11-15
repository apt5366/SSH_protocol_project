SSH Protocol Implementation & MITM Attack

A Secure File Transfer System using OpenSSL in C

📌 Overview

This project implements a simplified SSH-like protocol in C using the OpenSSL library. It supports secure file transfer between a client and server through:

Public-key cryptography (RSA)

Symmetric-key encryption (AES)

Integrity-protected communication

The project is built in two main parts:

Part 1 — Implementing the SSH protocol & secure file transfer

Part 2 — Implementing a server-spoofing Man-in-the-Middle (MITM) attack

🚀 Features
🔐 Part 1 — Secure SSH Protocol

Custom implementation of the initial SSH key-exchange steps

RSA used for sealing/unsealing the symmetric session key

AES used for encrypting file contents

Generation of cryptographically secure pseudo-random values

Fully encrypted file transfer from client to server

Server stores received files under a shared/ directory

🛑 Part 2 — MITM (Server Spoofing) Attack

A malicious proxy poses as the real server

MITM completes the SSH handshake with the client

MITM initiates a second SSH session with the real server

Client messages are intercepted and replayed

MITM replaces the client’s intended file with a malicious bad.txt

Demonstrates a classic server-spoofing vulnerability when server keys aren’t authenticated

