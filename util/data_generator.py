import os
import json

import numpy as np
import lightgbm
from sklearn.datasets import load_svmlight_file
import itertools
import sys
import re


NUM_OF_TREES = 10000
NUM_OF_LEAFS = 400


fold_path = sys.argv[1]

if fold_path is None:
    sys.exit(1)
for i in range(1, 6):

    cur_fold_path = f"{fold_path}/Fold{i}"
    test_file = f"{cur_fold_path}/test.txt"
    train_file = f"{cur_fold_path}/train.txt"

    test_data = load_svmlight_file(test_file, query_id=True)
    train_data = load_svmlight_file(train_file, query_id=True)

    query_lens = [
        sum(1 for _ in group) for key, group in itertools.groupby(train_data[2])
    ]
    train_lgb = lightgbm.Dataset(
        data=train_data[0], label=train_data[1], group=query_lens
    )

    params = {
        "objective": "lambdarank",  # what to optimize during training
        "max_position": 10,  # threshold used in optimizing lamdarank (NDCG)
        "learning_rate": 0.1,
        "num_leaves": NUM_OF_LEAFS,
        "min_data_in_leaf": 5,
        "metric": ["ndcg"],  # what to use/print for evaluation
        "ndcg_eval_at": 10,
    }

    lgbm_info = {}

    print("start model creation")

    lgbm_model = lightgbm.train(
        params,
        train_lgb,  # training dataser
        num_boost_round=NUM_OF_TREES,  # num trees
        valid_sets=[train_lgb],
        valid_names=["train"],
        evals_result=lgbm_info,
        verbose_eval=True,
    )

    print("model created")

    with open(f"{cur_fold_path}/test_scores.txt", "wb") as f:
        lines = [
            (str(x) + "\n").encode() for x in lgbm_model.predict(test_data[0])
        ]
        f.writelines(lines)

    with open(f"{cur_fold_path}/model.json", "wb") as f:
        f.write(json.dumps(lgbm_model.dump_model()).encode())
