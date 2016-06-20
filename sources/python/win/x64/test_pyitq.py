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
itq.load_hash('E:/GitHub/File-Based-ITQ/test',
              'E:/GitHub/File-Based-ITQ/test/ITQ_L-3_N-6_S-5000_I-200', 2, 30, 4096)
for each in queryset:
    print '----------------------------------------------'
    res = itq.query(each.tolist(), 5, 0)
    print res[0]
    print res[1]
    res = itq.query(each.tolist(), 5, 1)
    print res[0]
    print res[1]
