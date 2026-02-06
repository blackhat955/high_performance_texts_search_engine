To align the codebase with the high-performance claims provided, we need to completely re-architect the C++ addon. The current implementation performs a brute-force scan of the file on every request, which is inefficient and blocking.

Here is the plan to transform the project:

### **Phase 1: High-Performance Data Structures (The "Trie & HashMap" Claim)**
*   **Goal**: Reduce search time and memory usage.
*   **Action**:
    *   Replace the file scanning logic with an **Inverted Index** (`std::unordered_map`) for O(1) exact lookups.
    *   Implement a **Trie (Prefix Tree)** to handle fuzzy searching efficiently without checking every word in the document.
    *   **In-Memory Caching**: Create a C++ Class (`SearchEngine`) that loads the file **once** at startup and keeps the index in memory. This eliminates the "redundant API calls" (disk reads) mentioned in the claims.

### **Phase 2: Concurrency & Thread Safety (The "Multi-threaded & Shared Ptr" Claim)**
*   **Goal**: Non-blocking execution and thread safety.
*   **Action**:
    *   **AsyncWorker**: Implement `Napi::AsyncWorker` to offload search tasks to the libuv thread pool. This prevents the "blocking" behavior that currently freezes the Node.js server.
    *   **Shared Memory**: Use `std::shared_ptr` to manage the lifecycle of the index, allowing multiple search threads to read from the same memory safely.
    *   **Thread Safety**: Ensure the `SearchEngine` class is thread-safe using appropriate locking strategies (though the read-only index requires minimal locking).

### **Phase 3: Automation & Tooling (The "Automated Workflows" Claim)**
*   **Goal**: Streamline build and debugging.
*   **Action**:
    *   Create a `scripts/build.sh` to automate the build/rebuild process.
    *   Update `binding.gyp` to support **AddressSanitizer** (ASan) for memory auditing on macOS.
    *   Add a debug configuration to trace potential memory issues.

### **Phase 4: Integration**
*   **Goal**: Connect the new engine to the Node.js server.
*   **Action**:
    *   Refactor `search.cpp` to export a Class (`new SearchEngine("sample.txt")`) instead of a simple function.
    *   Update `server.js` to initialize the engine once and call the new asynchronous `search()` method.

**Proposed File Structure Changes:**
*   `src/SearchEngine.h` / `.cpp`: The core logic class.
*   `src/addon.cpp`: The Node-API binding code.
*   `scripts/build.sh`: Automation script.
