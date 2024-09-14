"""
this python function is taken from the book "hands on the machine learning" this python function divide the train and test dataset by the way
no matter how many times you re-run your notebook train set data will never fall into test set unlikely scikit-learn's train_test_split function

(github: MrHeaven1y)
"""
import hashlib
import numpy as np

def test_set_check(identifier,test_ratio,hash):
    """
    it's recommended to pass identifier from dataset which will not change or barely change you can gice index but if its not changing
    then it will be good to use hash of index either something like, 'longitude', 'latitude'... etc.
    """
    return hash(np.int64(identifier)).digest()[-1]<256*test_ratio

def split_train_test_by_id(data,test_ratio,id_column,hash=hashlib.md5):
    """
    it'll return train_set, test_set sequentially.
    """
    ids = data[id_column]
    in_test_set = ids.apply(lambda id_:test_set_check(id_,test_ratio=test_ratio,hash=hash))
    return data.loc[~in_test_set],data.loc[in_test_set]