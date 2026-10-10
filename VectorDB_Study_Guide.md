# 🎓 VectorDB & RAG: Complete Interview Study Guide
> **A Beginner-Friendly Guide to Vector Search, HNSW, KD-Trees, and RAG in C++**
> *Prepared for Dhruv Sharma*

---

# 1️⃣ Phase 1: Beginner's Glossary

### 📍 Vectors & Dimensions
*   **What it is (Analogy)**: Imagine finding a point on a flat sheet of paper. You need two coordinates: **X** (left/right) and **Y** (up/down). That is **2-Dimensional (2D)** space. If you want to locate a drone flying in a room, you need three numbers: **X**, **Y**, and **Z** (altitude). That is **3-Dimensional (3D)** space. 
    A **Vector** is simply that list of numbers. **Dimensions** are the count of numbers in that list. We can have 16 dimensions (16 numbers) or even 768 dimensions (768 numbers). While we cannot draw or visualize 768D space, the math works exactly the same!
*   **How it is used in this project**: 
    *   Our **Demo Vectors** are **16-Dimensional (16D)**. Each group of 4 numbers corresponds to a category: Computer Science, Mathematics, Food, and Sports.
    *   Our **Document Embeddings** are **768-Dimensional (768D)** because the local AI embedding model represents text chunks using 768 coordinates to capture fine details of language.

---

### 🔠 Vector Embeddings
*   **What it is (Analogy)**: Computers do not understand words, sentences, or concepts; they only understand numbers. An **Embedding** is a translator. It takes a piece of text (like *"Neapolitan Pizza"*) and converts it into a list of numbers (a vector). 
    Think of it like a **semantic recipe card**. If two recipes share similar ingredients, their ingredient lists are similar. Similarly, if two sentences share similar meanings, their embedding vectors will sit close together in space.
*   **How it is used in this project**: When you insert notes or documents, the C++ code calls the local Ollama client (`src/ollama.hpp`). Ollama reads the text and returns a 768D list of numbers representing the meaning of that text, which we then store in the database.

---

### 📏 Distance & Similarity Metrics
*   **What it is (Analogy)**: If you want to know how similar two vectors are, you calculate the "distance" between their points. We use three calculations:
    1.  **Euclidean Distance ($L_2$)**: The "as the crow flies" straight line between two points. (Analogy: measuring the distance between two houses with a ruler on a map).
    2.  **Manhattan Distance ($L_1$)**: The distance you must travel if you are driving on a grid of city blocks. (Analogy: going 3 blocks North, then 4 blocks East).
    3.  **Cosine Similarity**: Measures the *angle* between two arrows pointing from the center. If they point in the exact same direction, the cosine distance is 0. If they are orthogonal (90 degrees), it is 1. (Analogy: comparing the directions of two wind vanes).
*   **How it is used in this project**: Implemented in `src/distance.hpp`. We use Cosine Distance for document matching because it ignores text length and focuses purely on the *direction* of the topic.

---

### 🔍 Brute Force Search
*   **What it is (Analogy)**: Imagine you have a deck of cards and you want to find the Ace of Spades. **Brute Force** means looking at every single card in the deck one-by-one from top to bottom. It is guaranteed to find the card, but if the deck has 1 million cards, it takes a very long time.
*   **How it is used in this project**: Implemented in `src/bruteforce.hpp`. It is our exact search baseline. We compare our faster algorithms against it to make sure they are accurate.

---

### 🌳 KD-Tree (K-Dimensional Tree)
*   **What it is (Analogy)**: Think of the game "20 Questions." Instead of guessing random names, you ask: *"Is the person male or female?"*, splitting the group in half. Then you ask: *"Are they alive or dead?"*, splitting it in half again.
    A **KD-Tree** is a binary search tree that organizes coordinate points. It splits the space in half along one dimension (e.g., left/right), then splits the sub-spaces along the next dimension (e.g., up/down). This allows you to skip looking at entire regions of space during a search.
*   **How it is used in this project**: Implemented in `src/kdtree.hpp`. It is used to index and search our 16D demo vectors.

---

### 🌐 HNSW (Hierarchical Navigable Small World)
*   **What it is (Analogy)**: Imagine a map of the world. If you want to travel from a small town in India to a small town in the USA:
    *   **Layer 2 (Express Highway)**: You take an international flight from Delhi to New York (huge distance, very few airports).
    *   **Layer 1 (State Road)**: You take a highway from New York to a local city in New York state.
    *   **Layer 0 (Local Streets)**: You drive on local neighborhood streets to find the exact house.
    
    **HNSW** is a multi-layered graph index. The top layers have sparse, long-range connections. The bottom layer has all the points with short-range connections. It allows you to zoom in on the correct neighborhood instantly before searching locally.
