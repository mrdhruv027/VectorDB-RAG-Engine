# VectorDB — High-Performance Vector Database & RAG Engine in C++17

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Docker](https://img.shields.io/badge/Docker-Ready-2496ED.svg)](https://www.docker.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)]()

A production-grade, educational **Vector Database and Retrieval-Augmented Generation (RAG) Engine** written from scratch in C++17 with zero heavy external dependencies.

Implements **HNSW (Hierarchical Navigable Small World)**, **KD-Tree**, and **Brute Force** vector indexing side-by-side, complete with an interactive Web UI (2D PCA semantic scatter plot, benchmarking suite, and RAG document query terminal) powered by local LLMs via Ollama.

---

## Live Demo & Deployment

- **Live Public Application**: `https://vectordb-rag-engine.onrender.com` *(or your custom deployment URL)*
- **GitHub Repository**: [https://github.com/mrdhruv027/VectorDB-RAG-Engine](https://github.com/mrdhruv027/VectorDB-RAG-Engine)

---

## System Architecture

```
                       ┌───────────────────────────────┐
                       │     Browser Client (Web UI)   │
                       │   - 2D PCA Vector Visualizer  │
                       │   - Real-time Benchmark Lab   │
                       │   - RAG Document Explorer     │
                       └──────────────┬────────────────┘
                                      │ HTTP REST API (port 8080 or $PORT)
                                      ▼
                       ┌───────────────────────────────┐
                       │     C++17 HTTP Server         │
                       │     (httplib.h embedded)      │
                       └──────┬───────────────┬────────┘
                              │               │
            ┌─────────────────┴─┐           ┌─┴─────────────────┐
            │   Vector Engine   │           │   RAG Pipeline    │
            │   - BruteForce    │           │   - Text Chunking │
            │   - KD-Tree       │           │   - Ollama Client │
            │   - HNSW Graph    │           │   - Context Rank  │
            └─────────┬─────────┘           └─┬─────────────────┘
                      │                       │
                      ▼                       ▼
            ┌───────────────────┐   ┌───────────────────────────┐
            │ JSON Persistence  │   │   Ollama Local / Remote   │
            │ ./data/*.json     │   │   - nomic-embed-text (768D│
            └───────────────────┘   │   - llama3.2 (Generator)  │
                                    └───────────────────────────┘
```

---

## Features

| Feature | Description |
|---|---|
| **3 Search Algorithms** | **HNSW** (O(log N) approximate), **KD-Tree** (exact space partitioning), and **Brute Force** (exact baseline) |
| **3 Distance Metrics** | Cosine Similarity, Euclidean Distance ($L_2$), Manhattan Distance ($L_1$) |
| **Interactive 2D PCA** | Dimensionality reduction visualizer projecting 16D semantic vectors into 2D space in real-time |
| **Micro-Benchmark Suite** | Side-by-side latency comparison between HNSW, KD-Tree, and Brute Force |
| **Full RAG Pipeline** | Overlapping text chunker (250 words / 30 word overlap), vector embedding, context retrieval, and LLM response generation |
| **Persistence** | Lightweight JSON-based vector & document persistence in `./data/` |
| **Production Ready** | Multi-stage Docker containerization, dynamic `$PORT` routing, CORS handling, and `/health` monitoring |

---

## Project Structure

```
VectorDB-RAG-Engine/
├── Dockerfile                  # Multi-stage production container build
├── .dockerignore               # Docker build exclusions
├── Makefile                    # Linux / macOS / Docker build rules
├── compile.bat                 # Windows build script with compiler autodetection
├── render.yaml                 # Render Infrastructure-as-Code deployment blueprint
├── .env.example                # Environment configuration template
├── .gitignore                  # Git tracking exclusions
├── LICENSE                     # MIT License & attribution
├── httplib.h                   # Single-header cross-platform HTTP library
├── index.html                  # Embedded Single-Page Application (SPA) frontend
├── main.cpp                    # Application entry point, HTTP routing, and server loop
├── README.md                   # Project documentation
├── VectorDB_Study_Guide.md     # Deep dive into algorithms and complexity
├── SDE_Interview_Script_Walkthrough.md # Step-by-step interview presentation guide
├── data/
│   └── demo_vectors.json       # Pre-loaded 16D semantic vectors (CS, Math, Food, Sports)
└── src/
    ├── bruteforce.hpp          # Exact linear O(N*d) k-NN search
    ├── db.hpp                  # VectorDB and DocumentDB storage engines with mutex locks
    ├── distance.hpp            # Cosine, Euclidean, and Manhattan distance functions
    ├── hnsw.hpp                # Hierarchical Navigable Small World graph implementation
    ├── json_utils.hpp          # Fast zero-dependency JSON stringifiers & parsers
    ├── kdtree.hpp              # K-Dimensional Tree implementation with pruning
    ├── ollama.hpp              # HTTP client for Ollama embeddings and generation
    ├── tests.cpp               # Automated unit test suite
    └── types.hpp               # Core data structures (VectorItem, DistFn)
```

---

## Local Setup & Installation

### Option 1: Native Windows Setup

1. **Clone the repository**:
   ```powershell
   git clone https://github.com/mrdhruv027/VectorDB-RAG-Engine.git
   cd VectorDB-RAG-Engine
   ```

2. **Compile the server and test suite**:
   ```powershell
   .\compile.bat
   ```
   *The script autodetects installed modern C++17 GCC compilers.*

3. **Run the unit tests**:
   ```powershell
   .\run_tests.exe
   ```

4. **(Optional) Start Ollama for AI RAG features**:
   ```powershell
   ollama pull nomic-embed-text
   ollama pull llama3.2
   ollama serve
   ```

5. **Start the VectorDB server**:
   ```powershell
   .\db.exe
   ```

6. Open your browser at:
   ```
   http://localhost:8080
   ```

---

### Option 2: Linux / macOS Setup

1. **Clone & build**:
   ```bash
   git clone https://github.com/mrdhruv027/VectorDB-RAG-Engine.git
   cd VectorDB-RAG-Engine
   make
   ```

2. **Run tests**:
   ```bash
   make test
   ```

3. **Launch server**:
   ```bash
   ./db
   ```

---

### Option 3: Docker Setup (Cross-Platform)

1. **Build the Docker container**:
   ```bash
   docker build -t vectordb-engine .
   ```

2. **Run the container**:
   ```bash
   docker run -p 8080:8080 vectordb-engine
   ```

---

## Environment Variables

| Variable | Default | Description |
|---|---|---|
| `PORT` | `8080` | Port on which the HTTP server listens (assigned by cloud host) |
| `OLLAMA_HOST` | `127.0.0.1` | IP address or hostname of Ollama service |
| `OLLAMA_PORT` | `11434` | Port number of Ollama service |
| `EMBED_MODEL` | `nomic-embed-text` | Ollama model used for text embeddings |
| `GEN_MODEL` | `llama3.2` | Ollama model used for RAG response generation |

---

## REST API Reference

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/health` | Healthcheck endpoint (`{"status":"ok"}`) |
| `GET` | `/status` | Server status and Ollama availability |
| `GET` | `/stats` | Vector count, dimensions, algorithms, and metrics |
| `GET` | `/items` | List all stored demo vectors |
| `GET` | `/search?v=...&k=5&metric=cosine&algo=hnsw` | Perform k-NN similarity search |
| `POST` | `/insert` | Insert vector (`{"metadata":"...","category":"...","embedding":[...]}`) |
| `DELETE` | `/delete/:id` | Remove vector by numeric ID |
| `GET` | `/benchmark?v=...&k=5&metric=cosine` | Execute side-by-side performance benchmark |
| `GET` | `/hnsw-info` | Inspect HNSW multilayer graph structure and connectivity |
| `POST` | `/doc/insert` | Embed document text via Ollama into 768D vector |
| `GET` | `/doc/list` | List all ingested documents |
| `DELETE` | `/doc/delete/:id` | Delete document chunk |
| `POST` | `/doc/ask` | Retrieve relevant chunks and generate LLM answer |

---

## Technical Deep-Dive (Interview Discussion)

### 1. HNSW vs. KD-Tree vs. Brute Force
- **Brute Force ($O(N \cdot d)$)**: Exhaustively computes distance to every stored vector. Perfect recall, but scales linearly and fails at massive scale.
- **KD-Tree ($O(\log N)$ in low dimensions)**: Recursively splits vector space along alternating dimensions using median hyperplanes. However, it suffers severely from the **Curse of Dimensionality** ($d > 20$), where distance distributions concentrate and almost all bounding boxes intersect the query sphere, degrading to linear scan.
- **HNSW ($O(\log N)$ approximate nearest neighbor)**: Constructs a multilayer graph inspired by Skip Lists. Upper layers have sparse, long-distance highway links to navigate large distances across vector space quickly; layer 0 has dense, local connections for fine-grained nearest neighbor refinement.

### 2. Concurrency & Thread Safety
- The database engine (`VectorDB` and `DocumentDB` classes) protects index updates and queries with `std::mutex` and `std::lock_guard`, enabling safe concurrent query processing across multiple worker threads in `httplib::Server`.

---

## Author & License

- Developed and maintained by [Dhruv Sharma](https://github.com/mrdhruv027)
- Licensed under the [MIT License](LICENSE).
