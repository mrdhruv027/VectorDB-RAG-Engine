#ifndef DB_HPP
#define DB_HPP

#include "types.hpp"
#include "distance.hpp"
#include "bruteforce.hpp"
#include "kdtree.hpp"
#include "hnsw.hpp"
#include "json_utils.hpp"
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iostream>

class VectorDB {
    std::unordered_map<int, VectorItem> store;
    BruteForce bf;
    KDTree     kdt;
    HNSW       hnsw;
    std::mutex mu;
    int nextId = 1;

public:
    const int dims;
    explicit VectorDB(int d) : kdt(d), hnsw(16, 200), dims(d) {}

    int insert(const std::string& meta, const std::string& cat,
               const std::vector<float>& emb, DistFn dist)
    {
        std::lock_guard<std::mutex> lk(mu);
        VectorItem v{nextId++, meta, cat, emb};
        store[v.id] = v;
        bf.insert(v); kdt.insert(v); hnsw.insert(v, dist);
        return v.id;
    }

    bool remove(int id) {
        std::lock_guard<std::mutex> lk(mu);
        if (!store.count(id)) return false;
        store.erase(id); bf.remove(id); hnsw.remove(id);
        std::vector<VectorItem> rem;
        for (auto& [i, v] : store) rem.push_back(v);
        kdt.rebuild(rem);
        return true;
    }

    struct Hit { int id; std::string meta, cat; std::vector<float> emb; float dist; };
    struct SearchOut { std::vector<Hit> hits; long long us; std::string algo, metric; };

    SearchOut search(const std::vector<float>& q, int k,
                     const std::string& metric, const std::string& algo)
    {
        std::lock_guard<std::mutex> lk(mu);
        auto dfn = getDistFn(metric);
        auto t0  = std::chrono::high_resolution_clock::now();

        std::vector<std::pair<float,int>> raw;
        if      (algo == "bruteforce") raw = bf.knn(q, k, dfn);
        else if (algo == "kdtree")     raw = kdt.knn(q, k, dfn);
        else                           raw = hnsw.knn(q, k, 50, dfn);

        long long us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - t0).count();

        SearchOut out; out.us = us; out.algo = algo; out.metric = metric;
        for (auto& [d, id] : raw)
            if (store.count(id))
                out.hits.push_back({id, store[id].metadata, store[id].category, store[id].emb, d});
        return out;
    }

    struct BenchOut { long long bfUs, kdUs, hnswUs; int n; };

    BenchOut benchmark(const std::vector<float>& q, int k, const std::string& metric) {
        std::lock_guard<std::mutex> lk(mu);
        auto dfn  = getDistFn(metric);
        auto time = [&](auto fn) -> long long {
            auto t = std::chrono::high_resolution_clock::now();
            fn();
            return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t).count();
        };
        return {
            time([&]{ bf.knn(q, k, dfn); }),
            time([&]{ kdt.knn(q, k, dfn); }),
            time([&]{ hnsw.knn(q, k, 50, dfn); }),
            (int)store.size()
        };
    }

    std::vector<VectorItem> all() {
        std::lock_guard<std::mutex> lk(mu);
        std::vector<VectorItem> r;
        for (auto& [id, v] : store) r.push_back(v);
        return r;
    }

    HNSW::GraphInfo hnswInfo() {
        std::lock_guard<std::mutex> lk(mu);
        return hnsw.getInfo();
    }

    size_t size() {
        std::lock_guard<std::mutex> lk(mu);
        return store.size();
    }

    bool save(const std::string& filepath) {
        std::lock_guard<std::mutex> lk(mu);
        std::ofstream f(filepath);
        if (!f.is_open()) return false;
        f << "[\n";
        bool first = true;
        for (auto& [id, item] : store) {
            if (!first) f << ",\n";
            first = false;
            f << "  {\n"
              << "    \"id\": " << item.id << ",\n"
              << "    \"metadata\": " << jS(item.metadata) << ",\n"
              << "    \"category\": " << jS(item.category) << ",\n"
              << "    \"embedding\": " << jVec(item.emb) << "\n"
              << "  }";
        }
        f << "\n]";
        return true;
    }

    bool load(const std::string& filepath, DistFn dist) {
        std::lock_guard<std::mutex> lk(mu);
        std::ifstream f(filepath);
        if (!f.is_open()) return false;
        std::stringstream ss;
        ss << f.rdbuf();
        std::string content = ss.str();

        store.clear();
        bf.items.clear();
        hnsw = HNSW(16, 200);
        nextId = 1;

        size_t pos = 0;
        while (true) {
            size_t start = content.find('{', pos);
            if (start == std::string::npos) break;
            size_t end = content.find('}', start);
            if (end == std::string::npos) break;

            std::string block = content.substr(start, end - start + 1);
            std::string meta = extractStr(block, "metadata");
            std::string cat = extractStr(block, "category");
            int id = extractInt(block, "id", -1);
            
            size_t pArr = block.find("\"embedding\"");
            if (pArr != std::string::npos) {
                pArr = block.find('[', pArr);
                size_t eArr = block.find(']', pArr);
                if (pArr != std::string::npos && eArr != std::string::npos) {
                    std::vector<float> emb = parseVec(block.substr(pArr + 1, eArr - pArr - 1));
                    if (id != -1 && !meta.empty() && !emb.empty()) {
                        VectorItem item{id, meta, cat, emb};
                        store[id] = item;
                        bf.insert(item);
                        hnsw.insert(item, dist);
                        nextId = std::max(nextId, id + 1);
                    }
                }
            }
            pos = end + 1;
        }
        std::vector<VectorItem> items;
        for (auto& [id, v] : store) items.push_back(v);
        kdt.rebuild(items);
        return true;
    }
};