*   **How it is used in this project**: Implemented in `src/hnsw.hpp`. It is the main engine used to index and query our 768D document database chunks.

---

### 🤖 RAG (Retrieval-Augmented Generation)
*   **What it is (Analogy)**: Think of a student taking an open-book exam. 
    *   **Without RAG**: The student tries to answer from memory (the LLM's training data). They might make things up (hallucination).
    *   **With RAG**: The student first looks up the question in the index, finds the exact 3 pages in the textbook (**Retrieval**), reads those pages (**Augmentation**), and uses that information to write a perfect answer (**Generation**).
*   **How it is used in this project**: In the **Ask AI** tab, when you ask a question, the HNSW graph searches your documents, finds the top 3 most relevant paragraphs, pastes them into the prompt as context, and sends them to Llama 3.2 to answer.

---

### 📊 PCA (Principal Component Analysis)
*   **What it is (Analogy)**: Imagine holding a 3D toy in front of a flashlight. The shadow cast on the wall is a 2D projection. Even though the shadow is flat, you can still see the general shape of the toy.
    **PCA** is the mathematical equivalent. It flattens 16-dimensional vectors down to 2 dimensions so we can plot them as coordinates $(X, Y)$ on a flat computer screen.
*   **How it is used in this project**: The JavaScript code in `index.html` runs PCA on all database vectors to plot them on the interactive canvas.

---

### 🔒 Thread Safety & Mutex
*   **What it is (Analogy)**: Imagine a single-person public restroom. If there is no lock, two people might walk in at the same time, causing chaos. A **Mutex** (Mutual Exclusion) is the lock on the door. Only one person can have the key. When they are inside, everyone else must wait outside.
*   **How it is used in this project**: Web servers handle multiple requests at the same time. If one request is writing a vector to the HNSW graph while another is reading it, the server can crash. We use `std::mutex` and `std::lock_guard` in `src/db.hpp` to lock the database during updates.

---

# 2️⃣ Phase 2: System Walkthrough

Here is exactly what happens behind the scenes during the two main actions:

### Flow 1: Inserting a New Document
1. **User types text**: The user enters a document title and text in Chrome, then hits "EMBED & INSERT".
2. **Chunking**: The server divides the text into overlapping blocks of words (250 words each).
3. **Embedding**: For each chunk, the C++ code calls Ollama's local `nomic-embed-text` API.
4. **Vector Retrieval**: Ollama outputs a 768-dimensional vector embedding for that chunk.
5. **Graph Insertion**: The database locks the state (mutex) and inserts the vector and text chunk into the HNSW graph network.
6. **Disk Save**: The database appends the node details to `data/documents.json` so it is persistently stored.

---

### Flow 2: Asking a Question (RAG)
1. **Question Input**: The user types a question in the "Ask AI" tab and hits "ASK AI".
2. **Embedding**: The C++ server calls Ollama to convert the question text into a 768D vector embedding.
3. **Vector Search**: The HNSW index searches the graph layers and retrieves the top 3 most relevant document chunks.
4. **Context Building**: The C++ server extracts the raw text of those 3 chunks and packages them into a text block.
5. **Prompt construction**: C++ builds a prompt instructing the LLM to answer the question using the retrieved text.
6. **AI Response Generation**: The server sends the prompt to local Llama 3.2 via Ollama, which generates the final answer.
7. **Typewriter Effect**: The answer is streamed back and types out letter-by-letter in Chrome.

---

# 3️⃣ Phase 3: Algorithm Speed Comparison

Why do we have three different search algorithms in this project? This demonstrates to an interviewer that you understand execution complexity tradeoffs:

### Why HNSW wins in high dimensions:
1.  **Brute Force ($O(N \cdot d)$)** gets linearly slower as the database grows. If you have 10 million vectors, it is too slow for production.
2.  **KD-Tree ($O(\log N)$)** works by drawing boundaries. In 2D or 3D, this is fast. However, in **768 dimensions**, every point is practically a boundary point. The KD-Tree ends up having to check almost every branch, meaning it degrades to the speed of Brute Force.
3.  **HNSW ($O(\log N)$)** does not use geometric boundaries. It builds a graph where related points are linked together. By descending through sparse highway layers to dense neighborhood layers, it finds the closest matches in logarithmic time without degradation in high-dimensional spaces.

---

# 4️⃣ Phase 4: Beginner-Friendly Interview Questions & Answers

#### Q1: What is the main goal of your project?
> **Answer**: The goal is to build a fully working vector database from scratch in C++ to demonstrate how production databases like Pinecone or Chroma work. It implements exact search (Brute Force, KD-Tree) alongside approximate search (HNSW) and showcases a local RAG pipeline to chat with private files using Ollama.

#### Q2: How did you modularize the C++ code?
> **Answer**: I restructured the monolithic code into clean, header-only files in the `src/` folder. We have `distance.hpp` for distance calculations, `bruteforce.hpp`, `kdtree.hpp`, and `hnsw.hpp` for the search indices, `db.hpp` for database storage, `ollama.hpp` for API interactions, and a clean `main.cpp` for the server routes. This keeps compilation simple while maintaining clean software design.

#### Q3: What is the purpose of the `std::mutex` in your database?
> **Answer**: Our server handles concurrent HTTP API requests (like search and insertion requests happening at the same time). If one thread modifies the HNSW graph memory while another thread is reading from it, the application will crash. We use `std::mutex` and `std::lock_guard` to lock access so only one thread can modify or read the indexes at a time, ensuring thread safety.

#### Q4: How is database persistence handled?
> **Answer**: When vectors or documents are inserted or deleted, their data is serialized to JSON files (`demo_vectors.json` and `documents.json`). On startup, the database checks if these files exist. If they do, it reads the data and rebuilds the HNSW graph and KD-Tree indexes. This means your data is saved permanently even if the server is restarted.

#### Q5: What is RAG, and how did you implement it?
> **Answer**: RAG stands for Retrieval-Augmented Generation. In my project, when a user asks a question, we convert the question into a 768D vector, search the HNSW index to retrieve the top 3 most similar text chunks, inject those chunks as context into a prompt, and ask local Llama 3.2 via Ollama to generate an answer based on that context.

#### Q6: Why did you choose Cosine Distance, and how does it differ from Euclidean Distance?
> **Answer**: Cosine distance measures the angle between vectors (the direction of their topics), whereas Euclidean distance measures the straight-line physical distance. For text search, document length can vary. If two documents discuss the same topic but one is much longer, Euclidean distance will classify them as far apart, but Cosine distance will correctly classify them as very close.

#### Q7: What are the HNSW graph layers, and how does search work?
> **Answer**: HNSW works like a multi-level highway system. The top layer has very few nodes and long-range connections. The search starts at the top, hops quickly across long distances to get close to the query coordinate, drops down a layer, and repeats the process until it reaches Layer 0, where it runs a localized search to find the exact nearest neighbors.

#### Q8: What does the frontend PCA visualizer show?
> **Answer**: Since human eyes cannot see 16-dimensional or 768-dimensional space, the frontend uses Principal Component Analysis (PCA) to project vectors down to 2D coordinates. This allows us to plot them on a canvas, showing visually how semantically similar items (like computer science topics vs sports topics) naturally cluster together.

#### Q9: What happens if you delete a node from the HNSW graph?
> **Answer**: When a node is deleted, we must remove its ID from the neighbor connection lists of all other nodes on all layers. Additionally, if the deleted node was the global entry point of the graph, we must pick a new entry point from the remaining nodes and reset the graph's `topLayer` to the maximum layer of the new entry point.

#### Q10: Why did you choose a local LLM over a cloud LLM (like OpenAI)?
> **Answer**: Using a local LLM via Ollama ensures complete data privacy (no data leaves the local machine), works offline, has no API subscription costs, and is ideal for secure developer environments.

#### Q11: What is the purpose of text chunking with overlap?
> **Answer**: Large documents exceed the token limits of embedding models. We split them into smaller chunks (e.g., 250 words) so they can be embedded. We use a **30-word overlap** between adjacent chunks to ensure that text context at the boundary of a cut is preserved, preventing search terms from getting split across chunks.

#### Q12: How does the HNSW graph visualizer work on the screen?
> **Answer**: When you click a layer (L0, L1, etc.) under the HNSW panel, the web page queries the `/hnsw-info` endpoint to get the list of active links (edges) on that layer. The JavaScript matches the node IDs of the edges to their 2D PCA positions and draws line segments connecting them directly on the canvas.
