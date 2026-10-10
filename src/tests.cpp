#include "distance.hpp"
#include "bruteforce.hpp"
#include "kdtree.hpp"
#include "hnsw.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

#define RUN_TEST(name) \
    do { \
        std::cout << "[ RUN  ] " << #name << std::endl; \
        try { \
            name(); \
            std::cout << "[  OK  ] " << #name << std::endl; \
        } catch (const std::exception& e) { \
            std::cerr << "[ FAIL ] " << #name << " : " << e.what() << std::endl; \
            failed = true; \
        } catch (...) { \
            std::cerr << "[ FAIL ] " << #name << " : Unknown error" << std::endl; \
            failed = true; \
        } \
    } while (0)

bool failed = false;

// 1. Distance Metrics Tests
void test_distances() {
    std::vector<float> a = {1.0f, 2.0f, 3.0f};
    std::vector<float> b = {4.0f, 6.0f, 3.0f};

    // Euclidean: sqrt(3^2 + 4^2 + 0^2) = 5.0f
    float dist_eucl = euclidean(a, b);
    assert(std::abs(dist_eucl - 5.0f) < 1e-5f);

    // Manhattan: 3 + 4 + 0 = 7.0f
    float dist_manh = manhattan(a, b);
    assert(std::abs(dist_manh - 7.0f) < 1e-5f);

    // Cosine Similarity/Distance
    std::vector<float> c = {1.0f, 0.0f};
    std::vector<float> d = {0.0f, 1.0f}; // orthogonal
    float dist_cos_orth = cosine(c, d);
    assert(std::abs(dist_cos_orth - 1.0f) < 1e-5f); // 1 - 0 = 1.0

    std::vector<float> e = {2.0f, 2.0f};
    std::vector<float> f = {4.0f, 4.0f}; // collinear
    float dist_cos_coll = cosine(e, f);
    assert(std::abs(dist_cos_coll - 0.0f) < 1e-5f); // 1 - 1 = 0.0
}

// 2. BruteForce Index Tests
void test_bruteforce() {
    BruteForce bf;
    bf.insert({1, "item1", "test", {1.0f, 0.0f}});
    bf.insert({2, "item2", "test", {0.0f, 1.0f}});
    bf.insert({3, "item3", "test", {0.5f, 0.5f}});

    std::vector<float> q = {0.9f, 0.1f};
    auto results = bf.knn(q, 2, cosine);

    assert(results.size() == 2);
    assert(results[0].second == 1); // item1 should be nearest
    assert(results[1].second == 3); // item3 should be second
}

// 3. KDTree Equivalence Tests
void test_kdtree() {
    KDTree kdt(2);
    kdt.insert({1, "item1", "test", {1.0f, 0.0f}});
    kdt.insert({2, "item2", "test", {0.0f, 1.0f}});
    kdt.insert({3, "item3", "test", {0.5f, 0.5f}});

    std::vector<float> q = {0.1f, 0.9f};
    auto results = kdt.knn(q, 2, euclidean);

    assert(results.size() == 2);
    assert(results[0].second == 2); // item2 nearest to q
    assert(results[1].second == 3); // item3 second nearest
}

// 4. HNSW Search Correctness
void test_hnsw_basic() {
    HNSW hnsw(4, 50); // M=4, ef=50
    auto dist = cosine;

    hnsw.insert({1, "node1", "cs", {1.0f, 0.0f, 0.0f}}, dist);
    hnsw.insert({2, "node2", "math", {0.0f, 1.0f, 0.0f}}, dist);
    hnsw.insert({3, "node3", "food", {0.0f, 0.0f, 1.0f}}, dist);

    std::vector<float> q = {0.1f, 0.9f, 0.0f};
    auto results = hnsw.knn(q, 1, 10, dist);

    assert(!results.empty());
    assert(results[0].second == 2); // Should find node2
}

// 5. HNSW Deletion & Entry Point Reassignment
void test_hnsw_removal() {
    HNSW hnsw(4, 50);
    auto dist = euclidean;

    hnsw.insert({1, "node1", "cs", {1.0f, 0.0f}}, dist);
    hnsw.insert({2, "node2", "math", {2.0f, 0.0f}}, dist);
    hnsw.insert({3, "node3", "food", {3.0f, 0.0f}}, dist);

    // Verify size
    assert(hnsw.size() == 3);

    // Remove entry point or standard point
    hnsw.remove(1);
    assert(hnsw.size() == 2);

    // Query again to verify HNSW is still fully functional after removal
    std::vector<float> q = {2.1f, 0.0f};
    auto results = hnsw.knn(q, 1, 10, dist);
    assert(!results.empty());
    assert(results[0].second == 2); // node2 is closest

    // Remove all remaining
    hnsw.remove(2);
    hnsw.remove(3);
    assert(hnsw.size() == 0);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "         VectorDB Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    RUN_TEST(test_distances);
    RUN_TEST(test_bruteforce);
    RUN_TEST(test_kdtree);
    RUN_TEST(test_hnsw_basic);
    RUN_TEST(test_hnsw_removal);

    std::cout << "========================================" << std::endl;
    if (failed) {
        std::cout << "    SOME TESTS FAILED!" << std::endl;
        return 1;
    } else {
        std::cout << "    ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
        return 0;
    }
}
