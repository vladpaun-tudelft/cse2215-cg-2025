#include "helpers.h"
#include "your_code_here.h"
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
#include <algorithm>
#include <cmath>
#include <ostream>
#include <vector>

using namespace Catch;

TEST_CASE("Statistics")
{
    SECTION("Example input")
    {
        std::vector<float> values;
        values.push_back(-3.0f);
        values.push_back(-1.0f);
        values.push_back(1.0f);
        values.push_back(1.0f);
        values.push_back(1.0f);
        values.push_back(3.0f);
        values.push_back(3.0f);
        values.push_back(3.0f);
        CAPTURE(values); // Will be reported in the error message

        const auto [mean, std] = statistics(values);

        REQUIRE(mean == Approx(1.0f));
        REQUIRE(std == Approx(2.0f));
    }

    SECTION("Input 1")
    {
        std::vector<float> values;
        values.push_back(3.0f);
        values.push_back(4.0f);
        values.push_back(5.0f);
        values.push_back(6.0f);
        values.push_back(7.0f);
        values.push_back(8.0f);
        values.push_back(9.0f);
        CAPTURE(values);

        const auto [mean, std] = statistics(values);

        REQUIRE(mean == Approx(6.0f));
        REQUIRE(std == Approx(2.0f));
    }

    SECTION("Input 2")
    {
        std::vector<float> values;
        values.push_back(8.0f);
        values.push_back(7.0f);
        values.push_back(6.0f);
        values.push_back(5.0f);
        values.push_back(4.0f);
        values.push_back(3.0f);
        values.push_back(2.0f);
        values.push_back(1.0f);
        CAPTURE(values);

        const auto [mean, std] = statistics(values);

        REQUIRE(mean == Approx(4.5f));
        REQUIRE(std == Approx(std::sqrt((3.5f * 3.5f + 2.5f * 2.5f + 1.5f * 1.5f + 0.5f * 0.5f + 0.5f * 0.5f + 1.5f * 1.5f + 2.5f * 2.5f + 3.5f * 3.5f) / 8.0f)));
    }

    SECTION("Input 3")
    {
        std::vector<float> values;
        values.push_back(1.0f);
        CAPTURE(values);
        const auto [mean, std] = statistics(values);

        REQUIRE(mean == Approx(1.0f));
        REQUIRE(std == Approx(0.0f));
    }

    SECTION("Empty input")
    {
        std::vector<float> values;
        CAPTURE(values);
        const auto [mean, std] = statistics(values);
        REQUIRE(mean == Approx(0.0f));
        REQUIRE(std == Approx(0.0f));
    }
}

// Number of nodes per level:
// 1         = 1
// 1*2       = 2
// 1*2*3     = 6
// 1*2*3*4   = 24
// 1*2*3*4*5 = 120
//
// Value of nodes per level:
// 0
// 2
// 4
// 6
// 8
static void fillTree(Tree& tree, int level)
{
    tree.value = 2.0f * static_cast<float>(level);
    if (level >= 4)
        return;

    Tree child;
    fillTree(child, level + 1);

    for (int i = 0; i < level + 2; ++i) {
        tree.children.push_back(child);
    }
}

static void addTree(Tree& tree, float amount)
{
    tree.value += amount;
    for (auto& child : tree.children)
        addTree(child, amount);
}

// https://graphviz.org/
static void printTreeRecurse(std::ostream& stream, const Tree& tree, int& nodeIdx)
{
    const int thisNodeidx = nodeIdx;
    stream << "n" << nodeIdx << "[label=\"" << thisNodeidx << "\"] ";
    for (const auto& child : tree.children) {
        ++nodeIdx;
        stream << "n" << thisNodeidx << "->n" << nodeIdx << " ";
        printTreeRecurse(stream, child, nodeIdx);
    }
}

// https://graphviz.org/
std::ostream& operator<<(std::ostream& stream, const Tree& tree)
{
    stream << "digraph G { ";
    int nodeIdx = 0;
    printTreeRecurse(stream, tree, nodeIdx);
    stream << "}";
    return stream;
}

TEST_CASE("TreeTraversal")
{
    SECTION("Example input")
    {
        Tree tree;
        fillTree(tree, 0);

        CAPTURE(tree);
        REQUIRE(countTree(tree, false) == Approx(1132.0f));
        REQUIRE(countTree(tree, true) == Approx(984.0f));
    }

    SECTION("Input 1")
    {
        Tree tree;
        fillTree(tree, 0);
        addTree(tree.children[0], -1.0f);
        // Subtract 1 from all nodes in the left subtree.
        //  -1 at level 1
        //  -3 at level 2
        // -12 at level 3
        // -60 at level 4    +
        // --------------------
        // -76 compared to unmodified tree.
        // -63 if only even levels are counted

        CAPTURE(tree);
        REQUIRE(countTree(tree, false) == Approx(1056.0f)); // 1132-76
        REQUIRE(countTree(tree, true) == Approx(921.0f)); // 984 - 63
    }

    SECTION("Input 2")
    {
        Tree root;
        fillTree(root, 0);
        root.children[0].children[0].children.clear();
        // Remove a part of the tree.
        //  -4 * 6 at level 3
        // -20 * 8 at level 4  +
        // --------------------
        // -184 compared to unmodified tree.
        // -160 if only even levels are counted
        // 

        REQUIRE(countTree(root, false) == Approx(948.0f)); // 1132 - 184
        REQUIRE(countTree(root, true) == Approx(824.0f)); // 984 - 160
    }
}
