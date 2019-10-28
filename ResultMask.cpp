//
// Created by molin on 28/10/2019.
//

#include "ResultMask.h"

ResultMask::ResultMask(std::shared_ptr<Forest> forest) : forest(std::move(forest)) {
    for (auto &t : this->forest->trees) {
        masks.emplace_back(t.numberOfLeafs() / 8 + 1, 255);
    }
}
