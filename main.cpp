#include <iostream>
#include "json.hpp"
#include "Tree.h"
#include "OldFeaturesMap.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::shared_ptr<Node> parseNode(nlohmann::json json) {
    if (json.contains("split_feature")) {
        //Internal
        return std::make_shared<InternalNode>(
                json["split_feature"],
                json["threshold"],
                parseNode(json["left_child"]),
                parseNode(json["right_child"])
        );
    } else {
        //Leaf
        return std::make_shared<Leaf>(json["leaf_value"]);
    }
}

Tree parseTree(nlohmann::json json) {
    auto root = parseNode(json["tree_structure"]);
    return Tree(std::dynamic_pointer_cast<InternalNode>(root));
}

std::shared_ptr<Forest> parseForest(const std::string& filename) {
    std::ifstream file("forests/" + filename + ".json");
    nlohmann::json json;
    file >> json;
    file.close();

    std::vector<Tree> trees;
    for (auto &tree : json["tree_info"]) {
        trees.push_back(parseTree(tree));
    }
    return std::make_shared<Forest>(trees);
}

std::vector<double> parseDocumentLine(const std::string& line) {
    std::istringstream ss(line);
    std::string token;

    std::getline(ss, token, ' ');
    std::getline(ss, token, ' ');

    std::vector<double> ret;
    int featureId;
    double value;
    ret.push_back(0);
    while(std::getline(ss, token, ' ')) {
        sscanf(token.c_str(), "%d:%lf", &featureId, &value);
        ret.push_back(value); //TODO use feature ID
    }
    return ret;
}

std::vector<double> parseDocument(const std::string& filename) {
    std::ifstream file;
    file.open("documents/" + filename);
    std::string s;
    std::getline(file, s);
    return parseDocumentLine(s);
}


int main() {
    auto f = parseForest("Fold1_train");

    const std::vector<double> &doc = parseDocument("doc1");

    //double score = f->score(doc);
    OldFeaturesMap fmap(f, doc.size());
    double score = fmap.score(doc);
    
    std::cout << score << std::endl;
    return 0;
}