#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>

struct VectorItem {
    int id;
    std::string metadata;
    std::string category;
    std::vector<float> emb;
};

#endif // TYPES_HPP
