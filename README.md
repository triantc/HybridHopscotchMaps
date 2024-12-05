![run-tests](../../workflows/run-tests/badge.svg)

# 🚀 **UsingHopscotchHash & UsingHybridHash**

This repository contains the implementation of two advanced data structures (ADTs): **Hopscotch Hashing** and **Hybrid Hashing**. These data structures provide efficient solutions for managing hash tables with optimal performance in terms of lookup, insertion, and deletion operations.

---

## **Overview**

### **1. Hopscotch Hashing**
Hopscotch Hashing is a modern hashing technique designed to maintain high performance for dynamic hash tables. It improves upon traditional open addressing by keeping data clusters close to their initial hash bucket, reducing cache misses and enhancing performance.

**Features:**
- Near O(1) average lookup time.
- Supports dynamic resizing.
- Efficient in handling collisions using local neighborhood searching.

---

### **2. Hybrid Hashing**
Hybrid Hashing is a combination of multiple hashing strategies to improve flexibility and performance for specific workloads. This approach allows the hash table to adapt to the underlying data patterns.

**Features:**
- Combines different hashing techniques to balance space and time complexity.
- Optimized for a variety of use cases (e.g., high collision scenarios).

---

### **Build the project:**

```bash
make
```

### **Using the ADTs**
- Include the relevant header file in your project:
   ```cpp
   #include "include/HopscotchHash.h"
   #include "include/HybridHash.h"
   ```
   
---

## **Testing**

This repository includes a comprehensive set of tests to ensure the correctness and performance of the ADTs. The tests are located in the `tests/` directory.

To run the tests:
```bash
make test
./tests
```

---

## **References**

- Herlihy, M., Shavit, N., & Tzafrir, M. (2008). Hopscotch hashing. *Distributed Computing*, 22(3), 183-194. [Link to paper](https://link.springer.com/article/10.1007/s00446-008-0075-y)
- Further exploration of hybrid hashing: [Wikipedia](https://en.wikipedia.org/wiki/Hash_table)

---

## **License**

This project is licensed under the [MIT License](LICENSE).
