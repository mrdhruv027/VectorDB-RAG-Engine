# 🎙️ SDE Interview Walkthrough Script: Custom VectorDB & RAG
> **A Spoken Guide for Dhruv Sharma's Technical Interview**

---

### Step 1: The Inspiration (The "Why")
> "I noticed that most developers building AI applications use black-box APIs like OpenAI and pre-packaged cloud databases. I wanted to understand what was happening under the hood. So, I decided to build a high-performance Vector Database in C++17 from scratch to learn how vector indexing, graph traversal, and semantic search actually operate."

---

### Step 2: The Backend Architecture (Data Structures & Systems)
> "On the backend, I implemented three indexing structures to compare performance:
> *   A **Brute Force** index as an exact search baseline ($O(N)$).
> *   A **KD-Tree** for fast spatial partitioning in lower dimensions ($O(\log N)$).
> *   An **HNSW Graph** for high-dimensional approximate nearest neighbor search ($O(\log N)$).
>
> I used **16-dimensional vectors** for the local visualizer demo, which represents 4 distinct keyword categories. For real documents, I used **768-dimensional vectors**. 
>
> Since the database runs as a local REST API using an HTTP library, I ensured thread safety by wrapping database modifications in a `std::mutex` with `std::lock_guard` to prevent race conditions. I also wrote a serialization system to auto-save and auto-load the database state to JSON files on disk."

---

### Step 3: The AI Pipeline (RAG Flow)
> "For the document chat feature, I implemented a full RAG pipeline:
> *   When a document is uploaded, the backend splits it into overlapping chunks of 250 words.
> *   Each chunk is sent to local Ollama running `nomic-embed-text` to generate a 768-dimensional vector embedding.
> *   When a user asks a question, we embed the query, run an HNSW search to retrieve the top 3 most relevant paragraphs, paste those paragraphs into a prompt template, and send it to Llama 3.2 to generate a factual answer."

---

### Step 4: The Frontend Visualization
> "To demonstrate this visually, I built a web dashboard in HTML5 and Vanilla JavaScript. The frontend runs **PCA (Principal Component Analysis)** on the fly to project the vectors down to 2D coordinates so they can be plotted. 
>
> The coolest feature is that users can click on the different HNSW graph layers (like L0 or L1) on the sidebar, and the UI will draw the graph edges connecting the vectors directly on the canvas, showing how HNSW navigates the vector space."

---

### Step 5: Verification & Compilation
> "To guarantee everything worked correctly, I wrote a test suite in C++ verifying distance metrics, KD-tree correctness, and HNSW node deletions. The server compiles cleanly using `g++` with the Windows 10 target API macro `-D_WIN32_WINNT=0x0A00` to handle raw socket mapping safely."

---

## 💡 Technical Delivery Tips
1.  **Use the Whiteboard**: If a whiteboard is available, draw the 3 HNSW Layers (Layer 2 highways, Layer 1 state roads, Layer 0 local streets) when you reach **Step 2**.
2.  **Emphasize Systems Terms**: Put verbal emphasis on: **Thread Safety**, **Mutex**, **$O(\log N)$ Complexity**, and **Memory Serialization**.
3.  **Honest Focus**: If asked about the frontend, clearly state: *"I focused 100% of my development on the C++ systems backend; the frontend is a simple HTML template to visualize the results."*
