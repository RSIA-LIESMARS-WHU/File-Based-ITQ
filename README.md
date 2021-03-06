File-Based-ITQ
==============

###*File Based ITQ example for [LSHBOX](https://github.com/RSIA-LIESMARS-WHU/LSHBOX) - An Open Source C++ Toolbox of Locality-Sensitive Hashing for Large Scale Image Retrieval.*

### Usage

The `build` floder is compiled under Win64. And here is an example which describes the steps about how to use the code.

#### For C++

A. Run `test/mat2binary.m` to transform the original database from MATLAB's `.mat` to raw binary files. The code had limited the size of each binary file by `BATCH_SIZE = 1000000;`. (Just to illustrate the binary data format, MATLAB is not essential!)

B. After step A, `test/dataset` folder will be generated, and it contians some raw binary files, such as `data_0.bin`, `data_1.bin`,...

C. Create `data.meta` in `test/dataset` folder manually, and write the following configuration informations, which is used to interpret the raw data. 

>DIMENSIONS = 32

>TOTAL_SIZE = 2047379

>BATCH_SIZE = 1000000

D. Now, The `test/dataset` folder is the raw data. All the other original database show organized by the same way.

E. Copy `create_benchmark_filedb.exe`, `dbitq_save.exe`, and `dbitq_loads.exe` from `build/bin/x64/Release` folder to `test` folder.

F. Run the following command line to create benchmark file `data.ben-200-50`.

>create_benchmark_filedb . data.ben-200-50 200 50

G. Run the following command line to save the hash tables.

>dbitq_save . 2 5 . 20

H. Run the following command line to load the hash tables and query.

>dbitq_loads . ./ITQ_L-2_N-5_S-50000_I-100 data.ben-200-50 4096 0

#### For Python

After step A, you can also run the python code in `build/py_module/x64/Release/test_pyitq.py` or in `sources/python/win/x64/test_pyitq.py`.

```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-
# test_pyitq.py
from scipy.io import loadmat
import pyitq
queryset = loadmat('queryset.mat')['queryset']
itq = pyitq.itq_f()
itq.save_hash(
    'E:/GitHub/File-Based-ITQ/test',
    'E:/GitHub/File-Based-ITQ/test',
    20,
    3,
    6,
    5000,
    200
)
itq.load_hash(
    'E:/GitHub/File-Based-ITQ/test',
    'E:/GitHub/File-Based-ITQ/test/ITQ_L-3_N-6_S-5000_I-200',
    2,
    30,
    4096
)
for each in queryset:
    print '----------------------------------------------'
    res = itq.query(each.tolist(), 5, 0)
    print res[0]
    print res[1]
    res = itq.query(each.tolist(), 5, 1)
    print res[0]
    print res[1]
```

#####In order to understand this steps, you should go to read the source code in `sources` folder.








