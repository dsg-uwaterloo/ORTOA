# ORTOA

ORTOA - a One Round Trip Oblivious Access protocol that reads or writes data stored on remote storage *in one round without revealing the type of access*.

## Background

Encrypted databases (e.g., CryptDB) typically consist of a trusted front-end that stores the encryption key and routes all client requests to the untrusted storage. A simple encrypted key-value store design (supporting single object GET/PUT requests) serves client requests as follows: 
- For read requests, the front-end reads the appropriate encrypted value from the storage, decrypts it, and responds to the client
- For write requests, the front-end encrypts the value updated by the client and writes the encrypted value to the storage

This common approach of reading and writing encrypted data allows an adversary controlling the cloud to distinguish between read and write requests since only write requests update the data-base. Revealing the type of access – read vs. write – can violate an end user’s or an application’s privacy.

A straightforward approach to address this privacy challenge is to hide the type of operation by always reading an object followed by writing it, irrespective
of the type of client request. This sequential two round solution provides two major downsides
1. Doubles the end-to-end latency for *each* user access compared to plaintext datastores
2. With increasing privacy laws such as GDPR that prohibit data movement across continents, two rounds of cross-continent communication for each request becomes too expensive.

## ORTOA Protocols

This work proposes *ORTOA*, a family of one round trip data access protocols that *hides the type of client access to efficiently address the privacy challenges caused by revealing the type of access.* 

Specifically, we propose three different single-round access type hiding protocols:
- ORTOA-FHE: Leverages an existing crypto-graphic primitive, Fully Homomorphic Encryption (FHE)
- ORTOA-LBL: Represents plaintext values in a binary format and encodes each bit with a secret label generated using pseudo-random functions (PRFs)
- ORTOA-TEE: Leverages the cryptographic guarantees of Trusted Execution Environments (TEE) (hardware enclaves)
