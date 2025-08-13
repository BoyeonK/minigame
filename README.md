### Third-Party Libraries and Tools

This project uses the following third-party libraries and tools:

- **Unity Engine**
  License: Unity Terms of Service
  Copyright © Unity Technologies
  
  > Serves as the **primary real-time development platform** for creating the game client. It manages rendering, physics, input, and overall application logic on the client-side.

- **Google Protocol Buffers**
  License: BSD 3-Clause License
  Copyright © Google
  
  > Used for efficient, language-neutral, platform-neutral, extensible **serialization and deserialization** of structured data between the client and servers.

- **gRPC**
  License: Apache License 2.0
  Copyright © Google
  
  > A high-performance, open-source **universal RPC framework** used for inter-service communication between the game logic server and the database server.

- **OpenSSL**
  License: Apache License 2.0
  Copyright © The OpenSSL Project
  
  > Utilized for robust cryptographic operations. Its roles include:
  > 
  > 1. Generating **RSA key pairs (PKCS#8 / DER format)** for secure handshake initialization.
  > 2. Performing secure password management by **hashing user passwords with a salt** (using PBKDF2-HMAC-SHA256) for secure storage in the database.

- **BouncyCastle C#**
  License: MIT-style License
  Copyright © BouncyCastle
  
  > Provides **cryptographic APIs** for various security operations within the C# components of the project, complementing the features of other security libraries.

- **Microsoft SQL Server ODBC Driver**
  License: Proprietary License (Microsoft)
  
  > Employed in the **C++ backend** for establishing connections, executing queries, and interacting with Microsoft SQL Server databases.

- **Microsoft SQL Server Express**
  License: Proprietary License (Microsoft)
  
  > Serves as a **lightweight, free database solution** for development and small-scale deployments, maintaining full compatibility with the complete SQL Server system.
