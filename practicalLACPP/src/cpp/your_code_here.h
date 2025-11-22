#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <span>
#include <tuple>
#include <vector>

// ==================================
// ========    Exercise 1    ========
// ==================================
std::pair<float, float> statistics(std::span<const float> values)
{
    if (values.empty()) return {0.0f, 0.0f};
    float mean = 0.0;
    float sd = 0.0;
    for (float v : values) {
        mean += v;
    }
    mean /= (float)values.size();

    for (float v : values) {
        sd += (v - mean) * (v - mean);
    }
    sd = std::sqrt(sd / (float) values.size());
    return { mean, sd };
}

// ==================================
// ========    Exercise 2    ========
// ==================================
struct Tree {
    float value;
    std::vector<Tree> children;
};
float countTreeRecursive(const Tree& tree, bool countOnlyEvenLevels, bool oddLevel)
{
    if (tree.children.empty())
        return countOnlyEvenLevels && oddLevel ? 0.0f : tree.value;

    float sum = countOnlyEvenLevels && oddLevel ? 0.0f : tree.value;
    for (const Tree& child : tree.children) {
        sum += countTreeRecursive(child, countOnlyEvenLevels, !oddLevel);
    }
    return sum;
}
// Your implementation goes here. Feel free to define helper functions/structs/classes in this
// file if you need to. Make sure to put them above this function to prevent linker warnings.
float countTree(const Tree& tree, bool countOnlyEvenLevels)
{
    return countTreeRecursive(tree, countOnlyEvenLevels, false);
}