struct DocItem {
    int         id;
    std::string title;
    std::string text;
    std::vector<float> emb;
};

class DocumentDB {
    std::unordered_map<int, DocItem> store;
    HNSW       hnsw;
    BruteForce bf;
    std::mutex mu;
    int nextId = 1;
    int dims   = 0;

public:
    DocumentDB() : hnsw(16, 200) {}

    int insert(const std::string& title, const std::string& text,
               const std::vector<float>& emb)
    {
        std::lock_guard<std::mutex> lk(mu);
        if (dims == 0) dims = (int)emb.size();
        DocItem item{nextId++, title, text, emb};
        store[item.id] = item;
        VectorItem vi{item.id, title, "doc", emb};
        hnsw.insert(vi, cosine);
        bf.insert(vi);
        return item.id;
    }

    std::vector<std::pair<float, DocItem>> search(
        const std::vector<float>& q, int k, float max_dist = 0.7f)
    {
        std::lock_guard<std::mutex> lk(mu);
        if (store.empty()) return {};
        auto raw = (store.size() < 10)
                   ? bf.knn(q, k, cosine)
                   : hnsw.knn(q, k, 50, cosine);
        std::vector<std::pair<float, DocItem>> out;
        for (auto& [d, id] : raw)
            if (store.count(id) && d <= max_dist) out.push_back({d, store[id]});
        return out;
    }

    bool remove(int id) {
        std::lock_guard<std::mutex> lk(mu);
        if (!store.count(id)) return false;
        store.erase(id); hnsw.remove(id); bf.remove(id);
        return true;
    }

    std::vector<DocItem> all() {
        std::lock_guard<std::mutex> lk(mu);
        std::vector<DocItem> r;
        for (auto& [id, v] : store) r.push_back(v);
        return r;
    }

    size_t size() {
        std::lock_guard<std::mutex> lk(mu);
        return store.size();
    }

    int getDims() { return dims; }

    bool save(const std::string& filepath) {
        std::lock_guard<std::mutex> lk(mu);
        std::ofstream f(filepath);
        if (!f.is_open()) return false;
        f << "[\n";
        bool first = true;
        for (auto& [id, item] : store) {
            if (!first) f << ",\n";
            first = false;
            f << "  {\n"
              << "    \"id\": " << item.id << ",\n"
              << "    \"title\": " << jS(item.title) << ",\n"
              << "    \"text\": " << jS(item.text) << ",\n"
              << "    \"embedding\": " << jVec(item.emb) << "\n"
              << "  }";
        }
        f << "\n]";
        return true;
    }

    bool load(const std::string& filepath) {
        std::lock_guard<std::mutex> lk(mu);
        std::ifstream f(filepath);
        if (!f.is_open()) return false;
        std::stringstream ss;
        ss << f.rdbuf();
        std::string content = ss.str();

        store.clear();
        bf.items.clear();
        hnsw = HNSW(16, 200);
        nextId = 1;
        dims = 0;

        size_t pos = 0;
        while (true) {
            size_t start = content.find('{', pos);
            if (start == std::string::npos) break;
            size_t end = content.find('}', start);
            if (end == std::string::npos) break;

            std::string block = content.substr(start, end - start + 1);
            std::string title = extractStr(block, "title");
            std::string text = extractStr(block, "text");
            int id = extractInt(block, "id", -1);
            
            size_t pArr = block.find("\"embedding\"");
            if (pArr != std::string::npos) {
                pArr = block.find('[', pArr);
                size_t eArr = block.find(']', pArr);
                if (pArr != std::string::npos && eArr != std::string::npos) {
                    std::vector<float> emb = parseVec(block.substr(pArr + 1, eArr - pArr - 1));
                    if (id != -1 && !title.empty() && !emb.empty()) {
                        if (dims == 0) dims = (int)emb.size();
                        DocItem item{id, title, text, emb};
                        store[id] = item;
                        VectorItem vi{id, title, "doc", emb};
                        hnsw.insert(vi, cosine);
                        bf.insert(vi);
                        nextId = std::max(nextId, id + 1);
                    }
                }
            }
            pos = end + 1;
        }
        return true;
    }
};

#endif // DB_HPP
